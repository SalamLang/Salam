#
# Salam compiler - Makefile with incremental builds, auto‑dependencies,
# and separate debug/release targets.
#

CC       ?= cc
BUILD    ?= release
TARGET   := salam

# Host/target OS detection. MSYS2/MinGW shells report MINGW*/MSYS* via uname,
# and GNU make on Windows sets OS=Windows_NT. Either marks a Windows build.
UNAME_S := $(shell uname -s 2>/dev/null)
ifneq (,$(filter Windows_NT,$(OS)))
  IS_WINDOWS := 1
endif
ifneq (,$(findstring MINGW,$(UNAME_S)))
  IS_WINDOWS := 1
endif
ifneq (,$(findstring MSYS,$(UNAME_S)))
  IS_WINDOWS := 1
endif

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
CPPFLAGS += -DSALAM_STDLIB_PREFIX='"$(DATADIR)"'

GIT             := git
SALAM_GIT_COMMIT := $(shell $(GIT) rev-parse --short HEAD 2>/dev/null)
SALAM_GIT_DATE   := $(shell $(GIT) show -s --format=%cs HEAD 2>/dev/null)
SALAM_GIT_DIRTY  := $(shell test -n "$$($(GIT) status --porcelain 2>/dev/null)" && echo '-dirty')
ifneq ($(SALAM_GIT_COMMIT),)
  CPPFLAGS += -DSALAM_GIT_COMMIT='"$(SALAM_GIT_COMMIT)"' \
              -DSALAM_GIT_DATE='"$(SALAM_GIT_DATE)"'     \
              -DSALAM_GIT_DIRTY='"$(SALAM_GIT_DIRTY)"'
endif
CFLAGS   ?= $(OPTIMIZE) -std=gnu89 -Wall -Wextra                    \
            -Wno-unused-parameter -Wno-unused-function -Wno-unused-variable
DEPFLAGS  = -MMD -MP

ifneq (,$(findstring tcc,$(CC)))
  LDLIBS ?=
else
  LDLIBS ?= -lm
endif

WITH_LLVM       ?= 0
LLVM_CONFIG     ?= llvm-config
LLVM_LINK_STATIC ?= 0
LLVM_CXXLIB     ?= -lstdc++
ifeq ($(WITH_LLVM),1)
  LLVM_COMPONENTS := core irreader bitwriter analysis orcjit \
                     native passes target all-targets
  ifeq ($(LLVM_LINK_STATIC),1)
    LLVM_STATIC_FLAG := --link-static
    ifeq ($(IS_WINDOWS),1)
      LDFLAGS += -static -static-libgcc -static-libstdc++
    endif
  else
    LLVM_STATIC_FLAG :=
  endif
  CPPFLAGS += -DSALAM_HAVE_LLVM -I$(shell $(LLVM_CONFIG) --includedir) \
              -DSALAM_LLVM_VERSION_MAJOR=$(shell $(LLVM_CONFIG) --version | cut -d. -f1)
  LDFLAGS  += $(shell $(LLVM_CONFIG) $(LLVM_STATIC_FLAG) --ldflags)
  LLVM_SYSTEM_LIBS := $(shell $(LLVM_CONFIG) $(LLVM_STATIC_FLAG) --system-libs)
  ifeq ($(IS_WINDOWS),1)
    LLVM_SYSTEM_LIBS := $(patsubst %libzstd.dll.a,-l:libzstd.a,$(LLVM_SYSTEM_LIBS))
    LLVM_SYSTEM_LIBS := $(patsubst -lzstd,-l:libzstd.a,$(LLVM_SYSTEM_LIBS))
  endif
  # libLLVM is C++, so pull in the C++ runtime and LLVM's system deps last.
  LDLIBS   += $(shell $(LLVM_CONFIG) $(LLVM_STATIC_FLAG) --libs $(LLVM_COMPONENTS)) \
              $(LLVM_SYSTEM_LIBS) $(LLVM_CXXLIB)
endif

BUILD_DIR ?= build/$(BUILD)
OBJS      := $(SRCS:src/%.c=$(BUILD_DIR)/%.o)
MAIN_OBJ  := $(MAIN_SRC:src/%.c=$(BUILD_DIR)/%.o)
DEPS      := $(OBJS:.o=.d) $(MAIN_OBJ:.o=.d)

.PHONY: all test dist clean debug install uninstall

all: $(TARGET)

$(TARGET): $(OBJS) $(MAIN_OBJ)
	$(CC) $(LDFLAGS) $(EXTRA_LDFLAGS) -o $@ $^ $(LDLIBS) $(EXTRA_LDLIBS)

$(BUILD_DIR)/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEPFLAGS) -c $< -o $@

-include $(DEPS)

test: $(TARGET)
	sh tools/bash/run-tests.sh

dist: all
	sh tools/bash/build-release.sh

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
