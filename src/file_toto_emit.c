/*
 * Chain-of-thought (Step 1 — file scope):
 *
 * 1. Responsibility: stderr diagnostics via raw write(2), no stdio formatting
 *    on the diagnostic path.
 * 2. Syscalls: write(STDERR_FILENO, …) with EINTR / partial-write retry.
 * 3. Heap: none — fixed stack buffers for message assembly.
 * 4. Classify strategy: N/A.
 * 5. C11 via project Makefile.
 */

#include "file_toto_emit.h"

#include <errno.h>
#include <string.h>
#include <unistd.h>

static void try_write_all(int fd, const char *buf, size_t len)
{
    size_t off = 0u;

    while (off < len) {
        ssize_t n = write(fd, buf + off, len - off);
        if (n < 0) {
            if (errno == EINTR) {
                continue;
            }
            return;
        }
        if (n == 0) {
            return;
        }
        off += (size_t)n;
    }
}

static void emit_line(const char *line)
{
    try_write_all(STDERR_FILENO, line, strlen(line));
}

void file_toto_emit_error(const char *context)
{
    char line[512];
    const char *err;
    size_t n;

    if (context == NULL) {
        context = "?";
    }
    err = strerror(errno);
    if (err == NULL) {
        err = "Unknown error";
    }

    n = 0u;
    /* file-toto: <context>: <err>\n */
    {
        const char *p = "file-toto: ";
        while (*p != '\0' && n + 1u < sizeof line) {
            line[n++] = *p++;
        }
    }
    {
        const char *p = context;
        while (*p != '\0' && n + 1u < sizeof line) {
            line[n++] = *p++;
        }
    }
    if (n + 2u < sizeof line) {
        line[n++] = ':';
        line[n++] = ' ';
    }
    {
        const char *p = err;
        while (*p != '\0' && n + 1u < sizeof line) {
            line[n++] = *p++;
        }
    }
    if (n + 1u < sizeof line) {
        line[n++] = '\n';
    }
    line[n] = '\0';
    emit_line(line);
}

void file_toto_emit_bad_flag(const char *flag)
{
    char line[256];
    size_t n = 0u;
    const char *p;

    p = "file-toto: invalid option -- '";
    while (*p != '\0' && n + 1u < sizeof line) {
        line[n++] = *p++;
    }
    if (flag != NULL) {
        p = flag;
        while (*p != '\0' && n + 1u < sizeof line) {
            line[n++] = *p++;
        }
    }
    p = "'\n";
    while (*p != '\0' && n + 1u < sizeof line) {
        line[n++] = *p++;
    }
    line[n] = '\0';
    emit_line(line);
}

void file_toto_emit_msg(const char *msg)
{
    char line[512];
    size_t n = 0u;
    const char *p;

    p = "file-toto: ";
    while (*p != '\0' && n + 1u < sizeof line) {
        line[n++] = *p++;
    }
    if (msg != NULL) {
        p = msg;
        while (*p != '\0' && n + 1u < sizeof line) {
            line[n++] = *p++;
        }
    }
    if (n + 1u < sizeof line) {
        line[n++] = '\n';
    }
    line[n] = '\0';
    emit_line(line);
}
