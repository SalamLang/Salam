#!/bin/sh

set -eu

ROOT=$(cd "$(dirname "$0")/../.." && pwd)
PREFIX=${1:-$HOME/.local/tcc}

for tool in git make gcc; do
    command -v "$tool" >/dev/null 2>&1 || {
        echo "error: need $tool on PATH to build tcc" >&2
        echo "  Debian/Ubuntu/WSL: sudo apt-get install -y git build-essential" >&2
        exit 1
    }
done

sh "$ROOT/tools/ci/build-tcc.sh" "$PREFIX"

bin=$PREFIX
[ -x "$PREFIX/bin/tcc" ] && bin=$PREFIX/bin

cat <<EOF

Installed. Put it ahead of any distro tcc on PATH:

    export PATH="$bin:\$PATH"

and add that to ~/.bashrc. Check with: tcc -v
EOF
