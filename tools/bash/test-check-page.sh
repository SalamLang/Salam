#!/bin/sh
# 'salam check' on a page project reads salam.toml, checks the pages and, for
# web = "ssr", the server build, and writes nothing.

set -u

salam=${1:-}
root=$(CDPATH='' cd -- "$(dirname -- "$0")/../.." && pwd)
[ -n "$salam" ] || salam="$root/salam"
case "$salam" in /* | [A-Za-z]:*) ;; *) salam="$root/$salam" ;; esac

work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT INT TERM
export SALAM_STD="$root/std"
cp -R "$root/tests/en/project/check_page" "$work/app"
cd "$work/app" || exit 1

check() {
    title=$1
    shift
    echo "== $title"
    "$salam" check "$@" --no-color --log-level=error >"$work/out.txt" 2>&1
    echo "exit $?"
    grep -o 'nothing_here\|missing_name' "$work/out.txt" | sort -u
    grep -c "does not define a 'main' function\|unknown option" "$work/out.txt" | sed 's/^/entry complaints: /'
}

check "clean project" .
check "clean entry file" main.salam --platform=web
find . -type f | sort

cp pages/index.salam "$work/index.keep"
printf 'pub component Home:\n    paragraph: content = nothing_here end\nend\n' >pages/index.salam
check "error in a page, project" .
check "error in a page, file" main.salam --platform=web
cp "$work/index.keep" pages/index.salam

sed 's/ret n \* 3/ret missing_name/' main.salam >"$work/main.broken"
cp "$work/main.broken" main.salam
check "error in a server func" .

mkdir "$work/static" && cd "$work/static" || exit 1
printf 'server func Price(n: int): int:\n    ret n * 3\nend\n\npage:\n    paragraph:\n        content = "total {Price(2)}"\n    end\nend\n' >main.salam
check "server call, asked as ssr" main.salam --platform=web --ssr

mkdir "$work/persian" && cd "$work/persian" || exit 1
printf '[package]\nname = "persian"\nversion = "0.1.0"\n\n[project]\ntype = "dsl"\nplatforms = ["web"]\n' >salam.toml
printf '// زبان: فارسی\nصفحه:\n    پاراگراف:\n        محتوا = "درود"\n    پایان\nپایان\n' >main.salam
check "persian page project" .
