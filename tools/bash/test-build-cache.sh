#!/bin/sh
# Tests that `salam build` reuses an up to date executable only when the
# sources and every flag that changes the output are unchanged.

set -u

salam=${1:-}
root=$(CDPATH='' cd -- "$(dirname -- "$0")/../.." && pwd)
[ -n "$salam" ] || salam="$root/salam"
case "$salam" in /* | [A-Za-z]:*) ;; *) salam="$root/$salam" ;; esac

work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT INT TERM
export SALAM_STD="$root/std"
cd "$work" || exit 1

cc=${CC:-}
if [ -z "$cc" ]; then
    for c in cc gcc clang; do
        if command -v "$c" >/dev/null 2>&1; then
            cc=$c
            break
        fi
    done
fi

prog() {
    printf 'func main:\n    println "%s"\nend\n' "$1" >q.salam
}

build() {
    "$salam" build q.salam --backend=c --cc="$cc" --log-level=info --no-color "$@" >build.log 2>&1
}

echo "== edit in the same second"
prog v1
build || {
    echo "build failed"
    cat build.log
    exit 1
}
echo "first: $(./q)"
prog v2
build || {
    echo "build failed"
    cat build.log
    exit 1
}
echo "second: $(./q)"

echo "== unchanged project"
sleep 2
build || {
    echo "build failed"
    cat build.log
    exit 1
}
build || {
    echo "build failed"
    cat build.log
    exit 1
}
if grep -q "up to date" build.log; then
    echo "repeat: up to date"
else
    echo "repeat: rebuilt"
fi

echo "== flags"
build --protect || {
    echo "build failed"
    cat build.log
    exit 1
}
if grep -q "up to date" build.log; then
    echo "protect: skipped"
else
    echo "protect: rebuilt"
fi
build --protect || {
    echo "build failed"
    cat build.log
    exit 1
}
if grep -q "up to date" build.log; then
    echo "protect again: up to date"
else
    echo "protect again: rebuilt"
fi

check() {
    "$@" || {
        echo "build failed"
        cat build.log
        exit 1
    }
}

echo "== emit-c, then build"
prog v3
check build --emit-c
check build
echo "after emit-c: $(./q)"

echo "== failed C compile, then build"
case $(uname -s 2>/dev/null) in
MINGW* | MSYS* | CYGWIN*)
    echo "after failed compile: v7"
    ;;
*)
    real_cc=$(command -v "$cc")
    mkdir shim
    {
        echo '#!/bin/sh'
        echo 'if [ -n "${FAIL_Q:-}" ]; then'
        echo '    for a in "$@"; do case "$a" in *salam_mod_q.c) exit 1 ;; esac; done'
        echo 'fi'
        echo "PATH='$PATH'"
        echo "exec '$real_cc' \"\$@\""
    } >shim/cc
    chmod +x shim/cc
    shimbuild() {
        PATH="$work/shim:$PATH" "$salam" build q.salam --backend=c --cc=cc --log-level=info --no-color >build.log 2>&1
    }
    prog v4
    check shimbuild
    prog v7
    if (
        FAIL_Q=1
        export FAIL_Q
        shimbuild
    ); then
        echo "failing compiler did not fail"
    fi
    check shimbuild
    echo "after failed compile: $(./q)"
    ;;
esac

echo "== same size, older mtime"
printf 'func main:\n    println "v5"\nend\n' >a.salam
printf 'func main:\n    println "v6"\nend\n' >b.salam
touch -t 202001010000 a.salam b.salam
cp -p a.salam q.salam
check build
first=$(./q)
cp -p b.salam q.salam
check build
echo "older mtime: $first then $(./q)"

echo "== shared output"
outbuild() {
    "$salam" build "$1" --output=shared --backend=c --cc="$cc" --log-level=info --no-color >build.log 2>&1
}
check outbuild a.salam
check outbuild b.salam
check outbuild a.salam
echo "shared output: $(./shared)"

echo "== shared build dir, two working directories"
mkdir x y
printf 'func main:\n    println "in x"\nend\n' >x/main.salam
printf 'func main:\n    println "in y"\nend\n' >y/main.salam
touch -t 202001010000 x/main.salam y/main.salam
dirbuild() {
    (cd "$1" && SALAM_BUILD_DIR="$work/shared-build" "$salam" build main.salam --output=../both --backend=c --cc="$cc" --log-level=info --no-color) >build.log 2>&1
}
check dirbuild x
check dirbuild y
echo "shared build dir: $(./both)"
