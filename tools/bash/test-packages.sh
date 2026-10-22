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
unset SALAM_NO_GIT SALAM_RELEASES_URL SALAM_PACKAGES SALAM_REGISTRY

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

run "$salam" deps clean
check "deps clean empties the cache" "$([ ! -d "$SALAM_PKG/github.com" ] && echo 0 || echo 1)"

[ "$fails" -eq 0 ]
