#!/bin/sh
# Builds the self-hosted compiler: compiler/main.salam, compiled by a seed
# salam, producing a salam binary.
#
# This is what replaced `make -C c`. The seed is an already-built compiler -
# in CI, the released binary .github/actions/setup-salam installs; locally,
# whatever `salam` is on PATH. It has to be SEED_MIN (0.3.1) or newer, since
# older releases cannot resolve compiler/'s `../` imports.
#
# For a three-stage bootstrap with a fixpoint check, use
# compiler/tools/bash/bootstrap.sh instead. This is the one-stage version, for
# jobs that just need a working compiler.
#
# Usage:
#   tools/bash/build-selfhost.sh [--output FILE] [--seed PROG] [--llvm DIR]
#                                [--embed DIR] [-- <extra salam build args>]
#
# Env:
#   SALAM_SEED   same as --seed

set -eu

OUT=$(pwd)/salam
SEED=${SALAM_SEED:-}
LLVM_DIR=
EMBED_DIR=
EXTRA=

while [ $# -gt 0 ]; do
    case $1 in
    --output=*)
        OUT=${1#*=}
        shift
        ;;
    --seed=*)
        SEED=${1#*=}
        shift
        ;;
    --llvm=*)
        LLVM_DIR=${1#*=}
        shift
        ;;
    --embed=*)
        EMBED_DIR=${1#*=}
        shift
        ;;
    --output)
        OUT=$2
        shift 2
        ;;
    --seed)
        SEED=$2
        shift 2
        ;;
    --llvm)
        LLVM_DIR=$2
        shift 2
        ;;
    --embed)
        EMBED_DIR=$2
        shift 2
        ;;
    --)
        shift
        EXTRA="$*"
        break
        ;;
    -h | --help)
        sed -n '2,20p' "$0"
        exit 0
        ;;
    *)
        echo "unknown argument: $1" >&2
        exit 2
        ;;
    esac
done

if [ -z "$SEED" ]; then
    SEED=$(command -v salam 2>/dev/null || true)
fi
[ -n "$SEED" ] || {
    echo "error: no seed compiler. Install one with .github/actions/setup-salam," >&2
    echo "       put a released salam on PATH, or pass --seed /path/to/salam." >&2
    exit 1
}

ROOT=$(cd "$(dirname "$0")/../.." && pwd)

# This checkout's std, not the seed's. A released seed sits next to its own
# std/ and resolves there by default, which both builds the compiler against
# a stale stdlib and trips "package name 'mem' is claimed by two different
# files" when both trees are reachable. bootstrap.sh exports the same.
export SALAM_STD="$ROOT/std"

# -DSALAM_HAVE_LLVM makes llvm_bridge.salam import std/llvm, and --libpath is
# where its libsalam_llvm.a lives; -DSALAM_HAVE_EMBED does the same for the
# sysroot blobs in libsalam_embed.a. Both are opt-in: without them the
# compiler still builds, it just shells out to cc and finds sysroots on disk.
# Flags are accumulated as positional parameters rather than one string: the
# build-info values are quoted so a two-component version like "0.3" stays a
# string constant rather than reading as a number, and those quotes have to
# survive as characters instead of being re-split by the shell.
set --

# -DSALAM_HAVE_LLVM makes llvm_bridge.salam import std/llvm, and --libpath is
# where its libsalam_llvm.a lives; -DSALAM_HAVE_EMBED does the same for the
# sysroot blobs in libsalam_embed.a. Both are opt-in: without them the compiler
# still builds, it just shells out to cc and finds sysroots on disk.
[ -n "$LLVM_DIR" ] && set -- "$@" -DSALAM_HAVE_LLVM "--libpath=$LLVM_DIR"
[ -n "$EMBED_DIR" ] && set -- "$@" -DSALAM_HAVE_EMBED "--libpath=$EMBED_DIR"

# compiler/sal_core.salam reads these as compile-time constants, so without them
# the result inherits the *seed's* build info - a released binary's version
# number instead of this checkout's. Same stamping bootstrap.sh does. %cI, not
# %ci, so the date carries no spaces.
STAMP_VERSION="$(cat "$ROOT/VERSION" 2>/dev/null || echo 0.0.0-dev)"
STAMP_COMMIT="$(git -C "$ROOT" rev-parse --short HEAD 2>/dev/null || echo unknown)"
STAMP_DATE="$(git -C "$ROOT" show -s --format=%cI HEAD 2>/dev/null || echo unknown)"
STAMP_DIRTY=
if [ -n "$(git -C "$ROOT" status --porcelain 2>/dev/null)" ]; then
    STAMP_DIRTY=-dirty
fi
set -- "$@" "-dSALAM_VERSION=\"$STAMP_VERSION\"" "-dSALAM_GIT_COMMIT=\"$STAMP_COMMIT\""
set -- "$@" "-dSALAM_GIT_DATE=\"$STAMP_DATE\"" "-dSALAM_GIT_DIRTY=\"$STAMP_DIRTY\""

# shellcheck disable=SC2086 # a caller-supplied flag list; splitting is wanted
[ -n "$EXTRA" ] && set -- "$@" $EXTRA

echo "seed   : $SEED ($("$SEED" version 2>/dev/null | head -1))"
echo "output : $OUT"

"$SEED" build "$ROOT/compiler/main.salam" --output="$OUT" "$@"

[ -x "$OUT" ] || {
    echo "error: no binary at $OUT after the build" >&2
    exit 1
}
"$OUT" version | head -1
