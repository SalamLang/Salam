#!/bin/sh
# Build-and-run a single test in an isolated working directory so parallel
# jobs don't collide on the generated salam_mod_*.c/.h files (those are
# always written relative to the process's cwd). Prints PASS/FAIL lines in
# the same format run-tests.sh expects.
#
# args: <abs-salam-bin> <work-dir> <label> <salam-file> <lang> <expected-out> [extra salam-build args...]
set -u
SALAM_BIN="$1"
WORK="$2"
label="$3"
f="$4"
lang="$5"
exp="$6"
shift 6
if [ "$#" -eq 1 ] && [ "$1" = "-" ]; then shift; fi

# Resolve to absolute paths up front so the cd below (needed to isolate each
# job's generated salam_mod_*.c/.h in its own dir) can't turn an
# already-absolute path into a bogus nested one via naive concatenation.
case "$f" in /*) : ;; *) f="$(pwd)/$f" ;; esac
case "$WORK" in /*) : ;; *) WORK="$(pwd)/$WORK" ;; esac

jobid=$(echo "$label" | tr '/ ' '__')
jobdir="$WORK/job_${jobid}_$$"
mkdir -p "$jobdir"
exe="$WORK/exe_$jobid.exe"
rm -f "$exe"

build_once() {
    (cd "$jobdir" && "$SALAM_BIN" build "$f" --output="$exe" \
        --no-color --log-level=error --lang="$lang" "$@") >/dev/null 2>&1
}

build_once "$@"
if [ ! -x "$exe" ]; then
    sleep 1
    build_once "$@"
fi
if [ ! -x "$exe" ]; then
    echo "FAIL $label (build failed)"
    rm -rf "$jobdir"
    exit 0
fi

got=""
_try=1
while [ "$_try" -le 4 ]; do
    got=$("$exe" 2>&1 | tr -d '\r')
    case "$got" in *"Permission denied"* | "") sleep 1 ;; *) break ;; esac
    _try=$((_try + 1))
done
want=$(tr -d '\r' <"$exp")
if [ "$got" = "$want" ]; then
    echo "PASS $label"
else
    echo "FAIL $label"
    echo "  expected: $(echo "$want" | tr '\n' '|')"
    echo "  got:      $(echo "$got" | tr '\n' '|')"
fi
rm -rf "$jobdir"
rm -f "$exe"
