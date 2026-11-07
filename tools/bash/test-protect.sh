#!/bin/sh
# Tests `salam build --protect` and its `--obfuscate` alias: the program still
# runs, while string literals and symbol names are not visible in the
# executable.

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

secret="salam-secret-payload-7f3a"
message="hello protect"

cat >secret.salam <<EOF
func secret_message(): str:
    ret "$secret"
end

func main:
    println secret_message()
    println "$message"
end
EOF

echo "== --protect"
"$salam" build secret.salam --protect --cc="$cc" --no-color >build.log 2>&1
rc=$?
if [ "$rc" -ne 0 ]; then
    echo "protect: build failed"
    sed 's/^/  /' build.log | head -20
    exit 1
fi
echo "protect: build ok"

got=$(./secret 2>&1)
want="$secret
$message"
if [ "$got" = "$want" ]; then
    echo "protect: run ok"
else
    echo "protect: run mismatch"
    printf '%s\n' "$got" | sed 's/^/  got: /'
    exit 1
fi

if command -v strings >/dev/null 2>&1; then
    if strings ./secret | grep -F "$secret" >/dev/null 2>&1; then
        echo "protect: secret literal is still readable"
        exit 1
    fi
    if strings ./secret | grep -F "$message" >/dev/null 2>&1; then
        echo "protect: printed literal is still readable"
        exit 1
    fi
    echo "protect: strings hidden"
else
    echo "protect: strings skipped (no strings tool)"
fi

if command -v nm >/dev/null 2>&1; then
    if nm ./secret 2>/dev/null | grep -F "secret_message" >/dev/null 2>&1; then
        echo "protect: symbol names are still present"
        exit 1
    fi
    echo "protect: symbols stripped"
else
    echo "protect: symbols skipped (no nm tool)"
fi

echo "== --obfuscate alias"
if ! "$salam" build secret.salam --obfuscate --cc="$cc" -o flagged --no-color >flag.log 2>&1; then
    echo "protect: --obfuscate build failed"
    sed 's/^/  /' flag.log | head -20
    exit 1
fi
out=$(./flagged 2>&1)
if [ "$out" = "$want" ]; then
    echo "protect: --obfuscate ok"
else
    echo "protect: --obfuscate run mismatch"
    exit 1
fi
