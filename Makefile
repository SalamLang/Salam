#
# Salam compiler - top-level Makefile (alternative to CMake / tools/build-compiler.sh).
#   make            build ./salamc
#   make test       build, then run the test suite
#   make CC=tcc     build with tcc (the default backend; gcc/clang also work)
#   make clean
#

CC      ?= cc
# -std=gnu11 (not c11): keeps C11 language features but enables glibc's default
# feature set (_DEFAULT_SOURCE) so POSIX/BSD functions used across the tree
# (strcasecmp, usleep/useconds_t, fileno, strdup, isatty) are declared.
CFLAGS  ?= -O2 -Isrc -std=gnu11
SRCDIRS := core source preproc logger xml diag i18n langpack token lexer ast parser \
           semantic codegen llvm interp layout fmt cli driver
SRC     := $(filter-out src/salamc.c,$(wildcard $(addsuffix /*.c,$(addprefix src/,$(SRCDIRS)))))
ifneq (,$(findstring tcc,$(CC)))
LDLIBS ?=
else
LDLIBS ?= -lm
endif
all: salamc
salamc: src/salamc.c $(SRC)
	$(CC) $(CFLAGS) -o $@ src/salamc.c $(SRC) $(LDLIBS)
test: salamc
	sh tools/run-tests.sh
dist: all
	sh tools/build-release.sh
clean:
	rm -f salamc salamc.exe *.o
.PHONY: all test dist clean
