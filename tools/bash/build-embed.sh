#!/bin/sh

set -eu

OUT=$(pwd)
: "${CC:=cc}"
: "${AR:=ar}"

MUSL_X86_64=
MUSL_AARCH64=
MUSL_I686=
MUSL_ARM=
MINGW_X86_64=
MINGW_I686=
HOSTLIBS=
XL_X86_64_MUSL=
XL_AARCH64_MUSL=
XL_I686_MUSL=
XL_ARM_MUSL=
XL_X86_64_MINGW=

while [ $# -gt 0 ]; do
    case $1 in
    --out)
        OUT=$2
        shift 2
        ;;
    --musl-x86_64)
        MUSL_X86_64=$2
        shift 2
        ;;
    --musl-aarch64)
        MUSL_AARCH64=$2
        shift 2
        ;;
    --musl-i686)
        MUSL_I686=$2
        shift 2
        ;;
    --musl-arm)
        MUSL_ARM=$2
        shift 2
        ;;
    --mingw-x86_64)
        MINGW_X86_64=$2
        shift 2
        ;;
    --mingw-i686)
        MINGW_I686=$2
        shift 2
        ;;
    --hostlibs)
        HOSTLIBS=$2
        shift 2
        ;;
    --extralibs-x86_64-musl)
        XL_X86_64_MUSL=$2
        shift 2
        ;;
    --extralibs-aarch64-musl)
        XL_AARCH64_MUSL=$2
        shift 2
        ;;
    --extralibs-i686-musl)
        XL_I686_MUSL=$2
        shift 2
        ;;
    --extralibs-arm-musl)
        XL_ARM_MUSL=$2
        shift 2
        ;;
    --extralibs-x86_64-mingw)
        XL_X86_64_MINGW=$2
        shift 2
        ;;
    -h | --help)
        sed -n '2,22p' "$0"
        exit 0
        ;;
    *)
        echo "unknown argument: $1" >&2
        exit 2
        ;;
    esac
done

case "$($CC -dumpmachine 2>/dev/null)" in
i?86-* | arm-* | armv7*-* | armhf-* | powerpc-* | mips-* | mipsel-*) PTR=.long ;;
*) PTR=.quad ;;
esac

to_native_path() {
    if command -v cygpath >/dev/null 2>&1; then
        cygpath -m "$1"
    else
        printf '%s' "$1"
    fi
}

WORK=$(mktemp -d)
trap 'rm -rf "$WORK"' EXIT INT TERM
mkdir -p "$OUT"

S=$WORK/embed.S
: >"$S"
staged=0

emit() {
    stem=$1
    dir=$2
    if [ -n "$dir" ] && [ -d "$dir" ]; then
        tar cf "$WORK/$stem.tar" -C "$dir" .
        bytes=$(wc -c <"$WORK/$stem.tar" | tr -d ' ')
        {
            printf '.section .rodata\n'
            printf '%s_data:\n' "$stem"
            printf '.incbin "%s"\n' "$(to_native_path "$WORK/$stem.tar")"
            printf '%s_end:\n' "$stem"
            printf '.section .data\n'
            printf '.balign 8\n'
            printf '.globl salam_embed_%s_ptr\n' "$stem"
            printf 'salam_embed_%s_ptr: %s %s_data\n' "$stem" "$PTR" "$stem"
            printf '.balign 8\n'
            printf '.globl salam_embed_%s_len\n' "$stem"
            printf 'salam_embed_%s_len: .quad %s\n' "$stem" "$bytes"
        } >>"$S"
        echo "  embedded $stem ($(wc -c <"$WORK/$stem.tar") bytes) from $dir"
        staged=$((staged + 1))
    else
        {
            printf '.section .data\n'
            printf '.balign 8\n'
            printf '.globl salam_embed_%s_ptr\n' "$stem"
            printf 'salam_embed_%s_ptr: %s 0\n' "$stem" "$PTR"
            printf '.balign 8\n'
            printf '.globl salam_embed_%s_len\n' "$stem"
            printf 'salam_embed_%s_len: .quad 0\n' "$stem"
        } >>"$S"
    fi
}

emit musl_x86_64 "$MUSL_X86_64"
emit musl_aarch64 "$MUSL_AARCH64"
emit musl_i686 "$MUSL_I686"
emit musl_arm "$MUSL_ARM"
emit mingw_x86_64 "$MINGW_X86_64"
emit mingw_i686 "$MINGW_I686"

emit hostlibs "$HOSTLIBS"
emit extralibs_x86_64_linux_musl "$XL_X86_64_MUSL"
emit extralibs_aarch64_linux_musl "$XL_AARCH64_MUSL"
emit extralibs_i686_linux_musl "$XL_I686_MUSL"
emit extralibs_arm_linux_musleabihf "$XL_ARM_MUSL"
emit extralibs_x86_64_w64_windows_gnu "$XL_X86_64_MINGW"

$CC -c "$S" -o "$WORK/embed.o"
rm -f "$OUT/libsalam_embed.a"
$AR rcs "$OUT/libsalam_embed.a" "$WORK/embed.o"

echo "Built $OUT/libsalam_embed.a ($staged target(s) embedded)"
echo "  link it with: salam build -DSALAM_HAVE_EMBED --libpath=$OUT ..."
