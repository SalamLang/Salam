#!/bin/sh
# Prints the tests that do not apply when native-built programs run on a
# machine of the given kind, for tools/bash/native-stage-tests.sh.
#
# Only tests whose expected output cannot hold on that kind of machine belong
# here. On Windows, stdlib/os_process_fork, stdlib/log_fatal and stdlib/os_api
# print "fork unavailable" since Windows has no fork, and stdlib/os_shell_demo
# runs fewer checks (the /bin/sh and signal cases are POSIX only), so its count
# differs from the .out file.
#
# Usage: tools/bash/native-host-skips.sh <macos-arm64|macos-x64|windows-x64|windows-arm64|linux-arm64>

case "${1:-}" in
macos-*) echo "" ;;
windows-*) echo "stdlib/os_process_fork stdlib/log_fatal stdlib/os_api stdlib/os_shell_demo" ;;
*) echo "" ;;
esac
