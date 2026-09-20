#ifndef FILE_TOTO_MAGIC_H
#define FILE_TOTO_MAGIC_H

#include <stddef.h>

/*
 * match_magic — try embedded signatures against buf[0..len).
 *
 * On match: write human or MIME description into out, return 0.
 * On no match: return -1 (out unchanged).
 * Returns -1 also if out_cap is too small for the matched string.
 *
 * Prefer longest / most specific signature when several could apply.
 */
int match_magic(const unsigned char *buf, size_t len, int mime,
                char *out, size_t out_cap);

/*
 * file_toto_is_ascii_text — 1 if all bytes are printable ASCII, TAB, LF, CR;
 * otherwise 0. Empty len returns 0 (caller handles empty separately).
 */
int file_toto_is_ascii_text(const unsigned char *buf, size_t len);

/*
 * file_toto_is_utf8_text — 1 if buf is valid UTF-8 text (allows TAB/LF/CR and
 * printable ASCII plus well-formed multibyte sequences; rejects C0 controls
 * other than TAB/LF/CR and overlong/invalid sequences). Empty => 0.
 */
int file_toto_is_utf8_text(const unsigned char *buf, size_t len);

#endif /* FILE_TOTO_MAGIC_H */
