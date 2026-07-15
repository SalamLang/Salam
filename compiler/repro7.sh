#!/bin/bash
# Simulate CI Linux: salam at a dir with NO std next to it, no baked std, no SALAM_STD, cwd=jobdir
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
STD=/usr/local/share/salam/std
restore() { [ -d "$STD.bak" ] && sudo mv "$STD.bak" "$STD" 2>/dev/null; rm -rf /tmp/cirepo /tmp/jobdir; }
trap restore EXIT
sudo mv "$STD" "$STD.bak" 2>/dev/null
mkdir -p /tmp/cirepo
cp "$SCRIPT_DIR/salam" /tmp/cirepo/salam   # salam alone, no std beside it
printf 'func main:\n    __ "ok"\nend\n' > /tmp/smoke.salam
mkdir -p /tmp/jobdir
cd /tmp/jobdir || exit   # cwd is NOT compiler, like run-one-build.sh does
echo "=== CI-Linux sim: no SALAM_STD, salam has no adjacent std, cwd=jobdir ==="
env -u SALAM_STD /tmp/cirepo/salam build /tmp/smoke.salam --output=/tmp/ci.exe --cc=tcc --no-color --log-level=error 2>&1 | head -15
echo "exit=$?"; ls /tmp/ci.exe 2>&1
