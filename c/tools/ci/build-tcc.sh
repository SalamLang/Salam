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

# shellcheck disable=SC2086
cross_arg=""
[ -n "$CROSS_PREFIX" ] && cross_arg="--cross-prefix=$CROSS_PREFIX"
# shellcheck disable=SC2086
../configure --prefix="$PREFIX" $cross_arg $TCC_CONFIGURE_EXTRA

"$MAKE" -j"$(nproc 2>/dev/null || echo 4)"
"$MAKE" install

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
# `tcc -v` runs the built binary, so it only proves anything when the build
# is native; a cross-built one is checked by the job that consumes it.
if [ -z "$CROSS_PREFIX" ]; then
    "$TCC_BIN" -v
    printf 'int main(void){return 0;}\n' >"$WORK/smoke.c"
    "$TCC_BIN" "$WORK/smoke.c" -o "$WORK/smoke" && "$WORK/smoke"
    echo "smoke test: compiled and ran"
fi
