/*
 * Chain-of-thought: ELF magic 0x7F 'E' 'L' 'F' => ELF executable.
 */

#include "test_classify_elf.h"
#include "file_toto.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

void test_classify_elf(void)
{
    static const unsigned char elf[] = {
        0x7Fu, 'E', 'L', 'F', 1, 1, 1, 0,
        0, 0, 0, 0, 0, 0, 0, 0
    };
    char out[FILE_TOTO_DESC_CAP];
    int r;

    r = classify_content(elf, sizeof elf, 0, out, sizeof out);
    assert(r == 0);
    assert(strcmp(out, "ELF executable") == 0);

    printf("PASS: classify ELF magic\n");
}
