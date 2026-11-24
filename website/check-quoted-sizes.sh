#!/bin/sh

set -u
root=$(cd "$(dirname "$0")/.." && pwd)
cd "$root" || exit 1

tmp=$(mktemp) || exit 1
trap 'rm -f "$tmp"' EXIT

scan() {
    awk -v file="$1" '
        BEGIN { skipnames = "|content|title|alt|srcset|sizes|imagesizes|imagesrcset|"
            skipnames = skipnames "اندازه‌ها|اندازه‌های تصویر|مجموعه منبع|مجموعه منبع تصویر|محتوا|عنوان|نام|مقدار|متن|نشانی|برچسب|شرح|توضیح|جایگزین|"
            skipnames = skipnames "placeholder|splash_color|value|label|name|url|href|src|media|description|keywords|text|pattern|action|style|viewport|timeout|" }
        {
            line = $0
            gsub(/&quot;/, "\"", line)
            while (match(line, /[^[:space:][:digit:]=:",[#.-][^=:",[]* *= *("-?[0-9.]+ ?(px|em|rem|%|vh|vw|vmin|vmax|ms|s|deg|pt|fr|ex|ch|cm|mm|in)"")/)) {
                hit = substr(line, RSTART, RLENGTH)
                name = hit
                sub(/ *=.*$/, "", name)
                n = split(name, w, " ")
                key = w[n]
                full = name
                sub(/^ +/, "", full)
                sub(/ +$/, "", full)
                if (index(skipnames, "|" key "|") == 0 && index(skipnames, "|" full "|") == 0) {
                    print file ":" NR ": quoted size: " hit
                }
                line = substr(line, RSTART + RLENGTH)
            }
        }
    ' "$1"
}

is_page_code() {
    grep -q '^[[:space:]]*\(page\|صفحه\|component\|بخش\)[^=]*:' "$1"
    case $? in
    0) return 0 ;;
    1) return 1 ;;
    *) echo "website: cannot read $1" >&2; exit 2 ;;
    esac
}

for f in website/content/*/examples/*.salam examples/*.salam examples/*/*.salam; do
    [ -f "$f" ] || continue
    is_page_code "$f" || continue
    scan "$f" "$f" || exit 1
done >>"$tmp"

for f in website/content/*/learn/*.txt website/content/*/blog/*.md books/*/*/chapters/*.tex docs/*.md README.md SKILL.md; do
    [ -f "$f" ] || continue
    scan "$f" "$f" || exit 1
done >>"$tmp"

if [ -s "$tmp" ]; then
    cat "$tmp"
    echo "website: quoted sizes in page code; write 24px, 100%, 8, 16 without quotes (colours and words are quoted)"
    exit 1
fi
echo "website: no quoted sizes in page code"
