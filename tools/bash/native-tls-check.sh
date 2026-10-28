#!/bin/sh
# Checks thread-local storage in the built-in linker. A small C program with
# initialized, zeroed, extern, static and over-aligned __thread variables and
# a second thread is compiled with -fno-pic (local-exec and initial-exec) and
# with -fPIC (general- and local-dynamic, TLSDESC on AArch64), linked against
# the musl sysroot by compiler/nativelink and run: both threads must see their
# own copies, initialized from .tdata and zeroed from .tbss.
#
# Needs clang. Usage: tools/bash/native-tls-check.sh <salam> <arch> <runner>
#   e.g. SALAM_MUSL_SYSROOT=/tmp/muslcc/sr-aarch64 \
#        tools/bash/native-tls-check.sh ./salam aarch64 qemu-aarch64-static

set -u
SALAM=$1
case "$SALAM" in /*) ;; */*) SALAM="$(pwd)/$SALAM" ;; esac
ARCH=$2
RUNNER=$3
SR=${SALAM_MUSL_SYSROOT:?set SALAM_MUSL_SYSROOT to the musl sysroot for the target}
case "$SR" in /*) ;; *) SR="$(pwd)/$SR" ;; esac
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
case "$ARCH" in
x86_64) TRIPLE=x86_64-linux-musl ;;
i686) TRIPLE=i686-linux-musl ;;
aarch64) TRIPLE=aarch64-linux-musl ;;
arm) TRIPLE=armv7a-linux-musleabihf ;;
riscv64) TRIPLE="riscv64-linux-musl -march=rv64gc" ;;
*)
    echo "native tls check: no cases for $ARCH"
    exit 2
    ;;
esac
work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT
cd "$work" || exit 1
ln -s "$ROOT/compiler" compiler
cat >link.salam <<'EOF'
import os

include nl "compiler/nativelink/nativelink.salam"

func main:
    a := os.Args()
    mut l := nl.New()
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
    echo "native tls check: the link driver did not build"
    exit 1
fi
cat >m.c <<'EOF'
typedef unsigned long pthread_t;
int pthread_create(pthread_t *, const void *, void *(*)(void *), void *);
int pthread_join(pthread_t, void **);
int printf(const char *, ...);
__thread int tl_def = 5;
__thread long tl_bss;
extern __thread int tl_ext;
static __thread int tl_loc = 3;
__thread char big[100] = {1, 2, 3};
__thread double dd __attribute__((aligned(64))) = 2.5;
int get_ext(void);
static void *worker(void *arg) {
    (void)arg;
    tl_def = 1000; tl_bss = 2000; tl_ext = 3000; tl_loc = 4000; big[50] = 77; dd = 9.0;
    printf("thread %d %ld %d %d %d %d %g %d\n", tl_def, tl_bss, tl_ext, tl_loc, big[1], big[50], dd, ((long)&dd & 63) == 0);
    return 0;
}
int main(void) {
    tl_def += 1; tl_bss = 7; tl_ext += 10; tl_loc *= 4; big[50] = 9; dd *= 2;
    pthread_t t;
    pthread_create(&t, 0, worker, 0);
    pthread_join(t, 0);
    printf("main %d %ld %d %d %d %d %g %d %d\n", tl_def, tl_bss, tl_ext, tl_loc, big[2], big[50], dd, get_ext(), ((long)&dd & 63) == 0);
    return 0;
}
EOF
cat >e.c <<'EOF'
__thread int tl_ext = 100;
int get_ext(void) { return tl_ext; }
EOF
want="thread 1000 2000 3000 4000 2 77 9 1
main 6 7 110 12 3 9 5 110 1"
ok=0
bad=0
for pic in -fno-pic -fPIC; do
    # shellcheck disable=SC2086
    if ! clang --target=$TRIPLE "$pic" -O1 -c m.c -o m.o || ! clang --target=$TRIPLE "$pic" -O1 -c e.c -o e.o; then
        echo "FAIL $pic: does not compile"
        bad=$((bad + 1))
        continue
    fi
    if ! ./link "tls$pic" "$SR/crt1.o" "$SR/crti.o" "$SR/crtbeginT.o" m.o e.o "$SR/libc.a" "$SR/libgcc.a" \
        "$SR/libgcc_eh.a" "$SR/crtend.o" "$SR/crtn.o" >link.log 2>&1; then
        echo "FAIL $pic: $(head -c 300 link.log)"
        bad=$((bad + 1))
        continue
    fi
    # shellcheck disable=SC2086
    got=$($RUNNER "./tls$pic" 2>&1)
    if [ "$got" = "$want" ]; then
        ok=$((ok + 1))
    else
        echo "FAIL $pic: got"
        echo "$got"
        bad=$((bad + 1))
    fi
done
echo "native tls check ($ARCH): $ok ok, $bad failed"
[ "$bad" = 0 ]
