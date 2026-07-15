#!/bin/bash
cd /mnt/c/Users/Ali/Projects/SalamLang/Salam/compiler || exit 1
export SALAM_STD="$PWD/std"
WORK=$(mktemp -d)
trap 'rm -rf "$WORK"' EXIT
run() {
    lang="$1"; t="$2"
    f="tests/$lang/general/$t.salam"
    exp="tests/$lang/general/$t.out"
    ./salam build "$f" --output="$WORK/$t.exe" --no-color --log-level=error --lang="$lang" >"$WORK/berr" 2>&1
    if [ ! -x "$WORK/$t.exe" ]; then
        echo "$lang/$t: BUILD FAILED"; sed 's/^/    /' "$WORK/berr" | head -8; return
    fi
    got=$("$WORK/$t.exe" 2>&1 | tr -d '\r')
    want=$(tr -d '\r' <"$exp")
    if [ "$got" = "$want" ]; then echo "$lang/$t: PASS"; else
        echo "$lang/$t: MISMATCH"
        echo "  want=[$(echo "$want" | tr '\n' '|')]"
        echo "  got =[$(echo "$got" | tr '\n' '|')]"
    fi
}
run en web_loopback
run en web_router
run en web_server
run fa web_router
run fa web_server
