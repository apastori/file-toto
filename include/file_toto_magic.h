#ifndef FILE_TOTO_MAGIC_H
#define FILE_TOTO_MAGIC_H

#include <stddef.h>

/*
 * One compiled-in magic signature: where it sits, bytes to match, then
 * human / MIME labels. Used by the table in file_toto_magic.c.
 *
 * from_end == 0: the signature starts offset bytes after the start of the
 *                file and is looked for in the prefix.
 * from_end == 1: the signature starts offset bytes before the end of the
 *                file and is looked for in the tail.
 */
typedef struct {
    size_t offset;
    int from_end;
    const unsigned char *sig;
    size_t sig_len;
    const char *human;
    const char *mime;
} magic_entry_t;

/*
 * match_magic — try embedded signatures against the prefix buf[0..len) and
 * the tail tail[0..tail_len) (the last tail_len bytes of the file; may be
 * NULL / 0 when unknown), then the PE header check.
 *
 * On match: write human or MIME description into out, return 0.
 * On no match: return -1 (out unchanged).
 * Returns -1 also if out_cap is too small for the matched string.
 *
 * Prefer longest / most specific signature when several could apply.
 */
int match_magic(const unsigned char *buf, size_t len,
                const unsigned char *tail, size_t tail_len,
                int mime, char *out, size_t out_cap);

/*
 * file_toto_is_ascii_text — 1 if all bytes are printable ASCII, TAB, LF, CR;
 * otherwise 0. Empty len returns 0 (caller handles empty separately).
 */
int file_toto_is_ascii_text(const unsigned char *buf, size_t len);

/*
 * file_toto_is_utf8_text — 1 if buf is valid UTF-8 text (allows TAB/LF/CR and
 * printable ASCII plus well-formed multibyte sequences; rejects C0 controls
 * other than TAB/LF/CR and overlong/invalid sequences). Empty => 0.
 * A multibyte sequence cut off by the end of buf is accepted (buf may be a
 * file prefix).
 */
int file_toto_is_utf8_text(const unsigned char *buf, size_t len);

#endif /* FILE_TOTO_MAGIC_H */
