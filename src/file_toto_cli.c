/*
 * Chain-of-thought (Step 1 — file scope):
 *
 * 1. Responsibility: parse CLI flags into file_toto_opts_t; help/version text.
 * 2. Syscalls: write(STDOUT_FILENO) for help/version only.
 * 3. Heap: none — file pointers are aliases into argv.
 * 4. Classify strategy: N/A.
 * 5. C11; no getopt.
 */

#include "file_toto_cli.h"
#include "file_toto_emit.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void write_stdout(const char *s)
{
    size_t len = strlen(s);
    size_t off = 0u;

    while (off < len) {
        ssize_t n = write(STDOUT_FILENO, s + off, len - off);
        if (n < 0) {
            return;
        }
        if (n == 0) {
            return;
        }
        off += (size_t)n;
    }
}

void print_version(void)
{
    write_stdout("file-toto " FILE_TOTO_VERSION_STRING "\n");
}

void print_help(void)
{
    write_stdout(
        "Usage: file-toto [OPTION]... FILE...\n"
        "Determine type of FILEs.\n"
        "\n"
        "  -b, --brief            do not prepend filenames to output lines\n"
        "  -i, --mime, --mime-type\n"
        "                         output MIME type strings\n"
        "  -h, --no-dereference   do not follow symlinks (default)\n"
        "  -L, --dereference      follow symlinks\n"
        "  -E                     treat filesystem errors as fatal\n"
        "      --help, --h        display this help and exit\n"
        "      --version, --v     output version information and exit\n"
    );
}

static int is_help_flag(const char *s)
{
    return strcmp(s, "--help") == 0 || strcmp(s, "--h") == 0;
}

static int is_version_flag(const char *s)
{
    return strcmp(s, "--version") == 0 || strcmp(s, "--v") == 0;
}

static void apply_short_flags(const char *cluster, file_toto_opts_t *opts)
{
    size_t i;

    /* cluster points at first flag char after '-' */
    for (i = 0u; cluster[i] != '\0'; i++) {
        char c = cluster[i];
        char flag[2];

        flag[0] = c;
        flag[1] = '\0';
        switch (c) {
            case 'b':
                opts->brief = 1;
                break;
            case 'i':
                opts->mime = 1;
                break;
            case 'h':
                opts->dereference = 0;
                break;
            case 'L':
                opts->dereference = 1;
                break;
            case 'E':
                opts->fatal_errors = 1;
                break;
            default:
                file_toto_emit_bad_flag(flag);
                exit(FILE_TOTO_EXIT_ERR);
        }
    }
}

void init_opts(file_toto_opts_t *opts)
{
    opts->brief = 0;
    opts->mime = 0;
    opts->dereference = 0;
    opts->fatal_errors = 0;
    opts->files = NULL;
    opts->n_files = 0;
}

void parse_flags(int argc, char **argv, file_toto_opts_t *opts)
{
    int i;
    int end_flags = 0;
    int has_version = 0;

    init_opts(opts);

    for (i = 1; i < argc; i++) {
        if (is_help_flag(argv[i])) {
            print_help();
            exit(FILE_TOTO_EXIT_OK);
        } 
        if (is_version_flag(argv[i])) {
            has_version = 1;
        }
    }

    if (has_version) {
        print_version();
        exit(FILE_TOTO_EXIT_OK);
    }

    /* Collect file operands into a contiguous region at the end of a
     * temporary pointer list stored by rewriting: we count first, then
     * assign opts->files to the first operand pointer in argv. */
    for (i = 1; i < argc; i++) {
        const char *a = argv[i];

        // Check for -- separator flag
        if (!end_flags && strcmp(a, "--") == 0) {
            end_flags = 1;
            continue;
        }

        if (!end_flags && a[0] == '-' && a[1] != '\0') {
            // check for a long flag
            if (a[1] == '-') {
                if (strcmp(a, "--brief") == 0) {
                    opts->brief = 1;
                    continue;
                } 
                if (strcmp(a, "--mime") == 0 || strcmp(a, "--mime-type") == 0) {
                    opts->mime = 1;
                    continue;
                } 
                if (strcmp(a, "--no-dereference") == 0) {
                    opts->dereference = 0;
                    continue;
                } 
                if (strcmp(a, "--dereference") == 0) {
                    opts->dereference = 1;
                    continue;
                } 
                file_toto_emit_bad_flag(a);
                exit(FILE_TOTO_EXIT_ERR);
            }
            // Check for short flags 
            apply_short_flags(a + 1, opts);
            continue;
        }
        // file operand packed in the second pass below
    }

    /* Second pass: pack operand pointers contiguously into the start of
     * the first operand slot region by writing into a static-sized array
     * of argv pointers — use the argv slots themselves carefully.
     * Simpler: allocate no heap; store indices by rewriting argv packing
     * into opts by building pointer list at &argv[1] after compacting. */
    {
        int n = 0;
        end_flags = 0;
        for (i = 1; i < argc; i++) {
            const char *a = argv[i];

            if (!end_flags && strcmp(a, "--") == 0) {
                end_flags = 1;
                continue;
            }

            if (!end_flags && a[0] == '-' && a[1] != '\0') {
                continue;
            }
            /* copy the operand to the next free slot in argv */
            argv[1 + n] = argv[i];
            n++;
        }
        if (n == 0) {
            file_toto_emit_msg("missing file operand");
            exit(FILE_TOTO_EXIT_ERR);
        }
        opts->files = &argv[1];
        opts->n_files = n;
    }
}
