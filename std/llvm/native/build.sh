#!/bin/sh
# Usage:
#   std/llvm/native/build.sh [--out DIR] [--llvm-config PROG]
#
# Env:
#   CC, CXX, AR         host tools (default: cc, c++, llvm-ar or ar)
#   LLD_EXTRA_LIBDIR    extra directory to search for liblld*.a
#   LLD_EXTRA_INCLUDEDIR
#                       directory holding lld/Common/Driver.h, when lld lives
#                       in its own prefix. Inferred from LLD_EXTRA_LIBDIR's
#                       sibling include/ when unset.
#   WITH_LLD            1 to demand in-process LLD, 0 to stub it out.
#                       Default: autodetected from whether liblldCommon.a
#                       is findable.

set -eu

HERE=$(cd "$(dirname "$0")" && pwd)
OUT=$(pwd)
LLVM_CONFIG=${LLVM_CONFIG:-llvm-config}

while [ $# -gt 0 ]; do
    case $1 in
    --out)
        OUT=$2
        shift 2
        ;;
    --llvm-config)
        LLVM_CONFIG=$2
        shift 2
        ;;
    -h | --help)
        sed -n '2,$ {
            /^#/ !q
            p
        }' "$0"
        exit 0
        ;;
    *)
        echo "unknown argument: $1" >&2
        exit 2
        ;;
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
if [ -z "${LLD_EXTRA_LIBDIR:-}" ] && [ -n "${LLD_PREFIX:-}" ]; then
    LLD_EXTRA_LIBDIR="$LLD_PREFIX/lib"
fi
SEARCH="$LIBDIR ${LLD_EXTRA_LIBDIR:-}"

find_lib() {
    for d in $SEARCH; do
        [ -f "$d/$1" ] && {
            printf '%s\n' "$d/$1"
            return 0
        }
    done
    return 1
}
if [ -z "${WITH_LLD:-}" ]; then
    if find_lib liblldCommon.a >/dev/null 2>&1; then WITH_LLD=1; else WITH_LLD=0; fi
fi

lld_includedir() {
    if [ -n "${LLD_EXTRA_INCLUDEDIR:-}" ]; then
        printf '%s\n' "$LLD_EXTRA_INCLUDEDIR"
        return 0
    fi
    [ -n "${LLD_EXTRA_LIBDIR:-}" ] || return 1
    d=$(cd "$LLD_EXTRA_LIBDIR/.." 2>/dev/null && pwd) || return 1
    [ -f "$d/include/lld/Common/Driver.h" ] || return 1
    printf '%s\n' "$d/include"
}

WORK=$(mktemp -d)

to_ar_path() {
    if command -v cygpath >/dev/null 2>&1; then
        cygpath -m "$1"
    else
        printf '%s' "$1"
    fi
}
trap 'rm -rf "$WORK"' EXIT INT TERM

echo "llvm-config : $LLVM_CONFIG ($($LLVM_CONFIG --version))"
echo "ar          : ${AR:-none}"
echo "in-process LLD : $([ "$WITH_LLD" = 1 ] && echo yes || echo 'no (stubbed)')"

LLVM_INC=$($LLVM_CONFIG --includedir 2>/dev/null)
CPPDEFS="-DSALAM_HAVE_LLVM -I$LLVM_INC"
[ "$WITH_LLD" = 1 ] && CPPDEFS="$CPPDEFS -DSALAM_HAVE_LLD"

SHIMS=""
# shellcheck disable=SC2086
$CC -O2 -I"$HERE" $CPPDEFS -c "$HERE/orc_call.c" -o "$WORK/orc_call.o"
SHIMS="$SHIMS $WORK/orc_call.o"

if [ "$WITH_LLD" = 1 ]; then
    LLD_INC=
    if inc=$(lld_includedir); then
        LLD_INC="-I$inc"
        echo "lld headers : $inc"
    fi
    # shellcheck disable=SC2046,SC2086
    $CXX -O2 -I"$HERE" $CPPDEFS $LLD_INC $($LLVM_CONFIG --cxxflags) -c "$HERE/lld_link.cc" -o "$WORK/lld_link.o"
    SHIMS="$SHIMS $WORK/lld_link.o"
    LLD_LIBS="lldMinGW lldELF lldCOFF lldMachO lldWasm lldCommon"
else
    # shellcheck disable=SC2086
    $CC -O2 -I"$HERE" $CPPDEFS -c "$HERE/lld_stub.c" -o "$WORK/lld_stub.o"
    SHIMS="$SHIMS $WORK/lld_stub.o"
    LLD_LIBS=""
fi

case "$(uname -s 2>/dev/null || echo unknown)" in
MINGW* | MSYS* | CYGWIN*)
    $CC -c "$HERE/win_lld_demangle_shim.S" -o "$WORK/win_shim.o"
    SHIMS="$SHIMS $WORK/win_shim.o"
    ;;
esac

MRI=$WORK/mri
mkdir -p "$OUT"
: >"$MRI"
echo "create $(to_ar_path "$OUT/libsalam_llvm.a")" >>"$MRI"
for o in $SHIMS; do echo "addmod $(to_ar_path "$o")" >>"$MRI"; done

if ! LLVM_LIBS=$($LLVM_CONFIG --link-static --libs all 2>"$WORK/libs.err"); then
    echo "error: $LLVM_CONFIG --link-static --libs all failed:" >&2
    sed 's/^/       /' "$WORK/libs.err" >&2
    exit 1
fi
if [ -z "$LLVM_LIBS" ]; then
    echo "error: $LLVM_CONFIG --link-static --libs all listed no components" >&2
    echo "       This LLVM has no static libraries to merge. Install the static" >&2
    echo "       LLVM development files, or point --llvm-config at a build that" >&2
    echo "       has them." >&2
    exit 1
fi

n=0
missing=
missing_llvm=
for l in $LLVM_LIBS $(for x in $LLD_LIBS; do echo "-l$x"; done); do
    case "$l" in -l*) nm="lib${l#-l}.a" ;; *) continue ;; esac
    if a=$(find_lib "$nm"); then
        echo "addlib $(to_ar_path "$a")" >>"$MRI"
        n=$((n + 1))
    else
        case "$nm" in
        liblld*) missing="$missing $nm" ;;
        libPolly* | libLLVMPolly*) ;;
        *) missing_llvm="$missing_llvm $nm" ;;
        esac
    fi
done
if [ -n "$missing" ]; then
    echo "error: lld archives not found in [$SEARCH]:$missing" >&2
    echo "       lld is a separate package on some hosts (Homebrew's lld formula," >&2
    echo "       Debian's liblld-<ver>-dev). Install it, point LLD_EXTRA_LIBDIR at" >&2
    echo "       the directory holding liblldCommon.a, or pass WITH_LLD=0." >&2
    exit 1
fi
if [ -n "$missing_llvm" ]; then
    # shellcheck disable=SC2086
    n_missing=$(printf '%s\n' $missing_llvm | wc -l | tr -d ' ')
    echo "error: $n_missing LLVM archives are not in [$SEARCH], starting with:" >&2
    # shellcheck disable=SC2086
    printf '%s\n' $missing_llvm | head -5 | sed 's/^/         /' >&2
    echo "       $LLVM_CONFIG names these components but the static libraries are" >&2
    echo "       not installed. Merging what is left would produce an archive that" >&2
    echo "       only fails later, at the std/llvm link. Install the static LLVM" >&2
    echo "       development files for this version." >&2
    exit 1
fi
if [ "$n" = 0 ]; then
    echo "error: no LLVM archives were merged; refusing to write a shim-only" >&2
    echo "       $OUT/libsalam_llvm.a" >&2
    exit 1
fi
printf 'save\nend\n' >>"$MRI"

echo "merging $n LLVM/lld archives + salam shims -> $OUT/libsalam_llvm.a"
rm -f "$OUT/libsalam_llvm.a"
"$AR" -M <"$MRI"

echo "Built $OUT/libsalam_llvm.a"
echo "  std/llvm then links with: link static \"salam_llvm\""
echo "  plus system libs: $($LLVM_CONFIG --system-libs 2>/dev/null) -lstdc++"
