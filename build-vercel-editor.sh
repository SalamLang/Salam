#!/bin/sh

set -eu

repo_root="$(cd "$(dirname "$0")" && pwd)"
cd "$repo_root"

command -v gcc >/dev/null 2>&1 || command -v clang >/dev/null 2>&1 || {
    echo "error: need gcc or clang on PATH to bootstrap the native salam compiler" >&2
    exit 1
}

if ! command -v tcc >/dev/null 2>&1; then
    echo "==> tcc not found, building the pinned TinyCC commit ..."
    sh tools/ci/build-tcc.sh "$HOME/.local/tcc"
    export PATH="$HOME/.local/tcc/bin:$HOME/.local/tcc:$PATH"
fi

if ! command -v salam >/dev/null 2>&1; then
    echo "==> no seed compiler on PATH, installing a released one ..."
    SALAM_INSTALL_DIR="$HOME/.salam/bin" SALAM_NO_MODIFY_PATH=1 sh install.sh
    export PATH="$HOME/.salam/bin:$PATH"
    command -v salam >/dev/null 2>&1 || {
        echo "error: install.sh ran but left no salam on PATH" >&2
        exit 1
    }
fi
salam version --short || true

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

sh tools/bash/update-playground.sh --selfhost
