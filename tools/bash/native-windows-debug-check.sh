#!/bin/sh
# Checks the native backend's -g output for Windows targets.
#
# For x86-64 and i686 Windows, with each linker, it builds a small program
# with and without -g and requires that the two .text sections are
# byte-identical, that the -g build runs under wine with the same output,
# that the line table and DW_TAG_subprogram entries name the program's own
# source and functions, and, on i686 (which has no .pdata to unwind with),
# that .debug_frame has FDEs and each one lies inside .text. Only binutils
# objdump is needed, since llvm-dwarfdump --verify does not finish on i686
# PE files linked against the mingw-w64 CRT.
#
# Usage: tools/bash/native-windows-debug-check.sh <salam>

set -u
SALAM=$1
case "$SALAM" in /*) ;; */*) SALAM="$(pwd)/$SALAM" ;; esac
work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT
cat >"$work/wd.salam" <<'SRC'
struct Pt:
    pub x: int = 0
    pub y: int = 0
end

noinline func area(p: Pt, scale: f64): f64:
    w := p.x * 2
    h := p.y + 1
    ret ((w * h) as f64) * scale
end

noinline func walk(n: int, name: str): int:
    mut s := 0
    mut k := 3
    repeat n in i:
        s += i * k
        k += 1
    end
    println name
    ret s
end

func main:
    p := Pt { x = 3, y = 4 }
    println area(p, 1.5)
    println walk(5, "hello")
end
SRC
fail=0
pass=0
bad() {
    echo "FAIL $1"
    fail=$((fail + 1))
}
for triple in x86_64-w64-windows-gnu i686-w64-windows-gnu; do
    for lk in builtin system; do
        tag="$triple $lk"
        d="$work/$triple.$lk"
        mkdir -p "$d"
        if ! (cd "$d" && SALAM_LINKER=$lk "$SALAM" build ../wd.salam --backend=native --target="$triple" --output=plain.exe --log-level=error >log 2>&1); then
            bad "$tag (native build failed)"
            sed -n 1,5p "$d/log"
            continue
        fi
        if ! (cd "$d" && SALAM_LINKER=$lk "$SALAM" build ../wd.salam --backend=native -g --target="$triple" --output=dbg.exe --log-level=error >>log 2>&1); then
            bad "$tag (-g build failed)"
            sed -n 1,5p "$d/log"
            continue
        fi
        objcopy -O binary -j .text "$d/plain.exe" "$d/t1"
        objcopy -O binary -j .text "$d/dbg.exe" "$d/t2"
        if ! cmp -s "$d/t1" "$d/t2"; then
            bad "$tag (-g changed .text)"
            continue
        fi
        (cd "$d" && wine plain.exe 2>/dev/null | tr -d '\r' >out1)
        (cd "$d" && wine dbg.exe 2>/dev/null | tr -d '\r' >out2)
        if ! [ -s "$d/out1" ] || ! cmp -s "$d/out1" "$d/out2"; then
            bad "$tag (-g build prints something else)"
            continue
        fi
        if ! objdump --dwarf=decodedline "$d/dbg.exe" 2>/dev/null | grep -q 'wd\.salam'; then
            bad "$tag (line table does not name wd.salam)"
            continue
        fi
        info=$(objdump --dwarf=info "$d/dbg.exe" 2>/dev/null)
        if ! echo "$info" | grep -q 'DW_AT_name *: *walk$' || ! echo "$info" | grep -q 'DW_AT_name *: *area$'; then
            bad "$tag (no DW_TAG_subprogram for area and walk)"
            continue
        fi
        if [ "$triple" = i686-w64-windows-gnu ]; then
            text=$(objdump -h "$d/dbg.exe" | awk '$2 == ".text" { print $3 ":" $4 }')
            lo=$((0x${text##*:}))
            hi=$((lo + 0x${text%%:*}))
            n=0
            out=0
            for r in $(objdump --dwarf=frames "$d/dbg.exe" 2>/dev/null | sed -n 's/.* FDE .*pc=\([0-9a-f]*\)\.\.\([0-9a-f]*\).*/\1:\2/p'); do
                s=$((0x${r%%:*}))
                e=$((0x${r##*:}))
                n=$((n + 1))
                if [ "$s" -lt "$lo" ] || [ "$e" -gt "$hi" ] || [ "$s" -ge "$e" ]; then
                    out=$((out + 1))
                fi
            done
            if [ "$n" -eq 0 ] || [ "$out" -ne 0 ]; then
                bad "$tag (.debug_frame has $n FDEs, $out outside .text)"
                continue
            fi
        fi
        pass=$((pass + 1))
    done
done
echo "native windows debug check: $pass ok, $fail failed"
[ "$fail" -eq 0 ] && [ "$pass" -eq 4 ]
