#!/bin/sh
# Checks the native backend's x86-64 and i686 encoders against GNU as.
#
# For each program it builds an object with --backend=native --emit-asm,
# assembles the emitted .s with as, and compares the two disassemblies
# instruction by instruction. Branch targets, RIP displacements and
# padding are ignored, since as resolves local calls itself and pads
# differently (a jmp over padding, which as never emits, is dropped too);
# every other byte of every instruction has to match.
#
# Usage: tools/bash/native-encoder-check.sh <salam> <file.salam>...
# NATIVE_ENC_FLAGS adds build flags (for example --release --cpu=x86-64-v3).
# A --target=i686-... flag there checks the i686 encoder; its absolute
# addresses are ignored like the RIP displacements.

set -u
SALAM=$1
case "$SALAM" in /*) ;; */*) SALAM="$(pwd)/$SALAM" ;; esac
shift
work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT
fail=0
pass=0
asflags=
case "${NATIVE_ENC_FLAGS:-}" in
*--target=i686*) asflags="--32 -mtune=generic64" ;;
esac
norm() {
    objdump -d -M intel --no-show-raw-insn "$1" |
        grep -E '^ +[0-9a-f]+:' |
        awk '
        {
            a = $1; sub(":", "", a)
            ins = $0; sub(/^ +[0-9a-f]+:[[:space:]]+/, "", ins)
            if (held != "") {
                if (ins ~ /^(int3|nop|data16|cs nop|xchg[[:space:]]+ax,ax)/) { print; next }
                if (a != tgt) print held
                held = ""
            }
            if (ins ~ /^jmp[[:space:]]+[0-9a-f]+ </) { held = $0; tgt = ins; sub(/^jmp[[:space:]]+/, "", tgt); sub(/ .*/, "", tgt); next }
            print
        }
        END { if (held != "") print held }' |
        sed -E 's/^ +[0-9a-f]+:[[:space:]]+//; s/#.*//; s/(call|jmp|j[a-z]+)[[:space:]]+[0-9a-f]+ <[^>]*>/\1 T/; s/\[rip\+0x[0-9a-f]+\]/[rip+X]/; s/ds:0x[0-9a-f]+/ds:X/; s/[[:space:]]+$//' |
        grep -vE '^(int3|nop|data16|cs nop|xchg[[:space:]]+ax,ax)'
}
for f in "$@"; do
    case "$f" in /*) ;; *) f="$(pwd)/$f" ;; esac
    b=$(basename "$f" .salam)
    d="$work/$b"
    mkdir -p "$d"
    # shellcheck disable=SC2086
    if ! (cd "$d" && "$SALAM" obj "$f" --backend=native --emit-asm ${NATIVE_ENC_FLAGS:-} --output="$d/n.o" --log-level=error >"$d/log" 2>&1); then
        echo "SKIP $b (native build failed)"
        continue
    fi
    # shellcheck disable=SC2086
    if ! as $asflags "$d/$b.s" -o "$d/a.o" 2>"$d/as.log"; then
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
[ "$fail" -eq 0 ] && [ "$pass" -gt 0 ]
