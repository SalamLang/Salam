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
# The seed must be 0.2.9 or newer. 0.2.7 and older do not collapse '..' in
# import paths, so compiler/'s per-directory layout blows past their module
# limit (see the stage 1 hint below). 0.2.8 clears that but emits no
# prototype for the other-package functions a generic instantiation calls,
# and std/'s Vector<T> has one, so its stage 1 dies in the C compiler on an
# implicit declaration of _Salam_mem_AllocateZeroed_u64. `make -C c` builds a
# usable seed.
#
# Usage:
#   compiler/tools/bash/bootstrap.sh <seed-salam> [outdir] [--stages N]
#
# Every stage links static LLVM/LLD in and is compiled by it, so the chain
# matches what a default `make -C c` produces rather than only its last link.
# Two independent properties, both now per-stage:
#
#   linked   -DSALAM_HAVE_LLVM --libpath=DIR, so the stage's OWN binary can
#            codegen and link in-process. Needs the stage's builder to accept
#            --libpath (0.2.8+), which the seed is probed for.
#   used     the stage is compiled BY its builder's in-process LLVM. Needs
#            that builder to have been linked with it - true of every stage
#            from 2 on, and of a seed from a self-contained release.
#
# Env:
#   SALAM_CC   C compiler the Salam driver shells out to (default: cc/gcc/
#              clang). Only reached when a stage's builder has no in-process
#              LLVM, since passing --cc selects the C backend outright.
#   SALAM_BOOTSTRAP_BACKEND
#              auto (default) compiles each stage with its builder's
#              in-process LLVM when it has one; c forces the C backend for
#              every stage while still linking LLVM into the results.
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
# Why the linking is opt-in-by-detection rather than always on:
# libsalam_llvm.a is produced by `make -C c libsalam-llvm WITH_LLVM=1
# WITH_LLD=1`, which needs LLVM development libraries on the machine.
# Without the archive the link would fail, so a missing archive degrades to a
# compiler that shells out to a C toolchain instead of failing the bootstrap.
# Each stage then reports which of the two properties it actually got, and a
# stage asked to link LLVM that comes out without it is a hard failure - that
# is the silent downgrade this whole arrangement exists to prevent.
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

# Can this compiler take the LLVM flags at all? Only releases from 0.2.8 on
# know --libpath, and an older one does not ignore it - it exits with
# "unknown option '--libpath=...'" and takes the stage down with it. Every
# stage from 2 on is built from this checkout and always knows the flag; the
# seed is whatever `salam` the machine already had, so it is the one that has
# to be asked. `salam help` lists it, which is cheaper and more direct than
# compiling something to find out.
accepts_llvm_flags() {
    "$1" help 2>&1 | grep -q -- '--libpath'
}

# Does this compiler understand -dNAME=VALUE compile-time constants? A seed
# released before they existed rejects the flag outright rather than ignoring
# it, so stage 1 has to ask before stamping the build info onto it.
accepts_const_defines() {
    "$1" help 2>&1 | grep -q -- '-dNAME=VALUE'
}

# Escape hatch for the LLVM-builds-LLVM chain below. `c` puts every stage
# back on the C backend without giving up the LLVM the stages *link*, which
# is the knob to reach for if the LLVM backend ever cannot compile the
# compiler itself; SALAM_WITH_LLVM=0 is the bigger hammer that also stops
# linking it.
BOOTSTRAP_BACKEND=${SALAM_BOOTSTRAP_BACKEND:-auto}
case "$BOOTSTRAP_BACKEND" in
auto | c) ;;
*)
    echo "error: SALAM_BOOTSTRAP_BACKEND must be 'auto' or 'c', got '$BOOTSTRAP_BACKEND'" >&2
    exit 2
    ;;
esac

# Is static LLVM linked INTO this compiler, i.e. can it codegen in-process?
#
# Asked by building something with the DEFAULT backend and looking for the IR
# the LLVM path logs, because `auto` resolves to LLVM exactly when this
# binary has it in-process (use_llvm_backend -> NativeAvailable in
# compiler/driver/driver_proc.salam) and to the C backend otherwise. That
# makes the probe answer the precise question the caller has: will this
# compiler use LLVM if left alone?
#
# NOT `--backend=llvm`, which looks like the direct question and is not: it
# is a hard request for the IR path, and a compiler with no LLVM inside
# serves it by writing the .ll and shelling out to whatever external
# toolchain it can find. It exits 0 having used no in-process LLVM at all,
# so it reports every compiler as having it.
#
# "LLVM IR" rather than the whole sentence: the log line is translated, and
# those two words are the part every translation keeps verbatim.
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

# What `salam version` will report for every stage. compiler/sal_core.salam
# reads these as compile-time constants, so without them a stage inherits the
# *builder's* build info - which for stage 1 means a released seed's version
# number rather than this checkout's. VERSION is the repo file, the single
# place the release number lives; the git metadata describes the tree being
# compiled, not the compiler doing the compiling.
STAMP_VERSION="$(cat "$ROOT/VERSION" 2>/dev/null || echo 0.0.0-dev)"
STAMP_COMMIT="$(git -C "$ROOT" rev-parse --short HEAD 2>/dev/null || echo unknown)"
# Strict ISO 8601 (%cI, not %ci): the flag list below is word-split on the
# way to the build command, so a commit date with spaces in it would arrive
# as four separate arguments. The C Makefile stamps the same format.
STAMP_DATE="$(git -C "$ROOT" show -s --format=%cI HEAD 2>/dev/null || echo unknown)"
STAMP_DIRTY=
if [ -n "$(git -C "$ROOT" status --porcelain 2>/dev/null)" ]; then
    STAMP_DIRTY=-dirty
fi
# Quoted so a value that would otherwise read as a number (a two-component
# version like "0.3") stays a string constant.
BUILD_INFO_FLAGS="-dSALAM_VERSION=\"$STAMP_VERSION\" -dSALAM_GIT_COMMIT=\"$STAMP_COMMIT\""
BUILD_INFO_FLAGS="$BUILD_INFO_FLAGS -dSALAM_GIT_DATE=\"$STAMP_DATE\""
BUILD_INFO_FLAGS="$BUILD_INFO_FLAGS -dSALAM_GIT_DIRTY=\"$STAMP_DIRTY\""

# The seed decides whether stage 1 can have LLVM. Everything after it is
# built by a compiler from this checkout, so from stage 2 on the only
# question is whether libsalam_llvm.a was there to link.
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
    # Every stage links LLVM in, stage 1 included, so the whole chain matches
    # what `make -C c` produces by default rather than only the artifact at
    # the end of it. Stage 1 used to be excluded because the seed might
    # predate --libpath; that is now an actual capability probe instead of an
    # assumption, so a current seed gets a stage 1 with LLVM and an ancient
    # one still bootstraps.
    stage_llvm=$LLVM_FLAGS
    if [ "$stage" -eq 1 ]; then
        stage_llvm=$SEED_LLVM_FLAGS
    fi
    # And every stage is BUILT by LLVM whenever its builder has it, which is
    # a separate question from what the output links. `--cc` is not a hint
    # about which C compiler to fall back on: it sets backend=c outright (see
    # cli_parse.salam's --cc arm), so passing it unconditionally meant a
    # 161MB stage2 with LLVM statically inside it still went through the C
    # backend to produce stage3. It is now the fallback it reads as - used
    # only when the builder cannot codegen in-process.
    stage_cc="--cc=$SALAM_CC"
    stage_backend=C
    if [ "$BOOTSTRAP_BACKEND" != c ] && has_inprocess_llvm "$prev"; then
        stage_cc=
        stage_backend=LLVM
    fi
    echo "   backend: $stage_backend (via $(basename "$prev"))"
    # A seed too old to know -d gets no stamp and reports its own build info;
    # every later stage is built by a compiler from this checkout, so only
    # stage 1 can ever fall back.
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
            echo "      with 'make -C c' and pass ./salam." >&2
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
    # Neither does one that runs but silently came out without the backend it
    # was asked to link. `-DSALAM_HAVE_LLVM` compiles the arm of
    # llvm_bridge.salam that imports std/llvm; if the archive behind it were
    # missing the link would fail loudly, but a flag lost anywhere in this
    # chain would not - it would just produce a working compiler that quietly
    # shells out for the rest of its life. Ask the binary itself.
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
