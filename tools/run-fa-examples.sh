#!/bin/sh
# Build + run every Persian (fa) example and verify it compiles and runs.

set -u
. "$(dirname "$0")/lib.sh"
salam_ensure_compiler
pass=0; fail=0; skip=0
for f in examples/fa/*/*.salam; do
    case "$f" in */editor-selected/*) continue;; esac
    name=$(basename "$f" .salam)
    if grep -qE 'صفحه:|func main' "$f"; then
        echo "SKIP $name (not a fa console app)"; skip=$((skip+1)); continue
    fi
    rm -f salam_mod_*.* 2>/dev/null
    "$SALAM" build "$f" --output="/tmp/$name.exe" --lang=fa --no-color --log-level=error >/dev/null 2>&1
    if [ -x "/tmp/$name.exe" ] && out=$("/tmp/$name.exe" 2>&1) && [ -n "$out" ]; then
        echo "PASS $name"; pass=$((pass+1))
    else
        echo "FAIL $name"; fail=$((fail+1))
    fi
    rm -f "/tmp/$name.exe"
done
rm -f salam_mod_*.* salam_runtime.* 2>/dev/null
echo "----------------------------------------"
echo "RESULT: $pass passed, $fail failed, $skip skipped"
[ "$fail" -eq 0 ]
