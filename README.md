# file-toto

Unix `file`-like CLI: classifies each path operand using filesystem tests,
then embedded magic on an 8 KiB content prefix (`FILE_TOTO_BUF_SIZE`), then a
text/data heuristic. Prints one result line per operand on stdout.

No libmagic and no external magic database - libc only.

## Build

```sh
make
make debug
make test
make clean
# optional:
make install
```

Artefacts:

- `build/file-toto` (or `build/file-toto.exe` on Windows)
- `build/file-toto-debug`
- `build/tests/test_core`

**Linux:**

```sh
make clean && make && make test
```

**Windows (MSYS2 UCRT64):**

```sh
pacman -S make mingw-w64-ucrt-x86_64-gcc   # once
make clean && make && make test
```

Git Bash needs `/c/msys64/ucrt64/bin` early on `PATH`.
The native `.exe` runs in UCRT64, Git Bash, cmd, and PowerShell.

## Usage

Default output is `filename: description`.

| Flag | Meaning |
|------|---------|
| `-b`, `--brief` | Description only (no `filename: ` prefix) |
| `-i`, `--mime`, `--mime-type` | MIME type instead of human description |
| `-h`, `--no-dereference` | Do not follow symlinks (default) |
| `-L`, `--dereference` | Follow symlinks |
| `-E` | Treat filesystem errors as fatal (exit 1) |
| `--help`, `--h` | Usage on stdout, exit 0 |
| `--version`, `--v` | `file-toto <version>`, exit 0 |

Meta flags are double-dash forms only (`--help` / `--h`, `--version` / `--v`).
`-h` means no-dereference. Help anywhere in `argv` wins over version.

Examples:

```sh
./build/file-toto README.md
./build/file-toto -b README.md
./build/file-toto -i README.md
./build/file-toto a.png b.pdf
```

Missing files print a cannot-open result line on stdout and exit `0` unless
`-E` is set (then stderr diagnostic and exit `1`).

## Exit codes

| Code | Meaning |
|------|---------|
| `0` | Help, version, successful classification, or cannot-open without `-E` |
| `1` | Bad flags, write errors, or filesystem errors with `-E` |

Stderr diagnostics: `file-toto: <context>: <reason>`.

## Layout

```
LICENSE.txt
c_version.txt
Makefile
README.md
include/          file_toto.h + file_toto_*.h
src/              main.c + file_toto_{emit,identify,magic,cli}.c
tests/            test_runner.c + test_classify_*.c -> build/tests/test_core
build/            objects/binaries; dirs via .gitkeep, artefacts gitignored
```
