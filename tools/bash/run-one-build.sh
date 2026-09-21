#!/bin/sh

set -u
SALAM_BIN="$1"
WORK="$2"
label="$3"
f="$4"
lang="$5"
exp="$6"
shift 6
if [ "$#" -eq 1 ] && [ "$1" = "-" ]; then shift; fi

case "$f" in /* | [A-Za-z]:*) : ;; *) f="$(pwd)/$f" ;; esac
case "$WORK" in /* | [A-Za-z]:*) : ;; *) WORK="$(pwd)/$WORK" ;; esac

jobid=$(echo "$label" | tr '/ ' '__')
jobdir="$WORK/job_${jobid}_$$"
mkdir -p "$jobdir"
[ -d "$WORK/dbwork" ] && ln -s "$WORK/dbwork" "$jobdir/tests" 2>/dev/null
[ -d "$WORK/opencvwork" ] && ln -s "$WORK/opencvwork" "$jobdir/ocvtests" 2>/dev/null
[ -d "$WORK/cefwork" ] && ln -s "$WORK/cefwork" "$jobdir/ceftests" 2>/dev/null
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
_timedout=0
_try=1
_runout="$jobdir/run.out"
while [ "$_try" -le 4 ]; do
    timeout "${SALAM_TEST_TIMEOUT:-30}" "$exe" >"$_runout" 2>&1
    _rc=$?
    got=$(tr -d '\r' <"$_runout" 2>/dev/null)
    _timedout=0
    [ "$_rc" -eq 124 ] && _timedout=1
    if [ "$got" = "$want" ]; then break; fi
    case "$got" in *"Permission denied"* | "") sleep 1 ;; *) break ;; esac
    _try=$((_try + 1))
done
if [ "$got" = "$want" ]; then
    echo "PASS $label"
else
    echo "FAIL $label"
    if [ "$_timedout" -eq 1 ]; then
        echo "  timed out after ${SALAM_TEST_TIMEOUT:-30}s on attempt $_try of 4 - it hung, it did not finish and print this"
        echo "  anything it had not flushed died with it, so the text below may be short"
    fi
    echo "  expected: $(echo "$want" | tr '\n' '|')"
    echo "  got:      $(echo "$got" | tr '\n' '|')"
fi
rm -rf "$jobdir"
rm -f "$exe"
