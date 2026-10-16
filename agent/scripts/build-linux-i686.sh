#!/usr/bin/env bash
# Build the 32-bit Linux bundles natively inside an i386 Debian container,
# since Ubuntu ships no i386 WebKitGTK to cross-compile against.
# Usage, from the repository root:
#   docker run --rm --platform linux/386 -v "$PWD:/src" -w /src \
#     -e TAURI_CLI_VERSION -e HOST_UID="$(id -u)" -e HOST_GID="$(id -g)" \
#     i386/debian:bookworm linux32 bash agent/scripts/build-linux-i686.sh
set -euo pipefail

: "${TAURI_CLI_VERSION:?set TAURI_CLI_VERSION}"
export CARGO_HOME=/src/agent/src-tauri/target/cargo-i686
export RUSTUP_HOME=/src/agent/src-tauri/target/cargo-i686/rustup
export PATH="$CARGO_HOME/bin:$PATH"

restore_owner() {
    if [ -n "${HOST_UID:-}" ]; then
        chown -R "$HOST_UID:${HOST_GID:-$HOST_UID}" /src/agent/src-tauri/target 2>/dev/null || true
    fi
}
trap restore_owner EXIT

export DEBIAN_FRONTEND=noninteractive
apt-get update -qq
apt-get install -y -qq --no-install-recommends \
    build-essential ca-certificates curl file pkg-config \
    libwebkit2gtk-4.1-dev libayatana-appindicator3-dev librsvg2-dev \
    libdbus-1-dev

if ! command -v rustup >/dev/null 2>&1; then
    curl -fsSL --proto '=https' --tlsv1.2 -o /tmp/rustup-init \
        https://static.rust-lang.org/rustup/dist/i686-unknown-linux-gnu/rustup-init
    chmod +x /tmp/rustup-init
    /tmp/rustup-init -y --no-modify-path --profile minimal \
        --default-host i686-unknown-linux-gnu --default-toolchain stable
fi
rustup update stable --no-self-update

if [ "$(cargo tauri --version 2>/dev/null)" != "tauri-cli $TAURI_CLI_VERSION" ]; then
    cargo install tauri-cli --version "=$TAURI_CLI_VERSION" --locked
fi

cd /src/agent
cargo tauri build --ci --target i686-unknown-linux-gnu --bundles deb,rpm
