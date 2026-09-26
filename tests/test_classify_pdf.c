/*
 * Chain-of-thought: %PDF- prefix => PDF document.
 */

#include "test_classify_pdf.h"
#include "file_toto.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

void test_classify_pdf(void)
{
    static const unsigned char pdf[] = "%PDF-1.4\n%...";
    char out[FILE_TOTO_DESC_CAP];
    int r;

    r = classify_content(pdf, sizeof pdf - 1u, 0, out, sizeof out);
    assert(r == 0);
    assert(strcmp(out, "PDF document") == 0);

    printf("PASS: classify PDF prefix\n");
}
