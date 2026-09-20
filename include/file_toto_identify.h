#ifndef FILE_TOTO_IDENTIFY_H
#define FILE_TOTO_IDENTIFY_H

#include "file_toto.h"

/*
 * file_toto_run — classify each operand and write result lines to stdout.
 *
 * Returns FILE_TOTO_EXIT_OK or FILE_TOTO_EXIT_ERR.
 * Write failures call exit(FILE_TOTO_EXIT_ERR) and do not return.
 */
int file_toto_run(const file_toto_opts_t *opts);

#endif /* FILE_TOTO_IDENTIFY_H */
