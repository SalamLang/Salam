#!/bin/sh

set -u
root=$(cd "$(dirname "$0")/.." && pwd)
cd "$root" || exit 1

tmp=$(mktemp) || exit 1
trap 'rm -f "$tmp"' EXIT

scan() {
    awk -v file="$1" '
        BEGIN { skipnames = "|content|title|alt|srcset|sizes|imagesizes|imagesrcset|placeholder|value|label|name|url|href|src|media|description|keywords|text|pattern|action|style|viewport|timeout|" }
        {
            line = $0
            gsub(/&quot;/, "\"", line)
            if (line ~ /svg|SVG|canvas/) next
            while (match(line, /[A-Za-z_\307-\377][^=",\[]* = *"-?[0-9.]+ ?(px|em|rem|%|vh|vw|vmin|vmax|ms|s|deg|pt|fr|ex|ch|cm|mm|in)"/)) {
                hit = substr(line, RSTART, RLENGTH)
                name = hit
                sub(/ *= .*$/, "", name)
                n = split(name, w, " ")
                key = w[n]
                if (index(skipnames, "|" key "|") == 0) {
                    print file ":" NR ": quoted size: " hit
                }
                line = substr(line, RSTART + RLENGTH)
            }
        }
    ' "$1"
}

is_page_code() {
    grep -q '^[[:space:]]*\(page\|صفحه\|component\|بخش\)[^=]*:' "$1"
}

for f in website/content/*/examples/*.salam examples/*.salam examples/*/*.salam; do
    [ -f "$f" ] || continue
    is_page_code "$f" || continue
    scan "$f"
done >>"$tmp"

for f in website/content/*/learn/*.txt website/content/*/blog/*.md books/*/*/chapters/*.tex docs/*.md README.md SKILL.md; do
    [ -f "$f" ] || continue
    scan "$f"
done >>"$tmp"

if [ -s "$tmp" ]; then
    cat "$tmp"
    echo "website: quoted sizes in page code; write 24px, 100% and 8, 16 without quotes"
    exit 1
fi
echo "website: no quoted sizes in page code"
