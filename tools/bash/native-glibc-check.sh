#!/bin/sh
# Checks dynamic glibc links by the built-in linker for a cross target.
#
# 1. Test programs built with --backend=native for <arch>-linux-gnu link
#    against the glibc in <libc> (a PIE on x86-64 and AArch64, a fixed-address
#    executable elsewhere) and run under the runner.
# 2. A Salam program calls into a C shared library through its PLT.
# 3. A C program built with and without -fPIC uses a variable and a
#    thread-local variable from a shared library, a function pointer to one of
#    its functions and thread-locals of its own: copy relocations, canonical
#    PLT addresses and the TLS relocations must all line up with the library.
#
# Needs clang and ld.lld for the shared libraries. Use `env` as the runner for
# i686, which runs on the host through the target's own loader.
#
# Usage: tools/bash/native-glibc-check.sh <salam> <arch> <libc> <runner> [groups...]
#   e.g. tools/bash/native-glibc-check.sh ./salam aarch64 /usr/aarch64-linux-gnu qemu-aarch64-static basics llvm

set -u
SALAM=$1
case "$SALAM" in /*) ;; */*) SALAM="$(pwd)/$SALAM" ;; esac
ARCH=$2
LIBC=$3
RUNNER=$4
shift 4
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
case "$ARCH" in
arm) TRIPLE=arm-linux-gnueabihf ;;
*) TRIPLE=$ARCH-linux-gnu ;;
esac
MARCH=
[ "$ARCH" = riscv64 ] && MARCH=-march=rv64gc
PIC=-fno-pic
case "$ARCH" in x86_64 | aarch64) PIC=-fPIE ;; esac
if [ "$RUNNER" = env ]; then
    RUN="$LIBC/lib/ld-linux.so.2 --library-path $LIBC/lib"
else
    RUN="$RUNNER -L $LIBC"
fi
fail=0
SALAM_LINKER=builtin sh "$ROOT/tools/bash/native-cross-check.sh" "$SALAM" "$TRIPLE" "$RUN" "$@" || fail=1
if ! command -v ld.lld >/dev/null 2>&1; then
    echo "native glibc check: no ld.lld, the shared library cases were skipped"
    exit "$fail"
fi
work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT
cd "$work" || exit 1
run() {
    if [ "$RUNNER" = env ]; then
        "$LIBC/lib/ld-linux.so.2" --library-path "$LIBC/lib:$work" "$@"
    else
        # shellcheck disable=SC2086
        $RUNNER -L "$LIBC" -E LD_LIBRARY_PATH="$work" "$@"
    fi
}
cat >lib.c <<'EOF'
int foo_counter = 40;
int foo_table[4] = {1, 2, 3, 4};
int foo_add(int a, int b) { return a + b + foo_counter; }
int foo_get(void) { return foo_counter + foo_table[3]; }
void *foo_self(void) { return (void *)foo_add; }
__thread int lib_tls = 7;
int lib_tls_get(void) { return lib_tls; }
EOF
cat >main.c <<'EOF'
int printf(const char *, ...);
extern int foo_counter;
extern int foo_table[4];
int foo_add(int, int);
int foo_get(void);
void *foo_self(void);
extern __thread int lib_tls;
int lib_tls_get(void);
__thread int my_tls = 3;
__thread long my_zero;
int (*fp)(int, int) = foo_add;
int main(void) {
    foo_counter += 5;
    foo_table[3] = 100;
    lib_tls += 30;
    my_tls *= 11;
    my_zero += 2;
    printf("%d %d %d %d %d %d %d %ld\n", foo_counter, foo_get(), fp(1, 2), fp == (int (*)(int, int))foo_self(), lib_tls, lib_tls_get(), my_tls, my_zero);
    return 0;
}
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
# shellcheck disable=SC2086
if ! clang --target="$TRIPLE" $MARCH -fPIC -shared -fuse-ld=lld -nostdlib lib.c -o libfoo.so; then
    echo "FAIL libfoo.so does not build"
    exit 1
fi
if ! SALAM_LINKER=builtin "$SALAM" build usefoo.salam --backend=native --release --target="$TRIPLE" \
    --output=usefoo --log-level=error >build.log 2>&1; then
    echo "FAIL salam calling a shared library: $(head -c 300 build.log)"
    fail=1
elif [ "$(run ./usefoo 2>&1)" != "43
70" ]; then
    echo "FAIL salam calling a shared library: $(run ./usefoo 2>&1 | head -3)"
    fail=1
fi
ln -s "$ROOT/compiler" compiler
cat >link.salam <<'EOF'
import os

include nl "compiler/nativelink/nativelink.salam"

func main:
    a := os.Args()
    mut l := nl.New()
    nl.SetDynamic(l, "")
    nl.SetSysroot(l, os.Env("SALAM_GLIBC_SYSROOT"))
    repeat a.len() - 2 in i:
        nl.AddInput(l, a.get(2 + i))
    end
    if not nl.Link(l, a.get(1)):
        printerrln nl.Error(l)
        ret 1
    end
    ret 0
end
EOF
if ! "$SALAM" build link.salam --output=link --log-level=error >build.log 2>&1; then
    cat build.log
    echo "native glibc check: the link driver did not build"
    exit 1
fi
for pic in "$PIC" -fPIC; do
    # shellcheck disable=SC2086
    if ! clang --target="$TRIPLE" $MARCH "$pic" -O1 -c main.c -o main.o; then
        echo "FAIL $pic: main.c does not compile"
        fail=1
        continue
    fi
    if ! ./link "prog$pic" "$LIBC/lib/Scrt1.o" "$LIBC/lib/crti.o" main.o ./libfoo.so "$LIBC/lib/libc.so" \
        "$LIBC/lib/crtn.o" >link.log 2>&1; then
        echo "FAIL $pic: $(head -c 300 link.log)"
        fail=1
        continue
    fi
    got=$(run "./prog$pic" 2>&1)
    if [ "$got" != "45 145 48 1 37 37 33 2" ]; then
        echo "FAIL $pic: got $got"
        fail=1
    fi
done
[ "$fail" = 0 ] && echo "native glibc check ($ARCH): shared library, copy relocation and TLS cases passed"
exit "$fail"
