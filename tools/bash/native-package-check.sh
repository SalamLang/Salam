#!/bin/sh
# Smoke-tests a package from tools/bash/package-native.sh the way a user would
# run it: the archive is unpacked into an empty folder, SALAM_STD is unset so
# the compiler has to find the std/ next to itself, and it builds a few test
# programs with --backend=native on this machine and runs them (through
# tools/bash/native-host-check.sh), including the TCP and HTTP loopback tests
# and the TLS self-test. NATIVE_PACKAGE_SKIP lists test names to leave out.
#
# Usage: tools/bash/native-package-check.sh <salam-native-...tar.gz> [file.salam...]

set -u
TARBALL=$1
case "$TARBALL" in /*) ;; *) TARBALL="$(pwd)/$TARBALL" ;; esac
shift
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
if [ $# -eq 0 ]; then
    T=$ROOT/tests/en
    set -- "$T/llvm/ll_fib.salam" "$T/llvm/ll_hashmap.salam" "$T/llvm/ll_float.salam" \
        "$T/general/closure_env_drop.salam" "$T/stdlib/json_value_demo.salam" "$T/general/net_loopback.salam" \
        "$T/general/net_http_event_loop.salam" "$T/stdlib/tls_selftest.salam"
fi
work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT
tar -xzf "$TARBALL" -C "$work"
salam=$(find "$work" -maxdepth 2 \( -name salam -o -name salam.exe \) -type f | head -1)
if [ -z "$salam" ]; then
    echo "native package check: no salam in $TARBALL" >&2
    exit 1
fi
unset SALAM_STD
cd "$work" || exit 1
if ! "$salam" version; then
    echo "native package check: the packaged compiler does not run" >&2
    exit 1
fi
n=$#
for f in "$@"; do
    case " ${NATIVE_PACKAGE_SKIP:-} " in
    *" $(basename "$f" .salam) "*) ;;
    *) set -- "$@" "$f" ;;
    esac
done
shift "$n"
NATIVE_HOST_FLAGS=--backend=native sh "$ROOT/tools/bash/native-host-check.sh" "$salam" "$@"
