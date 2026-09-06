#!/usr/bin/env sh
# prek hook: keep docs/ai/stdlib-index.json in step with the std/ tree.
#
# The MCP server workflow rebuilds the index from std/ and fails the pull
# request when the committed copy disagrees. Regenerating here turns that
# into a blocked commit, which is a much shorter feedback loop than a red PR.
#
# Only a checkout that has built the server can regenerate the index, and a
# fresh clone (or the prek CI job, which never builds Salam) has neither
# salam-mcp nor necessarily node. There the hook says what is missing and
# passes; the workflow stays the real gate.
#
# Usage: tools/mcp/check-index.sh
set -eu

root=$(CDPATH='' cd -- "$(dirname -- "$0")/../.." && pwd)
index=docs/ai/stdlib-index.json

server=${SALAM_MCP:-"$root/salam-mcp"}
[ -x "$server" ] || server="$root/salam-mcp.exe"
if [ ! -x "$server" ]; then
    printf 'salam-mcp is not built; skipping the %s refresh.\n' "$index" >&2
    printf 'Run tools/mcp/build.sh to have this hook check the index.\n' >&2
    exit 0
fi

if ! command -v node >/dev/null 2>&1; then
    printf 'node is not installed; skipping the %s refresh.\n' "$index" >&2
    exit 0
fi

before=$(git -C "$root" hash-object "$index" 2>/dev/null || printf 'missing')
"$root/tools/mcp/gen-index.sh"
after=$(git -C "$root" hash-object "$index")

if [ "$before" = "$after" ]; then
    exit 0
fi

printf '%s was out of date and has been regenerated.\n' "$index" >&2
printf 'Stage it (git add %s) and commit again.\n' "$index" >&2
exit 1
