#!/bin/sh
# Turns the published Linux release tarballs into .deb packages.
#
#   packaging/debian/build-deb.sh --version 0.4.0 --input ./release --output ./debs
#
# --input holds the salam-<version>-linux-<arch>.tar.gz files as published on
# the GitHub release. One .deb is produced per tarball found; missing arches
# are skipped, not an error.
#
# Needs dpkg-deb (dpkg-dev on Debian/Ubuntu, `brew install dpkg` on macOS).

set -eu

VERSION=
INPUT=.
OUTPUT=./debs
REVISION=1
COMPRESSION=xz
COMPRESSION_LEVEL=6

while [ $# -gt 0 ]; do
        case $1 in
        --version)
                VERSION=$2
                shift 2
                ;;
        --version=*)
                VERSION=${1#*=}
                shift
                ;;
        --input)
                INPUT=$2
                shift 2
                ;;
        --input=*)
                INPUT=${1#*=}
                shift
                ;;
        --output)
                OUTPUT=$2
                shift 2
                ;;
        --output=*)
                OUTPUT=${1#*=}
                shift
                ;;
        --revision)
                REVISION=$2
                shift 2
                ;;
        --revision=*)
                REVISION=${1#*=}
                shift
                ;;
        --compression)
                COMPRESSION=$2
                shift 2
                ;;
        --compression=*)
                COMPRESSION=${1#*=}
                shift
                ;;
        --compression-level)
                COMPRESSION_LEVEL=$2
                shift 2
                ;;
        --compression-level=*)
                COMPRESSION_LEVEL=${1#*=}
                shift
                ;;
        -h | --help)
                sed -n '2,10p' "$0"
                exit 0
                ;;
        *)
                echo "build-deb.sh: unknown argument: $1" >&2
                exit 2
                ;;
        esac
done

[ -n "$VERSION" ] || {
        echo "build-deb.sh: --version is required" >&2
        exit 2
}
command -v dpkg-deb >/dev/null 2>&1 || {
        echo "build-deb.sh: dpkg-deb not found (install dpkg-dev, or 'brew install dpkg')" >&2
        exit 1
}

MAINTAINER='Seyyed Ali Mohammadiyeh (Max Base) <MaxBaseCode@gmail.com>'
HOMEPAGE='https://github.com/SalamLang/Salam'

mkdir -p "$OUTPUT"
OUTPUT=$(cd "$OUTPUT" && pwd)
INPUT=$(cd "$INPUT" && pwd)

WORK=$(mktemp -d)
trap 'rm -rf "$WORK"' EXIT INT TERM

built=0

DEPENDS='libc6 (>= 2.38), libgcc-s1, libtinfo6, libxml2'

for spec in \
        "x86_64:amd64" \
        "aarch64:arm64" \
        "armhf:armhf" \
        "i686:i386"; do
        rel_arch=${spec%%:*}
        deb_arch=${spec#*:}
        depends=$DEPENDS

        tarball="$INPUT/salam-$VERSION-linux-$rel_arch.tar.gz"
        [ -f "$tarball" ] || {
                echo "skip $deb_arch: no $(basename "$tarball")"
                continue
        }

        echo "==> $deb_arch"
        stage="$WORK/$deb_arch"
        root="$stage/root"
        rm -rf "$stage"
        mkdir -p "$root/DEBIAN" "$root/usr/bin" "$root/usr/lib/salam" \
                "$root/usr/share/doc/salam" "$root/usr/share/licenses/salam"

        mkdir -p "$stage/unpack"
        tar -xzf "$tarball" -C "$stage/unpack"
        src=$(find "$stage/unpack" -mindepth 1 -maxdepth 1 -type d | head -n 1)
        [ -n "$src" ] || {
                echo "  $(basename "$tarball") unpacked to nothing" >&2
                exit 1
        }

        install -m 755 "$src/salam" "$root/usr/lib/salam/salam"
        cp -a "$src/std" "$root/usr/lib/salam/std"
        [ -d "$src/sysroots" ] && cp -a "$src/sysroots" "$root/usr/lib/salam/sysroots"

        find "$root/usr/lib/salam/std" -type d -exec chmod 755 {} +
        find "$root/usr/lib/salam/std" -type f -exec chmod 644 {} +

        cat >"$root/usr/bin/salam" <<'EOF'
#!/bin/sh
SALAM_HOME=/usr/lib/salam
export SALAM_STD="${SALAM_STD:-$SALAM_HOME/std}"
exec "$SALAM_HOME/salam" "$@"
EOF
        chmod 755 "$root/usr/bin/salam"

        [ -f "$src/LICENSE" ] && install -m 644 "$src/LICENSE" "$root/usr/share/licenses/salam/LICENSE"
        [ -f "$src/README.md" ] && install -m 644 "$src/README.md" "$root/usr/share/doc/salam/README.md"

        installed_size=$(du -sk "$root" | cut -f1)

        cat >"$root/DEBIAN/control" <<EOF
Package: salam
Version: $VERSION-$REVISION
Architecture: $deb_arch
Maintainer: $MAINTAINER
Installed-Size: $installed_size
Depends: $depends
Section: devel
Priority: optional
Homepage: $HOMEPAGE
Description: General-purpose systems programming language with a built-in DSL
 Salam is a general-purpose and systems programming language with a built-in
 domain-specific language for describing user interfaces. It ships an
 interpreter, a C backend and an embedded LLVM backend that can cross-compile
 and JIT with no other toolchain installed.
 .
 This package installs the compiler and its standard library.
EOF

        dpkg-deb --root-owner-group \
                -Z"$COMPRESSION" -z"$COMPRESSION_LEVEL" \
                --build "$root" \
                "$OUTPUT/salam_${VERSION}-${REVISION}_${deb_arch}.deb" >/dev/null
        echo "  $OUTPUT/salam_${VERSION}-${REVISION}_${deb_arch}.deb"
        built=$((built + 1))
done

[ "$built" -gt 0 ] || {
        echo "build-deb.sh: no tarballs matched in $INPUT" >&2
        exit 1
}
echo "built $built package(s)"
