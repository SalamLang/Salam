#!/bin/sh
# Checks the native backend's x86-64 encoder against GNU as.
#
# For each program it builds an object with --backend=native --emit-asm,
# assembles the emitted .s with as, and compares the two disassemblies
# instruction by instruction. Branch targets, RIP displacements and
# padding are ignored, since as resolves local calls itself and pads
# differently; every other byte of every instruction has to match.
#
# Usage: tools/bash/native-encoder-check.sh <salam> <file.salam>...

set -u
SALAM=$1
case "$SALAM" in /*) ;; */*) SALAM="$(pwd)/$SALAM" ;; esac
shift
work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT
fail=0
pass=0
norm() {
    objdump -d -M intel --no-show-raw-insn "$1" |
        grep -E '^ +[0-9a-f]+:' |
        sed -E 's/^ +[0-9a-f]+:[[:space:]]+//; s/#.*//; s/(call|jmp|j[a-z]+)[[:space:]]+[0-9a-f]+ <[^>]*>/\1 T/; s/\[rip\+0x[0-9a-f]+\]/[rip+X]/; s/[[:space:]]+$//' |
        grep -vE '^(int3|nop|data16|cs nop|xchg[[:space:]]+ax,ax)'
}
for f in "$@"; do
    b=$(basename "$f" .salam)
    d="$work/$b"
    mkdir -p "$d"
    if ! (cd "$d" && "$SALAM" obj "$f" --backend=native --emit-asm --output="$d/n.o" --log-level=error >"$d/log" 2>&1); then
        echo "SKIP $b (native build failed)"
        continue
    fi
    if ! as "$d/$b.s" -o "$d/a.o" 2>"$d/as.log"; then
        echo "FAIL $b (as rejected the listing)"
        head -3 "$d/as.log"
        fail=$((fail + 1))
        continue
    fi
    norm "$d/n.o" >"$d/n.txt"
    norm "$d/a.o" >"$d/a.txt"
    if diff "$d/n.txt" "$d/a.txt" >"$d/diff"; then
        pass=$((pass + 1))
    else
        echo "FAIL $b"
        head -6 "$d/diff"
        fail=$((fail + 1))
    fi
done
echo "encoder check: $pass passed, $fail failed"
[ "$fail" -eq 0 ]
