#!/bin/sh
# Build-and-run a single JS-backend test in an isolated working directory so
# parallel jobs don't collide on the generated bundle file. Compiles with
# `salam js`, then runs the bundle under Node with tools/js/dom_stub.js
# preloaded (see that file for why: every bundle checks document.readyState
# unconditionally, even DOM-free programs). Prints PASS/FAIL lines in the same
# format run-tests.sh expects.
#
# args: <abs-salam-bin> <work-dir> <label> <salam-file> <lang> <expected-out>
#

set -u
SALAM_BIN="$1"
WORK="$2"
label="$3"
f="$4"
lang="$5"
exp="$6"

case "$f" in /* | [A-Za-z]:*) : ;; *) f="$(pwd)/$f" ;; esac
case "$WORK" in /* | [A-Za-z]:*) : ;; *) WORK="$(pwd)/$WORK" ;; esac

RUNNER="$(dirname "$0")/../js/run_bundle.js"
case "$RUNNER" in /* | [A-Za-z]:*) : ;; *) RUNNER="$(pwd)/$RUNNER" ;; esac

jobid=$(echo "$label" | tr '/ ' '__')
jobdir="$WORK/job_${jobid}_$$"
mkdir -p "$jobdir"
bundle="$jobdir/bundle.js"

build_once() {
    (cd "$jobdir" && "$SALAM_BIN" js "$f" --output="$bundle" \
        --no-color --log-level=error --lang="$lang") >/dev/null 2>&1
}

build_once
if [ ! -s "$bundle" ]; then
    sleep 1
    build_once
fi
if [ ! -s "$bundle" ]; then
    echo "FAIL $label (build failed)"
    rm -rf "$jobdir"
    exit 0
fi

want=$(tr -d '\r' <"$exp")
got=$(node "$RUNNER" "$bundle" 2>&1 | tr -d '\r')
if [ "$got" = "$want" ]; then
    echo "PASS $label"
else
    echo "FAIL $label"
    echo "  expected: $(echo "$want" | tr '\n' '|')"
    echo "  got:      $(echo "$got" | tr '\n' '|')"
fi
rm -rf "$jobdir"
