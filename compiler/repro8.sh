#!/bin/bash
# Re-run CI-Linux sim but now WITH the harness setting SALAM_STD to local std
sudo mv /usr/local/share/salam/std /usr/local/share/salam/std.bak 2>/dev/null
mkdir -p /tmp/cirepo
cp /mnt/c/Users/Ali/Projects/SalamLang/Salam/compiler/salam /tmp/cirepo/salam
printf 'func main:\n    __ "ok"\nend\n' > /tmp/smoke.salam
mkdir -p /tmp/jobdir
cd /tmp/jobdir || exit
echo "=== WITHOUT fix (baseline, should fail) ==="
env -u SALAM_STD /tmp/cirepo/salam build /tmp/smoke.salam --output=/tmp/nofix.exe --cc=tcc --no-color --log-level=error 2>&1 | head -3
ls /tmp/nofix.exe 2>&1
echo "=== WITH fix (harness exports SALAM_STD to checkout std) ==="
export SALAM_STD="/mnt/c/Users/Ali/Projects/SalamLang/Salam/compiler/std"
/tmp/cirepo/salam build /tmp/smoke.salam --output=/tmp/withfix.exe --cc=tcc --no-color --log-level=error 2>&1 | head -5
echo "exit=$?"; ls -la /tmp/withfix.exe 2>&1 && /tmp/withfix.exe
sudo mv /usr/local/share/salam/std.bak /usr/local/share/salam/std 2>/dev/null
rm -rf /tmp/cirepo /tmp/jobdir /tmp/nofix.exe /tmp/withfix.exe
