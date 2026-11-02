#!/bin/sh
# Prints the tests that do not apply when native-built programs run on a
# machine of the given kind, for tools/bash/native-stage-tests.sh.
#
# Usage: tools/bash/native-host-skips.sh <macos-arm64|macos-x64|windows-x64|linux-arm64>

case "${1:-}" in
macos-*) echo "" ;;
windows-*) echo "" ;;
*) echo "" ;;
esac
