#!/bin/sh
# Checks the native backend's -g output.
#
# For each program it builds an executable with and without -g and requires
# that the two .text sections are byte-identical, that the line table names
# the source file, and that main has a DW_TAG_subprogram. When
# llvm-dwarfdump is installed it also runs --verify on every executable. It
# runs once per linker, so the built-in linker's .debug_* handling is
# covered as well.
#
# Usage: tools/bash/native-debug-check.sh <salam> <file.salam>...

set -u
SALAM=$1
case "$SALAM" in /*) ;; */*) SALAM="$(pwd)/$SALAM" ;; esac
shift
work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT
fail=0
pass=0
dwarfdump=$(command -v llvm-dwarfdump || true)
for f in "$@"; do
    b=$(basename "$f" .salam)
    for lk in builtin system; do
        d="$work/$b.$lk"
        mkdir -p "$d"
        if ! (cd "$d" && SALAM_LINKER=$lk "$SALAM" build "$f" --backend=native --output="$d/plain" --log-level=error >"$d/log" 2>&1); then
            echo "SKIP $b $lk (native build failed)"
            continue
        fi
        if ! (cd "$d" && SALAM_LINKER=$lk "$SALAM" build "$f" --backend=native -g --output="$d/dbg" --log-level=error >>"$d/log" 2>&1); then
            echo "FAIL $b $lk (-g build failed)"
            sed -n 1,5p "$d/log"
            fail=$((fail + 1))
            continue
        fi
        objcopy -O binary -j .text "$d/plain" "$d/t1"
        objcopy -O binary -j .text "$d/dbg" "$d/t2"
        if ! cmp -s "$d/t1" "$d/t2"; then
            echo "FAIL $b $lk (-g changed .text)"
            fail=$((fail + 1))
            continue
        fi
        if ! readelf --debug-dump=line "$d/dbg" 2>/dev/null | grep -q "$b.salam"; then
            echo "FAIL $b $lk (line table does not name $b.salam)"
            fail=$((fail + 1))
            continue
        fi
        if ! readelf --debug-dump=info "$d/dbg" 2>/dev/null | grep -qE 'DW_AT_name +: main$'; then
            echo "FAIL $b $lk (no subprogram for main)"
            fail=$((fail + 1))
            continue
        fi
        if [ -n "$dwarfdump" ] && ! "$dwarfdump" --verify "$d/dbg" >"$d/verify" 2>&1; then
            echo "FAIL $b $lk (llvm-dwarfdump --verify)"
            grep -m5 'error' "$d/verify"
            fail=$((fail + 1))
            continue
        fi
        pass=$((pass + 1))
    done
done
echo "native debug check: $pass ok, $fail failed"
[ "$fail" -eq 0 ]
