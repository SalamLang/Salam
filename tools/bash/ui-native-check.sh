#!/bin/sh
# Checks that the native page target expands its elements the same way on the
# C and native backends and with every spelling of --stdlib-path: the repo
# root, the std folder, with and without a leading ./, and SALAM_STD. A path
# that was misread left the layout element tables empty, and every element
# then failed with E111 unknown layout element.
#
# Usage: tools/bash/ui-native-check.sh <salam>

set -u
SALAM=$1
case "$SALAM" in /*) ;; */*) SALAM="$(pwd)/$SALAM" ;; esac
HERE=$(cd "$(dirname "$0")" && pwd)
ROOT=$(cd "$HERE/../.." && pwd)
cd "$ROOT" || exit 1
fail=0

run_check() {
    label=$1
    shift
    out=$("$@" 2>&1)
    rc=$?
    if [ "$rc" -ne 0 ]; then
        echo "FAIL $label"
        echo "$out" | head -5
        fail=1
    else
        echo "ok   $label"
    fi
}

for src in examples/ui/calculator.salam tests/en/page/components/component_single.salam; do
    for backend in c native; do
        for sp in ./std std ./ . "$ROOT" "$ROOT/std"; do
            run_check "$src $backend --stdlib-path=$sp" \
                "$SALAM" check "$src" --ui=native --backend=$backend --stdlib-path="$sp" --log-level=error
        done
        run_check "$src $backend SALAM_STD" \
            env SALAM_STD="$ROOT/std" "$SALAM" check "$src" --ui=native --backend=$backend --log-level=error
    done
done

exit $fail
