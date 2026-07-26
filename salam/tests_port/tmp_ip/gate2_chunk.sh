#!/bin/sh
# Bonus gate v2 (en/general, split streams + timestamp-normalized stderr).
set -u
cd /mnt/c/Users/Admin/Projects/SalamLang/Salam
export SALAM_STD="$PWD/compiler"
REF=/tmp/salam
PORT=/tmp/ipport
TSFILTER='s/^\[[0-9][0-9][0-9][0-9]-[0-9][0-9]-[0-9][0-9] [0-9][0-9]:[0-9][0-9]:[0-9][0-9]\]//'
CHUNK="$1"
MISMATCH_LOG=/tmp/ip_gate2_general_mismatches.txt
COUNT_FILE=/tmp/ip_gate2_bonus_count.txt

gident=0
gtotal=0
while IFS= read -r name; do
    [ -z "$name" ] && continue
    f="compiler/tests/en/general/$name.salam"
    [ -e "$f" ] || continue
    gtotal=$((gtotal + 1))
    rout=$(timeout 15 "$REF" exec "$f" --no-color --log-level=error --lang=en 2>/tmp/rerr.$$ | tr -d '\r')
    rerr=$(tr -d '\r' </tmp/rerr.$$ | sed -E "$TSFILTER")
    rm -f /tmp/rerr.$$
    pout=$(timeout 15 "$PORT" "$f" en 2>/tmp/perr.$$ | tr -d '\r')
    perr=$(tr -d '\r' </tmp/perr.$$ | sed -E "$TSFILTER")
    rm -f /tmp/perr.$$
    if [ "$rout" = "$pout" ] && [ "$rerr" = "$perr" ]; then
        gident=$((gident + 1))
    else
        {
            echo "=== MISMATCH: $f ==="
            echo "--- stdout ref ---"; echo "$rout"
            echo "--- stdout port ---"; echo "$pout"
            echo "--- stderr ref (ts-stripped) ---"; echo "$rerr"
            echo "--- stderr port (ts-stripped) ---"; echo "$perr"
            echo
        } >>"$MISMATCH_LOG"
    fi
done <"$CHUNK"
echo "$gident $gtotal" >>"$COUNT_FILE"
echo "chunk $CHUNK: $gident / $gtotal identical"
