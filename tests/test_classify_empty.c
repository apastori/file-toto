/*
 * Chain-of-thought: empty buffer => "empty" / handled by classify_content.
 */

#include "test_classify_empty.h"
#include "file_toto.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

void test_classify_empty(void)
{
    char out[FILE_TOTO_DESC_CAP];
    int r;

    r = classify_content(NULL, 0u, 0, out, sizeof out);
    assert(r == 0);
    assert(strcmp(out, "empty") == 0);

    r = classify_content(NULL, 0u, 1, out, sizeof out);
    assert(r == 0);
    assert(strcmp(out, "inode/x-empty") == 0);

    printf("PASS: classify empty buffer\n");
}
