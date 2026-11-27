#!/bin/sh
# Builds pages that pull in std/gui/ui for the web under a CPU time limit, so a
# compiler slowdown on large imported packages fails here instead of hanging the
# playground.

set -u

salam=${1:-}
root=$(CDPATH='' cd -- "$(dirname -- "$0")/../.." && pwd)
[ -n "$salam" ] || salam="$root/salam"
case "$salam" in /* | [A-Za-z]:*) ;; *) salam="$root/$salam" ;; esac

limit=${SALAM_WEB_BUILD_CPU_LIMIT:-25}
work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT INT TERM
export SALAM_STD="$root/std"
cd "$work" || exit 1

for lang in en fa; do
    for name in events_rows scope_list; do
        cp "$root/tests/$lang/ui/$name.salam" "${lang}_$name.salam"
        if (
            # shellcheck disable=SC3045
            ulimit -t "$limit"
            exec "$salam" build "${lang}_$name.salam" --platform=web --lang="$lang" \
                --no-color --log-level=error
        ) >"${lang}_$name.log" 2>&1 && [ -s "${lang}_$name.js" ]; then
            echo "$lang $name: built within the CPU limit"
        else
            echo "$lang $name: failed or passed the ${limit}s CPU limit"
            tail -5 "${lang}_$name.log"
        fi
    done
done
