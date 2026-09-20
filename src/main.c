/*
 * Chain-of-thought (Step 1 — file scope):
 *
 * 1. Responsibility: program entry — parse CLI, call file_toto_run, exit.
 * 2. Syscalls: none beyond what callees perform.
 * 3. Heap: none.
 * 4. Classify strategy: delegated to identify/magic modules.
 * 5. C11.
 */

#include "file_toto.h"
#include "file_toto_cli.h"
#include "file_toto_identify.h"

#include <stdlib.h>

int main(int argc, char **argv)
{
    file_toto_opts_t opts;

    parse_flags(argc, argv, &opts);
    return file_toto_run(&opts);
}
