#!/bin/sh
# Builds tcc from the TinyCC commit pinned in tinycc-ref.txt and installs it
# into <prefix>, laid out exactly as the Savannah win*-bin zips were: the
# binary at <prefix>/tcc[.exe] with include/ and lib/ beside it, which is
# both what salam_find_bundled_tool() expects of a bundled toolchain and
# where tcc itself looks for libtcc1.a and its headers.
#
# Usage:
#   c/tools/ci/build-tcc.sh <prefix>
#
# Env:
#   TINYCC_REF     commit to build (default: contents of tinycc-ref.txt)
#   CROSS_PREFIX   e.g. i686-w64-mingw32- to cross-build; also implies the
#                  caller has set TCC_CONFIGURE_EXTRA appropriately
#   TCC_CONFIGURE_EXTRA
#                  extra ./configure flags (--targetos, --cpu, ...)
#   HOSTCC         compiler that produces binaries the *build* machine can
#                  run; only consulted when cross-building (default: cc)
#   MAKE           make to use (default: make)
#
# Run from anywhere. Exits non-zero if the build or the smoke test fails.

set -eu

if [ $# -lt 1 ]; then
    echo "usage: $0 <prefix>" >&2
    exit 2
fi

HERE=$(cd "$(dirname "$0")" && pwd)
PREFIX=$1
: "${TINYCC_REF:=$(grep -vE '^[[:space:]]*(#|$)' "$HERE/tinycc-ref.txt" | head -1)}"
: "${TCC_CONFIGURE_EXTRA:=}"
: "${CROSS_PREFIX:=}"
: "${HOSTCC:=cc}"
: "${MAKE:=make}"

[ -n "$TINYCC_REF" ] || {
    echo "error: no TINYCC_REF and none readable from $HERE/tinycc-ref.txt" >&2
    exit 2
}

WORK=$(mktemp -d 2>/dev/null || mktemp -d -t salam-tcc)
trap 'rm -rf "$WORK"' EXIT INT TERM

echo "tinycc ref : $TINYCC_REF"
echo "prefix     : $PREFIX"

# A blobless clone of the full history, because the pin is a plain commit on
# mob rather than a tag or a branch head: --depth 1 can only fetch the tip,
# and mob moves.
git clone --quiet --filter=blob:none https://github.com/TinyCC/tinycc.git "$WORK/tinycc"
git -C "$WORK/tinycc" checkout --quiet "$TINYCC_REF"

# Out-of-source: tcc's own build writes generated headers next to the
# sources, and reusing a tree across two targets picks up the first one's
# config.h.
mkdir -p "$WORK/tinycc/build"
cd "$WORK/tinycc/build"

# tcc's build system assumes throughout that the compiler it produces runs
# on the machine producing it. That holds for every native build and for the
# musl crosses (same OS, and the host can exec the result), but not for
# Linux -> Windows, where three separate steps hand a PE to the build host.
# The extra make arguments live in "$@" because one of them has to contain
# spaces; PREFIX was read out of $1 above, so the positional list is free.
set --
cross_arg=""
if [ -n "$CROSS_PREFIX" ]; then
    # 1/3, configure: the win32 build makes libtcc a DLL and then derives
    # libtcc.def from it with `tcc -impdef`, i.e. by running the tcc.exe it
    # just linked. --enable-static drops the DLL and the .def with it, and
    # is what a bundled toolchain wants regardless: one self-contained
    # tcc.exe instead of an .exe plus a DLL that has to travel beside it.
    cross_arg="--cross-prefix=$CROSS_PREFIX --enable-static"
    # 2/3, c2str: it turns include/tccdefs.h into a C string literal at build
    # time, and the Makefile compiles it with $(CC) - which under
    # --cross-prefix is the cross compiler, so the build then tries to *run*
    # a target binary on the host. Cross-building for Windows from Linux the
    # shell gets handed a PE image and reports
    # `./c2str.exe: 2: Syntax error: "(" unexpected`. Building it with the
    # host compiler first leaves make nothing to do (the same trick
    # build-musl-tcc.sh has always used).
    "$HOSTCC" -DC2STR ../conftest.c -o c2str.exe
    # 3/3, libtcc1.a: the compiler runtime tcc links into everything it
    # builds is itself compiled by invoking ./tcc.exe (lib/Makefile defaults
    # XCC and XAR to it). This is what upstream's "<target>-libtcc1-usegcc"
    # switch exists for; setting the four variables it sets saves having to
    # recompute the $(NATIVE_TARGET) the switch is named after, and spelling
    # the tools as $(CC)/$(AR) keeps them whatever --cross-prefix resolved to
    # rather than a second guess at it. BFLAGS empties out because both the
    # value it has by default (-bt) and the one the switch gives it (-gstabs)
    # are rejected by the compilers this runs on.
    #
    # XFLAGS has to be replaced along with them, not just inherited: its
    # default passes -B$(TOPSRC)/win32, and a -B prefix is an *include* path
    # to gcc, so tcc's own cut-down windows headers land ahead of the
    # toolchain's and collide with them ("static declaration of 'vsnprintf'
    # follows non-static declaration"). -std=gnu11 is ours rather than
    # upstream's: crt1.c declares `extern int __run_on_exit();` and calls it
    # with an argument, which a C23 default (gcc >= 15) rejects.
    set -- 'XCC=$(CC)' 'XAR=$(AR)' 'BFLAGS=' \
        'XFLAGS=$(CFLAGS) -std=gnu11 -fPIC -fno-omit-frame-pointer -Wno-unused-function -Wno-unused-variable -I$(TOP)'
fi
# shellcheck disable=SC2086
../configure --prefix="$PREFIX" $cross_arg $TCC_CONFIGURE_EXTRA

"$MAKE" -j"$(nproc 2>/dev/null || echo 4)" "$@"
"$MAKE" install "$@"

# Two install layouts, both of them tcc's own: the Windows build puts
# tcc.exe straight at the prefix with include/ and lib/ beside it (matching
# the Savannah zips), the unix build uses the usual prefix/bin.
TCC_BIN=""
for cand in "$PREFIX/tcc.exe" "$PREFIX/tcc" "$PREFIX/bin/tcc"; do
    [ -x "$cand" ] && {
        TCC_BIN=$cand
        break
    }
done
[ -n "$TCC_BIN" ] || {
    echo "error: no tcc binary under $PREFIX after install" >&2
    ls -R "$PREFIX" >&2 || true
    exit 1
}

echo "tcc installed: $TCC_BIN"

# A tcc without libtcc1.a links nothing: it holds the runtime helpers (64-bit
# division, alloca, chkstk, the crt stubs) that every program it compiles
# resolves against. Worth asserting rather than assuming, because the ways a
# cross build loses it - a lib/ sub-make that quietly did nothing - leave a
# perfectly good-looking tcc.exe behind.
LIBTCC1=$(find "$PREFIX" -name libtcc1.a -print 2>/dev/null | head -1)
[ -n "$LIBTCC1" ] || {
    echo "error: no libtcc1.a under $PREFIX after install" >&2
    ls -R "$PREFIX" >&2 || true
    exit 1
}
echo "runtime installed: $LIBTCC1"

# `tcc -v` runs the built binary, so it only proves anything when the build
# is native; a cross-built one is checked by the job that consumes it.
if [ -z "$CROSS_PREFIX" ]; then
    "$TCC_BIN" -v
    printf 'int main(void){return 0;}\n' >"$WORK/smoke.c"
    "$TCC_BIN" "$WORK/smoke.c" -o "$WORK/smoke" && "$WORK/smoke"
    echo "smoke test: compiled and ran"
fi
