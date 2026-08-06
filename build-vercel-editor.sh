#!/bin/sh
# Vercel build command for PR-preview deploys of the Salam web editor (editor/).

set -eu

repo_root="$(cd "$(dirname "$0")" && pwd)"
cd "$repo_root"

command -v gcc >/dev/null 2>&1 || command -v clang >/dev/null 2>&1 || {
    echo "error: need gcc or clang on PATH to bootstrap the native salam compiler" >&2
    exit 1
}

if ! command -v tcc >/dev/null 2>&1; then
    echo "==> tcc not found, building from source ..."
    tcc_src="$(mktemp -d)"
    git clone --depth 1 https://github.com/TinyCC/tinycc.git "$tcc_src"
    (
        cd "$tcc_src"
        ./configure --prefix="$HOME/.local"
        make -j"$(nproc)"
        make install
    )
    export PATH="$HOME/.local/bin:$PATH"
fi

emsdk_dir="$HOME/emsdk"
if [ ! -x "$emsdk_dir/emsdk" ]; then
    git clone --depth 1 https://github.com/emscripten-core/emsdk.git "$emsdk_dir"
fi
(
    cd "$emsdk_dir"
    if [ ! -d "upstream/emscripten" ]; then
        ./emsdk install latest
        ./emsdk activate latest
    fi
)
. "$emsdk_dir/emsdk_env.sh"

cd "$repo_root/c"
sh tools/bash/update-playground.sh
