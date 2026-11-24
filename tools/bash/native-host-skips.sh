#!/bin/sh
# Prints the tests that do not apply when native-built programs run on a
# machine of the given kind, for tools/bash/native-stage-tests.sh.
#
# Only tests whose expected output cannot hold on that kind of machine belong
# here. On Windows, stdlib/os_process_fork, stdlib/log_fatal and stdlib/os_api
# print "fork unavailable" since Windows has no fork, and stdlib/os_shell_demo
# runs fewer checks (the /bin/sh and signal cases are POSIX only), so its count
# differs from the .out file. stdlib/archive_tar, stdlib/archive_zip_upgrade and
# stdlib/rar_links_demo extract symbolic and hard links and check their modes
# and sizes; a Windows account cannot create symlinks without a privilege and
# has no Unix permission bits, so the link results differ from the .out file. The same holds for
# stdlib/archive_safety, stdlib/archive_tar_stream and general/net_http_static_jail,
# which build symbolic links inside archives and web roots.
#
# Usage: tools/bash/native-host-skips.sh <macos-arm64|macos-x64|windows-x64|windows-arm64|linux-arm64>

case "${1:-}" in
macos-*) echo "" ;;
windows-*)
    echo "stdlib/os_process_fork stdlib/log_fatal stdlib/os_api stdlib/os_shell_demo stdlib/archive_tar"
    echo "stdlib/archive_zip_upgrade stdlib/rar_links_demo stdlib/archive_safety stdlib/archive_tar_stream"
    echo "general/net_http_static_jail"
    ;;
*) echo "" ;;
esac
