#!/bin/bash
cd /mnt/c/Users/Ali/Projects/SalamLang/Salam || exit
echo "=== baked std in WSL? ==="
find /usr/local/share/salam -maxdepth 1 -name std 2>&1 | head
echo "=== CI-style: SALAM_STD=compiler/std, cwd=repo root ==="
printf 'func main:\n    __ "ok"\nend\n' > /tmp/smoke.salam
export SALAM_STD="$PWD/compiler/std"
echo "SALAM_STD=$SALAM_STD"
./compiler/salam build /tmp/smoke.salam --output=/tmp/smoke.exe --no-color --log-level=error --lang=en 2>&1 | head -30
echo "exit=$?"
ls -la /tmp/smoke.exe 2>&1
echo "=== now WITHOUT any baked std interference: unset, run from /tmp ==="
cd /tmp || exit
env -u SALAM_STD /mnt/c/Users/Ali/Projects/SalamLang/Salam/compiler/salam build /tmp/smoke.salam --output=/tmp/smoke2.exe --no-color --log-level=error --lang=en 2>&1 | head -20
echo "exit=$?"
ls -la /tmp/smoke2.exe 2>&1
