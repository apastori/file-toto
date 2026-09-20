#ifndef FILE_TOTO_CLI_H
#define FILE_TOTO_CLI_H

#include "file_toto.h"

/*
 * parse_flags — fill opts from argv; exit(0) on help/version; exit(1) on error.
 * Remaining file operands are pointers into argv (not copied).
 * Requires at least one file operand.
 */
void parse_flags(int argc, char **argv, file_toto_opts_t *opts);

void print_help(void);
void print_version(void);

#endif /* FILE_TOTO_CLI_H */
