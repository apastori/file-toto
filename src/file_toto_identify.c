/*
 * Chain-of-thought (Step 1 — file scope):
 *
 * 1. Responsibility: per-operand filesystem identify + content classify +
 *    write result lines to stdout.
 * 2. Syscalls: lstat/stat, open, read, close, write; Windows _setmode.
 * 3. Heap: none — stack buffers for prefix and description/line.
 * 4. Classify: FS types first, then classify_content() on prefix.
 * 5. C11 + POSIX.1-2008.
 */

#include "file_toto.h"
#include "file_toto_emit.h"
#include "file_toto_identify.h"

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#if defined(_WIN32)
#include <io.h>
#endif

static void maybe_normalize_broken_pipe_errno(int fd)
{
#if defined(_WIN32)
    if (errno == EINVAL) {
        /* Pipe write failure on some CRTs reports EINVAL. */
        (void)fd;
        errno = EPIPE;
    }
#else
    (void)fd;
#endif
}

static void try_write_all(int fd, const char *buf, size_t len)
{
    size_t off = 0u;

    while (off < len) {
        ssize_t n = write(fd, buf + off, len - off);
        if (n < 0) {
            if (errno == EINTR) {
                continue;
            }
            maybe_normalize_broken_pipe_errno(fd);
            file_toto_emit_error("write");
            exit(FILE_TOTO_EXIT_ERR);
        }
        if (n == 0) {
            file_toto_emit_error("write");
            exit(FILE_TOTO_EXIT_ERR);
        }
        off += (size_t)n;
    }
}

static void write_result_line(const file_toto_opts_t *opts, const char *name,
                              const char *desc)
{
    char line[FILE_TOTO_DESC_CAP + 512u];
    size_t n = 0u;

    if (!opts->brief) {
        const char *p = name;
        while (*p != '\0' && n + 1u < sizeof line) {
            line[n++] = *p++;
        }
        if (n + 2u < sizeof line) {
            line[n++] = ':';
            line[n++] = ' ';
        }
    }
    {
        const char *p = desc;
        while (*p != '\0' && n + 1u < sizeof line) {
            line[n++] = *p++;
        }
    }
    if (n + 1u < sizeof line) {
        line[n++] = '\n';
    }
    line[n] = '\0';
    try_write_all(STDOUT_FILENO, line, n);
}

static int set_binary_mode(int fd)
{
#if defined(_WIN32)
    if (_setmode(fd, _O_BINARY) == -1) {
        return -1;
    }
#else
    (void)fd;
#endif
    return 0;
}

static int do_stat(const char *path, int dereference, struct stat *st)
{
    /*
     * Linux/POSIX: lstat for no-dereference (default / -h); stat for -L.
     * Windows/MinGW UCRT64: no lstat / S_IFLNK in the CRT — always stat(2).
     * Do not invent fake symlink types on _WIN32.
     */
#if defined(_WIN32)
    (void)dereference;
    return stat(path, st);
#else
    if (dereference) {
        return stat(path, st);
    }
    return lstat(path, st);
#endif
}

static void fs_type_desc(const struct stat *st, int mime, char *out,
                         size_t out_cap)
{
    const char *h;
    const char *m;

    if (S_ISDIR(st->st_mode)) {
        h = "directory";
        m = "inode/directory";
#if defined(S_ISLNK)
    } else if (S_ISLNK(st->st_mode)) {
        h = "symbolic link";
        m = "inode/symlink";
#endif
#if defined(S_ISFIFO)
    } else if (S_ISFIFO(st->st_mode)) {
        h = "fifo";
        m = "inode/fifo";
#endif
#if defined(S_ISSOCK)
    } else if (S_ISSOCK(st->st_mode)) {
        h = "socket";
        m = "inode/socket";
#endif
#if defined(S_ISBLK)
    } else if (S_ISBLK(st->st_mode)) {
        h = "block special";
        m = "inode/blockdevice";
#endif
#if defined(S_ISCHR)
    } else if (S_ISCHR(st->st_mode)) {
        h = "character special";
        m = "inode/chardevice";
#endif
    } else if (S_ISREG(st->st_mode) && st->st_size == 0) {
        h = "empty";
        m = "inode/x-empty";
    } else {
        h = NULL;
        m = NULL;
    }

    if (h == NULL) {
        out[0] = '\0';
        return;
    }
    {
        const char *d = mime ? m : h;
        size_t n = strlen(d);
        if (n + 1u > out_cap) {
            out[0] = '\0';
            return;
        }
        memcpy(out, d, n + 1u);
    }
}

static ssize_t read_prefix(int fd, unsigned char *buf, size_t cap)
{
    size_t off = 0u;

    while (off < cap) {
        ssize_t n = read(fd, buf + off, cap - off);
        if (n < 0) {
            if (errno == EINTR) {
                continue;
            }
            return -1;
        }
        if (n == 0) {
            break;
        }
        off += (size_t)n;
        /* One successful read of any size is enough for magic; keep filling
         * until EOF or cap for better heuristics. */
    }
    return (ssize_t)off;
}

static int process_one_file(const char *path, const file_toto_opts_t *opts)
{
    struct stat st;
    char desc[FILE_TOTO_DESC_CAP];
    unsigned char buf[FILE_TOTO_BUF_SIZE];
    int fd;
    ssize_t nread;

    if (do_stat(path, opts->dereference, &st) != 0) {
        if (opts->fatal_errors) {
            file_toto_emit_error(path);
            return FILE_TOTO_EXIT_ERR;
        }
        {
            char msg[256];
            const char *err = strerror(errno);
            size_t n = 0u;
            const char *p = "cannot open (";
            while (*p && n + 1u < sizeof msg) {
                msg[n++] = *p++;
            }
            if (err) {
                while (*err && n + 1u < sizeof msg) {
                    msg[n++] = *err++;
                }
            }
            if (n + 2u < sizeof msg) {
                msg[n++] = ')';
                msg[n] = '\0';
            } else {
                msg[sizeof msg - 1u] = '\0';
            }
            write_result_line(opts, path, msg);
        }
        return FILE_TOTO_EXIT_OK;
    }

    /* Non-regular or empty regular handled via fs_type_desc. */
    if (!S_ISREG(st.st_mode) || st.st_size == 0) {
        fs_type_desc(&st, opts->mime, desc, sizeof desc);
        if (desc[0] != '\0') {
            write_result_line(opts, path, desc);
            return FILE_TOTO_EXIT_OK;
        }
    }

    fd = open(path, O_RDONLY);
    if (fd < 0) {
        if (opts->fatal_errors) {
            file_toto_emit_error(path);
            return FILE_TOTO_EXIT_ERR;
        }
        {
            char msg[256];
            const char *err = strerror(errno);
            size_t n = 0u;
            const char *p = "cannot open (";
            while (*p && n + 1u < sizeof msg) {
                msg[n++] = *p++;
            }
            if (err) {
                while (*err && n + 1u < sizeof msg) {
                    msg[n++] = *err++;
                }
            }
            if (n + 2u < sizeof msg) {
                msg[n++] = ')';
                msg[n] = '\0';
            } else {
                msg[sizeof msg - 1u] = '\0';
            }
            write_result_line(opts, path, msg);
        }
        return FILE_TOTO_EXIT_OK;
    }

    if (set_binary_mode(fd) != 0) {
        file_toto_emit_error(path);
        close(fd);
        return FILE_TOTO_EXIT_ERR;
    }

    nread = read_prefix(fd, buf, FILE_TOTO_BUF_SIZE);
    if (nread < 0) {
        int saved = errno;
        close(fd);
        errno = saved;
        if (opts->fatal_errors) {
            file_toto_emit_error(path);
            return FILE_TOTO_EXIT_ERR;
        }
        {
            char msg[256];
            const char *err = strerror(errno);
            size_t n = 0u;
            const char *p = "cannot read (";
            while (*p && n + 1u < sizeof msg) {
                msg[n++] = *p++;
            }
            if (err) {
                while (*err && n + 1u < sizeof msg) {
                    msg[n++] = *err++;
                }
            }
            if (n + 2u < sizeof msg) {
                msg[n++] = ')';
                msg[n] = '\0';
            } else {
                msg[sizeof msg - 1u] = '\0';
            }
            write_result_line(opts, path, msg);
        }
        return FILE_TOTO_EXIT_OK;
    }
    close(fd);

    if (classify_content(buf, (size_t)nread, opts->mime, desc,
                        sizeof desc) != 0) {
        file_toto_emit_msg("description buffer too small");
        return FILE_TOTO_EXIT_ERR;
    }
    write_result_line(opts, path, desc);
    return FILE_TOTO_EXIT_OK;
}

int file_toto_run(const file_toto_opts_t *opts)
{
    int status = FILE_TOTO_EXIT_OK;
    int i;

    if (opts == NULL || opts->files == NULL || opts->n_files <= 0) {
        file_toto_emit_msg("missing file operand");
        return FILE_TOTO_EXIT_ERR;
    }

    for (i = 0; i < opts->n_files; i++) {
        int r = process_one_file(opts->files[i], opts);
        if (r != FILE_TOTO_EXIT_OK) {
            status = r;
        }
    }
    return status;
}
