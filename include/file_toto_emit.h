#ifndef FILE_TOTO_EMIT_H
#define FILE_TOTO_EMIT_H

/*
 * file_toto_emit_error — stderr: file-toto: <context>: <strerror(errno)>
 * Preconditions: context != NULL; errno set by failed operation.
 */
void file_toto_emit_error(const char *context);

/*
 * file_toto_emit_bad_flag — stderr: file-toto: invalid option -- '<flag>'
 * flag is a single character for short options, or a token for long forms.
 */
void file_toto_emit_bad_flag(const char *flag);

/*
 * file_toto_emit_msg — stderr: file-toto: <message>
 */
void file_toto_emit_msg(const char *msg);

#endif /* FILE_TOTO_EMIT_H */
