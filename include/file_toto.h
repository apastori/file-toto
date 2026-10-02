/*
 * Chain-of-thought (Step 1 — before code):
 *
 * 1. Single responsibility: public contract for file-toto — buffer sizes,
 *    version, exit codes, option struct, classify_content().
 * 2. Syscalls: none in this header. classify_content() only examines bytes.
 * 3. Heap: none. Stack description buffers sized by FILE_TOTO_DESC_CAP.
 * 4. Classify strategy: magic first (match_magic, on the prefix and the
 *    tail), then text vs data.
 * 5. Standard: C11 — static inline, size_t from stddef.h.
 */

#ifndef FILE_TOTO_H
#define FILE_TOTO_H

#include "file_toto_magic.h"

#include <stddef.h>
#include <string.h>

/*
 * Content prefix read size (64 KiB, as real file's encoding limit).
 * The size_t cast makes the multiplication happen in size_t, so it cannot
 * wrap on a platform where unsigned int is only 16 bits.
 */
#define FILE_TOTO_BUF_SIZE ((size_t)64u * 1024u)

/*
 * End-of-file read size for signatures stored at the end of a file
 * (512 = size of the Apple DMG "koly" trailer).
 */
#define FILE_TOTO_TAIL_SIZE 512u

/* Stack capacity for one description string including NUL. */
#define FILE_TOTO_DESC_CAP 256u

#define FILE_TOTO_VERSION_STRING "1.0.0"

enum {
    FILE_TOTO_EXIT_OK = 0,
    FILE_TOTO_EXIT_ERR = 1
};

typedef struct {
    int brief;          /* -b / --brief */
    int mime;           /* -i / --mime / --mime-type */
    int dereference;    /* -L; default 0 => lstat / no-dereference */
    int fatal_errors;   /* -E */
    char **files;
    int n_files;
} file_toto_opts_t;

/*
 * classify_content — pure content classifier (magic then text/data).
 *
 * Preconditions: buf non-NULL if len > 0; out != NULL; out_cap >= 1.
 * tail / tail_len: the last tail_len bytes of the file (may point into buf);
 * NULL / 0 when unknown, which only disables end-of-file signatures.
 * Postcondition: out is NUL-terminated on success.
 * Returns 0 on success, -1 if out_cap is too small.
 * len == 0 => "empty" / "inode/x-empty".
 */
static inline int classify_content(const unsigned char *buf, size_t len,
                                   const unsigned char *tail, size_t tail_len,
                                   int mime, char *out, size_t out_cap)
{
    const char *desc = NULL;

    if (out == NULL || out_cap < 1u) {
        return -1;
    }

    do {
        if (len == 0u) {
            desc = mime ? "inode/x-empty" : "empty";
            break;
        }
        if (match_magic(buf, len, tail, tail_len, mime, out, out_cap) == 0) {
            return 0;
        }
        if (file_toto_is_ascii_text(buf, len)) {
            desc = mime ? "text/plain" : "ASCII text";
            break;
        }
        if (file_toto_is_utf8_text(buf, len)) {
            desc = mime ? "text/plain" : "UTF-8 Unicode text";
            break;
        }
        desc = mime ? "application/octet-stream" : "data";
    } while (0);

    if (strlen(desc) + 1u > out_cap) {
        return -1;
    }
    memcpy(out, desc, strlen(desc) + 1u);
    return 0;
}

#endif /* FILE_TOTO_H */
