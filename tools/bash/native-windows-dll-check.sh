#!/bin/sh
# Builds tests/en/lib/mathlib as a Windows DLL with the native backend and the
# built-in linker, then checks it the way a user would consume it:
#
#   - the export table lists the library's functions,
#   - a C program linked by mingw gcc against the import library
#     (libmathlib.dll.a) prints mathlib.out under wine,
#   - a Salam program linked by the built-in linker against the same import
#     library prints the same,
#   - a second copy of the DLL loaded with LoadLibrary next to the first one
#     has to be rebased (both ask for the same image base), so its base
#     relocations and the library's start-up code are exercised.
#
# Needs wine on PATH and the mingw-w64 runtime for the target. The steps that
# need a cross C compiler or llvm-readobj are skipped when it is missing.
# NATIVE_WIN_TRIPLE picks the target (x86_64-w64-windows-gnu by default;
# i686-w64-windows-gnu also runs under wine).
#
# Usage: tools/bash/native-windows-dll-check.sh <salam>

set -u
SALAM=$1
case "$SALAM" in /*) ;; */*) SALAM="$(pwd)/$SALAM" ;; esac
TRIPLE=${NATIVE_WIN_TRIPLE:-x86_64-w64-windows-gnu}
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
LIB="$ROOT/tests/en/lib"
case "$TRIPLE" in
    i686-*) CC=i686-w64-mingw32-gcc ;;
    aarch64-*) CC=aarch64-w64-mingw32-clang ;;
    *) CC=x86_64-w64-mingw32-gcc ;;
esac
work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT
export WINEDEBUG=-all
pass=0
fail=0

ok() {
    echo "ok   $1"
    pass=$((pass + 1))
}

bad() {
    echo "FAIL $1"
    fail=$((fail + 1))
}

same() {
    [ "$(tr -d '\r' <"$1")" = "$(tr -d '\r' <"$2")" ]
}

cp -r "$LIB/mathlib" "$work/lib"
if (cd "$work/lib" && "$SALAM" build . --lib --backend=native --linker=builtin \
    --target="$TRIPLE" --log-level=error >"$work/build.log" 2>&1) &&
    [ -f "$work/lib/mathlib.dll" ] && [ -f "$work/lib/libmathlib.dll.a" ] && [ -f "$work/lib/mathlib.h" ]; then
    ok "build mathlib.dll, libmathlib.dll.a and mathlib.h"
else
    bad "build"
    head -5 "$work/build.log"
    echo "dll check: $pass passed, $fail failed"
    exit 1
fi

if command -v llvm-readobj >/dev/null 2>&1; then
    llvm-readobj --file-headers --coff-exports "$work/lib/mathlib.dll" >"$work/hdr.txt" 2>&1
    if grep -q "IMAGE_FILE_DLL" "$work/hdr.txt" && grep -q "Name: salam_add" "$work/hdr.txt" &&
        grep -q "Name: salam_calls" "$work/hdr.txt" && grep -q "Name: salam_greet" "$work/hdr.txt"; then
        ok "DLL header and export table"
    else
        bad "DLL header and export table"
        grep -E "Characteristics|Name:" "$work/hdr.txt" | head -10
    fi
fi

case "$TRIPLE" in
    aarch64-*) run=false ;;
    *) run=true ;;
esac

if command -v "$CC" >/dev/null 2>&1; then
    if "$CC" -I"$work/lib" "$LIB/mathlib.c" -o "$work/lib/host.exe" -L"$work/lib" -lmathlib >"$work/cc.log" 2>&1; then
        if ! $run; then
            ok "C host links against the import library"
        elif (cd "$work/lib" && timeout 60 wine ./host.exe </dev/null >"$work/host.out" 2>/dev/null) &&
            same "$work/host.out" "$LIB/mathlib.out"; then
            ok "C host linked by $CC runs"
        else
            bad "C host linked by $CC"
            diff "$LIB/mathlib.out" "$work/host.out" | head -6
        fi
    else
        bad "C host does not link"
        head -5 "$work/cc.log"
    fi
fi

mkdir -p "$work/sal"
cat >"$work/sal/main.salam" <<'EOF'
link dynamic "mathlib"

extern:
    func salam_add(a: int, b: int): int
    func salam_calls(): int
    func salam_greet()
end

func main():
    println salam_add(40, 2)
    println salam_add(1, 1)
    println salam_calls()
    salam_greet()
end
EOF
if (cd "$work/sal" && "$SALAM" build main.salam --backend=native --linker=builtin --target="$TRIPLE" \
    --libpath="$work/lib" --output="$work/lib/salhost.exe" --log-level=error >"$work/sal.log" 2>&1); then
    if ! $run; then
        ok "Salam host links against the import library"
    elif (cd "$work/lib" && timeout 60 wine ./salhost.exe </dev/null >"$work/sal.out" 2>/dev/null) &&
        same "$work/sal.out" "$LIB/mathlib.out"; then
        ok "Salam host linked by the built-in linker runs"
    else
        bad "Salam host"
        diff "$LIB/mathlib.out" "$work/sal.out" | head -6
    fi
else
    bad "Salam host does not link"
    head -5 "$work/sal.log"
fi

if $run && command -v "$CC" >/dev/null 2>&1; then
    cp "$work/lib/mathlib.dll" "$work/lib/mathcopy.dll"
    cat >"$work/dyn.c" <<'EOF'
#include <stdio.h>
#include <windows.h>

typedef int (*add_fn)(int, int);
typedef void (*greet_fn)(void);

int main(void)
{
    HMODULE a = LoadLibraryA("mathlib.dll");
    HMODULE b = LoadLibraryA("mathcopy.dll");
    if (!a || !b || a == b)
        return 1;
    add_fn add = (add_fn)GetProcAddress(b, "salam_add");
    greet_fn greet = (greet_fn)GetProcAddress(b, "salam_greet");
    if (!add || !greet)
        return 2;
    printf("%d\n", add(40, 2));
    fflush(stdout);
    greet();
    return 0;
}
EOF
    printf '42\nhello from salam\n' >"$work/dyn.exp"
    if "$CC" "$work/dyn.c" -o "$work/lib/dyn.exe" >"$work/dyn.log" 2>&1 &&
        (cd "$work/lib" && timeout 60 wine ./dyn.exe </dev/null >"$work/dyn.out" 2>/dev/null) &&
        same "$work/dyn.out" "$work/dyn.exp"; then
        ok "a rebased second copy of the DLL runs"
    else
        bad "rebased second copy"
        head -5 "$work/dyn.log" "$work/dyn.out"
    fi
fi

echo "dll check ($TRIPLE): $pass passed, $fail failed"
[ "$fail" -eq 0 ]
