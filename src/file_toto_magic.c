/*
 * Chain-of-thought (Step 1 — file scope):
 *
 * 1. Responsibility: embedded magic signatures + ASCII / UTF-8 text heuristics.
 * 2. Syscalls: none — pure byte inspection of the prefix and the tail.
 * 3. Heap: none — static magic table of string pointers.
 * 4. Classify: table order, each signature at an offset from the start or
 *    from the end of the file; then the PE header check (MZ + 0x3C pointer).
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
/* ISO base media (MP4, MOV, ...): box size (4 bytes), then "ftyp". */
static const unsigned char SIG_FTYP[] = {
    'f', 't', 'y', 'p'
};
/* POSIX tar: "ustar" in the magic field of the first 512-byte header. */
static const unsigned char SIG_USTAR[] = {
    'u', 's', 't', 'a', 'r'
};
/* ISO 9660: "CD001" after the type byte of the first volume descriptor. */
static const unsigned char SIG_CD001[] = {
    'C', 'D', '0', '0', '1'
};
/* Apple DMG (UDIF): 512-byte "koly" trailer at the very end of the file. */
static const unsigned char SIG_KOLY[] = {
    'k', 'o', 'l', 'y'
};

static const magic_entry_t MAGIC_TABLE[] = {
    /* offset, from_end, signature, length, human, MIME */
    { 0u, 0, SIG_PNG,   sizeof SIG_PNG,   "PNG image data",  "image/png" },
    { 0u, 0, SIG_GIF89, sizeof SIG_GIF89, "GIF image data",  "image/gif" },
    { 0u, 0, SIG_GIF87, sizeof SIG_GIF87, "GIF image data",  "image/gif" },
    { 0u, 0, SIG_JPEG,  sizeof SIG_JPEG,  "JPEG image data", "image/jpeg" },
    { 0u, 0, SIG_ELF,   sizeof SIG_ELF,   "ELF executable",  "application/x-executable" },
    { 0u, 0, SIG_PDF,   sizeof SIG_PDF,   "PDF document",    "application/pdf" },
    { 0u, 0, SIG_ZIP,   sizeof SIG_ZIP,   "Zip archive data","application/zip" },
    { 4u, 0, SIG_FTYP,  sizeof SIG_FTYP,  "ISO Media",       "video/mp4" },
    { 257u, 0, SIG_USTAR, sizeof SIG_USTAR, "POSIX tar archive",
      "application/x-tar" },
    { 32769u, 0, SIG_CD001, sizeof SIG_CD001,
      "ISO 9660 CD-ROM filesystem data", "application/x-iso9660-image" },
    { 512u, 1, SIG_KOLY, sizeof SIG_KOLY, "Apple disk image (UDIF)",
      "application/x-apple-diskimage" },
};

/* PE: "MZ" DOS header, then "PE\0\0" at the offset stored at 0x3C. */
static const unsigned char SIG_MZ[] = {
    'M', 'Z'
};
static const unsigned char SIG_PE_HDR[] = {
    'P', 'E', 0x00u, 0x00u
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

/*
 * sig_location — where entry e's signature would be in the bytes we have:
 * inside the prefix buf[0..len) for from_end == 0, inside the tail
 * tail[0..tail_len) for from_end == 1. Returns NULL if the whole signature
 * does not fit in those bytes, so the caller can memcmp without bounds checks.
 */
static const unsigned char *sig_location(const magic_entry_t *e,
                                         const unsigned char *buf, size_t len,
                                         const unsigned char *tail,
                                         size_t tail_len)
{
    /* if the signature is not from the end of the file, return the location of the signature in the prefix */
    if (!e->from_end) {
        if (len < e->offset || len - e->offset < e->sig_len) {
            return NULL;
        }
        return buf + e->offset;
    }
    /* if the signature is from the end of the file, return the location of the signature in the tail */
    if (tail == NULL || e->offset > tail_len || e->offset < e->sig_len) {
        return NULL;
    }
    return tail + (tail_len - e->offset);
}

/*
 * match_pe — "MZ" executables. A PE file stores, at 0x3C, the little-endian
 * offset (e_lfanew) of its "PE\0\0" header; 24 bytes after that header
 * starts the optional header, whose 16-bit magic is 0x10B (PE32) or
 * 0x20B (PE32+). Without a valid PE header it is a plain MS-DOS executable.
 * Returns -1 if buf does not start with "MZ".
 */
static int match_pe(const unsigned char *buf, size_t len, int mime,
                    char *out, size_t out_cap)
{
    unsigned long lfanew;
    unsigned int opt_magic;
    const char *dos = mime ? "application/x-dosexec" : "MS-DOS executable";
    const char *pe_mime = "application/vnd.microsoft.portable-executable";

    if (len < sizeof SIG_MZ || memcmp(buf, SIG_MZ, sizeof SIG_MZ) != 0) {
        return -1;
    }
    if (len < 0x40u) {
        return copy_desc(out, out_cap, dos);
    }
    lfanew = (unsigned long)buf[0x3C]
           | ((unsigned long)buf[0x3D] << 8)
           | ((unsigned long)buf[0x3E] << 16)
           | ((unsigned long)buf[0x3F] << 24);
    /* "PE\0\0" (4) + COFF file header (20) + optional-header magic (2). */
    if (lfanew > len || len - lfanew < 26u) {
        return copy_desc(out, out_cap, dos);
    }
    if (memcmp(buf + lfanew, SIG_PE_HDR, sizeof SIG_PE_HDR) != 0) {
        return copy_desc(out, out_cap, dos);
    }
    opt_magic = (unsigned int)buf[lfanew + 24u]
              | ((unsigned int)buf[lfanew + 25u] << 8);
    if (opt_magic == 0x20Bu) {
        return copy_desc(out, out_cap,
                         mime ? pe_mime : "PE32+ executable, for MS Windows");
    }
    return copy_desc(out, out_cap,
                     mime ? pe_mime : "PE32 executable, for MS Windows");
}

int match_magic(const unsigned char *buf, size_t len,
                const unsigned char *tail, size_t tail_len,
                int mime, char *out, size_t out_cap)
{
    size_t i;

    if (buf == NULL || len == 0u) {
        return -1;
    }

    for (i = 0u; i < sizeof MAGIC_TABLE / sizeof MAGIC_TABLE[0]; i++) {
        const magic_entry_t *e = &MAGIC_TABLE[i];
        const unsigned char *p = sig_location(e, buf, len, tail, tail_len);
        if (p == NULL) {
            continue;
        }
        if (memcmp(p, e->sig, e->sig_len) == 0) {
            return copy_desc(out, out_cap, mime ? e->mime : e->human);
        }
    }
    return match_pe(buf, len, mime, out, out_cap);
}

int file_toto_is_ascii_text(const unsigned char *buf, size_t len)
{
    size_t i;

    if (buf == NULL || len == 0u) {
        return 0;
    }
    for (i = 0u; i < len; i++) {
        unsigned char c = buf[i];
        // check if the character is a tab, newline, or carriage return ASCII control characters
        if (c == '\t' || c == '\n' || c == '\r') {
            continue;
        }
        // check if the character is a printable ASCII character
        if (c < 0x20u || c > 0x7Eu) {
            return 0;
        }
    }
    return 1;
}

/*
 * continuations_ok — 1 if every continuation byte of the n-byte sequence
 * starting at buf[i] that lies inside buf[0..len) has the form 10xxxxxx;
 * otherwise 0. Bytes past len are not inspected.
 */
static int continuations_ok(const unsigned char *buf, size_t len,
                            size_t i, size_t n)
{
    size_t k;

    for (k = 1u; k < n && i + k < len; k++) {
        if ((buf[i + k] & 0xC0u) != 0x80u) {
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

        // check if the character is a tab, newline, or carriage return ASCII control characters
        if (c == '\t' || c == '\n' || c == '\r') {
            i++;
            continue;
        }
        // check if the character is a printable ASCII character
        if (c >= 0x20u && c <= 0x7Eu) {
            i++;
            continue;
        }
        /* Reject other C0 controls and DEL. */
        if (c < 0x80u) {
            return 0;
        }

        /* One multibyte UTF-8 sequence */
        do {
            // check if the character is a leading byte of a 2-byte UTF-8 character
            if ((c & 0xE0u) == 0xC0u) {
                if (c < 0xC2u) {
                    return 0;
                }
                /*
                 * buf may be a truncated prefix of a larger file: a sequence
                 * cut off only by the end of buf is accepted.
                 */
                if (i + 1u >= len) {
                    return 1;
                }
                if ((buf[i + 1u] & 0xC0u) != 0x80u) {
                    return 0;
                }
                i += 2u;
                break;
            }
            // check if the character is a leading byte of a 3-byte UTF-8 character
            if ((c & 0xF0u) == 0xE0u) {
                if (!continuations_ok(buf, len, i, 3u)) {
                    return 0;
                }
                /* Reject overlong forms and UTF-16 surrogates. */
                if (i + 1u < len && c == 0xE0u && buf[i + 1u] < 0xA0u) {
                    return 0;
                }
                if (i + 1u < len && c == 0xEDu && buf[i + 1u] >= 0xA0u) {
                    return 0;
                }
                if (i + 3u > len) {
                    return 1;
                }
                i += 3u;
                break;
            }
            // check if the character is a leading byte of a 4-byte UTF-8 character
            if ((c & 0xF8u) == 0xF0u) {
                if (c > 0xF4u) {
                    return 0;
                }
                if (!continuations_ok(buf, len, i, 4u)) {
                    return 0;
                }
                /* Reject overlong forms and code points above U+10FFFF. */
                if (i + 1u < len && c == 0xF0u && buf[i + 1u] < 0x90u) {
                    return 0;
                }
                if (i + 1u < len && c == 0xF4u && buf[i + 1u] >= 0x90u) {
                    return 0;
                }
                if (i + 4u > len) {
                    return 1;
                }
                i += 4u;
                break;
            }
            // reject other leading bytes
            return 0;
        } while (0);
    }
    return 1;
}
