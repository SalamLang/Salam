#!/bin/sh
# Stage trimmed musl sysroots for the release build to embed, so a released
# salam can produce fully static linux-musl binaries for a target without
# that target's toolchain installed.
#
# A sysroot here is five files - crt1.o, crti.o, crtn.o, libc.a, libgcc.a -
# plus, with --headers, musl's libc headers, taken out of a prebuilt
# cross toolchain and with everything else thrown away.
#
# Usage:
#   tools/bash/fetch-musl-sysroots.sh --out DIR [--arches "x86_64 aarch64 ..."]
#                                     [--headers]
#
# Writes SALAM_EMBED_MUSL_<ARCH>_DIR=<path> for each staged arch to
# $GITHUB_ENV when that is set (x86_64 gets the unsuffixed
# SALAM_EMBED_MUSL_DIR, which is the name the Makefile and the build steps
# already use for it). Always prints what it staged.
#
# Every arch is best-effort and independent: a failure warns and moves on,
# because an embed that is missing costs one cross-target on the released
# binary, not the release.

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

# arch -> musl.cc triple, cross-tools/musl-cross triple.
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

# cross-tools/musl-cross publishes the same prebuilt toolchains as GitHub
# Releases, which resolve over the network GitHub-hosted runners already
# use, so it is tried first. musl.cc is a small volunteer-run host serving
# ~100MB archives and has been observed fully unreachable from those runners
# (connection timeout, not just slow), so --connect-timeout stays short and
# there is no curl --retry: retrying a dead host only burns CI minutes.
# --max-time stays generous for a connection that DOES succeed but crawls.
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

# Read-only files and dirs (crosstool-NG leaves some behind) make a plain
# rm -rf fail outright, which under `set -e` would kill the whole job over
# scratch cleanup in a directory the runner throws away anyway.
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

    # tar's exit status is deliberately ignored, and the extraction is
    # judged by what it produced instead. These toolchains carry symlinks
    # (lib64 -> lib, the .so version chains, license links) that a Windows
    # runner cannot create - MSYS2 has no privilege to, and its copy
    # fallback cannot resolve a link whose target tar has not written yet.
    # GNU tar reports each one and exits 2 while every regular file, which
    # is all this script takes, came out fine. Gating on the status instead
    # cost the Windows release every one of its musl targets, under a
    # "could not fetch from any mirror" warning that blamed the mirrors for
    # an 81MB archive that had downloaded perfectly.
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
        # musl's libc headers, not gcc's own fixed-includes in the sibling
        # lib/gcc/.../include dir - clang needs these via -isystem to
        # cross-compile sqlite3/hiredis/openssl/mariadb for this target.
        # features.h is a stable, always-present marker for the real libc
        # include root.
        hdr=$(find "$arch-x" -type f -name features.h ! -path '*/c++/*' 2>/dev/null | head -1)
        rm -rf "$SR/include"
        if [ -n "$hdr" ]; then
            mkdir -p "$SR/include"
            # `|| true` for the same reason tar's status is ignored above:
            # a header tree can carry a symlink this host cannot reproduce,
            # and cp would then fail the whole step over one file. What
            # matters is whether features.h arrived, so check that instead.
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
