#!/bin/sh
# Checks dynamic musl links by the built-in linker: a Salam program that links
# a C shared library (built with clang and ld.lld) is linked against musl's
# libc.so instead of libc.a, with /lib/ld-musl-<arch>.so.1 as its
# interpreter, and must call into the library when run.
#
# Usage: SALAM_MUSL_SYSROOT=<sysroot with libc.so and Scrt1.o> \
#        tools/bash/native-musl-dyn-check.sh <salam> <arch> <runner>
#   Use `env` as the runner for a target the host runs directly.

set -u
SALAM=$1
case "$SALAM" in /*) ;; */*) SALAM="$(pwd)/$SALAM" ;; esac
ARCH=$2
RUNNER=$3
SR=${SALAM_MUSL_SYSROOT:?set SALAM_MUSL_SYSROOT}
case "$SR" in /*) ;; *) SR="$(pwd)/$SR" ;; esac
if [ ! -f "$SR/libc.so" ] || [ ! -f "$SR/Scrt1.o" ]; then
    echo "native musl dyn check: $SR has no libc.so or Scrt1.o, skipped"
    exit 0
fi
if ! command -v ld.lld >/dev/null 2>&1; then
    echo "native musl dyn check: no ld.lld, skipped"
    exit 0
fi
case "$ARCH" in
arm)
    TRIPLE=arm-linux-musleabihf
    CTRIPLE=armv7a-linux-musleabihf
    LD=armhf
    ;;
i686)
    TRIPLE=i686-linux-musl
    CTRIPLE=$TRIPLE
    LD=i386
    ;;
*)
    TRIPLE=$ARCH-linux-musl
    CTRIPLE=$TRIPLE
    LD=$ARCH
    ;;
esac
MARCH=
[ "$ARCH" = riscv64 ] && MARCH=-march=rv64gc
work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT
cd "$work" || exit 1
mkdir -p pfx/lib
ln -s "$SR/libc.so" "pfx/lib/ld-musl-$LD.so.1"
cat >foo.c <<'EOF'
int foo_counter = 40;
int foo_add(int a, int b) { return a + b + foo_counter; }
EOF
cat >usefoo.salam <<EOF
link dynamic "$work/libfoo.so"

extern:
    func foo_add(a: int, b: int): int
end

func main:
    println foo_add(1, 2)
    println foo_add(10, 20)
end
EOF
# shellcheck disable=SC2086
if ! clang --target="$CTRIPLE" $MARCH -fPIC -shared -fuse-ld=lld -nostdlib foo.c -o libfoo.so; then
    echo "FAIL libfoo.so does not build"
    exit 1
fi
if ! SALAM_LINKER=builtin "$SALAM" build usefoo.salam --backend=native --release --target="$TRIPLE" \
    --output=usefoo --log-level=error >build.log 2>&1; then
    echo "FAIL: $(head -c 300 build.log)"
    exit 1
fi
if [ "$RUNNER" = env ]; then
    got=$(LD_LIBRARY_PATH="$work" "pfx/lib/ld-musl-$LD.so.1" ./usefoo 2>&1)
else
    # shellcheck disable=SC2086
    got=$($RUNNER -L "$work/pfx" -E LD_LIBRARY_PATH="$work" ./usefoo 2>&1)
fi
if [ "$got" != "43
70" ]; then
    echo "FAIL: got $got"
    exit 1
fi
echo "native musl dyn check ($ARCH): ok"
