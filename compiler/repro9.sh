#!/bin/bash
sudo mv /usr/local/share/salam/std /usr/local/share/salam/std.bak 2>/dev/null
cd /mnt/c/Users/Ali/Projects/SalamLang/Salam/compiler || exit
# Simulate CI: no SALAM_STD preset; harness must set it. Use en only for speed.
env -u SALAM_STD SALAM=./salam LANGS=en bash tools/bash/run-tests.sh general > /tmp/gen.log 2>&1
echo "harness exit=$?"
tail -3 /tmp/gen.log
echo "=== any build failures? ==="
grep -c "build failed" /tmp/gen.log
sudo mv /usr/local/share/salam/std.bak /usr/local/share/salam/std 2>/dev/null
