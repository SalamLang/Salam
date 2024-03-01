#!/usr/bin/env sh
# Starts salam-mcp for an MCP client, building it first when the binary is
# missing or older than the server sources. stdout belongs to the JSON-RPC
# stream, so everything the build prints goes to stderr.
set -eu

here=$(CDPATH='' cd -- "$(dirname -- "$0")" && pwd)
root=$(CDPATH='' cd -- "$here/../.." && pwd)
bin=${SALAM_MCP:-"$root/salam-mcp"}

needs_build() {
    [ -x "$bin" ] || return 0
    for src in "$here"/*.salam; do
        [ "$src" -nt "$bin" ] && return 0
    done
    return 1
}

if needs_build; then
    printf '[salam-mcp] %s is missing or stale; building it\n' "$bin" >&2
    tmp="$bin.tmp.$$"
    if ! "$here/build.sh" "$tmp" 1>&2; then
        rm -f "$tmp"
        printf '[salam-mcp] build failed. Build the compiler with tools/bash/build-selfhost.sh, or set SALAM to a salam binary.\n' >&2
        exit 1
    fi
    mv -f "$tmp" "$bin"
fi

SALAM_MCP_ROOT=${SALAM_MCP_ROOT:-"$root"}
export SALAM_MCP_ROOT
exec "$bin" "$@"
