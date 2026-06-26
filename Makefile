#
# Salam compiler - Makefile with incremental builds, auto‑dependencies,
# and separate debug/release targets.
#

CC       ?= cc
BUILD    ?= release
TARGET   := salamc

SRCDIRS := core source preproc logger xml diag i18n langpack         \
           token lexer ast parser semantic codegen llvm interp       \
           layout fmt cli driver

SRCS     := $(filter-out src/salamc.c, $(wildcard $(addsuffix /*.c, $(addprefix src/,$(SRCDIRS)))))
MAIN_SRC := src/salamc.c

ifeq ($(BUILD),debug)
  OPTIMIZE := -O0 -g
else
  OPTIMIZE := -O2
endif

CPPFLAGS ?= -Isrc
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

.PHONY: all test dist clean debug

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

clean:
	rm -rf build $(TARGET) $(TARGET).exe
