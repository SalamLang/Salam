#!/bin/sh
set -u
cd /mnt/c/Users/Admin/Projects/SalamLang/Salam
export SALAM_STD="$PWD/compiler"
REF=/tmp/salam
PORT=/tmp/ipport
TSFILTER='s/^\[[0-9][0-9][0-9][0-9]-[0-9][0-9]-[0-9][0-9] [0-9][0-9]:[0-9][0-9]:[0-9][0-9]\]//'
dir="$1"
lang="$2"
MISMATCH_LOG=/tmp/ip_gate2_mismatches.txt
COUNT_FILE=/tmp/ip_gate2_count.txt
total=0
ident=0
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
echo "$ident $total" >>"$COUNT_FILE"
echo "$dir ($lang): $ident / $total identical"
