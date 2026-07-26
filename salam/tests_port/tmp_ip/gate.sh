#!/bin/sh
# Interp port equivalence gate: /tmp/salam exec (reference) vs /tmp/ipport
# (the Salam interp.salam port), invoked exactly like the repo's own
# run-tests.sh exec section: --no-color --log-level=error --lang=<L>,
# stdout+stderr merged, exit code also compared (repo's own harness doesn't
# check exit code for exec/, but we additionally track it for extra rigor).
set -u
cd /mnt/c/Users/Admin/Projects/SalamLang/Salam
export SALAM_STD="$PWD/compiler"
REF=/tmp/salam
PORT=/tmp/ipport

total=0
ident=0
mism=0
MISMATCH_LOG=/tmp/ip_gate_mismatches.txt
: >"$MISMATCH_LOG"

run_dir() {
    dir="$1"
    lang="$2"
    require_out="$3"   # 1 = only files with a matching .out; 0 = all .salam files
    for f in "$dir"/*.salam; do
        [ -e "$f" ] || continue
        name=$(basename "$f" .salam)
        case "$name" in _*) continue ;; esac
        exp="$dir/$name.out"
        if [ "$require_out" = "1" ] && [ ! -f "$exp" ]; then continue; fi
        total=$((total + 1))
        rout=$("$REF" exec "$f" --no-color --log-level=error --lang="$lang" 2>&1 | tr -d '\r')
        rrc=$?
        pout=$("$PORT" "$f" "$lang" 2>&1 | tr -d '\r')
        prc=$?
        if [ "$rout" = "$pout" ]; then
            ident=$((ident + 1))
        else
            mism=$((mism + 1))
            {
                echo "=== MISMATCH: $f (lang=$lang) ==="
                echo "--- reference (rc=$rrc) ---"
                echo "$rout"
                echo "--- port (rc=$prc) ---"
                echo "$pout"
                echo
            } >>"$MISMATCH_LOG"
        fi
    done
}

echo "== en/exec =="
run_dir compiler/tests/en/exec en 1
echo "== fa/exec =="
run_dir compiler/tests/fa/exec fa 1
echo "== ar/exec =="
run_dir compiler/tests/ar/exec ar 1

echo "PRIMARY (exec corpora): $ident / $total identical"
echo "$ident $total" > /tmp/ip_gate_primary.txt

# Bonus: en/general (not exec-tested by the repo's own harness — see
# run-tests.sh, general/ only goes through the compiled backend — but every
# file is still valid interpreter input, so this is a useful EXTRA
# port-vs-reference equivalence signal, tracked separately from the primary
# gate).
gtotal=0
gident=0
gmism=0
GEN_MISMATCH_LOG=/tmp/ip_gate_general_mismatches.txt
PROGRESS_LOG=/tmp/ip_gate_progress.txt
: >"$GEN_MISMATCH_LOG"
: >"$PROGRESS_LOG"
for f in compiler/tests/en/general/*.salam; do
    [ -e "$f" ] || continue
    name=$(basename "$f" .salam)
    case "$name" in _*) continue ;; esac
    gtotal=$((gtotal + 1))
    echo "$gtotal $name" >>"$PROGRESS_LOG"
    rout=$(timeout 15 "$REF" exec "$f" --no-color --log-level=error --lang=en 2>&1 | tr -d '\r')
    pout=$(timeout 15 "$PORT" "$f" en 2>&1 | tr -d '\r')
    if [ "$rout" = "$pout" ]; then
        gident=$((gident + 1))
    else
        gmism=$((gmism + 1))
        {
            echo "=== MISMATCH: $f ==="
            echo "--- reference ---"
            echo "$rout"
            echo "--- port ---"
            echo "$pout"
            echo
        } >>"$GEN_MISMATCH_LOG"
    fi
done
echo "BONUS (en/general, not officially exec-tested): $gident / $gtotal identical"
echo "$gident $gtotal" > /tmp/ip_gate_bonus.txt
