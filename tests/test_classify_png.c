/*
 * Chain-of-thought: PNG 8-byte signature => PNG image data.
 */

#include "test_classify_png.h"
#include "file_toto.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

void test_classify_png(void)
{
    static const unsigned char png[] = {
        0x89u, 0x50u, 0x4Eu, 0x47u, 0x0Du, 0x0Au, 0x1Au, 0x0Au,
        0, 0, 0, 0
    };
    char out[FILE_TOTO_DESC_CAP];
    int r;

    r = classify_content(png, sizeof png, NULL, 0u, 0, out, sizeof out);
    assert(r == 0);
    assert(strcmp(out, "PNG image data") == 0);

    printf("PASS: classify PNG signature\n");
}
