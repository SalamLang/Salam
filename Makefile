#
# Salam compiler - Makefile with incremental builds, auto‑dependencies,
# and separate debug/release targets.
#

CC       ?= cc
BUILD    ?= release
TARGET   := salam

# Install layout (override on the command line, e.g. `make install PREFIX=$HOME/.local`).
# DESTDIR supports staged installs for packagers.
PREFIX   ?= /usr/local
DESTDIR  ?=
BINDIR   := $(PREFIX)/bin
DATADIR  := $(PREFIX)/share/salam

SRCDIRS := core source preproc logger xml diag i18n langpack         \
           token lexer ast parser semantic codegen llvm interp       \
           layout fmt cli driver

SRCS     := $(filter-out src/salam.c, $(wildcard $(addsuffix /*.c, $(addprefix src/,$(SRCDIRS)))))
MAIN_SRC := src/salam.c

ifeq ($(BUILD),debug)
  OPTIMIZE := -O0 -g
else
  OPTIMIZE := -O2
endif

CPPFLAGS ?= -Isrc
# Bake the install prefix so an installed binary finds its std/ tree even when
# launched from an unrelated directory. Resolution still prefers --stdlib-path,
# $SALAM_STD, a sibling salam.cfg, and binary-relative discovery first.
CPPFLAGS += -DSALAM_STDLIB_PREFIX='"$(DATADIR)"'
CFLAGS   ?= $(OPTIMIZE) -std=gnu89 -Wall -Wextra                    \
            -Wno-unused-parameter -Wno-unused-function -Wno-unused-variable
DEPFLAGS  = -MMD -MP

ifneq (,$(findstring tcc,$(CC)))
  LDLIBS ?=
else
  LDLIBS ?= -lm
endif

BUILD_DIR ?= build/$(BUILD)
OBJS      := $(SRCS:src/%.c=$(BUILD_DIR)/%.o)
MAIN_OBJ  := $(MAIN_SRC:src/%.c=$(BUILD_DIR)/%.o)
DEPS      := $(OBJS:.o=.d) $(MAIN_OBJ:.o=.d)

.PHONY: all test dist clean debug install uninstall

all: $(TARGET)

$(TARGET): $(OBJS) $(MAIN_OBJ)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

$(BUILD_DIR)/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEPFLAGS) -c $< -o $@

-include $(DEPS)

test: $(TARGET)
	sh tools/run-tests.sh

dist: all
	sh tools/build-release.sh

debug: BUILD = debug
debug: clean all

install: all
	@echo "Installing salam -> $(DESTDIR)$(BINDIR)"
	mkdir -p $(DESTDIR)$(BINDIR)
	cp -f $(TARGET) $(DESTDIR)$(BINDIR)/$(TARGET)
	chmod 0755 $(DESTDIR)$(BINDIR)/$(TARGET)
	@echo "Installing std/ -> $(DESTDIR)$(DATADIR)/std"
	rm -rf $(DESTDIR)$(DATADIR)/std
	mkdir -p $(DESTDIR)$(DATADIR)/std
	cp -R std/. $(DESTDIR)$(DATADIR)/std/
	@echo "Done. 'salam' resolves std/ from $(DATADIR)/std automatically."

uninstall:
	rm -f  $(DESTDIR)$(BINDIR)/$(TARGET)
	rm -rf $(DESTDIR)$(DATADIR)

clean:
	rm -rf build $(TARGET) $(TARGET).exe
