#!/bin/sh
set -u

salam=${1:-}
root=$(CDPATH='' cd -- "$(dirname -- "$0")/../.." && pwd)
[ -n "$salam" ] || salam="$root/salam"
case "$salam" in /*) ;; *) salam="$root/$salam" ;; esac
command -v git >/dev/null 2>&1 || {
    echo "skip: git is not installed"
    exit 0
}

work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT INT TERM
mkdir -p "$work/home" "$work/remote"
export HOME="$work/home"
export SALAM_STD="$root/std"
export SALAM_PKG="$work/pkg"
export SALAM_BIN="$work/bin"
export GIT_CONFIG_NOSYSTEM=1
export GIT_CONFIG_COUNT=1
export GIT_CONFIG_KEY_0="url.file://$work/remote/.insteadOf"
export GIT_CONFIG_VALUE_0="https://"
export GIT_AUTHOR_NAME=salam GIT_AUTHOR_EMAIL=salam@example.com
export GIT_COMMITTER_NAME=salam GIT_COMMITTER_EMAIL=salam@example.com
unset SALAM_NO_GIT SALAM_REGISTRY
: >"$work/sums.txt"
export SALAM_SUMDB="$work/sums.txt"

fails=0
check() {
    if [ "$2" = 0 ]; then
        echo "ok $1"
    else
        echo "FAIL $1"
        [ -f "$work/last.log" ] && sed 's/^/    /' "$work/last.log" | head -8
        fails=$((fails + 1))
    fi
}
run() {
    "$@" >"$work/last.log" 2>&1
}
has() {
    grep -q -- "$1" "$2" 2>/dev/null
}

repo() {
    d="$work/remote/$1"
    mkdir -p "$d"
    (cd "$d" && git init -q && git checkout -q -b main 2>/dev/null)
}
commit_tag() {
    (cd "$work/remote/$1" && git add -A && git commit -qm "$2" && git tag "$2")
}

repo github.com/fixture/dep
cat >"$work/remote/github.com/fixture/dep/dep.salam" <<'EOF'
package dep

pub func Mark(): str:
    ret "!"
end
EOF
printf '[package]\nname = "dep"\n' >"$work/remote/github.com/fixture/dep/salam.toml"
commit_tag github.com/fixture/dep v1.0.0

repo github.com/fixture/greet
g="$work/remote/github.com/fixture/greet"
printf '[package]\nname = "greet"\n' >"$g/salam.toml"
cat >"$g/greet.salam" <<'EOF'
package greet

pub func Hello(n: str): str:
    ret "hello " + n
end
EOF
commit_tag github.com/fixture/greet v1.0.0
printf '[package]\nname = "greet"\n\n[dependencies]\n"github.com/fixture/dep" = "v1.0.0"\n' >"$g/salam.toml"
cat >"$g/greet.salam" <<'EOF'
package greet

import "github.com/fixture/dep"

pub func Hello(n: str): str:
    ret "hi " + n + dep.Mark()
end
EOF
mkdir -p "$g/cmd/hello"
cat >"$g/cmd/hello/main.salam" <<'EOF'
func main:
    println "hello from a tool"
end
EOF
commit_tag github.com/fixture/greet v1.1.0
cat >"$g/greet.salam" <<'EOF'
package greet

pub func Hello(n: str): str:
    ret "hey " + n
end
EOF
cat >"$g/greet2.salam" <<'EOF'
package greet2

pub func Hello(n: str): str:
    ret "hey " + n
end
EOF
printf '[package]\nname = "greet"\n' >"$g/salam.toml"
commit_tag github.com/fixture/greet v2.0.0

repo github.com/fixture/future
printf '[package]\nname = "future"\nsalam = ">=99.0.0"\n' >"$work/remote/github.com/fixture/future/salam.toml"
printf 'package future\n\npub func X(): int:\n    ret 1\nend\n' >"$work/remote/github.com/fixture/future/future.salam"
commit_tag github.com/fixture/future v1.0.0

cd "$work" || exit 1
run "$salam" new app
check "new writes salam.toml with a salam version" "$(has 'salam = ">=' app/salam.toml && echo 0 || echo 1)"
cd app || exit 1
cat >main.salam <<'EOF'
import "github.com/fixture/greet"

func main:
    println greet.Hello("salam")
end
EOF

run "$salam" add github.com/fixture/greet@v1.0.0
check "add pins the asked version" "$(has '"github.com/fixture/greet" = "v1.0.0"' salam.toml && echo 0 || echo 1)"
check "lock records commit and sum" "$(has 'commit = ' salam.lock && has 'sum = "sha256:' salam.lock && echo 0 || echo 1)"
run "$salam" run --log-level=error
check "a program imports the package" "$(has '^hello salam$' "$work/last.log" && echo 0 || echo 1)"

run "$salam" update
check "update stays on major 1" "$(has '"github.com/fixture/greet" = "v1.1.0"' salam.toml && echo 0 || echo 1)"
check "update says a new major is out" "$(has 'v2.0.0 is out' "$work/last.log" && echo 0 || echo 1)"
check "a dependency's dependency is locked" "$(has 'github.com/fixture/dep' salam.lock && echo 0 || echo 1)"
run "$salam" run --log-level=error
check "the transitive import works" "$(has '^hi salam!$' "$work/last.log" && echo 0 || echo 1)"

run "$salam" update github.com/fixture/greet@latest
check "update @latest crosses the major" "$(has '"github.com/fixture/greet" = "v2.0.0"' salam.toml && echo 0 || echo 1)"
run "$salam" run --log-level=error
check "the new major builds" "$(has '^hey salam$' "$work/last.log" && echo 0 || echo 1)"

rm -rf "$SALAM_PKG"
run "$salam" deps
check "deps downloads into an empty cache" "$(has 'github.com/fixture/greet v2.0.0' "$work/last.log" && echo 0 || echo 1)"
echo "// changed" >>"$SALAM_PKG/github.com/fixture/greet@v2.0.0/greet.salam"
run "$salam" deps
check "deps notices a changed download" "$(has 'checksum mismatch' "$work/last.log" && echo 0 || echo 1)"
rm -rf "$SALAM_PKG/github.com/fixture/greet@v2.0.0"
run "$salam" deps

run "$salam" add github.com/fixture/future
check "add refuses a package that needs a newer Salam" "$(has 'needs Salam 99.0.0' "$work/last.log" && echo 0 || echo 1)"
run "$salam" remove github.com/fixture/future

run "$salam" remove github.com/fixture/greet
run "$salam" check main.salam
check "remove takes the package out" "$(has 'not a dependency of this project' "$work/last.log" && echo 0 || echo 1)"

run "$salam" install github.com/fixture/greet/cmd/hello@v1.1.0 --log-level=error
check "install builds a program into SALAM_BIN" "$([ -x "$SALAM_BIN/hello" ] && "$SALAM_BIN/hello" | grep -q 'hello from a tool' && echo 0 || echo 1)"
run "$salam" uninstall
check "uninstall lists installed programs" "$(has 'hello  github.com/fixture/greet/cmd/hello v1.1.0' "$work/last.log" && echo 0 || echo 1)"
run "$salam" uninstall hello
check "uninstall removes the program" "$([ ! -e "$SALAM_BIN/hello" ] && echo 0 || echo 1)"

printf '[[package]]\nname = "github.com/fixture/greet"\ndescription = "Says hello"\ntags = ["greeting"]\n' >"$work/registry.toml"
SALAM_REGISTRY="$work/registry.toml" run "$salam" search greeting
check "search reads the registry" "$(has 'github.com/fixture/greet  Says hello' "$work/last.log" && echo 0 || echo 1)"

mkdir -p "$work/app2"
cd "$work/app2" || exit 1
printf '[package]\nname = "app2"\n' >salam.toml
run "$salam" add 'github.com/fixture/greet@^1.0'
check "add records a version range" "$(has '"github.com/fixture/greet" = "^1.0"' salam.toml && echo 0 || echo 1)"
check "the lock takes the newest release in the range" "$(has 'version = "v1.1.0"' salam.lock && echo 0 || echo 1)"
run "$salam" add github.com/fixture/greet/v2
check "a /v2 path picks major 2" "$(has '"github.com/fixture/greet/v2" = "v2.0.0"' salam.toml && echo 0 || echo 1)"
cat >main.salam <<'EOF'
import "github.com/fixture/greet"
import g2 "github.com/fixture/greet/v2"

func main:
    println greet.Hello("one")
    println g2.Hello("two")
end
EOF
run "$salam" run --log-level=error
check "major 1 and major 2 build side by side" "$(has '^hi one!$' "$work/last.log" && has '^hey two$' "$work/last.log" && echo 0 || echo 1)"

mkdir -p "$work/app3"
cd "$work/app3" || exit 1
printf '[package]\nname = "app3"\n' >salam.toml
run "$salam" add github.com/fixture/greet@v1.0.0
(cd "$g" && git tag -d v1.0.0 >/dev/null && echo '// moved' >>greet.salam && git commit -qam moved && git tag v1.0.0)
rm -rf "$SALAM_PKG/github.com/fixture/greet@v1.0.0" salam.lock
run "$salam" add github.com/fixture/greet@v1.0.0
check "a moved tag does not match the first download" "$(has 'does not match the copy this machine downloaded first' "$work/last.log" && echo 0 || echo 1)"
cd "$work" || exit 1

mkdir -p "$work/app4"
cd "$work/app4" || exit 1
printf '[package]\nname = "app4"\n' >salam.toml
printf 'github.com/fixture/dep@v1.0.0 sha256:%s\n' "0000000000000000000000000000000000000000000000000000000000000000" >"$work/sums.txt"
run "$salam" add github.com/fixture/dep@v1.0.0
check "a sum that differs from the published one is refused" "$(has 'published in the Salam registry' "$work/last.log" && echo 0 || echo 1)"
: >"$work/sums.txt"
run "$salam" add github.com/fixture/dep@v1.0.0
run "$salam" deps sums
check "deps sums prints the lines to publish" "$(grep -q '^github.com/fixture/dep@v1.0.0 sha256:[0-9a-f]\{64\}$' "$work/last.log" && echo 0 || echo 1)"
cd "$work" || exit 1

run "$salam" deps clean
check "deps clean empties the cache" "$([ ! -d "$SALAM_PKG/github.com" ] && echo 0 || echo 1)"

platform=""
case "$(uname -s 2>/dev/null)-$(uname -m 2>/dev/null)" in
Linux-x86_64) platform=linux-x86_64 ;;
Linux-aarch64) platform=linux-aarch64 ;;
Darwin-arm64) platform=macos-aarch64 ;;
Darwin-x86_64) platform=macos-x86_64 ;;
esac
if [ -n "$platform" ] && command -v python3 >/dev/null 2>&1 && command -v cc >/dev/null 2>&1 && command -v ar >/dev/null 2>&1; then
    mkdir -p "$work/nat/lib" "$work/natsrv"
    printf 'int fastc_twice(int x) { return 2 * x; }\n' >"$work/nat/fastc.c"
    cc -c -fPIC "$work/nat/fastc.c" -o "$work/nat/fastc.o" && ar rcs "$work/nat/lib/libfastc.a" "$work/nat/fastc.o"
    (cd "$work/nat" && tar -czf "$work/natsrv/libfastc.tar.gz" lib)
    nsum=$(python3 -c 'import hashlib, sys; print(hashlib.sha256(open(sys.argv[1], "rb").read()).hexdigest())' "$work/natsrv/libfastc.tar.gz")
    nport=$(python3 -c 'import socket; s = socket.socket(); s.bind(("127.0.0.1", 0)); print(s.getsockname()[1])')
    (cd "$work/natsrv" && exec python3 -m http.server "$nport" --bind 127.0.0.1 >/dev/null 2>&1) &
    nserver=$!
    sleep 1
    repo github.com/fixture/fast
    f="$work/remote/github.com/fixture/fast"
    printf '[package]\nname = "fast"\n\n[native.fastc.prebuilt.%s]\nurl = "http://127.0.0.1:%s/libfastc.tar.gz"\nsha256 = "%s"\nlib = "lib/libfastc.a"\n' "$platform" "$nport" "$nsum" >"$f/salam.toml"
    cat >"$f/fast.salam" <<'EOF'
package fast

link static "fastc"

extern:
    func fastc_twice(x: int): int
end

pub func Twice(x: int): int:
    ret fastc_twice(x)
end
EOF
    commit_tag github.com/fixture/fast v1.0.0
    mkdir -p "$work/natapp"
    cd "$work/natapp" || exit 1
    printf '[package]\nname = "natapp"\n' >salam.toml
    printf 'import "github.com/fixture/fast"\n\nfunc main:\n    println fast.Twice(21)\nend\n' >main.salam
    run "$salam" add github.com/fixture/fast
    run "$salam" run --backend=c --log-level=error
    check "a prebuilt native library is downloaded and linked" "$(has '^42$' "$work/last.log" && echo 0 || echo 1)"
    kill "$nserver" 2>/dev/null
    cd "$work" || exit 1
fi

mkdir -p "$work/builtin"
cd "$work/builtin" || exit 1
printf '[package]\nname = "builtin"\n' >salam.toml
printf 'import db.redis\n\nfunc main:\n    println redis.Ok(redis.Client {})\nend\n' >main.salam
run "$salam" run --log-level=error
check "a standard library package imports without salam add" "$(has '^false$' "$work/last.log" && echo 0 || echo 1)"
run "$salam" add db.redis
check "add says a standard library package needs no download" "$(has 'part of the standard library' "$work/last.log" && ! has 'db.redis' salam.toml && echo 0 || echo 1)"
printf '[package]\nname = "builtin"\n\n[dependencies]\n"db.redis" = "v0.5.9"\n' >salam.toml
run "$salam" deps
check "deps skips a standard library package an old manifest lists" "$(! has 'db.redis' salam.lock && echo 0 || echo 1)"
run "$salam" run --log-level=error
check "a manifest that still lists a standard library package builds" "$(has '^false$' "$work/last.log" && echo 0 || echo 1)"
run "$salam" search redis
check "search finds a standard library package" "$(has 'db.redis  (standard library' "$work/last.log" && echo 0 || echo 1)"
cd "$work" || exit 1

[ "$fails" -eq 0 ]
