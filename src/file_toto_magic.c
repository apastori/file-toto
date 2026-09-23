/*
 * Chain-of-thought (Step 1 — file scope):
 *
 * 1. Responsibility: embedded magic signatures + ASCII text heuristic.
 * 2. Syscalls: none — pure byte inspection.
 * 3. Heap: none — static magic table of string pointers.
 * 4. Cl,assify: longest / most specific signature first in table order.
 * 5. C11.
 */

#include "file_toto_magic.h"

#include <string.h>

/* Longest / most specific entries first where prefixes overlap. */
static const unsigned char SIG_PNG[] = {
    0x89u, 0x50u, 0x4Eu, 0x47u, 0x0Du, 0x0Au, 0x1Au, 0x0Au
};
static const unsigned char SIG_GIF87[] = {
    'G', 'I', 'F', '8', '7', 'a'
};
static const unsigned char SIG_GIF89[] = {
    'G', 'I', 'F', '8', '9', 'a'
};
static const unsigned char SIG_JPEG[] = {
    0xFFu, 0xD8u, 0xFFu
};
static const unsigned char SIG_ELF[] = {
    0x7Fu, 'E', 'L', 'F'
};
static const unsigned char SIG_PDF[] = {
    '%', 'P', 'D', 'F', '-'
};
static const unsigned char SIG_ZIP[] = {
    'P', 'K', 0x03u, 0x04u
};
static const unsigned char SIG_PE[] = {
    'M', 'Z'
};

static const magic_entry_t MAGIC_TABLE[] = {
    { SIG_PNG,   sizeof SIG_PNG,   "PNG image data",  "image/png" },
    { SIG_GIF89, sizeof SIG_GIF89, "GIF image data",  "image/gif" },
    { SIG_GIF87, sizeof SIG_GIF87, "GIF image data",  "image/gif" },
    { SIG_JPEG,  sizeof SIG_JPEG,  "JPEG image data", "image/jpeg" },
    { SIG_ELF,   sizeof SIG_ELF,   "ELF executable",  "application/x-executable" },
    { SIG_PDF,   sizeof SIG_PDF,   "PDF document",    "application/pdf" },
    { SIG_ZIP,   sizeof SIG_ZIP,   "Zip archive data","application/zip" },
    { SIG_PE,    sizeof SIG_PE,    "PE executable",   "application/x-dosexec" },
};

static int copy_desc(char *out, size_t out_cap, const char *desc)
{
    size_t n;

    if (out == NULL || out_cap < 1u || desc == NULL) {
        return -1;
    }
    n = strlen(desc);
    if (n + 1u > out_cap) {
        return -1;
    }
    memcpy(out, desc, n + 1u);
    return 0;
}

int match_magic(const unsigned char *buf, size_t len, int mime,
                char *out, size_t out_cap)
{
    size_t i;

    if (buf == NULL || len == 0u) {
        return -1;
    }

    for (i = 0u; i < sizeof MAGIC_TABLE / sizeof MAGIC_TABLE[0]; i++) {
        const magic_entry_t *e = &MAGIC_TABLE[i];
        if (len < e->sig_len) {
            continue;
        }
        if (memcmp(buf, e->sig, e->sig_len) == 0) {
            return copy_desc(out, out_cap, mime ? e->mime : e->human);
        }
    }
    return -1;
}

int file_toto_is_ascii_text(const unsigned char *buf, size_t len)
{
    size_t i;

    if (buf == NULL || len == 0u) {
        return 0;
    }
    for (i = 0u; i < len; i++) {
        unsigned char c = buf[i];
        if (c == '\t' || c == '\n' || c == '\r') {
            continue;
        }
        if (c < 0x20u || c > 0x7Eu) {
            return 0;
        }
    }
    return 1;
}

int file_toto_is_utf8_text(const unsigned char *buf, size_t len)
{
    size_t i = 0u;

    if (buf == NULL || len == 0u) {
        return 0;
    }

    while (i < len) {
        unsigned char c = buf[i];

        if (c == '\t' || c == '\n' || c == '\r') {
            i++;
            continue;
        }
        if (c >= 0x20u && c <= 0x7Eu) {
            i++;
            continue;
        }
        /* Reject other C0 controls and DEL / C1 as leading bytes. */
        if (c < 0x80u) {
            return 0;
        }

        /* Multibyte UTF-8 */
        if ((c & 0xE0u) == 0xC0u) {
            if (c < 0xC2u || i + 1u >= len) {
                return 0;
            }
            if ((buf[i + 1u] & 0xC0u) != 0x80u) {
                return 0;
            }
            i += 2u;
        } else if ((c & 0xF0u) == 0xE0u) {
            if (i + 2u >= len) {
                return 0;
            }
            if ((buf[i + 1u] & 0xC0u) != 0x80u
                || (buf[i + 2u] & 0xC0u) != 0x80u) {
                return 0;
            }
            /* Reject overlong and surrogate halves coarsely. */
            if (c == 0xE0u && buf[i + 1u] < 0xA0u) {
                return 0;
            }
            if (c == 0xEDu && buf[i + 1u] >= 0xA0u) {
                return 0;
            }
            i += 3u;
        } else if ((c & 0xF8u) == 0xF0u) {
            if (c > 0xF4u || i + 3u >= len) {
                return 0;
            }
            if ((buf[i + 1u] & 0xC0u) != 0x80u
                || (buf[i + 2u] & 0xC0u) != 0x80u
                || (buf[i + 3u] & 0xC0u) != 0x80u) {
                return 0;
            }
            if (c == 0xF0u && buf[i + 1u] < 0x90u) {
                return 0;
            }
            if (c == 0xF4u && buf[i + 1u] >= 0x90u) {
                return 0;
            }
            i += 4u;
        } else {
            return 0;
        }
    }
    return 1;
}
