#!/bin/bash
# Links test programs for one target with the built-in linker and with a
# reference linker, and compares the results.
#
# Usage: tools/linker/cross-check.sh <salam> <target> [test dirs...]
#
# Linux targets (x86_64/i686/aarch64/arm *-linux-musl) need SALAM_MUSL_SYSROOT
# pointing at the target's musl sysroot and ld.lld; they run natively or
# under qemu-<arch>(-static) when the host cannot run them. Windows targets
# need the matching mingw-w64 gcc and wine. macOS targets need ld64.lld and
# llvm-objdump; there is nothing to run them on, so the check compares the
# disassembly with ld64.lld's and verifies the code signature.

set -u
SALAM=$1
TARGET=$2
shift 2
R=$(cd "$(dirname "$0")/../.." && pwd)
DIRS=${*:-basics features types data match switch llvm}
W=$(mktemp -d)
trap 'rm -rf "$W"' EXIT
export WINEDEBUG=-all
pass=0
fail=0
skip=0

run_cmd() {
    case "$TARGET" in
    aarch64*) command -v qemu-aarch64-static || command -v qemu-aarch64 ;;
    arm*) command -v qemu-arm-static || command -v qemu-arm ;;
    *) echo "" ;;
    esac
}
RUN=$(run_cmd)

for dir in $DIRS; do
    for f in "$R"/tests/en/$dir/*.salam; do
        exp=${f%.salam}.out
        [ -f "$exp" ] || continue
        id=$dir/$(basename "$f" .salam)
        w=$W/${id//\//_}
        mkdir -p "$w"
        common="--backend=llvm --release --target=$TARGET --log-level=error"
        # shellcheck disable=SC2086
        if ! (cd "$w" && "$SALAM" obj "$f" $common --output=p.o >obj.err 2>&1); then
            skip=$((skip + 1))
            continue
        fi
        case "$TARGET" in
        *linux-musl*)
            S=$SALAM_MUSL_SYSROOT
            em=elf_x86_64
            case "$TARGET" in i686*) em=elf_i386 ;; aarch64*) em=aarch64linux ;; arm*) em=armelf_linux_eabi ;; esac
            rt=$S/libgcc.a
            [ -f "$S/libclang_rt.builtins-x86_64.a" ] && rt=$S/libclang_rt.builtins-x86_64.a
            ld.lld -m $em -static -o "$w/ref" "$S/crt1.o" "$S/crti.o" "$w/p.o" "$S/libc.a" "$rt" "$S/crtn.o" >/dev/null 2>&1 || {
                skip=$((skip + 1))
                continue
            }
            # shellcheck disable=SC2086
            if ! (cd "$w" && SALAM_MUSL_SYSROOT=$S "$SALAM" build "$f" $common --linker=builtin --output=out >nl.err 2>&1); then
                echo "FAIL $id (link)"
                fail=$((fail + 1))
                continue
            fi
            r=$(timeout 30 $RUN "$w/ref" </dev/null 2>&1)
            n=$(timeout 30 $RUN "$w/out" </dev/null 2>&1)
            ;;
        *windows*)
            cc=${TARGET%%-*}-w64-mingw32-gcc
            $cc -o "$w/ref.exe" "$w/p.o" -lws2_32 >/dev/null 2>&1 || {
                skip=$((skip + 1))
                continue
            }
            # shellcheck disable=SC2086
            if ! (cd "$w" && "$SALAM" build "$f" $common --linker=builtin --output=out.exe >nl.err 2>&1); then
                echo "FAIL $id (link)"
                fail=$((fail + 1))
                continue
            fi
            r=$(timeout 30 wine "$w/ref.exe" </dev/null 2>/dev/null | tr -d '\r')
            n=$(timeout 30 wine "$w/out.exe" </dev/null 2>/dev/null | tr -d '\r')
            ;;
        *apple*)
            arch=${TARGET%%-*}
            syms=$(llvm-nm -u "$w/p.o" | awk '{print $NF}' | grep -v '^___dso_handle$' | sed "s/.*/'&'/" | paste -sd, -)
            {
                printf -- "--- !tapi-tbd\ntbd-version: 4\ntargets: [ %s-macos ]\n" "$arch"
                printf -- "install-name: '/usr/lib/libSystem.B.dylib'\nexports:\n"
                printf -- "  - targets: [ %s-macos ]\n    symbols: [ %s, 'dyld_stub_binder' ]\n...\n" "$arch" "$syms"
            } >"$w/libSystem.tbd"
            if ! ld64.lld -arch "$arch" -platform_version macos 11.0 11.0 -ignore_optimization_hints \
                -o "$w/ref" "$w/p.o" -L"$w" -lSystem >/dev/null 2>&1; then
                skip=$((skip + 1))
                continue
            fi
            # shellcheck disable=SC2086
            if ! (cd "$w" && "$SALAM" build "$f" $common --linker=builtin --output=out >nl.err 2>&1); then
                echo "FAIL $id (link)"
                fail=$((fail + 1))
                continue
            fi
            chk=$R/tools/linker/macho_check.py
            if ! python3 "$chk" sig "$w/out" >/dev/null || ! python3 "$chk" cmp "$w/out" "$w/ref" >"$w/cmp"; then
                echo "FAIL $id (Mach-O check)"
                head -3 "$w/cmp"
                fail=$((fail + 1))
                continue
            fi
            pass=$((pass + 1))
            continue
            ;;
        *)
            echo "unsupported target $TARGET"
            exit 2
            ;;
        esac
        if [ "$n" = "$r" ]; then
            pass=$((pass + 1))
        else
            echo "FAIL $id (output differs from the reference link)"
            fail=$((fail + 1))
        fi
    done
done
echo "linker cross-check $TARGET: $pass ok, $fail failed, $skip skipped"
[ "$fail" -eq 0 ]
