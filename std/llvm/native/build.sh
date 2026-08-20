#!/bin/sh
# Builds libsalam_llvm.a: the six native shim objects merged with every static
# LLVM (and, when available, LLD) archive on the host, so `link static
# "salam_llvm"` in std/llvm/llvm.salam resolves without llvm-*-dev installed
# on the consuming machine.
#
# This used to be c/Makefile's `libsalam-llvm` target. It lives here now, next
# to the sources it compiles, because the self-hosted compiler needs the same
# archive and the C compiler that used to build it is gone.
#
# Usage:
#   std/llvm/native/build.sh [--out DIR] [--llvm-config PROG]
#
# Env:
#   CC, CXX, AR         host tools (default: cc, c++, llvm-ar or ar)
#   LLD_EXTRA_LIBDIR    extra directory to search for liblld*.a
#   WITH_LLD            1 to demand in-process LLD, 0 to stub it out.
#                       Default: autodetected from whether liblldCommon.a
#                       is findable.

set -eu

HERE=$(cd "$(dirname "$0")" && pwd)
OUT=$(pwd)
LLVM_CONFIG=${LLVM_CONFIG:-llvm-config}

while [ $# -gt 0 ]; do
    case $1 in
    --out) OUT=$2; shift 2 ;;
    --llvm-config) LLVM_CONFIG=$2; shift 2 ;;
    -h | --help) sed -n '2,24p' "$0"; exit 0 ;;
    *) echo "unknown argument: $1" >&2; exit 2 ;;
    esac
done

command -v "$LLVM_CONFIG" >/dev/null 2>&1 || {
    echo "error: $LLVM_CONFIG not found; install LLVM development files or pass" >&2
    echo "       --llvm-config /path/to/llvm-config-NN" >&2
    exit 1
}

: "${CC:=cc}"
: "${CXX:=c++}"
LIBDIR=$($LLVM_CONFIG --libdir 2>/dev/null || true)
BINDIR=$($LLVM_CONFIG --bindir 2>/dev/null || true)
: "${AR:=$(command -v "$BINDIR/llvm-ar" 2>/dev/null || command -v llvm-ar 2>/dev/null || command -v ar)}"
SEARCH="$LIBDIR ${LLD_EXTRA_LIBDIR:-}"

# LLD is a separate package on most hosts, and its archives are the only
# thing that can be absorbed here (a .dylib/.so cannot). Without them the
# stub keeps std/llvm resolving and the compiler links through the host cc.
find_lib() {
    for d in $SEARCH; do
        [ -f "$d/$1" ] && { printf '%s\n' "$d/$1"; return 0; }
    done
    return 1
}
if [ -z "${WITH_LLD:-}" ]; then
    if find_lib liblldCommon.a >/dev/null 2>&1; then WITH_LLD=1; else WITH_LLD=0; fi
fi

WORK=$(mktemp -d)
trap 'rm -rf "$WORK"' EXIT INT TERM

echo "llvm-config : $LLVM_CONFIG ($($LLVM_CONFIG --version))"
echo "in-process LLD : $([ "$WITH_LLD" = 1 ] && echo yes || echo 'no (stubbed)')"

# -DSALAM_HAVE_LLVM is what makes orc_call.c define the five
# salam_llvm_init_all_* wrappers at all; without it the file compiles to
# just salam_orc_call_main and std/llvm fails to link on the five macros
# LLVM only publishes as macros. The include path is for llvm-c/*.h.
LLVM_INC=$($LLVM_CONFIG --includedir 2>/dev/null)
CPPDEFS="-DSALAM_HAVE_LLVM -I$LLVM_INC"
[ "$WITH_LLD" = 1 ] && CPPDEFS="$CPPDEFS -DSALAM_HAVE_LLD"

SHIMS=""
# shellcheck disable=SC2086
$CC -O2 -I"$HERE" $CPPDEFS -c "$HERE/orc_call.c" -o "$WORK/orc_call.o"
SHIMS="$SHIMS $WORK/orc_call.o"

if [ "$WITH_LLD" = 1 ]; then
    # shellcheck disable=SC2046
    $CXX -O2 -I"$HERE" $CPPDEFS $($LLVM_CONFIG --cxxflags) -c "$HERE/lld_link.cc" -o "$WORK/lld_link.o"
    SHIMS="$SHIMS $WORK/lld_link.o"
    LLD_LIBS="lldMinGW lldELF lldCOFF lldMachO lldWasm lldCommon"
else
    # shellcheck disable=SC2086
    $CC -O2 -I"$HERE" $CPPDEFS -c "$HERE/lld_stub.c" -o "$WORK/lld_stub.o"
    SHIMS="$SHIMS $WORK/lld_stub.o"
    LLD_LIBS=""
fi

# MSYS2 only; see the file's own comment for the __imp_ story.
case "$(uname -s 2>/dev/null || echo unknown)" in
MINGW* | MSYS* | CYGWIN*)
    $CC -c "$HERE/win_lld_demangle_shim.S" -o "$WORK/win_shim.o"
    SHIMS="$SHIMS $WORK/win_shim.o"
    ;;
esac

MRI=$WORK/mri
mkdir -p "$OUT"
: > "$MRI"
echo "create $OUT/libsalam_llvm.a" >> "$MRI"
for o in $SHIMS; do echo "addmod $o" >> "$MRI"; done

n=0
missing=
for l in $($LLVM_CONFIG --link-static --libs all 2>/dev/null) $(for x in $LLD_LIBS; do echo "-l$x"; done); do
    case "$l" in -l*) nm="lib${l#-l}.a" ;; *) continue ;; esac
    if a=$(find_lib "$nm"); then
        echo "addlib $a" >> "$MRI"
        n=$((n + 1))
    else
        case "$nm" in liblld*) missing="$missing $nm" ;; esac
    fi
done
if [ -n "$missing" ]; then
    echo "error: lld archives not found in [$SEARCH]:$missing" >&2
    echo "       lld is a separate package on some hosts (Homebrew's lld formula," >&2
    echo "       Debian's liblld-<ver>-dev). Install it, point LLD_EXTRA_LIBDIR at" >&2
    echo "       the directory holding liblldCommon.a, or pass WITH_LLD=0." >&2
    exit 1
fi
printf 'save\nend\n' >> "$MRI"

echo "merging $n LLVM/lld archives + salam shims -> $OUT/libsalam_llvm.a"
rm -f "$OUT/libsalam_llvm.a"
"$AR" -M < "$MRI"

echo "Built $OUT/libsalam_llvm.a"
echo "  std/llvm then links with: link static \"salam_llvm\""
echo "  plus system libs: $($LLVM_CONFIG --system-libs 2>/dev/null) -lstdc++"
