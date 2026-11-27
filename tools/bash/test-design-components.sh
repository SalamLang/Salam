#!/bin/sh

set -eu

SALAM="${1:-./salam}"
ROOT="$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)"
SALAM="$(CDPATH= cd -- "$(dirname -- "$SALAM")" && pwd)/$(basename -- "$SALAM")"
export SALAM_STD="${SALAM_STD:-$ROOT/std}"

dir="$(mktemp -d)"
trap 'rm -rf "$dir"' EXIT
status=0

rules() {
    tr -d '\n' <"$1" | sed 's/.*<style>//; s/<\/style>.*//' | tr '}' '\n' |
        grep -E 'data-slot|data-variant' | sed 's/^[[:space:]]*//' | sort -u
}

for lang in en fa; do
    for variant in components inline; do
        (cd "$dir" && "$SALAM" build "$ROOT/tests/$lang/page/projects/design_$variant/main.salam" \
            --platform=web --no-minify --log-level=error --output="$dir/$lang-$variant.html")
        rules "$dir/$lang-$variant.html" >"$dir/$lang-$variant.css"
    done
    missing="$(comm -13 "$dir/$lang-components.css" "$dir/$lang-inline.css")"
    count="$(wc -l <"$dir/$lang-inline.css")"
    if [ "$count" -lt 20 ]; then
        echo "FAIL $lang: only $count design rules in the reference page"
        status=1
    elif [ -n "$missing" ]; then
        echo "FAIL $lang: rules missing when the controls live in components/"
        echo "$missing" | head -20
        status=1
    elif ! grep -q 'window.salamDsAccordion' "$dir/$lang-components.html"; then
        echo "FAIL $lang: the accordion script is missing"
        status=1
    else
        echo "PASS $lang: $count design rules in both layouts"
    fi
done

exit "$status"
