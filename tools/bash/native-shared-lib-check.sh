#!/bin/sh
# Checks shared libraries built by the native backend and the built-in linker.
#
# Two Salam libraries are built with `--lib --backend=native` for the target:
# the mathlib test project and a larger one that uses strings, vectors, hash
# maps, libm and globals with initialisers. Each must be an ET_DYN with a
# DT_SONAME, no interpreter, NODELETE, its exported functions in .dynsym and
# no copy relocations. A C program and a Salam program then call into both
# libraries and must print the expected output. On the build machine itself
# the C program also runs under valgrind, loads the library with dlopen and
# dlclose, and gets the same output from the libraries built by the C backend.
#
# glibc targets read the C library from <libc> (default
# $SALAM_GLIBC_SYSROOT/usr/<triple>, else /usr/<triple>); musl targets from
# $SALAM_MUSL_SYSROOT, which must have libc.so and Scrt1.o. The C program for
# another target needs clang and ld.lld and is skipped without them.
#
# For a macOS triple the dylibs are checked with llvm-objdump and the code
# signature check, linked against by ld64.lld, and on x86-64 Linux loaded into
# the process by tools/linker/machodylib.py, which calls their exports.
#
# Usage: tools/bash/native-shared-lib-check.sh <salam> <triple|host> <runner> [libc]
#   e.g. tools/bash/native-shared-lib-check.sh ./salam host env
#        SALAM_MUSL_SYSROOT=/tmp/muslcc/sr-aarch64 \
#          tools/bash/native-shared-lib-check.sh ./salam aarch64-linux-musl qemu-aarch64-static

set -u
SALAM=$1
case "$SALAM" in /*) ;; */*) SALAM="$(pwd)/$SALAM" ;; esac
TRIPLE=$2
RUNNER=$3
LIBC=${4:-}
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT
fail=0
TFLAG=
ARCH=$(uname -m)
KIND=host
case "$TRIPLE" in
host) ;;
*-apple-*)
    KIND=macho
    TFLAG=--target=$TRIPLE
    ;;
*-linux-musl*)
    KIND=musl
    TFLAG=--target=$TRIPLE
    SR=${SALAM_MUSL_SYSROOT:?set SALAM_MUSL_SYSROOT}
    case "$SR" in /*) ;; *) SR="$(pwd)/$SR" ;; esac
    if [ ! -f "$SR/libc.so" ] || [ ! -f "$SR/Scrt1.o" ]; then
        echo "native shared lib check: $SR has no libc.so or Scrt1.o, skipped"
        exit 0
    fi
    ;;
*-linux-gnu*)
    KIND=glibc
    TFLAG=--target=$TRIPLE
    if [ -z "$LIBC" ]; then
        LIBC=/usr/$TRIPLE
        [ -n "${SALAM_GLIBC_SYSROOT:-}" ] && LIBC=$SALAM_GLIBC_SYSROOT/usr/$TRIPLE
    fi
    ;;
*)
    echo "native shared lib check: unsupported target $TRIPLE"
    exit 1
    ;;
esac
[ "$KIND" != host ] && ARCH=${TRIPLE%%-*}
[ "$ARCH" = arm64 ] && ARCH=aarch64
CT=$TRIPLE
MARCH=
LD=$ARCH
case "$ARCH" in
arm)
    LD=armhf
    [ "$KIND" = musl ] && CT=armv7a-linux-musleabihf
    ;;
i686) LD=i386 ;;
riscv64) MARCH=-march=rv64gc ;;
esac
if [ "$KIND" = musl ]; then
    INTERP=/lib/ld-musl-$LD.so.1
    CRT=$SR
    LIBCSO=$SR/libc.so
else
    case "$ARCH" in
    x86_64) INTERP=/lib64/ld-linux-x86-64.so.2 ;;
    aarch64) INTERP=/lib/ld-linux-aarch64.so.1 ;;
    i686) INTERP=/lib/ld-linux.so.2 ;;
    arm) INTERP=/lib/ld-linux-armhf.so.3 ;;
    riscv64) INTERP=/lib/ld-linux-riscv64-lp64d.so.1 ;;
    esac
    CRT=$LIBC/lib
    LIBCSO=$LIBC/lib/libc.so.6
fi

run() {
    case "$KIND" in
    host) LD_LIBRARY_PATH="$work/m:$work/r" "$@" ;;
    musl)
        if [ "$RUNNER" = env ]; then
            LD_LIBRARY_PATH="$work/m:$work/r" "$SR/libc.so" "$@"
        else
            mkdir -p "$work/pfx/lib"
            ln -sf "$SR/libc.so" "$work/pfx/lib/ld-musl-$LD.so.1"
            "$RUNNER" -L "$work/pfx" -E LD_LIBRARY_PATH="$work/m:$work/r" "$@"
        fi
        ;;
    glibc)
        if [ "$RUNNER" = env ] && [ "$ARCH" = i686 ]; then
            "$LIBC/lib/ld-linux.so.2" --library-path "$LIBC/lib:$work/m:$work/r" "$@"
        elif [ "$RUNNER" = env ]; then
            LD_LIBRARY_PATH="$work/m:$work/r" "$@"
        else
            "$RUNNER" -L "$LIBC" -E LD_LIBRARY_PATH="$work/m:$work/r" "$@"
        fi
        ;;
    esac
}

cp -R "$ROOT/tests/en/lib/mathlib" "$work/m"
mkdir -p "$work/r"
cat >"$work/r/salam.toml" <<'EOF'
[package]
name = "richlib"
EOF
cat >"$work/r/main.salam" <<'EOF'
import math
import str

mut g_words := Vector {} as Vector<str>
mut g_counts := HashMap {} as HashMap<str, int>
mut g_total := 0
g_label := "rich" + "lib"
g_scale := math.Sqrt(16.0)

func remember(w: str):
    g_words.push(w)
    if g_counts.has(w):
        g_counts.put(w, g_counts.get(w) + 1)
    else:
        g_counts.put(w, 1)
    end
    g_total += 1
end

export:
    func rich_add_word(w: str): int:
        remember(str.Upper(w))
        ret g_total
    end

    func rich_count(w: str): int:
        u := str.Upper(w)
        if g_counts.has(u):
            ret g_counts.get(u)
        end
        ret 0
    end

    func rich_join(): str:
        mut out := g_label + ":"
        each w in g_words:
            out = out + " " + w
        end
        ret out
    end

    func rich_hyp(a: f64, b: f64): f64:
        ret math.Sqrt(a * a + b * b) * g_scale
    end

    func rich_fib(n: int): int:
        mut memo := Vector {} as Vector<int>
        repeat n + 1 in i:
            if i < 2:
                memo.push(i)
            else:
                memo.push(memo[i - 1] + memo[i - 2])
            end
        end
        ret memo[n]
    end

    func rich_report():
        println "words=" + g_total
        each w in g_words:
            println "  " + w
        end
    end
end
EOF
cat >"$work/host.c" <<'EOF'
int printf(const char *, ...);
int fflush(void *);
int salam_add(int, int);
int salam_calls(void);
void salam_greet(void);
int rich_add_word(const char *);
int rich_count(const char *);
const char *rich_join(void);
double rich_hyp(double, double);
int rich_fib(int);
void rich_report(void);
int main(void)
{
    int a = salam_add(40, 2);
    int b = salam_add(1, 1);
    printf("%d %d %d\n", a, b, salam_calls());
    a = rich_add_word("alpha");
    b = rich_add_word("beta");
    printf("%d %d %d\n", a, b, rich_add_word("Alpha"));
    printf("%d %d %d\n", rich_count("ALPHA"), rich_count("beta"), rich_count("gamma"));
    printf("%s\n", rich_join());
    printf("%.3f %d\n", rich_hyp(3.0, 4.0), rich_fib(40));
    fflush(0);
    salam_greet();
    return 0;
}
EOF
cat >"$work/use.salam" <<EOF
link dynamic "$work/m/libmathlib.so"
link dynamic "$work/r/librichlib.so"

extern:
    func salam_add(a: int, b: int): int
    func salam_calls(): int
    func salam_greet()
    func rich_add_word(w: str): int
    func rich_join(): str
    func rich_fib(n: int): int
end

func main:
    println salam_add(40, 2) + salam_add(1, 1) + salam_calls()
    println rich_add_word("alpha") + rich_add_word("beta")
    println rich_join()
    println rich_fib(40)
    salam_greet()
end
EOF
cat >"$work/dl.c" <<'EOF'
#include <dlfcn.h>
#include <stdio.h>
int main(int argc, char **argv)
{
    if (argc < 2) return 2;
    for (int round = 0; round < 2; round++) {
        void *h = dlopen(argv[1], RTLD_NOW);
        if (!h) { printf("dlopen: %s\n", dlerror()); return 1; }
        int (*add)(int, int) = (int (*)(int, int))dlsym(h, "salam_add");
        int (*calls)(void) = (int (*)(void))dlsym(h, "salam_calls");
        void (*greet)(void) = (void (*)(void))dlsym(h, "salam_greet");
        if (!add || !calls || !greet) { printf("dlsym failed\n"); return 1; }
        int r = add(2, 3);
        printf("%d %d\n", r, calls());
        fflush(stdout);
        greet();
        if (dlclose(h) != 0) { printf("dlclose: %s\n", dlerror()); return 1; }
    }
    printf("closed\n");
    return 0;
}
EOF
want_c="42 2 2
1 2 3
2 1 0
richlib: ALPHA BETA ALPHA
20.000 102334155
hello from salam"
want_s="46
3
richlib: ALPHA BETA
102334155
hello from salam"

build_lib() {
    dir=$1
    backend=$2
    # shellcheck disable=SC2086
    if ! (cd "$dir" && SALAM_LINKER=builtin "$SALAM" build --lib --backend="$backend" $TFLAG --log-level=error) >"$work/build.log" 2>&1; then
        echo "FAIL $TRIPLE: $(basename "$dir") library does not build with --backend=$backend: $(head -c 400 "$work/build.log")"
        return 1
    fi
    return 0
}

check_shape() {
    so=$1
    shift
    name=$(basename "$so")
    if ! readelf -hW "$so" | grep -q "DYN (Shared object"; then
        echo "FAIL $TRIPLE: $name is not ET_DYN"
        fail=1
    fi
    if readelf -lW "$so" | grep -q INTERP; then
        echo "FAIL $TRIPLE: $name has an interpreter"
        fail=1
    fi
    dyn=$(readelf -dW "$so")
    case "$dyn" in *"Library soname: [$name]"*) ;; *)
        echo "FAIL $TRIPLE: $name has no DT_SONAME $name"
        fail=1
        ;;
    esac
    case "$dyn" in *NODELETE*) ;; *)
        echo "FAIL $TRIPLE: $name is not NODELETE"
        fail=1
        ;;
    esac
    if [ "$ARCH" != i686 ] && printf '%s\n' "$dyn" | grep -q TEXTREL; then
        echo "FAIL $TRIPLE: $name has text relocations"
        fail=1
    fi
    if readelf -rW "$so" | grep -q "_COPY"; then
        echo "FAIL $TRIPLE: $name has copy relocations"
        fail=1
    fi
    syms=$(readelf --dyn-syms -W "$so")
    for fn in "$@"; do
        if ! printf '%s\n' "$syms" | grep -E " FUNC +GLOBAL +DEFAULT +[0-9]+ $fn\$" >/dev/null; then
            echo "FAIL $TRIPLE: $name does not export $fn"
            fail=1
        fi
    done
}

build_lib "$work/m" native || exit 1
build_lib "$work/r" native || exit 1
if [ "$KIND" = macho ]; then
    for so in "$work/m/libmathlib.dylib" "$work/r/librichlib.dylib"; do
        name=$(basename "$so")
        if ! command -v llvm-objdump >/dev/null 2>&1; then
            echo "native shared lib check ($TRIPLE): no llvm-objdump, the structure checks were skipped"
            break
        fi
        hdr=$(llvm-objdump --macho --private-headers "$so")
        case "$hdr" in *" DYLIB "*) ;; *)
            echo "FAIL $TRIPLE: $name is not MH_DYLIB"
            fail=1
            ;;
        esac
        case "$hdr" in *"name @rpath/$name "*) ;; *)
            echo "FAIL $TRIPLE: $name has no LC_ID_DYLIB @rpath/$name"
            fail=1
            ;;
        esac
        case "$hdr" in *LC_MAIN* | *LC_LOAD_DYLINKER* | *__PAGEZERO*)
            echo "FAIL $TRIPLE: $name has executable load commands"
            fail=1
            ;;
        esac
        trie=$(llvm-objdump --macho --exports-trie "$so")
        for fn in salam_add rich_join rich_fib; do
            case "$name:$fn" in libmathlib*:rich_* | librichlib*:salam_add) continue ;; esac
            if ! printf '%s\n' "$trie" | grep -q " _$fn\$"; then
                echo "FAIL $TRIPLE: $name does not export _$fn"
                fail=1
            fi
        done
        if ! python3 -I "$ROOT/tools/linker/macho_check.py" sig "$so" >"$work/sig.log" 2>&1; then
            echo "FAIL $TRIPLE: $name has a bad code signature: $(head -c 300 "$work/sig.log")"
            fail=1
        fi
    done
    if command -v clang >/dev/null 2>&1 && command -v ld64.lld >/dev/null 2>&1; then
        march=$ARCH
        [ "$ARCH" = aarch64 ] && march=arm64
        if clang --target="$march-apple-macos11" -c "$work/host.c" -o "$work/hm.o" 2>"$work/cc.log" &&
            ld64.lld -arch "$march" -platform_version macos 11.0 11.0 "$work/hm.o" "$work/m/libmathlib.dylib" \
                "$work/r/librichlib.dylib" -undefined dynamic_lookup -o "$work/hm" 2>>"$work/cc.log"; then
            binds=$(llvm-nm -m "$work/hm")
            for pair in salam_add:libmathlib rich_join:librichlib; do
                if ! printf '%s\n' "$binds" | grep -q "_${pair%%:*} (from ${pair#*:})"; then
                    echo "FAIL $TRIPLE: ld64.lld does not bind _${pair%%:*} to ${pair#*:}"
                    fail=1
                fi
            done
        else
            echo "FAIL $TRIPLE: ld64.lld does not link a program against the dylibs: $(head -c 300 "$work/cc.log")"
            fail=1
        fi
    fi
    if [ "$ARCH" = x86_64 ] && [ "$(uname -s)-$(uname -m)" = Linux-x86_64 ]; then
        got=$(python3 -I "$ROOT/tools/linker/machodylib.py" "$work/m/libmathlib.dylib" i:_salam_add:ii:40,2 \
            i:_salam_add:ii:1,1 i:_salam_calls:: v:_salam_greet:: 2>&1 | tr '\n' ' ')
        if [ "$got" != "42 2 2 hello from salam " ]; then
            echo "FAIL $TRIPLE: libmathlib.dylib loaded on Linux printed: $got"
            fail=1
        fi
        got=$(python3 -I "$ROOT/tools/linker/machodylib.py" "$work/r/librichlib.dylib" i:_rich_add_word:s:alpha \
            i:_rich_add_word:s:Alpha i:_rich_count:s:ALPHA s:_rich_join:: d:_rich_hyp:dd:3,4 i:_rich_fib:i:40 \
            v:_rich_report:: 2>&1 | tr '\n' ' ')
        if [ "$got" != "1 2 2 richlib: ALPHA ALPHA 20.000 102334155 words=2   ALPHA   ALPHA " ]; then
            echo "FAIL $TRIPLE: librichlib.dylib loaded on Linux printed: $got"
            fail=1
        fi
    fi
    [ "$fail" = 0 ] && echo "native shared lib check ($TRIPLE): dylibs passed"
    exit "$fail"
fi
check_shape "$work/m/libmathlib.so" salam_add salam_calls salam_greet
check_shape "$work/r/librichlib.so" rich_add_word rich_count rich_join rich_hyp rich_fib rich_report

# shellcheck disable=SC2086
if ! (cd "$work" && SALAM_LINKER=builtin "$SALAM" build use.salam --backend=native $TFLAG --output=use --log-level=error) >"$work/build.log" 2>&1; then
    echo "FAIL $TRIPLE: a Salam program does not link against the libraries: $(head -c 400 "$work/build.log")"
    fail=1
else
    got=$(run "$work/use" 2>&1)
    if [ "$got" != "$want_s" ]; then
        echo "FAIL $TRIPLE: Salam program calling the libraries printed:"
        printf '%s\n' "$got" | head -12 | sed 's/^/  /'
        fail=1
    fi
fi

hostc=
if [ "$KIND" = host ]; then
    if cc "$work/host.c" -L"$work/m" -L"$work/r" -lmathlib -lrichlib -o "$work/hostc" 2>"$work/cc.log"; then
        hostc=$work/hostc
    else
        echo "FAIL host: the C program does not link: $(head -c 300 "$work/cc.log")"
        fail=1
    fi
elif command -v clang >/dev/null 2>&1 && command -v ld.lld >/dev/null 2>&1; then
    extra=
    [ "$KIND" = glibc ] && [ -f "$LIBC/lib/libc_nonshared.a" ] && extra=$LIBC/lib/libc_nonshared.a
    # shellcheck disable=SC2086
    if clang --target="$CT" $MARCH -fPIE -O1 -c "$work/host.c" -o "$work/host.o" 2>"$work/cc.log" &&
        ld.lld -pie --dynamic-linker="$INTERP" "$CRT/Scrt1.o" "$CRT/crti.o" "$work/host.o" -L"$work/m" -L"$work/r" \
            -lmathlib -lrichlib "$LIBCSO" $extra "$CRT/crtn.o" -o "$work/hostc" 2>>"$work/cc.log"; then
        hostc=$work/hostc
    else
        echo "FAIL $TRIPLE: the C program does not link: $(head -c 300 "$work/cc.log")"
        fail=1
    fi
else
    echo "native shared lib check ($TRIPLE): no clang or ld.lld, the C program was skipped"
fi
if [ -n "$hostc" ]; then
    got=$(run "$hostc" 2>&1)
    if [ "$got" != "$want_c" ]; then
        echo "FAIL $TRIPLE: C program calling the libraries printed:"
        printf '%s\n' "$got" | head -12 | sed 's/^/  /'
        fail=1
    fi
fi

if [ "$KIND" = host ] && [ -n "$hostc" ]; then
    if command -v valgrind >/dev/null 2>&1; then
        if ! run valgrind -q --error-exitcode=9 "$hostc" >/dev/null 2>"$work/vg.log"; then
            echo "FAIL host: valgrind reports errors in the C program:"
            head -20 "$work/vg.log" | sed 's/^/  /'
            fail=1
        fi
    fi
    if cc "$work/dl.c" -o "$work/dl" -ldl 2>"$work/cc.log"; then
        got=$("$work/dl" "$work/m/libmathlib.so" 2>&1)
        if [ "$got" != "5 1
5 2
hello from salam
hello from salam
closed" ]; then
            echo "FAIL host: dlopen and dlclose of the library printed:"
            printf '%s\n' "$got" | head -8 | sed 's/^/  /'
            fail=1
        fi
    fi
    mkdir -p "$work/c"
    cp -R "$work/r" "$work/c/r"
    rm -f "$work/c/r/librichlib.so"
    cat >"$work/hostr.c" <<'EOF'
int printf(const char *, ...);
int fflush(void *);
int rich_add_word(const char *);
int rich_count(const char *);
const char *rich_join(void);
double rich_hyp(double, double);
int rich_fib(int);
void rich_report(void);
int main(void)
{
    int a = rich_add_word("one");
    printf("%d %d\n", a, rich_count("ONE"));
    printf("%s %.3f %d\n", rich_join(), rich_hyp(6.0, 8.0), rich_fib(30));
    fflush(0);
    rich_report();
    return 0;
}
EOF
    if build_lib "$work/c/r" c && cc "$work/hostr.c" -L"$work/r" -lrichlib -o "$work/hostr" 2>"$work/cc.log"; then
        mine=$(LD_LIBRARY_PATH="$work/r" "$work/hostr" 2>&1)
        ref=$(LD_LIBRARY_PATH="$work/c/r" "$work/hostr" 2>&1)
        if [ "$mine" != "$ref" ]; then
            echo "FAIL host: the library built by the C backend printed something else:"
            printf '%s\n' "$ref" | head -12 | sed 's/^/  /'
            fail=1
        fi
    else
        echo "FAIL host: the C backend comparison does not build: $(head -c 300 "$work/build.log" "$work/cc.log")"
        fail=1
    fi
fi
[ "$fail" = 0 ] && echo "native shared lib check ($TRIPLE): libraries, C and Salam callers passed"
exit "$fail"
