#!/bin/sh

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
        LLVM_STATE="off (no $LLVM_LIBDIR/libsalam_llvm.a; build it with 'sh std/llvm/native/build.sh')"
    else
        echo "error: SALAM_WITH_LLVM=${SALAM_WITH_LLVM} but $LLVM_LIBDIR/libsalam_llvm.a is missing." >&2
        echo "       Build it with: sh std/llvm/native/build.sh" >&2
        exit 2
    fi
    ;;
esac

if [ -n "$LLVM_FLAGS" ] && [ "$(uname -s)" = Darwin ]; then
    for d in "$(brew --prefix zstd 2>/dev/null)/lib" \
        /opt/homebrew/opt/zstd/lib /usr/local/opt/zstd/lib; do
        [ -d "$d" ] || continue
        LLVM_FLAGS="$LLVM_FLAGS --libpath=$d"
        LLVM_STATE="$LLVM_STATE, zstd from $d"
        break
    done
fi

# a caller can point this at a directory holding its own static libs (e.g. a
# libxml2.a built without LZMA/ICONV, to dodge a system libxml2 that needs
# them) so every stage's link sees it, regardless of the LLVM state above.
if [ -n "${SALAM_EXTRA_LIBPATH:-}" ]; then
    for d in $SALAM_EXTRA_LIBPATH; do
        [ -d "$d" ] || continue
        LLVM_FLAGS="${LLVM_FLAGS:+$LLVM_FLAGS }--libpath=$d"
    done
fi

accepts_llvm_flags() {
    "$1" help 2>&1 | grep -q -- '--libpath'
}

accepts_const_defines() {
    "$1" help 2>&1 | grep -q -- '-dNAME=VALUE'
}

BOOTSTRAP_BACKEND=${SALAM_BOOTSTRAP_BACKEND:-auto}
case "$BOOTSTRAP_BACKEND" in
auto | c) ;;
*)
    echo "error: SALAM_BOOTSTRAP_BACKEND must be 'auto' or 'c', got '$BOOTSTRAP_BACKEND'" >&2
    exit 2
    ;;
esac

has_inprocess_llvm() {
    _probe_dir="$OUTDIR/.llvm-probe"
    rm -rf "$_probe_dir"
    mkdir -p "$_probe_dir"
    printf 'func main:\n    ret 0\nend\n' >"$_probe_dir/probe.salam"
    (cd "$_probe_dir" && "$1" build probe.salam --output=probe_exe \
        --log-level=info) >"$_probe_dir/log" 2>&1
    _rc=1
    if grep -q "LLVM IR" "$_probe_dir/log" 2>/dev/null; then
        _rc=0
    fi
    rm -rf "$_probe_dir"
    return $_rc
}

STAMP_VERSION="$(cat "$ROOT/VERSION" 2>/dev/null || echo 0.0.0-dev)"
STAMP_COMMIT="$(git -C "$ROOT" rev-parse HEAD 2>/dev/null || echo unknown)"
STAMP_DATE="$(git -C "$ROOT" show -s --format=%cI HEAD 2>/dev/null || echo unknown)"
STAMP_DIRTY=
if [ -n "$(git -C "$ROOT" status --porcelain 2>/dev/null)" ]; then
    STAMP_DIRTY=-dirty
fi
BUILD_INFO_FLAGS="-dSALAM_VERSION=\"$STAMP_VERSION\" -dSALAM_GIT_COMMIT=\"$STAMP_COMMIT\""
BUILD_INFO_FLAGS="$BUILD_INFO_FLAGS -dSALAM_GIT_DATE=\"$STAMP_DATE\""
BUILD_INFO_FLAGS="$BUILD_INFO_FLAGS -dSALAM_GIT_DIRTY=\"$STAMP_DIRTY\""

SEED_LLVM_FLAGS=
if [ -n "$LLVM_FLAGS" ]; then
    if accepts_llvm_flags "$SEED"; then
        SEED_LLVM_FLAGS=$LLVM_FLAGS
    else
        echo "::warning::seed $("$SEED" version 2>&1 | head -1) predates --libpath;" \
            "stage 1 gets no in-process LLVM (stage 2 and 3 still do)" >&2
    fi
fi

echo "root   : $ROOT"
echo "seed   : $SEED"
"$SEED" version 2>&1 | head -1 || true
if [ "$BOOTSTRAP_BACKEND" = c ]; then
    echo "cc     : $SALAM_CC (forced by SALAM_BOOTSTRAP_BACKEND=c)"
else
    echo "cc     : $SALAM_CC (fallback; unused by a stage whose builder has LLVM)"
fi
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
    stage_llvm=$LLVM_FLAGS
    if [ "$stage" -eq 1 ]; then
        stage_llvm=$SEED_LLVM_FLAGS
    fi
    stage_cc="--cc=$SALAM_CC"
    stage_backend=C
    if [ "$BOOTSTRAP_BACKEND" != c ] && has_inprocess_llvm "$prev"; then
        stage_cc=
        stage_backend=LLVM
    fi
    echo "   backend: $stage_backend (via $(basename "$prev"))"
    stage_info=$BUILD_INFO_FLAGS
    if ! accepts_const_defines "$prev"; then
        stage_info=
        echo "::warning::$(basename "$prev") predates -dNAME=VALUE;" \
            "stage $stage reports its builder's version, not $STAMP_VERSION" >&2
    fi
    # shellcheck disable=SC2086 # flag lists; splitting is wanted
    (
        cd "$ROOT" &&
            "$prev" build compiler/main.salam \
                --output="$out" $stage_cc --log-level=error $stage_llvm $stage_info
    ) || {
        echo "::error::stage $stage build failed" >&2
        if [ "$stage_backend" = LLVM ]; then
            echo "hint: this stage was compiled by $(basename "$prev")'s in-process LLVM" >&2
            echo "      backend. To put the whole bootstrap back on the C backend without" >&2
            echo "      giving up the LLVM the stages link, re-run with" >&2
            echo "      SALAM_BOOTSTRAP_BACKEND=c." >&2
        fi
        if [ "$stage" -eq 1 ]; then
            echo "hint: stage 1 is built by the seed, and seeds older than 0.2.8 do not" >&2
            echo "      collapse '..' in import paths - compiler/'s per-directory layout" >&2
            echo "      then overruns their 64-entry module work list and whole modules" >&2
            echo "      go missing, which surfaces as the C compiler failing on a" >&2
            echo "      'salam_mod_*.h: No such file or directory'." >&2
            echo "hint: 0.2.8 seeds get that far but emit no prototype for the" >&2
            echo "      other-package functions a generic instantiation calls, so" >&2
            echo "      std/'s Vector<T> lands in another module's header calling a" >&2
            echo "      mem function nothing declared yet - an implicit declaration of" >&2
            echo "      _Salam_mem_AllocateZeroed_u64 that gcc 16 and clang reject." >&2
            echo "hint: use a 0.2.9 or newer seed, or build one from this checkout" >&2
            echo "      with 'sh tools/bash/build-selfhost.sh' and pass ./salam." >&2
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
    "$out" version >/dev/null 2>&1 ||
        {
            echo "::error::stage $stage binary cannot run" >&2
            exit 1
        }
    "$out" version 2>&1 | head -1
    if [ -n "$stage_llvm" ]; then
        if has_inprocess_llvm "$out"; then
            echo "in-process LLVM: yes"
        else
            echo "::error::stage $stage was built with $stage_llvm but the" \
                "result has no in-process LLVM" >&2
            exit 1
        fi
    else
        echo "in-process LLVM: no (nothing to link it from)"
    fi
    echo
    prev=$out
    stage=$((stage + 1))
done

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
