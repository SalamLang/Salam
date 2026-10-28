#!/bin/sh
# Checks dynamic glibc links by the built-in linker for a cross target: test
# programs built with --backend=native for <arch>-linux-gnu must link as PIE
# against the glibc in <sysroot> and run under the runner, and a program that
# calls into a C shared library (built with clang and ld.lld, when they are
# there) must find it through its PLT.
#
# Usage: tools/bash/native-glibc-check.sh <salam> <arch> <sysroot> <runner> [groups...]
#   e.g. tools/bash/native-glibc-check.sh ./salam aarch64 /usr/aarch64-linux-gnu qemu-aarch64-static basics llvm

set -u
SALAM=$1
case "$SALAM" in /*) ;; */*) SALAM="$(pwd)/$SALAM" ;; esac
ARCH=$2
LIBC=$3
RUNNER=$4
shift 4
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
fail=0
export SALAM_GLIBC_SYSROOT="$LIBC"
SALAM_LINKER=builtin sh "$ROOT/tools/bash/native-cross-check.sh" "$SALAM" "$ARCH-linux-gnu" "$RUNNER -L $LIBC" "$@" || fail=1
if ! command -v ld.lld >/dev/null 2>&1; then
    echo "native glibc check: no ld.lld, the shared library case was skipped"
    exit "$fail"
fi
work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT
cd "$work" || exit 1
cat >foo.c <<'EOF'
int foo_counter = 40;
int foo_add(int a, int b) { return a + b + foo_counter; }
EOF
cat >usefoo.salam <<'EOF'
link dynamic "./libfoo.so"

extern:
    func foo_add(a: int, b: int): int
end

func main:
    println foo_add(1, 2)
    println foo_add(10, 20)
end
EOF
if ! clang --target="$ARCH-linux-gnu" -fPIC -shared -fuse-ld=lld -nostdlib foo.c -o libfoo.so; then
    echo "FAIL shared library: libfoo.so does not build"
    exit 1
fi
if ! SALAM_LINKER=builtin "$SALAM" build usefoo.salam --backend=native --release --target="$ARCH-linux-gnu" \
    --output=usefoo --log-level=error >build.log 2>&1; then
    echo "FAIL shared library: $(head -c 300 build.log)"
    exit 1
fi
# shellcheck disable=SC2086
if ! got=$($RUNNER -L "$LIBC" -E LD_LIBRARY_PATH="$work" ./usefoo 2>&1); then
    echo "FAIL shared library: the program exited with an error: $got"
    exit 1
fi
if [ "$got" != "43
70" ]; then
    echo "FAIL shared library: got $got"
    exit 1
fi
echo "native glibc check: the shared library case passed"
exit "$fail"
