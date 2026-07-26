#!/bin/sh
# Stage-C corpus gate: link+run every compiler/tests/en/llvm/*.salam via the
# salam/llvm.salam port's own lv.NativeRun, compare stdout against the .out
# fixture (same corpus stage B's Gate 2 already IR/exec-validated).
set -u
cd /mnt/c/Users/Admin/Projects/SalamLang/Salam
export SALAM_STD="$PWD/compiler"
PORT=/tmp/lv_native_run
DIR=compiler/tests/en/llvm
TMP=salam/tests_port/tmp_lv
mkdir -p "$TMP"

total=0
match=0
mism=0
MISMATCH_LOG=/tmp/lv_gate_mismatches.txt
: >"$MISMATCH_LOG"

for f in "$DIR"/*.salam; do
    [ -e "$f" ] || continue
    name=$(basename "$f" .salam)
    exp="$DIR/$name.out"
    [ -f "$exp" ] || continue
    total=$((total + 1))
    ll="$TMP/${name}.ll"
    exe="$TMP/${name}_exe"
    pout=$("$PORT" "$f" "$name" "$ll" "$exe" 2>/dev/null | tr -d '\r')
    eout=$(cat "$exp" | tr -d '\r')
    if [ "$pout" = "$eout" ]; then
        match=$((match + 1))
    else
        mism=$((mism + 1))
        {
            echo "=== MISMATCH: $f ==="
            echo "--- expected ---"
            echo "$eout"
            echo "--- port ---"
            echo "$pout"
            echo
        } >>"$MISMATCH_LOG"
    fi
    rm -f "$ll" "$exe" "$ll.run.sh"
done

echo "GATE C: $match / $total identical"
echo "$match $total" > /tmp/lv_gate_c_result.txt
