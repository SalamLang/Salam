#!/bin/bash
# Re-run CI-Linux sim but now WITH the harness setting SALAM_STD to local std
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
STD=/usr/local/share/salam/std
restore() { [ -d "$STD.bak" ] && sudo mv "$STD.bak" "$STD" 2>/dev/null; rm -rf /tmp/cirepo /tmp/jobdir /tmp/nofix.exe /tmp/withfix.exe; }
trap restore EXIT
sudo mv "$STD" "$STD.bak" 2>/dev/null
mkdir -p /tmp/cirepo
cp "$SCRIPT_DIR/salam" /tmp/cirepo/salam
printf 'func main:\n    __ "ok"\nend\n' > /tmp/smoke.salam
mkdir -p /tmp/jobdir
cd /tmp/jobdir || exit
echo "=== WITHOUT fix (baseline, should fail) ==="
env -u SALAM_STD /tmp/cirepo/salam build /tmp/smoke.salam --output=/tmp/nofix.exe --cc=tcc --no-color --log-level=error 2>&1 | head -3
ls /tmp/nofix.exe 2>&1
echo "=== WITH fix (harness exports SALAM_STD to checkout std) ==="
export SALAM_STD="$SCRIPT_DIR/std"
/tmp/cirepo/salam build /tmp/smoke.salam --output=/tmp/withfix.exe --cc=tcc --no-color --log-level=error 2>&1 | head -5
echo "exit=$?"; ls -la /tmp/withfix.exe 2>&1 && /tmp/withfix.exe
