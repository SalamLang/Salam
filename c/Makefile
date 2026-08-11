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
ifneq (,$(findstring Darwin,$(UNAME_S)))
  IS_MACOS := 1
endif

PREFIX   ?= /usr/local
DESTDIR  ?=
BINDIR   := $(PREFIX)/bin
LIBDIR   := $(PREFIX)/lib
DATADIR  := $(PREFIX)/share/salam

SRCDIRS := core source condcomp logger xml diag i18n langpack        \
           token lexer ast parser semantic codegen jsgen llvm interp \
           layout fmt doc cli driver minify

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

LLVM_CONFIG     ?= llvm-config
CXX             ?= c++

#
# LLVM/LLD are the default backend, so a plain `make` builds them in
# whenever this host can - a salam without them falls back to the C backend
# and needs a compiler installed on the *end user's* machine, which is
# exactly what shipping LLVM in-process is meant to avoid.
#
# "Can" means llvm-config resolves AND the static lld archives are present
# (llvm-config knows nothing about lld, so it is probed separately). A host
# missing either still builds, just without the LLVM path - so this stays a
# capability upgrade rather than a new hard build dependency. Set
# WITH_LLVM=0 to opt out explicitly.
#
LLVM_CONFIG_OK := $(shell $(LLVM_CONFIG) --version >/dev/null 2>&1 && echo 1)
ifeq ($(LLVM_CONFIG_OK),1)
  LLVM_LIBDIR_PROBE := $(shell $(LLVM_CONFIG) --libdir 2>/dev/null)
  LLD_STATIC_OK := $(if $(wildcard $(LLVM_LIBDIR_PROBE)/liblldCommon.a),1)
endif

WITH_LLVM       ?= $(if $(LLVM_CONFIG_OK),1,0)
WITH_LLD        ?= $(if $(LLD_STATIC_OK),1,0)
LLVM_LINK_STATIC ?= $(if $(LLVM_CONFIG_OK),1,0)


LLD_LIBS  :=
LLD_SRCS  :=
WIN_LLD_SHIM_OBJS :=
ifeq ($(WITH_LLD),1)
  override WITH_LLVM := 1
  CPPFLAGS += -DSALAM_HAVE_LLD
  LLD_LIBS := -llldMinGW -llldELF -llldCOFF -llldMachO -llldWasm -llldCommon -lz -lzstd
  LLD_SRCS := src/llvm/lld_link.cc
  # MSYS2's mingw-w64-x86_64-lld/-llvm packages ship their static archives
  # built as if always consumed alongside the shared libLLVM-*.dll/libxml2
  # DLL, so those archives reach the demangler and libxml2 through
  # __imp_-prefixed dllimport pointers instead of calling the (also
  # statically available) real symbols directly - see
  # win_lld_demangle_shim.S for the full explanation and undefined-symbol
  # list this resolves. Only needed on the MSYS2 toolchain; Linux/macOS
  # LLVM/lld packages link these calls directly with no __imp_ involved.
  ifeq ($(IS_WINDOWS),1)
    WIN_LLD_SHIM_OBJS = $(BUILD_DIR)/llvm/win_lld_demangle_shim.o
  endif
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
    # Linux got neither of these, so `ldd` on a LLVM_LINK_STATIC=1 build named
    # libstdc++.so.6, libgcc_s.so.1, libz.so.1, libzstd.so.1 and libxml2.so.2
    # (which drags libicuuc, libicudata and liblzma in behind it) - eight
    # shared objects a user of a "self-contained" compiler should never have
    # to install. LLVM and LLD are C++, so the C++ runtime is not optional
    # here; folding it in is what makes the binary portable across distros
    # with different libstdc++ ABIs.
    #
    # Deliberately not plain -static: a fully static glibc binary silently
    # loses NSS (getaddrinfo, getpwnam) at runtime, which std/net and
    # std/os/user need. libc stays dynamic - it is on every target machine by
    # definition - and only the extras are folded in.
    ifneq ($(IS_WINDOWS)$(IS_MACOS),1)
      LDFLAGS += -static-libgcc -static-libstdc++
    endif
  else
    LLVM_STATIC_FLAG :=
  endif
  CPPFLAGS += -DSALAM_HAVE_LLVM -I$(shell $(LLVM_CONFIG) --includedir) \
              -DSALAM_LLVM_VERSION_MAJOR=$(shell $(LLVM_CONFIG) --version | cut -d. -f1)
  # Homebrew ships lld as its own formula with its own prefix, so lld's
  # headers are NOT under `llvm-config --includedir` the way they are in the
  # apt.llvm.org packages. lld_link.cc includes <lld/Common/Driver.h>, so on
  # macOS the C++ shim failed with "file not found" and libsalam-llvm died -
  # which, while that failure was only a ::warning::, is why every macOS
  # stage2 quietly shipped on the C backend instead of LLVM. Added only when
  # the prefix resolves, so a machine without the formula is unaffected.
  ifeq ($(WITH_LLD),1)
    BREW_LLD_PREFIX := $(shell brew --prefix lld 2>/dev/null)
    ifneq (,$(wildcard $(BREW_LLD_PREFIX)/include/lld/Common/Driver.h))
      CPPFLAGS += -I$(BREW_LLD_PREFIX)/include
      # The archives live beside the headers in that same keg, and
      # `llvm-config --ldflags` only ever points at llvm's own libdir, so
      # -llldCommon would not resolve either.
      LDFLAGS  += -L$(BREW_LLD_PREFIX)/lib
    endif
  endif
  LDFLAGS  += $(shell $(LLVM_CONFIG) $(LLVM_STATIC_FLAG) --ldflags)
  # z3 is dropped everywhere: Homebrew (and several Linux distros) build
  # their llvm package with LLVM_ENABLE_Z3_SOLVER=ON, so llvm-config reports
  # Z3 as a system lib. Linking it stamps a dynamic reference to a *versioned*
  # Cellar dylib - /opt/homebrew/opt/z3/lib/libz3.<maj>.<min>.dylib - into the
  # binary, a path that exists only on a machine with that exact Homebrew z3
  # installed. Every released macOS salam therefore aborted under dyld on any
  # other Mac ("Library not loaded: .../libz3.4.16.dylib"). Z3 is clang's
  # static-analyzer constraint solver; salam links LLVM libraries only, never
  # clang, so no object here references a Z3 symbol and dropping it is safe.
  # z3 is keg-only in Homebrew, so llvm-config emits its *full path*
  # (.../opt/z3/lib/libz3.dylib), not a bare "-lz3" flag - match on the
  # substring the same way the zstd filter below does, not an anchored
  # "^-lz3$", or the keg-only path form slips through unfiltered.
  LLVM_SYSTEM_LIBS := $(shell $(LLVM_CONFIG) $(LLVM_STATIC_FLAG) --system-libs \
      | tr ' ' '\n' | grep -viE 'z3' | tr '\n' ' ')
  ifeq ($(IS_WINDOWS),1)
    LLVM_SYSTEM_LIBS := $(shell printf '%s' "$(LLVM_SYSTEM_LIBS)" \
        | tr ' ' '\n' | grep -viE 'zstd' | tr '\n' ' ') $(LLVM_ZSTD_LIB)
  endif
  # Same reasoning as z3, for the one system lib macOS cannot get from the
  # SDK: LLVM's Support library really does call into zstd, and with
  # -L$(brew --prefix)/lib on the link line -lzstd resolves to Homebrew's
  # *dylib*. Prefer the static archive Homebrew ships beside it (Apple's ld
  # has no -l:libzstd.a, so this has to be a full path) and fall back to the
  # dylib only when it is genuinely absent.
  ifeq ($(IS_MACOS),1)
    MACOS_ZSTD_A := $(firstword $(wildcard \
        $(shell brew --prefix zstd 2>/dev/null)/lib/libzstd.a \
        /opt/homebrew/opt/zstd/lib/libzstd.a \
        /usr/local/opt/zstd/lib/libzstd.a))
    ifneq (,$(MACOS_ZSTD_A))
      LLVM_SYSTEM_LIBS := $(shell printf '%s' "$(LLVM_SYSTEM_LIBS)" \
          | tr ' ' '\n' | grep -viE 'zstd' | tr '\n' ' ') $(MACOS_ZSTD_A)
    endif
  endif
  # Same idea on Linux, where llvm-config's -lz/-lzstd resolve to the shared
  # objects and put libz.so.1/libzstd.so.1 in the released binary's ldd. Each
  # is probed separately and left alone when its archive is not installed:
  # a missing .a should cost portability on that build host, not break the
  # link. -l: is GNU ld's exact-archive-name syntax, so no path is needed.
  #
  # libxml2 is deliberately NOT folded in here. LLVMWindowsManifest really
  # does call xmlNewDoc (on every host, not just Windows - see std/llvm), but
  # libxml2.a pulls ICU and lzma behind it, and libicudata.a alone is ~30 MB
  # of collation tables this compiler never reads. That one stays dynamic
  # until the WindowsManifest component can be dropped from the link instead.
  #
  # Substituted in BOTH places each -l appears, not just here: LLD_LIBS
  # carries its own hardcoded -lz -lzstd and sits earlier on the command
  # line, and ld takes the first match, so rewriting only the later copy
  # changed the link line without changing a single entry in `ldd`.
  ifeq ($(IS_WINDOWS)$(IS_MACOS),)
    LINUX_LIBDIR := /usr/lib/$(shell uname -m)-linux-gnu
    ifneq (,$(wildcard $(LINUX_LIBDIR)/libz.a))
      LLD_LIBS := $(patsubst -lz,-l:libz.a,$(LLD_LIBS))
      LLVM_SYSTEM_LIBS := $(shell printf '%s' "$(LLVM_SYSTEM_LIBS)" \
          | tr ' ' '\n' | grep -vxE '\-lz' | tr '\n' ' ') -l:libz.a
    endif
    ifneq (,$(wildcard $(LINUX_LIBDIR)/libzstd.a))
      LLD_LIBS := $(patsubst -lzstd,-l:libzstd.a,$(LLD_LIBS))
      LLVM_SYSTEM_LIBS := $(shell printf '%s' "$(LLVM_SYSTEM_LIBS)" \
          | tr ' ' '\n' | grep -vxE '\-lzstd' | tr '\n' ' ') -l:libzstd.a
    endif
    # -static-libstdc++ only governs the libstdc++ the compiler driver adds
    # implicitly. This link is driven by cc, not c++, so the C++ runtime is
    # requested explicitly through LLVM_CXXLIB - and an explicit -lstdc++
    # resolves to the shared object regardless of the flag, which is why
    # libstdc++.so.6 stayed in ldd. Name the archive instead.
    LLVM_CXXLIB := $(if $(wildcard $(shell $(CC) -print-file-name=libstdc++.a)),-l:libstdc++.a,-lstdc++)
  endif
  # `llvm-config --libs all` can advertise Polly on an installation that
  # does not ship libPolly.a, because distributions split it into a separate
  # libpolly-N-dev package. Drop the -l for an archive that is not there,
  # probing each of the pair separately since they are advertised together
  # but not guaranteed to be packaged together.
  #
  # Note this only rescues an install whose LLVM does not *reference* Polly.
  # Where libLLVMLTO.a was built against it - Debian/Ubuntu's llvm-N-dev,
  # among others - there is no winning move here: keeping -lPolly gives
  # "cannot find -lPolly" and dropping it gives "undefined reference to
  # getPollyPluginInfo()". That install is simply incomplete, so say so
  # rather than letting the linker produce a mystifying error.
  LLVM_LIBDIR_L := $(shell $(LLVM_CONFIG) --libdir 2>/dev/null)
  LLVM_ALL_LIBS := $(shell $(LLVM_CONFIG) $(LLVM_STATIC_FLAG) --libs $(LLVM_COMPONENTS))
  LLVM_DROP_POLLY := $(strip \
      $(if $(wildcard $(LLVM_LIBDIR_L)/libPolly.a),,-lPolly) \
      $(if $(wildcard $(LLVM_LIBDIR_L)/libPollyISL.a),,-lPollyISL))
  ifneq (,$(filter $(LLVM_DROP_POLLY),$(LLVM_ALL_LIBS)))
    $(warning llvm-config advertises Polly but $(LLVM_LIBDIR_L) has no libPolly.a.)
    $(warning If the link below fails on 'getPollyPluginInfo', this LLVM was)
    $(warning built against Polly and the install is incomplete - fix it with)
    $(warning "apt-get install libpolly-$(shell $(LLVM_CONFIG) --version | cut -d. -f1)-dev" or the equivalent.)
  endif
  LLVM_COMPONENT_LIBS := $(filter-out $(LLVM_DROP_POLLY),$(LLVM_ALL_LIBS))
  LDLIBS   += $(LLD_LIBS) $(LLVM_COMPONENT_LIBS) \
              $(LLVM_SYSTEM_LIBS) $(LLVM_CXXLIB)
  ifeq ($(IS_WINDOWS),1)
    ifeq ($(WITH_LLD),1)
      # libxml2.a (pulled in via libLLVMWindowsManifest.a, itself pulled in
      # by liblldCOFF.a) needs zlib/iconv/bcrypt symbols - llvm-config's
      # --system-libs above already lists -lz, but earlier than -lxml2, and
      # never lists -liconv/-lbcrypt at all. GNU ld resolves an undefined
      # symbol only from an archive it hasn't scanned past yet, so these
      # have to be repeated here, after -lxml2, to actually satisfy them.
      LDLIBS += -lz -liconv -lbcrypt
    endif
  endif
endif

LLVM_CXXFLAGS := $(if $(filter 1,$(WITH_LLD)),$(shell $(LLVM_CONFIG) --cxxflags))

BUILD_DIR ?= build/$(BUILD)

#
# CPPFLAGS is assembled from the WITH_*/SALAM_EMBED_* switches above, but
# the compile rules depend only on sources - so flipping a switch (adding an
# embedded sysroot, say) silently reuses objects compiled without it, and
# the resulting salam reports "no mingw sysroot" despite having one baked
# in. Recording the flag set in a file that every object depends on makes
# such a change rebuild exactly what it must, instead of needing a clean.
#
CONFIG_STAMP := $(BUILD_DIR)/.config-stamp

OBJS      := $(SRCS:src/%.c=$(BUILD_DIR)/%.o)
CXXOBJS   := $(LLD_SRCS:src/%.cc=$(BUILD_DIR)/%.o)
MAIN_OBJ  := $(MAIN_SRC:src/%.c=$(BUILD_DIR)/%.o)
DEPS      := $(OBJS:.o=.d) $(MAIN_OBJ:.o=.d)

EMBED_OBJS := $(WIN_LLD_SHIM_OBJS)
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
	printf '.section .rodata\n.global salam_embed_musl_$(1)\nsalam_embed_musl_$(1):\n.incbin "%s"\n.global salam_embed_musl_$(1)_end\nsalam_embed_musl_$(1)_end:\n' "$$(BUILD_DIR)/embed/musl_$(1).tar" > $$(BUILD_DIR)/embed/musl_$(1).S
	$$(CC) -c $$(BUILD_DIR)/embed/musl_$(1).S -o $$@
endef
$(eval $(call embed_musl_arch,aarch64,AARCH64))
$(eval $(call embed_musl_arch,i686,I686))
$(eval $(call embed_musl_arch,arm,ARM))

# The same, for mingw. SALAM_EMBED_MINGW_DIR above is the x86_64 slot (its
# name predates there being more than one); this adds the others. Without an
# i686 slot, 32-bit Windows was the one target with no embedded sysroot from
# any host - `--target=i686-w64-windows-gnu` had to find a mingw32 install on
# the user's machine, while every other target linked from what the compiler
# carries. sema.c's mingw_sysroot_subdir already maps the triple to
# i686-w64-mingw32, so only the staging and embedding were missing.
define embed_mingw_arch
SALAM_EMBED_MINGW_$(2)_DIR ?=
ifneq ($$(SALAM_EMBED_MINGW_$(2)_DIR),)
  CPPFLAGS   += -DSALAM_HAVE_EMBED_MINGW_$(2)
  EMBED_OBJS += $$(BUILD_DIR)/embed/mingw_$(1).o
endif
$$(BUILD_DIR)/embed/mingw_$(1).o:
	@test -d "$$(SALAM_EMBED_MINGW_$(2)_DIR)" || { echo "SALAM_EMBED_MINGW_$(2)_DIR='$$(SALAM_EMBED_MINGW_$(2)_DIR)' is not a directory"; exit 1; }
	@mkdir -p $$(BUILD_DIR)/embed
	tar cf $$(BUILD_DIR)/embed/mingw_$(1).tar -C "$$(SALAM_EMBED_MINGW_$(2)_DIR)" .
	printf '.section .rodata\n.global salam_embed_mingw_$(1)\nsalam_embed_mingw_$(1):\n.incbin "%s"\n.global salam_embed_mingw_$(1)_end\nsalam_embed_mingw_$(1)_end:\n' "$$(BUILD_DIR)/embed/mingw_$(1).tar" > $$(BUILD_DIR)/embed/mingw_$(1).S
	$$(CC) -c $$(BUILD_DIR)/embed/mingw_$(1).S -o $$@
endef
$(eval $(call embed_mingw_arch,i686,I686))

# Static builds of third-party libraries (sqlite3, openssl, hiredis,
# mariadb-client) for a given --target= triple, embedded independently of
# the musl/mingw crt+libc sysroots above: not every target's crt/libc
# sysroot is actually embedded on every build host (some resolve it from a
# system package - e.g. musl-tools/mingw-w64 - at link time instead), but
# every target should still get static third-party libs regardless of
# where its crt/libc came from. One embed slot per target holds whatever
# .a files a given rollout phase has staged into it (starts with just
# libsqlite3.a; other libraries land in the same directory in later
# phases) - it's just an extra -L search path, never a full sysroot.
define embed_extralibs_target
SALAM_EMBED_EXTRALIBS_$(2)_DIR ?=
ifneq ($$(SALAM_EMBED_EXTRALIBS_$(2)_DIR),)
  CPPFLAGS   += -DSALAM_HAVE_EMBED_EXTRALIBS_$(2)
  EMBED_OBJS += $$(BUILD_DIR)/embed/extralibs_$(1).o
endif
$$(BUILD_DIR)/embed/extralibs_$(1).o:
	@test -d "$$(SALAM_EMBED_EXTRALIBS_$(2)_DIR)" || { echo "SALAM_EMBED_EXTRALIBS_$(2)_DIR='$$(SALAM_EMBED_EXTRALIBS_$(2)_DIR)' is not a directory"; exit 1; }
	@mkdir -p $$(BUILD_DIR)/embed
	tar cf $$(BUILD_DIR)/embed/extralibs_$(1).tar -C "$$(SALAM_EMBED_EXTRALIBS_$(2)_DIR)" .
	printf '.section .rodata\n.global salam_embed_extralibs_$(1)\nsalam_embed_extralibs_$(1):\n.incbin "%s"\n.global salam_embed_extralibs_$(1)_end\nsalam_embed_extralibs_$(1)_end:\n' "$$(BUILD_DIR)/embed/extralibs_$(1).tar" > $$(BUILD_DIR)/embed/extralibs_$(1).S
	$$(CC) -c $$(BUILD_DIR)/embed/extralibs_$(1).S -o $$@
endef
$(eval $(call embed_extralibs_target,x86_64_linux_musl,X86_64_LINUX_MUSL))
$(eval $(call embed_extralibs_target,aarch64_linux_musl,AARCH64_LINUX_MUSL))
$(eval $(call embed_extralibs_target,i686_linux_musl,I686_LINUX_MUSL))
$(eval $(call embed_extralibs_target,arm_linux_musleabihf,ARM_LINUX_MUSLEABIHF))
$(eval $(call embed_extralibs_target,x86_64_w64_windows_gnu,X86_64_W64_WINDOWS_GNU))

# Same idea as SALAM_EMBED_EXTRALIBS_*_DIR above, but for a plain native
# build (no --target=): a single slot, since a given salam binary only ever
# runs on the one host it was built for. Consumed by the tcc/C backend
# (build.c) and the LLVM external-tool link path (llvm_native.c's
# link_executable) as an extra -L search directory, so `link dynamic
# "sqlite3"` etc. resolve to a bundled static archive even for same-host
# builds.
SALAM_EMBED_HOSTLIBS_DIR ?=
ifneq ($(SALAM_EMBED_HOSTLIBS_DIR),)
  CPPFLAGS   += -DSALAM_HAVE_EMBED_HOSTLIBS
  EMBED_OBJS += $(BUILD_DIR)/embed/hostlibs.o
endif
$(BUILD_DIR)/embed/hostlibs.o:
	@test -d "$(SALAM_EMBED_HOSTLIBS_DIR)" || { echo "SALAM_EMBED_HOSTLIBS_DIR='$(SALAM_EMBED_HOSTLIBS_DIR)' is not a directory"; exit 1; }
	@mkdir -p $(BUILD_DIR)/embed
	tar cf $(BUILD_DIR)/embed/hostlibs.tar -C "$(SALAM_EMBED_HOSTLIBS_DIR)" .
	printf '.section .rodata\n.global salam_embed_hostlibs\nsalam_embed_hostlibs:\n.incbin "%s"\n.global salam_embed_hostlibs_end\nsalam_embed_hostlibs_end:\n' "$(BUILD_DIR)/embed/hostlibs.tar" > $(BUILD_DIR)/embed/hostlibs.S
	$(CC) -c $(BUILD_DIR)/embed/hostlibs.S -o $@

$(BUILD_DIR)/llvm/win_lld_demangle_shim.o: src/llvm/win_lld_demangle_shim.S
	@mkdir -p $(BUILD_DIR)/llvm
	$(CC) -c $< -o $@

.PHONY: all test dist clean debug install uninstall stage-sysroots self-contained

.DEFAULT_GOAL := all

all: $(OUTBIN)

$(OUTBIN): $(OBJS) $(CXXOBJS) $(EMBED_OBJS) $(MAIN_OBJ)
	@mkdir -p $(OUTDIR)
	$(CC) $(LDFLAGS) $(EXTRA_LDFLAGS) -o $@ $^ $(LDLIBS) $(EXTRA_LDLIBS)

$(BUILD_DIR)/embed/musl.o:
	@test -d "$(SALAM_EMBED_MUSL_DIR)" || { echo "SALAM_EMBED_MUSL_DIR='$(SALAM_EMBED_MUSL_DIR)' is not a directory (try 'make stage-sysroots')"; exit 1; }
	@mkdir -p $(BUILD_DIR)/embed
	tar cf $(BUILD_DIR)/embed/musl.tar -C "$(SALAM_EMBED_MUSL_DIR)" .
	printf '.section .rodata\n.global salam_embed_musl\nsalam_embed_musl:\n.incbin "%s"\n.global salam_embed_musl_end\nsalam_embed_musl_end:\n' "$(BUILD_DIR)/embed/musl.tar" > $(BUILD_DIR)/embed/musl.S
	$(CC) -c $(BUILD_DIR)/embed/musl.S -o $@

$(BUILD_DIR)/embed/mingw.o:
	@test -d "$(SALAM_EMBED_MINGW_DIR)" || { echo "SALAM_EMBED_MINGW_DIR='$(SALAM_EMBED_MINGW_DIR)' is not a directory (try 'make stage-sysroots')"; exit 1; }
	@mkdir -p $(BUILD_DIR)/embed
	tar cf $(BUILD_DIR)/embed/mingw.tar -C "$(SALAM_EMBED_MINGW_DIR)" .
	printf '.section .rodata\n.global salam_embed_mingw\nsalam_embed_mingw:\n.incbin "%s"\n.global salam_embed_mingw_end\nsalam_embed_mingw_end:\n' "$(BUILD_DIR)/embed/mingw.tar" > $(BUILD_DIR)/embed/mingw.S
	$(CC) -c $(BUILD_DIR)/embed/mingw.S -o $@

$(CONFIG_STAMP): FORCE
	@mkdir -p $(dir $@)
	@printf '%s\n' "$(CPPFLAGS)" > $@.new
	@cmp -s $@.new $@ || mv -f $@.new $@
	@rm -f $@.new

.PHONY: FORCE
FORCE:

$(BUILD_DIR)/%.o: src/%.c $(CONFIG_STAMP)
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEPFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: src/%.cc $(CONFIG_STAMP)
	@mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(LLVM_CXXFLAGS) $(CXXFLAGS) -c $< -o $@

-include $(DEPS)

#
# One merged static archive holding everything std/llvm needs: every LLVM
# component `llvm-config --libs all` resolves, the six lld archives, and
# salam's own C shims over them (llvm_native.o, lld_link.o).
#
# This exists for the *self-hosted* compiler. A Salam source file can only
# say `link static "NAME"`; it cannot shell out to `llvm-config --libs` the
# way this Makefile does, and LLVM ships ~200 version-dependent component
# archives rather than one libLLVM.a. std/llvm/llvm.salam records that as
# its blocking TODO(build-pipeline). Merging them here turns the whole
# graph into a single name, so std/llvm can link with:
#
#     link static "salam_llvm"
#
# plus the handful of system libs below, which are not LLVM's to merge.
# Build with: make libsalam-llvm
#
LLVM_MERGE_LIBS = $(shell $(LLVM_CONFIG) --link-static --libs all 2>/dev/null) $(LLD_LIBS)

# Salam's own C shims that std/llvm binds to. llvm_native.o and lld_link.o
# carry salam_llvm_native/salam_lld_link; orc_call.o carries
# salam_orc_call_main plus the salam_llvm_init_all_* wrappers around LLVM-C's
# header-inline target initializers (see orc_call.h). Omitting orc_call.o
# left both undefined at link time.
LLVM_MERGE_SHIMS = $(BUILD_DIR)/llvm/llvm_native.o $(BUILD_DIR)/llvm/orc_call.o \
                   $(CXXOBJS) $(WIN_LLD_SHIM_OBJS)
LLVM_MERGE_DIR  = $(BUILD_DIR)/merge
SALAM_LLVM_A    = $(OUTDIR)/libsalam_llvm.a

#
# The archive merge is driven by an MRI script (`ar -M`), which is a GNU
# binutils extension. macOS ships BSD ar, which has no -M at all and answers
# with its usage text, so libsalam-llvm could never succeed there - and while
# that failure was only a ::warning::, that is why every macOS stage2 fell
# back to the C backend.
#
# llvm-ar reads the same MRI script and ships inside the very LLVM
# installation this target already requires, so it is preferred on every
# host rather than special-casing macOS. The plain `ar` fallback keeps a
# GNU host working if the LLVM package somehow omits it.
#
LLVM_BINDIR := $(shell $(LLVM_CONFIG) --bindir 2>/dev/null)
AR_MRI      := $(firstword $(wildcard $(LLVM_BINDIR)/llvm-ar) \
                           $(shell command -v llvm-ar 2>/dev/null) ar)

.PHONY: libsalam-llvm
libsalam-llvm: $(LLVM_MERGE_SHIMS)
	@test "$(WITH_LLVM)$(WITH_LLD)" = "11" \
	  || { echo "libsalam-llvm needs WITH_LLVM=1 WITH_LLD=1 (llvm-config: $(LLVM_CONFIG))"; exit 1; }
	@rm -rf $(LLVM_MERGE_DIR); mkdir -p $(LLVM_MERGE_DIR)
	@LIBDIR=$$($(LLVM_CONFIG) --libdir); \
	 : > $(LLVM_MERGE_DIR)/mri; \
	 echo "create $(SALAM_LLVM_A)" >> $(LLVM_MERGE_DIR)/mri; \
	 for o in $(LLVM_MERGE_SHIMS); do \
	   echo "addmod $$o" >> $(LLVM_MERGE_DIR)/mri; \
	 done; \
	 n=0; \
	 for l in $(LLVM_MERGE_LIBS); do \
	   case "$$l" in -l*) a="$$LIBDIR/lib$${l#-l}.a";; *) continue;; esac; \
	   if [ -f "$$a" ]; then echo "addlib $$a" >> $(LLVM_MERGE_DIR)/mri; n=$$((n+1)); fi; \
	 done; \
	 echo "save"  >> $(LLVM_MERGE_DIR)/mri; \
	 echo "end"   >> $(LLVM_MERGE_DIR)/mri; \
	 echo "merging $$n LLVM/lld archives + salam shims -> $(SALAM_LLVM_A)"; \
	 rm -f $(SALAM_LLVM_A); \
	 $(AR_MRI) -M < $(LLVM_MERGE_DIR)/mri
	@echo "Built $(SALAM_LLVM_A)"
	@echo "  std/llvm then links with: link static \"salam_llvm\""
	@echo "  plus system libs: $(LLVM_SYSTEM_LIBS) $(LLVM_CXXLIB)"

test: $(OUTBIN)
	SALAM=$(abspath $(OUTBIN)) sh ../tools/bash/run-tests.sh

dist: all
	sh ../tools/bash/build-release.sh

STAGE_DIR  ?= $(BUILD_DIR)/sysroots
MUSL_ARCH  ?= x86_64
MINGW_ARCH ?= x86_64
MUSL_SR    := $(STAGE_DIR)/$(MUSL_ARCH)-linux-musl
MINGW_SR   := $(STAGE_DIR)/$(MINGW_ARCH)-w64-mingw32

#
# Where a mingw sysroot lives differs by build host: a Linux cross-build has
# it under /usr/<triple>, while an MSYS2 host keeps the same files directly
# in /mingw64/lib. Both are searched so `make stage-sysroots` works natively
# on Windows instead of being Linux-only.
#
MINGW_LIBDIRS := /usr/$(MINGW_ARCH)-w64-mingw32/lib \
                 /mingw64/lib /clang64/lib /ucrt64/lib \
                 /c/msys64/mingw64/lib

# The 32-bit Windows counterpart. Debian's mingw-w64 package ships both
# arches, and MSYS2 keeps i686 in its own /mingw32 prefix.
MINGW32_ARCH   := i686
MINGW32_SR     := $(STAGE_DIR)/$(MINGW32_ARCH)-w64-mingw32
MINGW32_LIBDIRS := /usr/$(MINGW32_ARCH)-w64-mingw32/lib \
                   /mingw32/lib /c/msys64/mingw32/lib
MINGW32_GCCDIRS := $(wildcard /usr/lib/gcc/$(MINGW32_ARCH)-w64-mingw32/*win32) \
                   $(wildcard /usr/lib/gcc/$(MINGW32_ARCH)-w64-mingw32/*) \
                   $(wildcard /mingw32/lib/gcc/$(MINGW32_ARCH)-w64-mingw32/*) \
                   $(wildcard /c/msys64/mingw32/lib/gcc/$(MINGW32_ARCH)-w64-mingw32/*)
MINGW_GCCDIRS := $(wildcard /usr/lib/gcc/$(MINGW_ARCH)-w64-mingw32/*win32) \
                 $(wildcard /usr/lib/gcc/$(MINGW_ARCH)-w64-mingw32/*) \
                 $(wildcard /mingw64/lib/gcc/$(MINGW_ARCH)-w64-mingw32/*) \
                 $(wildcard /c/msys64/mingw64/lib/gcc/$(MINGW_ARCH)-w64-mingw32/*)

# Import libs beyond the bare CRT. These are what `link dynamic "ws2_32"`
# and friends resolve against, so a sysroot without them links hello-world
# but not anything using sockets, crypto or COM. Missing ones are skipped,
# not fatal - the set differs between mingw-w64 packagings.
MINGW_EXTRA_LIBS := libws2_32.a libgdi32.a libole32.a liboleaut32.a libuuid.a \
                    libcomdlg32.a libwinspool.a libpsapi.a libiphlpapi.a \
                    libcrypt32.a libbcrypt.a libsecur32.a libwinmm.a \
                    libversion.a libnetapi32.a libuserenv.a libdbghelp.a

stage-sysroots:
	@echo "Staging sysroots into $(STAGE_DIR) ..."
	@mkdir -p "$(MUSL_SR)" "$(MINGW_SR)/lib"
	@if [ -f /usr/lib/$(MUSL_ARCH)-linux-musl/crt1.o ]; then \
	   cp /usr/lib/$(MUSL_ARCH)-linux-musl/crt1.o /usr/lib/$(MUSL_ARCH)-linux-musl/crti.o \
	      /usr/lib/$(MUSL_ARCH)-linux-musl/crtn.o /usr/lib/$(MUSL_ARCH)-linux-musl/libc.a \
	      "$(MUSL_SR)/"; \
	   cp $$(ls /usr/lib/llvm-*/lib/clang/*/lib/linux/libclang_rt.builtins-$(MUSL_ARCH).a 2>/dev/null | head -1) \
	      "$(MUSL_SR)/" 2>/dev/null || echo "  (compiler-rt builtins not bundled; optional)"; \
	   echo "  musl  -> $(MUSL_SR)"; \
	 else \
	   echo "  (musl not found - skipped; 'apt install musl-dev' to cross-compile Linux)"; \
	   rmdir "$(MUSL_SR)" 2>/dev/null || true; \
	 fi
	@SRC=""; for d in $(MINGW_LIBDIRS); do \
	   if [ -f "$$d/crt2.o" ]; then SRC="$$d"; break; fi; \
	 done; \
	 if [ -z "$$SRC" ]; then \
	   echo "  (mingw not found - skipped; 'apt install mingw-w64', or MSYS2 mingw-w64-x86_64-gcc)"; \
	   rm -rf "$(MINGW_SR)"; \
	 else \
	   for x in crt2.o dllcrt2.o libmingw32.a libmingwex.a libmoldname.a libmsvcrt.a \
	            libadvapi32.a libshell32.a libuser32.a libkernel32.a; do \
	     cp "$$SRC/$$x" "$(MINGW_SR)/lib/" || { echo "  ! incomplete mingw sysroot in $$SRC ($$x)"; exit 1; }; \
	   done; \
	   for x in $(MINGW_EXTRA_LIBS); do cp "$$SRC/$$x" "$(MINGW_SR)/lib/" 2>/dev/null || true; done; \
	   G=""; for g in $(MINGW_GCCDIRS); do if [ -f "$$g/crtbegin.o" ]; then G="$$g"; break; fi; done; \
	   test -n "$$G" || { echo "  ! gcc-mingw runtime (crtbegin.o/libgcc.a) not found"; exit 1; }; \
	   cp "$$G/crtbegin.o" "$$G/crtend.o" "$$G/libgcc.a" "$$G/libgcc_eh.a" "$(MINGW_SR)/lib/"; \
	   echo "  mingw -> $(MINGW_SR) (from $$SRC)"; \
	 fi
	@mkdir -p "$(MINGW32_SR)/lib"
	@SRC=""; for d in $(MINGW32_LIBDIRS); do \
	   if [ -f "$$d/crt2.o" ]; then SRC="$$d"; break; fi; \
	 done; \
	 if [ -z "$$SRC" ]; then \
	   echo "  (mingw i686 not found - skipped; 'apt install mingw-w64' ships both, or MSYS2 mingw-w64-i686-gcc)"; \
	   rm -rf "$(MINGW32_SR)"; \
	 else \
	   for x in crt2.o dllcrt2.o libmingw32.a libmingwex.a libmoldname.a libmsvcrt.a \
	            libadvapi32.a libshell32.a libuser32.a libkernel32.a; do \
	     cp "$$SRC/$$x" "$(MINGW32_SR)/lib/" || { echo "  ! incomplete mingw i686 sysroot in $$SRC ($$x)"; exit 1; }; \
	   done; \
	   for x in $(MINGW_EXTRA_LIBS); do cp "$$SRC/$$x" "$(MINGW32_SR)/lib/" 2>/dev/null || true; done; \
	   G=""; for g in $(MINGW32_GCCDIRS); do if [ -f "$$g/crtbegin.o" ]; then G="$$g"; break; fi; done; \
	   test -n "$$G" || { echo "  ! gcc-mingw i686 runtime (crtbegin.o/libgcc.a) not found"; exit 1; }; \
	   cp "$$G/crtbegin.o" "$$G/crtend.o" "$$G/libgcc.a" "$$G/libgcc_eh.a" "$(MINGW32_SR)/lib/"; \
	   echo "  mingw32 -> $(MINGW32_SR) (from $$SRC)"; \
	 fi

# Only pass an embed directory that stage-sysroots actually produced -
# naming a missing one makes the embed rule fail the whole build, which
# would turn "this host cannot cross-compile to Linux" into "this host
# cannot build salam at all". Tested in the recipe, not with $(wildcard),
# since the directories do not exist yet when this Makefile is parsed.
self-contained: stage-sysroots
	@EMB=""; \
	 test -f "$(MUSL_SR)/crt1.o"     && EMB="$$EMB SALAM_EMBED_MUSL_DIR=$(MUSL_SR)"; \
	 test -f "$(MINGW_SR)/lib/crt2.o" && EMB="$$EMB SALAM_EMBED_MINGW_DIR=$(MINGW_SR)"; \
	 test -f "$(MINGW32_SR)/lib/crt2.o" && EMB="$$EMB SALAM_EMBED_MINGW_I686_DIR=$(MINGW32_SR)"; \
	 echo "  embedding:$${EMB:- (none - host-only build)}"; \
	 $(MAKE) WITH_LLVM=1 WITH_LLD=1 LLVM_LINK_STATIC=1 $$EMB \
	         LLVM_CONFIG="$(LLVM_CONFIG)" CXX="$(CXX)" CC="$(CC)"
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
	@# std/llvm says `link static "salam_llvm"`, which the linker resolves
	@# by name off its search path - so the merged archive has to land in
	@# libdir for `import llvm` to work from an installed salam. Optional:
	@# only built by `make libsalam-llvm`, and nothing else needs it.
	@if [ -f "$(SALAM_LLVM_A)" ]; then \
	   echo "Installing libsalam_llvm.a -> $(DESTDIR)$(LIBDIR)"; \
	   mkdir -p $(DESTDIR)$(LIBDIR); \
	   cp -f "$(SALAM_LLVM_A)" $(DESTDIR)$(LIBDIR)/libsalam_llvm.a; \
	 fi
	@echo "Done. 'salam' resolves std/ from $(DATADIR)/std automatically."

uninstall:
	rm -f  $(DESTDIR)$(BINDIR)/$(TARGET)
	rm -f  $(DESTDIR)$(LIBDIR)/libsalam_llvm.a
	rm -rf $(DESTDIR)$(DATADIR)

clean:
	rm -rf build $(OUTBIN) $(OUTBIN).exe $(TARGET) $(TARGET).exe $(SALAM_LLVM_A)
