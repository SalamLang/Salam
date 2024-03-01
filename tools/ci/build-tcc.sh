#!/bin/sh

set -eu

if [ $# -lt 1 ]; then
    echo "usage: $0 <prefix>" >&2
    exit 2
fi

HERE=$(cd "$(dirname "$0")" && pwd)
PREFIX=$1
: "${TINYCC_REF:=$(grep -vE '^[[:space:]]*(#|$)' "$HERE/tinycc-ref.txt" | head -1)}"
: "${TCC_CONFIGURE_EXTRA:=}"
: "${CROSS_PREFIX:=}"
: "${HOSTCC:=cc}"
: "${MAKE:=make}"
JOBS=$(nproc 2>/dev/null || echo 4)

tcc_cross_target() {
    cpu=""
    os=""
    for arg in $TCC_CONFIGURE_EXTRA; do
        case $arg in
        --cpu=*) cpu=${arg#--cpu=} ;;
        --targetos=*) os=${arg#--targetos=} ;;
        esac
    done
    [ -n "$cpu" ] || return 1
    case $os in
    WIN32 | Windows*) echo "$cpu-win32" ;;
    Darwin) echo "$cpu-osx" ;;
    *) echo "$cpu" ;;
    esac
}

[ -n "$TINYCC_REF" ] || {
    echo "error: no TINYCC_REF and none readable from $HERE/tinycc-ref.txt" >&2
    exit 2
}

WORK=$(mktemp -d 2>/dev/null || mktemp -d -t salam-tcc)
trap 'rm -rf "$WORK"' EXIT INT TERM

echo "tinycc ref : $TINYCC_REF"
echo "prefix     : $PREFIX"

git clone --quiet --filter=blob:none https://github.com/TinyCC/tinycc.git "$WORK/tinycc"
git -C "$WORK/tinycc" checkout --quiet "$TINYCC_REF"

mkdir -p "$WORK/tinycc/build"
cd "$WORK/tinycc/build"

cross_arg=""
CROSS_TARGET=""
if [ -n "$CROSS_PREFIX" ]; then
    cross_arg="--cross-prefix=$CROSS_PREFIX --enable-static"
    "$HOSTCC" -DC2STR ../conftest.c -o c2str.exe
    : "${TCC_CROSS_TARGET:=$(tcc_cross_target || true)}"
    CROSS_TARGET=$TCC_CROSS_TARGET
    [ -n "$CROSS_TARGET" ] || {
        echo "error: cross-building needs TCC_CROSS_TARGET, and none could" >&2
        echo "  be derived from TCC_CONFIGURE_EXTRA='$TCC_CONFIGURE_EXTRA'" >&2
        exit 2
    }
fi
# shellcheck disable=SC2086
../configure --prefix="$PREFIX" $cross_arg $TCC_CONFIGURE_EXTRA

EXESUF=$(sed -n 's/^EXESUF=//p' config.mak | head -1)
TCCDIR=$(sed -n 's/^tccdir=\$(DESTDIR)//p' config.mak | head -1)

if [ -n "$CROSS_TARGET" ]; then
    "$MAKE" -j"$JOBS" "tcc$EXESUF"

    mkdir -p "$WORK/tinycc/build-host"
    (
        cd "$WORK/tinycc/build-host"
        ../configure --cc="$HOSTCC" --prefix="$WORK/host-prefix"
        "$MAKE" -j"$JOBS" "cross-$CROSS_TARGET"
    )
    RUNTIME="$WORK/tinycc/build-host/$CROSS_TARGET-libtcc1.a"
    [ -f "$RUNTIME" ] || {
        echo "error: 'make cross-$CROSS_TARGET' produced no $CROSS_TARGET-libtcc1.a" >&2
        ls -la "$WORK/tinycc/build-host" >&2 || true
        exit 1
    }
else
    "$MAKE" -j"$JOBS"
fi

"$MAKE" install

if [ -n "$CROSS_TARGET" ]; then
    dest=$TCCDIR
    if grep -q '^CONFIG_WIN32=yes' config.mak; then
        dest=$TCCDIR/lib
    fi
    mkdir -p "$dest"
    cp "$RUNTIME" "$dest/libtcc1.a"
fi

TCC_BIN=""
for cand in "$PREFIX/tcc.exe" "$PREFIX/tcc" "$PREFIX/bin/tcc"; do
    [ -x "$cand" ] && {
        TCC_BIN=$cand
        break
    }
done
[ -n "$TCC_BIN" ] || {
    echo "error: no tcc binary under $PREFIX after install" >&2
    ls -R "$PREFIX" >&2 || true
    exit 1
}

echo "tcc installed: $TCC_BIN"

LIBTCC1=$(find "$PREFIX" -name libtcc1.a -print 2>/dev/null | head -1)
[ -n "$LIBTCC1" ] || {
    echo "error: no libtcc1.a under $PREFIX after install" >&2
    ls -R "$PREFIX" >&2 || true
    exit 1
}
echo "runtime installed: $LIBTCC1"

printf 'int main(void){return 0;}\n' >"$WORK/smoke.c"
if [ -z "$CROSS_PREFIX" ]; then
    "$TCC_BIN" -v
    "$TCC_BIN" "$WORK/smoke.c" -o "$WORK/smoke" && "$WORK/smoke"
    echo "smoke test: compiled and ran"
else
    cp "$RUNTIME" "$dest/$CROSS_TARGET-libtcc1.a"
    "$WORK/tinycc/build-host/$CROSS_TARGET-tcc" -B"$TCCDIR" \
        "$WORK/smoke.c" -o "$WORK/smoke$EXESUF"
    rm -f "$dest/$CROSS_TARGET-libtcc1.a"
    [ -s "$WORK/smoke$EXESUF" ] || {
        echo "error: linking against $LIBTCC1 produced nothing" >&2
        exit 1
    }
    echo "smoke test: linked a $CROSS_TARGET program against the installed runtime"
fi
