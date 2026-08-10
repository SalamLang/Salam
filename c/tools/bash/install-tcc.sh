#!/bin/sh
# Installs the same tcc CI builds and the release bundles ship - the TinyCC
# commit pinned in c/tools/ci/tinycc-ref.txt - for local development (WSL,
# a Linux box, or an MSYS2 MINGW64 shell on Windows).
#
# Why not the distro package: `apt install tcc` on Ubuntu is
# 0.9.27+git20200814, a 2020 snapshot, and the last tagged upstream release
# (0.9.27) is from December 2017. Testing against either means testing a
# different compiler from the one users get.
#
# Usage:
#   c/tools/bash/install-tcc.sh [prefix]      # default: $HOME/.local/tcc
#
# Then put it first on PATH, ahead of any distro tcc:
#   export PATH="$HOME/.local/tcc/bin:$PATH"    # Linux/WSL
#   export PATH="$HOME/.local/tcc:$PATH"        # MSYS2/Windows layout
#
# Add that line to ~/.bashrc to make it stick. Verify with `tcc -v`: it
# should say 0.9.28rc and the pinned short SHA, not 0.9.27.

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
