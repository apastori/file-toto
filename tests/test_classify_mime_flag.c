/*
 * Chain-of-thought: mime=1 yields type/subtype for the same fixtures.
 */

#include "test_classify_mime_flag.h"
#include "file_toto.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

void test_classify_mime_flag(void)
{
    static const unsigned char empty_ignored = 0;
    static const unsigned char text[] = "plain\n";
    static const unsigned char elf[] = { 0x7Fu, 'E', 'L', 'F', 0, 0, 0, 0 };
    static const unsigned char png[] = {
        0x89u, 0x50u, 0x4Eu, 0x47u, 0x0Du, 0x0Au, 0x1Au, 0x0Au
    };
    static const unsigned char pdf[] = "%PDF-1.7";
    static const unsigned char blob[] = { 0x00u, 0xFFu, 0x10u };
    char out[FILE_TOTO_DESC_CAP];

    (void)empty_ignored;

    assert(classify_content(NULL, 0u, 1, out, sizeof out) == 0);
    assert(strcmp(out, "inode/x-empty") == 0);

    assert(classify_content(text, sizeof text - 1u, 1, out, sizeof out) == 0);
    assert(strcmp(out, "text/plain") == 0);

    assert(classify_content(elf, sizeof elf, 1, out, sizeof out) == 0);
    assert(strcmp(out, "application/x-executable") == 0);

    assert(classify_content(png, sizeof png, 1, out, sizeof out) == 0);
    assert(strcmp(out, "image/png") == 0);

    assert(classify_content(pdf, sizeof pdf - 1u, 1, out, sizeof out) == 0);
    assert(strcmp(out, "application/pdf") == 0);

    assert(classify_content(blob, sizeof blob, 1, out, sizeof out) == 0);
    assert(strcmp(out, "application/octet-stream") == 0);

    printf("PASS: classify MIME mode type/subtype\n");
}
