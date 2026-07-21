#!/bin/sh
# Build-and-run a single test in an isolated working directory so parallel
# jobs don't collide on the generated salam_mod_*.c/.h files (those are
# always written relative to the process's cwd). Prints PASS/FAIL lines in
# the same format run-tests.sh expects.
#
# args: <abs-salam-bin> <work-dir> <label> <salam-file> <lang> <expected-out> [extra salam-build args...]
#

set -u
SALAM_BIN="$1"
WORK="$2"
label="$3"
f="$4"
lang="$5"
exp="$6"
shift 6
if [ "$#" -eq 1 ] && [ "$1" = "-" ]; then shift; fi

# /* and C:\ or C:/ are already absolute; only prefix genuinely relative paths.
# (On Git Bash/MSYS runners TMPDIR is a Windows path, so prefixing it breaks it.)
case "$f" in /* | [A-Za-z]:*) : ;; *) f="$(pwd)/$f" ;; esac
case "$WORK" in /* | [A-Za-z]:*) : ;; *) WORK="$(pwd)/$WORK" ;; esac

jobid=$(echo "$label" | tr '/ ' '__')
jobdir="$WORK/job_${jobid}_$$"
mkdir -p "$jobdir"
[ -d "$WORK/dbwork" ] && ln -s "$WORK/dbwork" "$jobdir/tests" 2>/dev/null
exe="$WORK/exe_$jobid.exe"
rm -f "$exe"

buildlog="$jobdir/build.log"
build_once() {
    (cd "$jobdir" && "$SALAM_BIN" build "$f" --output="$exe" \
        --no-color --log-level=error --lang="$lang" "$@") >"$buildlog" 2>&1
}

build_once "$@"
_btry=1
while [ ! -x "$exe" ] && [ "$_btry" -le 3 ]; do
    sleep "$_btry"
    build_once "$@"
    _btry=$((_btry + 1))
done
if [ ! -x "$exe" ]; then
    echo "FAIL $label (build failed)"
    sed 's/^/  /' "$buildlog" 2>/dev/null | head -20
    rm -rf "$jobdir"
    exit 0
fi

want=$(tr -d '\r' <"$exp")
got=""
_try=1
while [ "$_try" -le 4 ]; do
    got=$("$exe" 2>&1 | tr -d '\r')
    if [ "$got" = "$want" ]; then break; fi
    case "$got" in *"Permission denied"* | "") sleep 1 ;; *) break ;; esac
    _try=$((_try + 1))
done
if [ "$got" = "$want" ]; then
    echo "PASS $label"
else
    echo "FAIL $label"
    echo "  expected: $(echo "$want" | tr '\n' '|')"
    echo "  got:      $(echo "$got" | tr '\n' '|')"
fi
rm -rf "$jobdir"
rm -f "$exe"
