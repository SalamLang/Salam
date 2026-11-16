#!/bin/sh
set -u
usage() {
    echo "usage: migrate-style.sh SALAM MIGRATE_TOOL [--write] file.salam..." >&2
    exit 2
}
[ "$#" -ge 3 ] || usage
SALAM="$1"
TOOL="$2"
shift 2
write=""
if [ "$1" = "--write" ]; then
    write="--write"
    shift
fi
tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT
status=0
for f in "$@"; do
    if grep -q '[^ -~]' "$f" && grep -q 'خطا\|پایان\|چیدمان\|صفحه' "$f"; then
        cp "$f" "$tmp/in.salam"
        "$SALAM" translate en "$tmp/in.salam" --output="$tmp/en.salam" >/dev/null 2>&1 || { echo "translate failed: $f" >&2; status=1; continue; }
        "$TOOL" --write "$tmp/en.salam" 2>"$tmp/report" || status=1
        sed "s|$tmp/en.salam|$f|" "$tmp/report" >&2
        "$SALAM" translate fa "$tmp/en.salam" --output="$tmp/out.salam" >/dev/null 2>&1 || { echo "translate back failed: $f" >&2; status=1; continue; }
        if ! head -n 1 "$f" | grep -q '^// زبان'; then
            sed -i '1{/^\/\/ زبان/d}' "$tmp/out.salam"
        fi
        if [ -n "$write" ]; then
            cp "$tmp/out.salam" "$f"
        else
            cat "$tmp/out.salam"
        fi
    else
        "$TOOL" $write "$f" || status=1
    fi
done
exit $status
