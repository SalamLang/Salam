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
    sh tools/ci/build-tcc.sh "$HOME/.local/tcc"
    export PATH="$HOME/.local/tcc/bin:$HOME/.local/tcc:$PATH"
fi

# Since c/ was dropped, tools/bash/build-selfhost.sh has no in-tree way to
# bootstrap: the compiler is written in Salam, so it needs an existing Salam as
# the seed. CI gets one from .github/actions/setup-salam; Vercel runs with
# installCommand "true" and has nothing on PATH, which is what has been failing
# every preview deploy since #1546.
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

# tools/ scripts self-anchor to the repository root, so no cd into c/.
# --selfhost matches the two editor CI workflows, so a preview deploy is built
# by the same compiler as the published playground rather than by the C one.
sh tools/bash/update-playground.sh --selfhost
