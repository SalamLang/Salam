#!/bin/sh
# Builds tcc from the TinyCC commit pinned in tinycc-ref.txt and installs it
# into <prefix>, laid out exactly as the Savannah win*-bin zips were: the
# binary at <prefix>/tcc[.exe] with include/ and lib/ beside it, which is
# both what salam_find_bundled_tool() expects of a bundled toolchain and
# where tcc itself looks for libtcc1.a and its headers.
#
# Usage:
#   tools/ci/build-tcc.sh <prefix>
#
# Env:
#   TINYCC_REF     commit to build (default: contents of tinycc-ref.txt)
#   CROSS_PREFIX   e.g. i686-w64-mingw32- to cross-build; also implies the
#                  caller has set TCC_CONFIGURE_EXTRA appropriately
#   TCC_CONFIGURE_EXTRA
#                  extra ./configure flags (--targetos, --cpu, ...)
#   HOSTCC         compiler that produces binaries the *build* machine can
#                  run; only consulted when cross-building (default: cc)
#   TCC_CROSS_TARGET
#                  the name tcc's own Makefile gives the target being built
#                  (i386-win32, x86_64-win32, arm64-win32, ...); only used
#                  when cross-building, and derived from the --cpu/--targetos
#                  in TCC_CONFIGURE_EXTRA when unset
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
JOBS=$(nproc 2>/dev/null || echo 4)

# The name tcc's Makefile gives a target: "<cpu>" for unix, "<cpu>-win32"
# for Windows, "<cpu>-osx" for macOS. It is what `make cross-<target>` and
# the <target>-libtcc1.a that target produces are called, and it is derived
# from the same --cpu/--targetos the caller already passes to configure.
tcc_cross_target() {
    cpu=""
    os=""
    for arg in $TCC_CONFIGURE_EXTRA; do
        case $arg in
        --cpu=*) cpu=${arg#--cpu=} ;;
        --targetos=*) os=${arg#--targetos=} ;;
        esac
    done
    [ -n "$cpu" ] || return 1
    case $os in
    WIN32 | Windows*) echo "$cpu-win32" ;;
    Darwin) echo "$cpu-osx" ;;
    *) echo "$cpu" ;;
    esac
}

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
cross_arg=""
CROSS_TARGET=""
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
    # builds is compiled by invoking ./tcc.exe (lib/Makefile defaults XCC and
    # XAR to it), which this host cannot run either. Pointing XCC at the
    # cross compiler instead - upstream's "<target>-libtcc1-usegcc" switch -
    # is a dead end: lib/*.c is written for tcc, and llvm-mingw's clang
    # rejects it outright (the i386 asm in libtcc1.c casts its output
    # operands, -Winvalid-gnu-asm-cast, and stdatomic.c redeclares clang's
    # own __atomic_is_lock_free builtin with an `unsigned long` size where
    # clang's is size_t - only equal on LP64, and this target is ILP32).
    #
    # So the runtime is built the way upstream builds its own cross
    # distributions: a second, host-native tcc tree whose `make cross-<T>`
    # produces a <T>-tcc that runs here and a <T>-libtcc1.a compiled by it,
    # which is dropped into the install below. tcc compiles its own runtime,
    # exactly as in the native x86_64 Windows bundle - the C compiler this
    # script was handed never touches lib/ at all.
    : "${TCC_CROSS_TARGET:=$(tcc_cross_target || true)}"
    CROSS_TARGET=$TCC_CROSS_TARGET
    [ -n "$CROSS_TARGET" ] || {
        echo "error: cross-building needs TCC_CROSS_TARGET, and none could" >&2
        echo "  be derived from TCC_CONFIGURE_EXTRA='$TCC_CONFIGURE_EXTRA'" >&2
        exit 2
    }
fi
# shellcheck disable=SC2086
../configure --prefix="$PREFIX" $cross_arg $TCC_CONFIGURE_EXTRA

# Whatever configure decided the target's executables and install layout
# look like, rather than a second guess at it from here.
EXESUF=$(sed -n 's/^EXESUF=//p' config.mak | head -1)
TCCDIR=$(sed -n 's/^tccdir=\$(DESTDIR)//p' config.mak | head -1)

if [ -n "$CROSS_TARGET" ]; then
    # Only the compiler: `make` with no target would also try to build
    # libtcc1.a here, with the cross compiler, which is what 3/3 above is
    # avoiding. libtcc.a comes along as a dependency of tcc itself.
    "$MAKE" -j"$JOBS" "tcc$EXESUF"

    # The host-native tree that builds the runtime. Its own config is
    # deliberately plain - no --cross-prefix, no --targetos - because it
    # exists to produce a <target>-tcc that runs *here*; the target is named
    # on the make command line instead.
    mkdir -p "$WORK/tinycc/build-host"
    (
        cd "$WORK/tinycc/build-host"
        ../configure --cc="$HOSTCC" --prefix="$WORK/host-prefix"
        "$MAKE" -j"$JOBS" "cross-$CROSS_TARGET"
    )
    RUNTIME="$WORK/tinycc/build-host/$CROSS_TARGET-libtcc1.a"
    [ -f "$RUNTIME" ] || {
        echo "error: 'make cross-$CROSS_TARGET' produced no $CROSS_TARGET-libtcc1.a" >&2
        ls -la "$WORK/tinycc/build-host" >&2 || true
        exit 1
    }
else
    "$MAKE" -j"$JOBS"
fi

"$MAKE" install

if [ -n "$CROSS_TARGET" ]; then
    # install skipped libtcc1.a (this tree never built one), so put the
    # tcc-built runtime where tcc looks for it: $(tccdir)/lib on Windows,
    # $(tccdir) itself elsewhere - the two layouts install-win/install-unx
    # use for it.
    dest=$TCCDIR
    if grep -q '^CONFIG_WIN32=yes' config.mak; then
        dest=$TCCDIR/lib
    fi
    mkdir -p "$dest"
    cp "$RUNTIME" "$dest/libtcc1.a"
fi

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
# is native. A cross-built tcc.exe can't run here, but the runtime beside it
# can still be exercised: the host-native <target>-tcc from the tree that
# produced libtcc1.a generates the same code the installed tcc.exe does, so
# linking a program with it against the *installed* prefix proves the
# archive is complete and for the right CPU rather than leaving that to
# whoever unzips the bundle. It looks for its runtime under the crossprefix
# ($(CONFIG_TCC_CROSSPREFIX)libtcc1.a), so the copy under that name goes in
# for the test and comes straight back out.
printf 'int main(void){return 0;}\n' >"$WORK/smoke.c"
if [ -z "$CROSS_PREFIX" ]; then
    "$TCC_BIN" -v
    "$TCC_BIN" "$WORK/smoke.c" -o "$WORK/smoke" && "$WORK/smoke"
    echo "smoke test: compiled and ran"
else
    cp "$RUNTIME" "$dest/$CROSS_TARGET-libtcc1.a"
    "$WORK/tinycc/build-host/$CROSS_TARGET-tcc" -B"$TCCDIR" \
        "$WORK/smoke.c" -o "$WORK/smoke$EXESUF"
    rm -f "$dest/$CROSS_TARGET-libtcc1.a"
    [ -s "$WORK/smoke$EXESUF" ] || {
        echo "error: linking against $LIBTCC1 produced nothing" >&2
        exit 1
    }
    echo "smoke test: linked a $CROSS_TARGET program against the installed runtime"
fi
