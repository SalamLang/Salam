#!/usr/bin/env bash
# Builds the interop programs with a scratch compiler copy; usage: build.sh SALAM_BIN OUT_DIR
set -eu
salam=$1
out=$2
root=$(cd "$(dirname "$0")/../.." && pwd)
mkdir -p "$out"
cd "$out"
ulimit -v 4000000
"$salam" build "$root/examples/vpn/vpn_cli.salam" --output="$out/vpn" >/dev/null
for f in "$root"/tools/interop/*.salam; do
    [ -e "$f" ] || continue
    n=$(basename "$f" .salam)
    "$salam" build "$f" --output="$out/$n" >/dev/null
done
rm -rf "$out/.salam-build"
