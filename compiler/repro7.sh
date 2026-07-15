#!/bin/bash
# Simulate CI Linux: salam at a dir with NO std next to it, no baked std, no SALAM_STD, cwd=jobdir
sudo mv /usr/local/share/salam/std /usr/local/share/salam/std.bak 2>/dev/null
mkdir -p /tmp/cirepo
cp /mnt/c/Users/Ali/Projects/SalamLang/Salam/compiler/salam /tmp/cirepo/salam   # salam alone, no std beside it
printf 'func main:\n    __ "ok"\nend\n' > /tmp/smoke.salam
mkdir -p /tmp/jobdir
cd /tmp/jobdir || exit   # cwd is NOT compiler, like run-one-build.sh does
echo "=== CI-Linux sim: no SALAM_STD, salam has no adjacent std, cwd=jobdir ==="
env -u SALAM_STD /tmp/cirepo/salam build /tmp/smoke.salam --output=/tmp/ci.exe --cc=tcc --no-color --log-level=error 2>&1 | head -15
echo "exit=$?"; ls /tmp/ci.exe 2>&1
sudo mv /usr/local/share/salam/std.bak /usr/local/share/salam/std 2>/dev/null
rm -rf /tmp/cirepo /tmp/jobdir
