#
# Salam compiler - Makefile with incremental builds, auto‑dependencies,
# and separate debug/release targets.
#

CC       ?= cc
BUILD    ?= release
TARGET   := salam

OUTDIR   ?= ..
OUTBIN   := $(OUTDIR)/$(TARGET)

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
SALAM_GIT_DATE   := $(shell $(GIT) show -s --format=%ci HEAD 2>/dev/null)
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
  ifeq ($(IS_WINDOWS),1)
    LDLIBS ?=          # Windows tcc bundles libm in its runtime
  else
    LDLIBS ?= -lm      # Unix tcc still needs libm for math symbols
  endif
  DEPFLAGS :=          # tcc lacks -MMD/-MP dependency generation
else
  LDLIBS ?= -lm
endif

WITH_LLVM       ?= 0
WITH_LLD        ?= 0
LLVM_CONFIG     ?= llvm-config
LLVM_LINK_STATIC ?= 0
CXX             ?= c++

LLD_LIBS  :=
LLD_SRCS  :=
ifeq ($(WITH_LLD),1)
  override WITH_LLVM := 1
  CPPFLAGS += -DSALAM_HAVE_LLD
  LLD_LIBS := -llldMinGW -llldELF -llldCOFF -llldMachO -llldWasm -llldCommon -lz -lzstd
  LLD_SRCS := src/llvm/lld_link.cc
endif
LLVM_CXXLIB     ?= -lstdc++
LLVM_ZSTD_LIB   ?= -l:libzstd.a
ifeq ($(WITH_LLVM),1)
  ifeq ($(WITH_LLD),1)
    LLVM_COMPONENTS := all
  else
    LLVM_COMPONENTS := core irreader bitwriter analysis orcjit \
                       native passes target all-targets
  endif
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
  ifeq ($(IS_WINDOWS),1)
    LLVM_SYSTEM_LIBS := $(shell $(LLVM_CONFIG) $(LLVM_STATIC_FLAG) --system-libs \
        | tr ' ' '\n' | grep -viE 'zstd' | tr '\n' ' ') $(LLVM_ZSTD_LIB)
  else
    LLVM_SYSTEM_LIBS := $(shell $(LLVM_CONFIG) $(LLVM_STATIC_FLAG) --system-libs)
  endif
  # NOTE: `--libs all` (WITH_LLD) lists -lPolly/-lPollyISL, and LLVM's own
  # libLLVMLTO.a references getPollyPluginInfo(), so Polly's static libs must be
  # INSTALLED (do not filter them). LLVM 22 ships them; on LLVM 18 install the
  # `libpolly-18-dev` apt package alongside llvm-18-dev / liblld-18-dev.
  LDLIBS   += $(LLD_LIBS) \
              $(shell $(LLVM_CONFIG) $(LLVM_STATIC_FLAG) --libs $(LLVM_COMPONENTS)) \
              $(LLVM_SYSTEM_LIBS) $(LLVM_CXXLIB)
endif

LLVM_CXXFLAGS := $(if $(filter 1,$(WITH_LLD)),$(shell $(LLVM_CONFIG) --cxxflags))

BUILD_DIR ?= build/$(BUILD)
OBJS      := $(SRCS:src/%.c=$(BUILD_DIR)/%.o)
CXXOBJS   := $(LLD_SRCS:src/%.cc=$(BUILD_DIR)/%.o)
MAIN_OBJ  := $(MAIN_SRC:src/%.c=$(BUILD_DIR)/%.o)
DEPS      := $(OBJS:.o=.d) $(MAIN_OBJ:.o=.d)

EMBED_OBJS :=
SALAM_EMBED_MUSL_DIR ?=
ifneq ($(SALAM_EMBED_MUSL_DIR),)
  CPPFLAGS   += -DSALAM_HAVE_EMBED_MUSL
  EMBED_OBJS += $(BUILD_DIR)/embed/musl.o
endif
SALAM_EMBED_MINGW_DIR ?=
ifneq ($(SALAM_EMBED_MINGW_DIR),)
  CPPFLAGS   += -DSALAM_HAVE_EMBED_MINGW
  EMBED_OBJS += $(BUILD_DIR)/embed/mingw.o
endif

# Per-arch embedded musl sysroots (opt-in via SALAM_EMBED_MUSL_<ARCH>_DIR). Each
# arch is a distinct tar with its own symbol so ONE flagship salam can cross-link
# static musl ELF for several targets. musl.o above stays the x86_64 host case.
# $(call embed_musl_arch,<stem>,<ARCHUPPER>)
define embed_musl_arch
SALAM_EMBED_MUSL_$(2)_DIR ?=
ifneq ($$(SALAM_EMBED_MUSL_$(2)_DIR),)
  CPPFLAGS   += -DSALAM_HAVE_EMBED_MUSL_$(2)
  EMBED_OBJS += $$(BUILD_DIR)/embed/musl_$(1).o
endif
$$(BUILD_DIR)/embed/musl_$(1).o:
	@test -d "$$(SALAM_EMBED_MUSL_$(2)_DIR)" || { echo "SALAM_EMBED_MUSL_$(2)_DIR='$$(SALAM_EMBED_MUSL_$(2)_DIR)' is not a directory"; exit 1; }
	@mkdir -p $$(BUILD_DIR)/embed
	tar cf $$(BUILD_DIR)/embed/musl_$(1).tar -C "$$(SALAM_EMBED_MUSL_$(2)_DIR)" .
	printf '.section .rodata\n.global salam_embed_musl_$(1)\nsalam_embed_musl_$(1):\n.incbin "%s"\n.global salam_embed_musl_$(1)_end\nsalam_embed_musl_$(1)_end:\n.section .note.GNU-stack,"",@progbits\n' "$$(BUILD_DIR)/embed/musl_$(1).tar" > $$(BUILD_DIR)/embed/musl_$(1).S
	$$(CC) -c $$(BUILD_DIR)/embed/musl_$(1).S -o $$@
endef
$(eval $(call embed_musl_arch,aarch64,AARCH64))
$(eval $(call embed_musl_arch,i686,I686))
$(eval $(call embed_musl_arch,arm,ARM))

.PHONY: all test dist clean debug install uninstall stage-sysroots self-contained

# The per-arch embed rules above are defined via $(eval) before `all`, which would
# otherwise make the first of them the default goal. Pin `all` as the default.
.DEFAULT_GOAL := all

all: $(OUTBIN)

$(OUTBIN): $(OBJS) $(CXXOBJS) $(EMBED_OBJS) $(MAIN_OBJ)
	$(CC) $(LDFLAGS) $(EXTRA_LDFLAGS) -o $@ $^ $(LDLIBS) $(EXTRA_LDLIBS)

$(BUILD_DIR)/embed/musl.o:
	@test -d "$(SALAM_EMBED_MUSL_DIR)" || { echo "SALAM_EMBED_MUSL_DIR='$(SALAM_EMBED_MUSL_DIR)' is not a directory (try 'make stage-sysroots')"; exit 1; }
	@mkdir -p $(BUILD_DIR)/embed
	tar cf $(BUILD_DIR)/embed/musl.tar -C "$(SALAM_EMBED_MUSL_DIR)" .
	printf '.section .rodata\n.global salam_embed_musl\nsalam_embed_musl:\n.incbin "%s"\n.global salam_embed_musl_end\nsalam_embed_musl_end:\n.section .note.GNU-stack,"",@progbits\n' "$(BUILD_DIR)/embed/musl.tar" > $(BUILD_DIR)/embed/musl.S
	$(CC) -c $(BUILD_DIR)/embed/musl.S -o $@

$(BUILD_DIR)/embed/mingw.o:
	@test -d "$(SALAM_EMBED_MINGW_DIR)" || { echo "SALAM_EMBED_MINGW_DIR='$(SALAM_EMBED_MINGW_DIR)' is not a directory (try 'make stage-sysroots')"; exit 1; }
	@mkdir -p $(BUILD_DIR)/embed
	tar cf $(BUILD_DIR)/embed/mingw.tar -C "$(SALAM_EMBED_MINGW_DIR)" .
	printf '.section .rodata\n.global salam_embed_mingw\nsalam_embed_mingw:\n.incbin "%s"\n.global salam_embed_mingw_end\nsalam_embed_mingw_end:\n.section .note.GNU-stack,"",@progbits\n' "$(BUILD_DIR)/embed/mingw.tar" > $(BUILD_DIR)/embed/mingw.S
	$(CC) -c $(BUILD_DIR)/embed/mingw.S -o $@

$(BUILD_DIR)/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEPFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: src/%.cc
	@mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(LLVM_CXXFLAGS) $(CXXFLAGS) -c $< -o $@

-include $(DEPS)

test: $(OUTBIN)
	SALAM=$(OUTBIN) sh tools/bash/run-tests.sh

dist: all
	sh tools/bash/build-release.sh

STAGE_DIR  ?= $(BUILD_DIR)/sysroots
MUSL_ARCH  ?= x86_64
MINGW_ARCH ?= x86_64
MUSL_SR    := $(STAGE_DIR)/$(MUSL_ARCH)-linux-musl
MINGW_SR   := $(STAGE_DIR)/$(MINGW_ARCH)-w64-mingw32

stage-sysroots:
	@echo "Staging sysroots into $(STAGE_DIR) ..."
	@mkdir -p "$(MUSL_SR)" "$(MINGW_SR)/lib"
	@cp /usr/lib/$(MUSL_ARCH)-linux-musl/crt1.o /usr/lib/$(MUSL_ARCH)-linux-musl/crti.o \
	    /usr/lib/$(MUSL_ARCH)-linux-musl/crtn.o /usr/lib/$(MUSL_ARCH)-linux-musl/libc.a \
	    "$(MUSL_SR)/" || { echo "  ! musl not found - 'apt install musl-dev'"; exit 1; }
	@cp $$(ls /usr/lib/llvm-*/lib/clang/*/lib/linux/libclang_rt.builtins-$(MUSL_ARCH).a 2>/dev/null | head -1) \
	    "$(MUSL_SR)/" 2>/dev/null || echo "  (compiler-rt builtins not bundled; optional)"
	@for x in crt2.o dllcrt2.o libmingw32.a libmingwex.a libmoldname.a libmsvcrt.a \
	          libadvapi32.a libshell32.a libuser32.a libkernel32.a; do \
	    cp /usr/$(MINGW_ARCH)-w64-mingw32/lib/$$x "$(MINGW_SR)/lib/" 2>/dev/null \
	      || { echo "  ! mingw not found - 'apt install mingw-w64'"; exit 1; }; \
	  done
	@G=$$(ls -d /usr/lib/gcc/$(MINGW_ARCH)-w64-mingw32/*win32 2>/dev/null | head -1); \
	  test -n "$$G" && cp "$$G/crtbegin.o" "$$G/crtend.o" "$$G/libgcc.a" "$$G/libgcc_eh.a" \
	     "$(MINGW_SR)/lib/" || { echo "  ! gcc-mingw runtime not found"; exit 1; }
	@echo "Staged:"
	@echo "  musl  -> $(MUSL_SR)"
	@echo "  mingw -> $(MINGW_SR)"

self-contained: stage-sysroots
	$(MAKE) WITH_LLVM=1 WITH_LLD=1 LLVM_LINK_STATIC=1 \
	        SALAM_EMBED_MUSL_DIR="$(MUSL_SR)" SALAM_EMBED_MINGW_DIR="$(MINGW_SR)" \
	        LLVM_CONFIG=$(LLVM_CONFIG) CXX=$(CXX)
	@echo "Built self-contained salam -> $(OUTBIN)"
	@echo "  cross-compile with e.g.:"
	@echo "    $(OUTBIN) build app.salam --target=x86_64-linux-musl      -o app"
	@echo "    $(OUTBIN) build app.salam --target=x86_64-w64-windows-gnu -o app.exe"

debug: BUILD = debug
debug: clean all

install: all
	@echo "Installing salam -> $(DESTDIR)$(BINDIR)"
	mkdir -p $(DESTDIR)$(BINDIR)
	cp -f $(OUTBIN) $(DESTDIR)$(BINDIR)/$(TARGET)
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
	rm -rf build $(OUTBIN) $(OUTBIN).exe $(TARGET) $(TARGET).exe
