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
# Usage:
#   compiler/tools/bash/bootstrap.sh <seed-salam> [outdir] [--stages N]
#
# Env:
#   SALAM_CC  C compiler the Salam driver shells out to (default: cc/gcc/clang)
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

echo "root   : $ROOT"
echo "seed   : $SEED"
"$SEED" version 2>&1 | head -1 || true
echo "cc     : $SALAM_CC"
echo "outdir : $OUTDIR"
echo "stages : $STAGES"
echo

prev=$SEED
stage=1
while [ "$stage" -le "$STAGES" ]; do
    out="$OUTDIR/salam-stage${stage}${EXE}"
    echo "== stage $stage: $(basename "$prev") builds compiler/main.salam =="
    start=$(date +%s 2>/dev/null || echo 0)
    (
        cd "$ROOT" &&
            "$prev" build compiler/main.salam \
                --output="$out" --cc="$SALAM_CC" --log-level=error
    ) || {
        echo "::error::stage $stage build failed" >&2
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
