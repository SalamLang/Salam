#!/bin/sh
# Bootstrap the self-hosted Salam compiler from a seed compiler.
#
#   stage1 = seed   builds compiler/main.salam   (seed is whatever `salam`
#                                                 you already have - e.g. the
#                                                 released binary install.sh
#                                                 fetches)
#   stage2 = stage1 builds compiler/main.salam   (first compiler built BY a
#                                                 self-hosted compiler)
#   stage3 = stage2 builds compiler/main.salam   (fixpoint check)
#
# stage2 and stage3 are both produced by a self-hosted compiler from
# identical sources, so a correct, deterministic compiler makes them
# byte-identical. That is the classic bootstrap fixpoint test and it is the
# real proof the self-hosted compiler is sound - stage1 only proves the seed
# can parse the sources. stage2 is the artifact you ship/test with.
#
# The seed must be 0.2.8 or newer: earlier ones do not collapse '..' in
# import paths, so compiler/'s per-directory layout blows past their module
# limit (see the stage 1 hint below). `make -C c` builds a usable one.
#
# Usage:
#   compiler/tools/bash/bootstrap.sh <seed-salam> [outdir] [--stages N]
#
# Env:
#   SALAM_CC   C compiler the Salam driver shells out to (default: cc/gcc/clang)
#   SALAM_WITH_LLVM
#              Statically link in-process LLVM/LLD into the self-hosted
#              compiler, so it needs no clang/llvm/lld installed to build
#              native code. Values:
#                (unset)  auto - use it when libsalam_llvm.a is found
#                1        require it - fail if the archive is missing
#                0        never
#   SALAM_LLVM_LIB
#              Directory holding libsalam_llvm.a. Default: the repository
#              root, where `make -C c libsalam-llvm` puts it.
#
# Why this is opt-in-by-detection rather than always on: libsalam_llvm.a is
# produced by `make -C c libsalam-llvm WITH_LLVM=1 WITH_LLD=1`, which needs
# LLVM development libraries on the machine. Without the archive the link
# would fail, so a missing archive degrades to a compiler that shells out to
# a C toolchain instead of failing the bootstrap.
#
# Run from the repository root. Exits non-zero if any stage fails to build
# or the resulting binary cannot report its version.

set -eu

ROOT=$(cd "$(dirname "$0")/../../.." && pwd)
STAGES=3

if [ $# -lt 1 ]; then
    echo "usage: $0 <seed-salam> [outdir] [--stages N]" >&2
    exit 2
fi
SEED=$1
shift
OUTDIR=
while [ $# -gt 0 ]; do
    case "$1" in
    --stages)
        STAGES=$2
        shift 2
        ;;
    --stages=*)
        STAGES=${1#--stages=}
        shift
        ;;
    -*)
        echo "unknown option: $1" >&2
        exit 2
        ;;
    *)
        [ -z "$OUTDIR" ] || {
            echo "unexpected argument: $1" >&2
            exit 2
        }
        OUTDIR=$1
        shift
        ;;
    esac
done
[ -n "$OUTDIR" ] || OUTDIR="$ROOT/build/selfhost"

case "$STAGES" in
'' | *[!0-9]*)
    echo "error: --stages needs a number, got '$STAGES'" >&2
    exit 2
    ;;
esac
[ "$STAGES" -ge 1 ] || {
    echo "error: --stages must be >= 1" >&2
    exit 2
}

[ -x "$SEED" ] || {
    echo "error: seed compiler is not executable: $SEED" >&2
    exit 2
}
SEED=$(cd "$(dirname "$SEED")" && pwd)/$(basename "$SEED")

# The self-hosted driver shells out to a C compiler and defaults to tcc,
# which is not present on most CI images and cannot codegen every construct
# in the corpus. Pick a real one unless told otherwise.
if [ -z "${SALAM_CC:-}" ]; then
    for c in cc gcc clang; do
        command -v "$c" >/dev/null 2>&1 && {
            SALAM_CC=$c
            break
        }
    done
fi
[ -n "${SALAM_CC:-}" ] || {
    echo "error: no C compiler found (set SALAM_CC)" >&2
    exit 2
}

mkdir -p "$OUTDIR"
OUTDIR=$(cd "$OUTDIR" && pwd)
export SALAM_STD="$ROOT/std"

case "$(uname -s)" in MINGW* | MSYS* | CYGWIN*) EXE=.exe ;; *) EXE= ;; esac

# In-process LLVM/LLD for the self-hosted compiler. compiler/llvm_bridge.salam
# gates `import llvm` (std/llvm, which carries `link static "salam_llvm"`) on
# the SALAM_HAVE_LLVM condcomp flag; without the -D it compiles the else arm
# whose Available() returns false, and the built compiler has no LLVM at all.
# That is why a plain bootstrap produces a stage2 that always falls back to
# the C backend.
LLVM_LIBDIR=${SALAM_LLVM_LIB:-$ROOT}
LLVM_FLAGS=
LLVM_STATE=off
case "${SALAM_WITH_LLVM:-auto}" in
0 | no | off)
    LLVM_STATE="off (SALAM_WITH_LLVM=0)"
    ;;
*)
    if [ -f "$LLVM_LIBDIR/libsalam_llvm.a" ]; then
        LLVM_FLAGS="-DSALAM_HAVE_LLVM --libpath=$LLVM_LIBDIR"
        LLVM_STATE="static in-process ($LLVM_LIBDIR/libsalam_llvm.a)"
    elif [ "${SALAM_WITH_LLVM:-auto}" = auto ]; then
        LLVM_STATE="off (no $LLVM_LIBDIR/libsalam_llvm.a; build it with 'make -C c libsalam-llvm WITH_LLVM=1 WITH_LLD=1')"
    else
        echo "error: SALAM_WITH_LLVM=${SALAM_WITH_LLVM} but $LLVM_LIBDIR/libsalam_llvm.a is missing." >&2
        echo "       Build it with: make -C c libsalam-llvm WITH_LLVM=1 WITH_LLD=1" >&2
        exit 2
    fi
    ;;
esac

# The one LLVM system library macOS does not ship in its SDK is zstd, and
# Homebrew keeps it in a keg that is not on the linker's default search path,
# so std/llvm's `link dynamic "zstd"` failed the stage 2 link with
# `ld: library 'zstd' not found`. c/Makefile solves the same problem for the C
# build by resolving the full archive path from `brew --prefix zstd`; a .salam
# file cannot shell out, so hand the directory to the driver instead. Only the
# keg's own lib dir is added, never `$(brew --prefix)/lib` wholesale, which
# would put Homebrew's copy of every other library ahead of the SDK's.
#
# Apple's linker searches libzstd.dylib before libzstd.a and has no way to ask
# for the archive short of naming its full path, so the stage2 built here does
# carry a dynamic reference into the keg. That is fine for a bootstrap on the
# machine that produced it; a *released* macOS binary must not, which is why
# c/Makefile hands the C build the .a by path instead.
if [ -n "$LLVM_FLAGS" ] && [ "$(uname -s)" = Darwin ]; then
    for d in "$(brew --prefix zstd 2>/dev/null)/lib" \
        /opt/homebrew/opt/zstd/lib /usr/local/opt/zstd/lib; do
        [ -d "$d" ] || continue
        LLVM_FLAGS="$LLVM_FLAGS --libpath=$d"
        LLVM_STATE="$LLVM_STATE, zstd from $d"
        break
    done
fi

echo "root   : $ROOT"
echo "seed   : $SEED"
"$SEED" version 2>&1 | head -1 || true
echo "cc     : $SALAM_CC"
echo "outdir : $OUTDIR"
echo "stages : $STAGES"
echo "llvm   : $LLVM_STATE"
echo

prev=$SEED
stage=1
while [ "$stage" -le "$STAGES" ]; do
    out="$OUTDIR/salam-stage${stage}${EXE}"
    echo "== stage $stage: $(basename "$prev") builds compiler/main.salam =="
    start=$(date +%s 2>/dev/null || echo 0)
    # LLVM flags go to stage 2 and later only. Stage 1 is built by the SEED,
    # which is whatever `salam` the machine already has - typically an older
    # release that does not know --libpath and would abort on it. Stage 1 is
    # a throwaway parse-check anyway; stage 2 is the shipped artifact, and it
    # is built by stage 1, which is current and does understand the flags.
    stage_llvm=
    if [ "$stage" -ge 2 ]; then
        stage_llvm=$LLVM_FLAGS
    fi
    # shellcheck disable=SC2086 # $stage_llvm is a flag list; splitting is wanted
    (
        cd "$ROOT" &&
            "$prev" build compiler/main.salam \
                --output="$out" --cc="$SALAM_CC" --log-level=error $stage_llvm
    ) || {
        echo "::error::stage $stage build failed" >&2
        if [ "$stage" -eq 1 ]; then
            echo "hint: stage 1 is built by the seed, and seeds older than 0.2.8 do not" >&2
            echo "      collapse '..' in import paths - compiler/'s per-directory layout" >&2
            echo "      then overruns their 64-entry module work list and whole modules" >&2
            echo "      go missing, which surfaces as the C compiler failing on a" >&2
            echo "      'salam_mod_*.h: No such file or directory'. Use a newer seed, or" >&2
            echo "      build one from this checkout with 'make -C c' and pass ./salam." >&2
        fi
        exit 1
    }
    [ -x "$out" ] || {
        echo "::error::stage $stage produced no executable at $out" >&2
        exit 1
    }
    end=$(date +%s 2>/dev/null || echo 0)
    printf 'built %s (%s bytes, %ss)\n' \
        "$(basename "$out")" \
        "$(wc -c <"$out" | tr -d ' ')" \
        "$((end - start))"
    # A binary that builds but cannot run is a failed stage, not a pass.
    "$out" version >/dev/null 2>&1 ||
        {
            echo "::error::stage $stage binary cannot run" >&2
            exit 1
        }
    "$out" version 2>&1 | head -1
    echo
    prev=$out
    stage=$((stage + 1))
done

# Fixpoint: stage N-1 and stage N are both self-host-built from the same
# sources. A mismatch means the compiler is non-deterministic or stage N-1
# miscompiled itself - report it loudly, but do not fail the build on it
# yet: LTO and archive/link ordering are not reproducible on every
# toolchain, so this needs a known-good baseline before it can gate.
if [ "$STAGES" -ge 3 ]; then
    a="$OUTDIR/salam-stage$((STAGES - 1))${EXE}"
    b="$OUTDIR/salam-stage${STAGES}${EXE}"
    echo "== fixpoint: $(basename "$a") vs $(basename "$b") =="
    if cmp -s "$a" "$b"; then
        echo "identical - bootstrap has reached a fixpoint."
    else
        echo "::warning::stage$((STAGES - 1)) and stage$STAGES differ ($(wc -c <"$a" | tr -d ' ') vs $(wc -c <"$b" | tr -d ' ') bytes)."
        echo "Not fatal by itself (LTO/link order need not be reproducible), but"
        echo "if both stages were built by the same compiler from the same sources"
        echo "on the same host, a persistent difference is worth investigating."
    fi
fi

echo
echo "bootstrap ok - ship/test with $OUTDIR/salam-stage2${EXE}"
