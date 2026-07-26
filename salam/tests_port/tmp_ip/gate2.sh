#!/bin/sh
# Interp port equivalence gate v2: compares stdout and stderr SEPARATELY
# (not merged via 2>&1) plus exit code. Splitting the streams avoids a false
# "mismatch" from C stdio's stdout-is-fully-buffered-when-piped vs
# stderr-is-unbuffered interleaving order, which is an artifact of the OS
# stdio layer, not the interpreter's actual behavior. stderr is normalized
# by stripping the leading `[YYYY-MM-DD HH:MM:SS]` logger timestamp (see
# salam/PORTING.md's documented normalization) before comparing.
set -u
cd /mnt/c/Users/Admin/Projects/SalamLang/Salam
export SALAM_STD="$PWD/compiler"
REF=/tmp/salam
PORT=/tmp/ipport
TSFILTER='s/^\[[0-9][0-9][0-9][0-9]-[0-9][0-9]-[0-9][0-9] [0-9][0-9]:[0-9][0-9]:[0-9][0-9]\]//'

total=0
ident=0
MISMATCH_LOG=/tmp/ip_gate2_mismatches.txt
: >"$MISMATCH_LOG"

run_dir() {
    dir="$1"
    lang="$2"
    for f in "$dir"/*.salam; do
        [ -e "$f" ] || continue
        name=$(basename "$f" .salam)
        case "$name" in _*) continue ;; esac
        exp="$dir/$name.out"
        [ -f "$exp" ] || continue
        total=$((total + 1))
        rout=$(timeout 15 "$REF" exec "$f" --no-color --log-level=error --lang="$lang" 2>/tmp/rerr.$$ | tr -d '\r')
        rrc=$?
        rerr=$(tr -d '\r' </tmp/rerr.$$ | sed -E "$TSFILTER")
        rm -f /tmp/rerr.$$
        pout=$(timeout 15 "$PORT" "$f" "$lang" 2>/tmp/perr.$$ | tr -d '\r')
        prc=$?
        perr=$(tr -d '\r' </tmp/perr.$$ | sed -E "$TSFILTER")
        rm -f /tmp/perr.$$
        if [ "$rout" = "$pout" ] && [ "$rerr" = "$perr" ]; then
            ident=$((ident + 1))
        else
            {
                echo "=== MISMATCH: $f (lang=$lang) rc: ref=$rrc port=$prc ==="
                echo "--- stdout ref ---"; echo "$rout"
                echo "--- stdout port ---"; echo "$pout"
                echo "--- stderr ref (ts-stripped) ---"; echo "$rerr"
                echo "--- stderr port (ts-stripped) ---"; echo "$perr"
                echo
            } >>"$MISMATCH_LOG"
        fi
    done
}

echo "== en/exec =="
run_dir compiler/tests/en/exec en
echo "== fa/exec =="
run_dir compiler/tests/fa/exec fa
echo "== ar/exec =="
run_dir compiler/tests/ar/exec ar

echo "PRIMARY v2 (exec corpora, split streams): $ident / $total identical"
echo "$ident $total" > /tmp/ip_gate2_primary.txt
