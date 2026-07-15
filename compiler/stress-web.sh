#!/bin/bash
cd /mnt/c/Users/Ali/Projects/SalamLang/Salam/compiler || exit 1
export SALAM_STD="$PWD/std"
WORK=$(mktemp -d)
trap 'rm -rf "$WORK"' EXIT
# background CPU load to simulate a busy CI runner
for _ in 1 2 3 4; do (while :; do :; done) & done
LOADPIDS=$(jobs -p)
kill_load() { kill $LOADPIDS 2>/dev/null; }
trap 'kill_load; rm -rf "$WORK"' EXIT

for t in web_loopback web_server web_router socket_loopback; do
    f="tests/en/general/$t.salam"
    exp="tests/en/general/$t.out"
    [ -f "$f" ] || continue
    ./salam build "$f" --output="$WORK/$t.exe" --no-color --log-level=error --lang=en >/dev/null 2>&1
    if [ ! -x "$WORK/$t.exe" ]; then echo "$t: BUILD FAILED"; continue; fi
    want=$(tr -d '\r' <"$exp")
    bad=0
    for i in $(seq 1 40); do
        got=$("$WORK/$t.exe" 2>&1 | tr -d '\r')
        [ "$got" = "$want" ] || { bad=$((bad + 1)); [ "$bad" -le 2 ] && echo "$t MISMATCH #$i: got=[$(echo "$got" | tr '\n' '|')]"; }
    done
    echo "$t: $bad / 40 failed"
done
