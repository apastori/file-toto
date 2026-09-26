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

int main(void)
{
    test_classify_empty();
    test_classify_ascii_text();
    test_classify_elf();
    test_classify_png();
    test_classify_pdf();
    test_classify_binary_data();
    test_classify_mime_flag();
    return 0;
}
