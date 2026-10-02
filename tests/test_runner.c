/*
 * Chain-of-thought: test runner — invoke classify suites in fixed order.
 * No heap; assert-based; C11.
 */

#include "test_classify_empty.h"
#include "test_classify_ascii_text.h"
#include "test_classify_elf.h"
#include "test_classify_png.h"
#include "test_classify_pdf.h"
#include "test_classify_binary_data.h"
#include "test_classify_mime_flag.h"
#include "test_classify_tar.h"
#include "test_classify_iso9660.h"
#include "test_classify_mp4.h"
#include "test_classify_pe.h"
#include "test_classify_dmg_tail.h"

int main(void)
{
    test_classify_empty();
    test_classify_ascii_text();
    test_classify_elf();
    test_classify_png();
    test_classify_pdf();
    test_classify_binary_data();
    test_classify_mime_flag();
    test_classify_tar();
    test_classify_iso9660();
    test_classify_mp4();
    test_classify_pe();
    test_classify_dmg_tail();
    return 0;
}
