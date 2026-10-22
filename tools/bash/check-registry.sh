#!/bin/sh
set -eu

root=$(CDPATH='' cd -- "$(dirname -- "$0")/../.." && pwd)
file=${1:-$root/registry/index.toml}

awk '
function fail(msg) { print FILENAME ":" NR ": " msg > "/dev/stderr"; bad = 1 }
function finish() {
    if (!open) return
    if (name == "") fail("entry without a name")
    if (desc == "") fail(name ": no description")
    if (length(desc) > 200) fail(name ": description longer than 200 characters")
    open = 0
}
/^[ \t]*#/ || /^[ \t]*$/ { next }
/^\[\[package\]\][ \t]*$/ { finish(); open = 1; name = ""; desc = ""; count++; next }
/^\[/ { fail("only [[package]] tables are allowed"); next }
!open { fail("key outside a [[package]] table"); next }
/^name[ \t]*=/ {
    v = $0; sub(/^name[ \t]*=[ \t]*"/, "", v); sub(/"[ \t]*$/, "", v)
    if (v !~ /^[a-z0-9-]+(\.[a-z0-9-]+)+\/[A-Za-z0-9_.-]+\/[A-Za-z0-9_.-]+(\/[A-Za-z0-9_.-]+)*$/) fail("name must be host/owner/repo: " v)
    if (v in seen) fail("listed twice: " v)
    seen[v] = 1; name = v; next
}
/^description[ \t]*=/ { v = $0; sub(/^description[ \t]*=[ \t]*"/, "", v); sub(/"[ \t]*$/, "", v); desc = v; next }
/^tags[ \t]*=/ { if ($0 !~ /^tags[ \t]*=[ \t]*\[([ \t]*"[a-z0-9.+-]+"[ \t]*,?)*[ \t]*\][ \t]*$/) fail("tags must be a list of lower-case words"); next }
{ fail("unknown key: " $0) }
END { finish(); if (bad) exit 1; print "registry: " count + 0 " package(s), all valid" }
' "$file"
