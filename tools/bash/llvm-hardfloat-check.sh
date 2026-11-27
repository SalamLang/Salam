#!/bin/sh
set -eu

[ $# -eq 1 ] || {
    echo "usage: $0 <salam>" >&2
    exit 2
}
SALAM=$1
case "$SALAM" in /*) ;; *) SALAM="$(pwd)/$SALAM" ;; esac
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
export SALAM_STD="$ROOT/std"

work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT INT TERM
cd "$work"
printf 'func add(a: f64, b: f64): f64:\n    ret a + b\nend\n\nfunc main:\n    println add("1.5".ToFloat(), 2.0)\nend\n' >hf.salam

for triple in arm-linux-musleabihf arm-linux-gnueabihf armv7-linux-gnueabihf \
    arm-unknown-linux-musleabihf; do
    "$SALAM" llvm hf.salam --target="$triple" --emit-asm -o "$triple.s" --log-level=warn
    grep -Eq '\.eabi_attribute[[:space:]]+28, 1' "$triple.s" || {
        echo "error: $triple did not get the hard-float ABI" >&2
        exit 1
    }
    grep -q 'vadd\.f64' "$triple.s" || {
        echo "error: $triple does not use the FPU" >&2
        exit 1
    }
    echo "hard-float ABI: $triple"
done
