#!/usr/bin/env sh
# Regenerates docs/ai/stdlib-index.json from the std/ tree.
#
# The server computes the index live; this just captures that output into the
# committed copy so tools that cannot run the server (or read it straight from
# GitHub) see the same data. CI fails if the two disagree.
#
# Usage: tools/mcp/gen-index.sh
set -eu

root=$(CDPATH='' cd -- "$(dirname -- "$0")/../.." && pwd)
server=${SALAM_MCP:-"$root/salam-mcp"}
[ -x "$server" ] || server="$root/salam-mcp.exe"
if [ ! -x "$server" ]; then
    printf 'salam-mcp not found; run tools/mcp/build.sh first\n' >&2
    exit 1
fi

response=$(mktemp)
trap 'rm -f "$response"' EXIT

printf '%s\n' \
    '{"jsonrpc":"2.0","id":1,"method":"resources/read","params":{"uri":"salam://stdlib/index.json"}}' |
    SALAM_MCP_ROOT="$root" "$server" 2>/dev/null >"$response"

RESPONSE_PATH="$response" OUT_PATH="$root/docs/ai/stdlib-index.json" node -e '
    const fs = require("fs");
    const line = fs.readFileSync(process.env.RESPONSE_PATH, "utf8").trim().split("\n")[0];
    const obj = JSON.parse(JSON.parse(line).result.contents[0].text);
    fs.writeFileSync(process.env.OUT_PATH, JSON.stringify(obj, null, 2) + "\n");
    const names = Object.keys(obj.packages);
    const decls = names.reduce((n, k) => n + obj.packages[k].length, 0);
    console.error(`wrote ${process.env.OUT_PATH}: ${names.length} packages, ${decls} declarations`);
'
