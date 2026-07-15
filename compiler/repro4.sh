#!/bin/bash
cd /tmp || exit
printf 'func main:\n    __ "ok"\nend\n' > /tmp/smoke.salam
SALAM=/mnt/c/Users/Ali/Projects/SalamLang/Salam/compiler/salam
echo "=== TEST A: SALAM_STD points to BOGUS path (baked std removed from equation) ==="
echo "If this still builds, SALAM_STD is being IGNORED and baked prefix masks it."
env SALAM_STD=/tmp/does-not-exist-std $SALAM build /tmp/smoke.salam --output=/tmp/a.exe --no-color --log-level=error --lang=en 2>&1 | head -15
echo "exit=$?"; ls /tmp/a.exe 2>&1
echo ""
echo "=== TEST B: temporarily hide baked std, SALAM_STD=real compiler/std ==="
sudo mv /usr/local/share/salam/std /usr/local/share/salam/std.bak 2>/dev/null
env SALAM_STD=/mnt/c/Users/Ali/Projects/SalamLang/Salam/compiler/std $SALAM build /tmp/smoke.salam --output=/tmp/b.exe --no-color --log-level=error --lang=en 2>&1 | head -20
echo "exit=$?"; ls /tmp/b.exe 2>&1
echo ""
echo "=== TEST C: baked hidden, NO SALAM_STD (should fail like CI if resolution broken) ==="
env -u SALAM_STD $SALAM build /tmp/smoke.salam --output=/tmp/c.exe --no-color --log-level=error --lang=en 2>&1 | head -20
echo "exit=$?"; ls /tmp/c.exe 2>&1
sudo mv /usr/local/share/salam/std.bak /usr/local/share/salam/std 2>/dev/null
