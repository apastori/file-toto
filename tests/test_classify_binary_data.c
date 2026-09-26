/*
 * Chain-of-thought: non-text bytes with no magic => data.
 */

#include "test_classify_binary_data.h"
#include "file_toto.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

void test_classify_binary_data(void)
{
    static const unsigned char blob[] = {
        0x00u, 0x01u, 0x02u, 0xFFu, 0x80u, 0x10u
    };
    char out[FILE_TOTO_DESC_CAP];
    int r;

    r = classify_content(blob, sizeof blob, 0, out, sizeof out);
    assert(r == 0);
    assert(strcmp(out, "data") == 0);

    printf("PASS: classify binary data fallback\n");
}
