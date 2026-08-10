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
    # Via build-tcc.sh so this gets the same pinned TinyCC commit as CI and
    # the release bundles. It used to clone master unpinned, which meant a
    # preview deploy could be built by a different compiler than anything
    # else in the project, chosen by whatever landed upstream that morning.
    echo "==> tcc not found, building the pinned TinyCC commit ..."
    sh c/tools/ci/build-tcc.sh "$HOME/.local/tcc"
    export PATH="$HOME/.local/tcc/bin:$HOME/.local/tcc:$PATH"
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
