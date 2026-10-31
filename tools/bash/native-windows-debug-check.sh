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
# PE files linked against the mingw-w64 CRT. With the built-in linker the -g
# build also gets a PDB: when llvm-pdbutil is there, every part of it must
# parse, the Pt struct must be in its type stream with area's p typed as it,
# step must show up as an inline site in walk in a --release build,
# and llvm-symbolizer must find walk's file and line through the PDB alone
# once the DWARF sections are stripped.
#
# Usage: tools/bash/native-windows-debug-check.sh <salam>

set -u
SALAM=$1
case "$SALAM" in /*) ;; */*) SALAM="$(pwd)/$SALAM" ;; esac
work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT
cat >"$work/wd.salam" <<'SRC'
import os

struct Pt:
    pub x: int = 0
    pub y: int = 0
end

noinline func area(p: Pt, scale: f64): f64:
    w := p.x * 2
    h := p.y + 1
    ret ((w * h) as f64) * scale
end

func step(a: int, b: int): int:
    c := a * b
    ret c + 1
end

noinline func walk(n: int, name: str): int:
    mut s := 0
    mut k := 3
    repeat n in i:
        s += step(i, k)
        k += 1
    end
    println name
    ret s
end

func main:
    p := Pt { x = 3, y = 4 }
    println area(p, 1.5)
    println walk(os.Args().len() + 4, "hello")
end
SRC
if ! command -v llvm-pdbutil >/dev/null 2>&1; then
    for llvm_dir in /usr/lib/llvm-*/bin; do
        if [ -x "$llvm_dir/llvm-pdbutil" ]; then
            PATH="$llvm_dir:$PATH"
        fi
    done
fi
pdbutil=$(command -v llvm-pdbutil || true)
symbolizer=""
if command -v llvm-objcopy >/dev/null 2>&1 && command -v llvm-readobj >/dev/null 2>&1; then
    symbolizer=$(command -v llvm-symbolizer || true)
fi
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
        if [ "$lk" = builtin ] && [ -n "$pdbutil" ]; then
            if ! [ -s "$d/dbg.pdb" ] || [ -e "$d/plain.pdb" ]; then
                bad "$tag (-g build has no dbg.pdb, or a plain build wrote one)"
                continue
            fi
            perr=""
            for part in -summary -modules -files -l -symbols -publics -globals -section-contribs -section-map -section-headers; do
                if ! "$pdbutil" dump $part "$d/dbg.pdb" >/dev/null 2>"$d/pdb.err"; then
                    perr="$part: $(head -c 200 "$d/pdb.err")"
                    break
                fi
            done
            if [ -n "$perr" ]; then
                bad "$tag (llvm-pdbutil cannot read the PDB, $perr)"
                continue
            fi
            if ! "$pdbutil" dump -types "$d/dbg.pdb" | grep -q 'LF_STRUCTURE .*`Pt`' ||
                ! "$pdbutil" dump -symbols "$d/dbg.pdb" | grep -A1 'S_LOCAL .*`p`' | grep -q '(Pt)'; then
                bad "$tag (the PDB has no Pt type, or area's p is not typed Pt)"
                continue
            fi
            if ! (cd "$d" && SALAM_LINKER=$lk "$SALAM" build ../wd.salam --backend=native --release -g --target="$triple" --output=rel.exe --log-level=error >>log 2>&1) ||
                ! "$pdbutil" dump -symbols "$d/rel.pdb" | grep -q 'inlinee = 0x[0-9A-F]* (step)'; then
                bad "$tag (a --release -g build has no S_INLINESITE for the inlined step)"
                continue
            fi
            if [ -n "$symbolizer" ]; then
                mkdir -p "$d/nd"
                llvm-objcopy --strip-debug "$d/dbg.exe" "$d/nd/dbg.exe" && cp "$d/dbg.pdb" "$d/nd/dbg.pdb"
                at=$("$pdbutil" dump -symbols "$d/dbg.pdb" | grep -A1 'PROC32 .*`walk`' | sed -n 's/.*addr = 0001:\([0-9]*\).*/\1/p' | head -1)
                text=$(llvm-readobj --sections "$d/dbg.exe" | grep -A3 'Name: .text ' | sed -n 's/.*VirtualAddress: \(0x[0-9A-F]*\).*/\1/p')
                if [ -z "$at" ] || [ -z "$text" ]; then
                    bad "$tag (no S_GPROC32/S_LPROC32 for walk in the PDB)"
                    continue
                fi
                got=$("$symbolizer" --obj="$d/nd/dbg.exe" --relative-address $((text + at + 1)) 2>/dev/null | tr '\n' ' ')
                case "$got" in
                walk*wd.salam:1[2-9]*) ;;
                *)
                    bad "$tag (llvm-symbolizer through the PDB alone gave: $got)"
                    continue
                    ;;
                esac
            fi
        fi
        pass=$((pass + 1))
    done
done
echo "native windows debug check: $pass ok, $fail failed"
[ "$fail" -eq 0 ] && [ "$pass" -eq 4 ]
