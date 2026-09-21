#!/bin/sh

set -eu

OUT=
ARCHES="x86_64 aarch64 i686 arm"
WANT_HEADERS=0

while [ $# -gt 0 ]; do
    case "$1" in
    --out)
        OUT=$2
        shift 2
        ;;
    --out=*)
        OUT=${1#--out=}
        shift
        ;;
    --arches)
        ARCHES=$2
        shift 2
        ;;
    --arches=*)
        ARCHES=${1#--arches=}
        shift
        ;;
    --headers)
        WANT_HEADERS=1
        shift
        ;;
    *)
        echo "unknown argument: $1" >&2
        exit 2
        ;;
    esac
done

[ -n "$OUT" ] || {
    echo "usage: $0 --out DIR [--arches \"...\"] [--headers]" >&2
    exit 2
}

triple_of() {
    case "$1" in
    x86_64) echo x86_64-linux-musl ;;
    aarch64) echo aarch64-linux-musl ;;
    i686) echo i686-linux-musl ;;
    arm) echo arm-linux-musleabihf ;;
    *) echo "" ;;
    esac
}

ctriple_of() {
    case "$1" in
    x86_64) echo x86_64-unknown-linux-musl ;;
    aarch64) echo aarch64-unknown-linux-musl ;;
    i686) echo i686-unknown-linux-musl ;;
    arm) echo arm-unknown-linux-musleabihf ;;
    *) echo "" ;;
    esac
}

varname_of() {
    if [ "$1" = x86_64 ]; then
        echo SALAM_EMBED_MUSL_DIR
    else
        echo "SALAM_EMBED_MUSL_$(echo "$1" | tr '[:lower:]' '[:upper:]')_DIR"
    fi
}

download() {
    out=$1
    shift
    for url in "$@"; do
        echo "  trying $url"
        if curl -fL --connect-timeout 5 --max-time 900 \
            --speed-limit 512 --speed-time 30 \
            -sS -o "$out" "$url"; then
            return 0
        fi
    done
    return 1
}

scrub() {
    chmod -R u+rwX "$1" 2>/dev/null || true
    rm -rf "$1" 2>/dev/null || true
}

mkdir -p "$OUT"
cd "$OUT"

staged_any=0
for arch in $ARCHES; do
    triple=$(triple_of "$arch")
    ctriple=$(ctriple_of "$arch")
    [ -n "$triple" ] || {
        echo "::warning::unknown musl arch '$arch'; skipping"
        continue
    }

    SR="$OUT/sr-$arch"
    VAR=$(varname_of "$arch")

    have=1
    for f in crt1.o crti.o crtn.o libc.a libgcc.a; do
        [ -f "$SR/$f" ] || have=0
    done
    [ "$WANT_HEADERS" = 1 ] && [ ! -f "$SR/include/features.h" ] && have=0
    if [ "$have" = 1 ]; then
        echo "reusing cached $arch musl sysroot -> $SR"
        [ -n "${GITHUB_ENV:-}" ] && echo "$VAR=$SR" >>"$GITHUB_ENV"
        staged_any=1
        continue
    fi

    echo "fetching $arch ($triple)"
    scrub "$arch-x"
    mkdir -p "$arch-x"
    if ! download "$arch.tar" \
        "https://github.com/cross-tools/musl-cross/releases/latest/download/$ctriple.tar.xz" \
        "https://musl.cc/$triple-cross.tgz" \
        "https://more.musl.cc/11.2.1/x86_64-linux-musl/$triple-cross.tgz" \
        "https://more.musl.cc/11/x86_64-linux-musl/$triple-cross.tgz"; then
        echo "::warning::could not download the $arch ($triple) musl toolchain from any mirror; not embedding $arch."
        scrub "$arch-x"
        rm -f "$arch.tar"
        continue
    fi

    tar xf "$arch.tar" -C "$arch-x" 2>"$arch-tar.err" || true

    mkdir -p "$SR"
    missing=
    for f in crt1.o crti.o crtn.o libc.a libgcc.a; do
        found=$(find "$arch-x" -name "$f" -type f 2>/dev/null | head -1)
        if [ -n "$found" ]; then
            cp "$found" "$SR/$f"
        else
            missing="$missing $f"
        fi
    done

    for f in crtbeginT.o crtend.o libgcc_eh.a; do
        found=$(find "$arch-x" -name "$f" -type f 2>/dev/null | head -1)
        [ -n "$found" ] && cp "$found" "$SR/$f"
    done

    if [ -n "$missing" ]; then
        echo "::warning::the $arch ($triple) toolchain is missing$missing; not embedding $arch."
        echo "  last lines of tar's output:"
        tail -5 "$arch-tar.err" 2>/dev/null | sed 's/^/    /' || true
        rm -rf "$SR"
        scrub "$arch-x"
        rm -f "$arch.tar" "$arch-tar.err"
        continue
    fi

    if [ "$WANT_HEADERS" = 1 ]; then
        hdr=$(find "$arch-x" -type f -name features.h ! -path '*/c++/*' 2>/dev/null | head -1)
        rm -rf "$SR/include"
        if [ -n "$hdr" ]; then
            mkdir -p "$SR/include"
            cp -r "$(dirname "$hdr")/." "$SR/include/" 2>/dev/null || true
        fi
        if [ ! -f "$SR/include/features.h" ]; then
            rm -rf "$SR/include"
            echo "::warning::no musl libc headers in the fetched $arch toolchain; static third-party libs for $triple will be skipped"
        fi
    fi

    [ -n "${GITHUB_ENV:-}" ] && echo "$VAR=$SR" >>"$GITHUB_ENV"
    staged_any=1
    echo "staged $arch musl sysroot -> $SR"
    ls "$SR"

    scrub "$arch-x"
    rm -f "$arch.tar" "$arch-tar.err"
done

[ "$staged_any" = 1 ] || echo "::warning::no musl sysroots staged at all; this build embeds no linux-musl target"
exit 0
