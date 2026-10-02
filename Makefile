# file-toto — POSIX-oriented Makefile (gcc or clang on Unix, MSYS2, etc.)
#
# Compiler flag rationale (release CFLAGS):
#   -std=c11          ISO C11 baseline requested by the project spec.
#   -O2               Strong optimisation without -O3's aggressive trade-offs.
#   -Wall -Wextra     Enable most warnings.
#   -Wpedantic        Reject common extensions and dubious constructs.
#   -Werror           Treat warnings as build-breaking (zero-warning policy).
#   -D_POSIX_C_SOURCE=200809L  Expose POSIX.1-2008 (open, read, write, lstat).
#
# Artefacts live under build/ (objects + binaries). Tests under build/tests/.

CC := $(shell command -v gcc >/dev/null 2>&1 && echo gcc || echo clang)

CFLAGS_COMMON := -std=c11 -Wall -Wextra -Wpedantic -Werror \
	-D_POSIX_C_SOURCE=200809L -Iinclude

CFLAGS := $(CFLAGS_COMMON) -O2

# Sanitizers need matching compile+link flags.
CFLAGS_DEBUG := $(CFLAGS_COMMON) -g -O1 -fsanitize=address,undefined \
	-fno-omit-frame-pointer

BUILD_DIR := build
TEST_BUILD_DIR := $(BUILD_DIR)/tests

MAIN_SRCS := src/main.c \
	src/file_toto_emit.c \
	src/file_toto_identify.c \
	src/file_toto_magic.c \
	src/file_toto_cli.c

HDRS := $(wildcard include/*.h)

TEST_SRCS := tests/test_runner.c \
	tests/test_classify_empty.c \
	tests/test_classify_ascii_text.c \
	tests/test_classify_elf.c \
	tests/test_classify_png.c \
	tests/test_classify_pdf.c \
	tests/test_classify_binary_data.c \
	tests/test_classify_mime_flag.c \
	tests/test_classify_tar.c \
	tests/test_classify_iso9660.c \
	tests/test_classify_mp4.c \
	tests/test_classify_pe.c \
	tests/test_classify_dmg_tail.c

TEST_HDRS := $(wildcard tests/*.h)

MAIN_OBJS := $(patsubst src/%.c,$(BUILD_DIR)/%.o,$(MAIN_SRCS))
TEST_OBJS := $(patsubst tests/%.c,$(TEST_BUILD_DIR)/%.o,$(TEST_SRCS))

# Tests exercise classify_content() + magic tables; link magic TU only.
TEST_MAGIC_OBJ := $(BUILD_DIR)/file_toto_magic.o

FILE_TOTO := $(BUILD_DIR)/file-toto
FILE_TOTO_DEBUG := $(BUILD_DIR)/file-toto-debug
TEST_CORE := $(TEST_BUILD_DIR)/test_core

.PHONY: all debug test clean install

all: $(FILE_TOTO)

$(FILE_TOTO): $(MAIN_OBJS) $(HDRS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $(MAIN_OBJS)

debug: $(FILE_TOTO_DEBUG)

# Debug/sanitizer binary: compile+link from sources with CFLAGS_DEBUG
# (must not reuse release .o files built with -O2).
$(FILE_TOTO_DEBUG): $(MAIN_SRCS) $(HDRS) | $(BUILD_DIR)
	$(CC) $(CFLAGS_DEBUG) -o $@ $(MAIN_SRCS)

$(TEST_CORE): $(TEST_OBJS) $(TEST_MAGIC_OBJ) $(HDRS) $(TEST_HDRS) | $(TEST_BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $(TEST_OBJS) $(TEST_MAGIC_OBJ)

test: $(TEST_CORE)
	./$(TEST_CORE)

$(BUILD_DIR)/%.o: src/%.c $(HDRS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(TEST_BUILD_DIR)/%.o: tests/%.c $(HDRS) $(TEST_HDRS) | $(TEST_BUILD_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR) $(TEST_BUILD_DIR):
	mkdir -p $@

clean:
	rm -f $(MAIN_OBJS) $(TEST_OBJS) $(FILE_TOTO) $(FILE_TOTO_DEBUG) $(TEST_CORE)
	rm -f $(FILE_TOTO).exe $(FILE_TOTO_DEBUG).exe $(TEST_CORE).exe

install: $(FILE_TOTO)
	install -m 755 $(FILE_TOTO) /usr/local/bin
