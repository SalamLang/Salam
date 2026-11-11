#!/bin/sh

set -eu
root=$(cd "$(dirname "$0")/.." && pwd)
cd "$root"
: "${SALAM:=$root/salam}"
export SALAM
out=website/.cache/blog-code/run-$$
rm -rf "$out"
mkdir -p "$out"
trap 'rm -rf "$out"' EXIT

if [ "$#" -eq 0 ]; then
    set -- website/content/*/blog/*.md
fi

for md in "$@"; do
    [ -f "$md" ] || continue
    lang=$(basename "$(dirname "$(dirname "$md")")")
    slug=$(basename "$md" .md)
    mkdir -p "$out/$lang"
    awk -v dir="$out/$lang" -v slug="$slug" '
        function trim(s) { sub(/^[ \t]+/, "", s); sub(/[ \t]+$/, "", s); return s }
        function indent(s) { match(s, /^ */); return RLENGTH }
        {
            t = trim($0)
            if (mode == "") {
                if (pending != "" && t == "") next
                if (pending != "" && t == "```output") { mode = "out"; ind = indent($0); file = pending ".out"; printf "" > file; pending = ""; next }
                pending = ""
                if (t ~ /^```/) {
                    info = substr(t, 4)
                    n++
                    if (info ~ /^salam([ \t]|$)/ && info !~ /fragment/) {
                        base = sprintf("%s/%s-%02d", dir, slug, n)
                        file = base ".salam"
                        printf "" > file
                        mode = "code"
                    } else {
                        mode = "skip"
                    }
                    ind = indent($0)
                }
                next
            }
            if (t == "```") {
                if (mode == "code") pending = base
                close(file)
                mode = ""
                next
            }
            if (mode == "skip") next
            line = $0
            cut = indent(line) < ind ? indent(line) : ind
            print substr(line, cut + 1) >> file
        }
    ' "$md"
done

count=$(find "$out" -name '*.salam' | wc -l | tr -d ' ')
jobs=${BLOG_JOBS:-$(nproc 2>/dev/null || echo 4)}
echo "website: checking $count code blocks in the magazine"
[ "$count" -gt 0 ] || exit 0
find "$out" -name '*.salam' | LC_ALL=C sort | tr '\n' '\0' |
    xargs -0 -P "$jobs" -n 1 sh website/check-example.sh
