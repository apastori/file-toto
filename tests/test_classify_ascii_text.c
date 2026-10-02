/*
 * Chain-of-thought: printable ASCII => ASCII text, not data.
 */

#include "test_classify_ascii_text.h"
#include "file_toto.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

void test_classify_ascii_text(void)
{
    static const unsigned char sample[] = "Hello, file-toto.\n";
    char out[FILE_TOTO_DESC_CAP];
    int r;

    r = classify_content(sample, sizeof sample - 1u, NULL, 0u, 0, out,
                         sizeof out);
    assert(r == 0);
    assert(strcmp(out, "ASCII text") == 0);

    printf("PASS: classify ASCII text\n");
}
