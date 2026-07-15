#!/bin/bash
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
STD=/usr/local/share/salam/std
restore() { [ -d "$STD.bak" ] && sudo mv "$STD.bak" "$STD" 2>/dev/null; }
trap restore EXIT
sudo mv "$STD" "$STD.bak" 2>/dev/null
cd "$SCRIPT_DIR" || exit
# Simulate CI: no SALAM_STD preset; harness must set it. Use en only for speed.
env -u SALAM_STD SALAM=./salam LANGS=en bash tools/bash/run-tests.sh general > /tmp/gen.log 2>&1
echo "harness exit=$?"
tail -3 /tmp/gen.log
echo "=== any build failures? ==="
grep -c "build failed" /tmp/gen.log
