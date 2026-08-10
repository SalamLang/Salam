#!/bin/sh
# Exercise the salam subcommands run-tests.sh never reaches, so the leak gate
# covers the whole CLI rather than the eight verbs the corpus happens to use.
#
#   run-tests.sh runs: build, exec, format, inspect, js, layout build, llvm, web
#   this script adds: version, help (per language), new, run, obj, doc, cli
#                     (REPL), layout (REPL), --time-report/--time-trace,
#                     --keep-c, -g, --release, -B, --asan, and the argument
#                     error paths (bad file, bad option, unknown command)
#
# It grades nothing: exit codes are printed for eyeballing, and the verdict is
# whatever LSan wrote to $ASAN_OPTIONS' log_path. Run it through
# leakcheck.sh, which sets that up; on its own it is just a smoke sweep.
#
# Usage: sh tools/bash/leakcheck-sweep.sh <salam-binary>

set -u

. "$(dirname "$0")/lib.sh"

SALAM_BIN="${1:-}"
[ -n "$SALAM_BIN" ] || {
    echo "usage: $0 <salam-binary>" >&2
    exit 2
}
case "$SALAM_BIN" in /* | [A-Za-z]:*) ;; *) SALAM_BIN="$(pwd)/$SALAM_BIN" ;; esac
[ -x "$SALAM_BIN" ] || {
    echo "sweep: '$SALAM_BIN' is not executable" >&2
    exit 2
}

# lib.sh puts us at the repository root, where std/ and tests/ live.
ROOT=$(pwd)
if [ -z "${SALAM_STD:-}" ] && [ -d "$ROOT/std" ]; then
    SALAM_STD="$ROOT/std"
    export SALAM_STD
fi

W=${TMPDIR:-/tmp}/salam-leak-sweep.$$
rm -rf "$W"
mkdir -p "$W"
trap 'rm -rf "$W"' EXIT INT TERM

SRC="$ROOT/tests/en/basics/hello.salam"
[ -f "$SRC" ] || {
    echo "sweep: missing $SRC" >&2
    exit 2
}
cp "$SRC" "$W/hello.salam"
cd "$W" || exit 1

# Pick a real C compiler for the handful of cases that link: tcc is the
# default and is not installed everywhere this runs.
CCPICK=""
for c in gcc clang cc tcc; do
    command -v "$c" >/dev/null 2>&1 && {
        CCPICK="--cc=$c"
        break
    }
done

n=0
try() {
    lbl="$1"
    shift
    n=$((n + 1))
    "$@" >/dev/null 2>&1
    printf '  %-28s rc=%s\n' "$lbl" "$?"
}

try version "$SALAM_BIN" version
try version-short "$SALAM_BIN" version --short
try help "$SALAM_BIN" help
try help-subcommand "$SALAM_BIN" help build
try help-fa "$SALAM_BIN" help --lang=fa
try help-ar "$SALAM_BIN" help --lang=ar
try no-args "$SALAM_BIN"

try new-en "$SALAM_BIN" new p_en
try new-fa "$SALAM_BIN" new p_fa --lang=fa
try new-ar "$SALAM_BIN" new p_ar --lang=ar

# shellcheck disable=SC2086 # $CCPICK is a single optional flag
try run "$SALAM_BIN" run hello.salam $CCPICK
# shellcheck disable=SC2086
try obj "$SALAM_BIN" obj hello.salam $CCPICK

try doc-file "$SALAM_BIN" doc hello.salam --output=d1.html
try doc-dir "$SALAM_BIN" doc "$ROOT/std/str" --output=d2.html
try doc-fa "$SALAM_BIN" doc hello.salam --output=d3.html --lang=fa

try inspect-ast "$SALAM_BIN" inspect hello.salam --emit-ast-xml
try inspect-symbols "$SALAM_BIN" inspect hello.salam --emit-symbol-xml

# shellcheck disable=SC2086
try time-report "$SALAM_BIN" build hello.salam --output=t1 $CCPICK --time-report
# shellcheck disable=SC2086
try time-report-json "$SALAM_BIN" build hello.salam --output=t2 $CCPICK --time-report=json
# shellcheck disable=SC2086
try time-trace "$SALAM_BIN" build hello.salam --output=t3 $CCPICK --time-trace=tr.json
# shellcheck disable=SC2086
try keep-c "$SALAM_BIN" build hello.salam --output=t4 $CCPICK --keep-c
# shellcheck disable=SC2086
try debug-info "$SALAM_BIN" build hello.salam --output=t5 $CCPICK -g
# shellcheck disable=SC2086
try release "$SALAM_BIN" build hello.salam --output=t6 $CCPICK --release
# shellcheck disable=SC2086
try force-rebuild "$SALAM_BIN" build hello.salam --output=t7 $CCPICK -B
# shellcheck disable=SC2086
try asan-build "$SALAM_BIN" build hello.salam --output=t8 $CCPICK --asan

try js "$SALAM_BIN" js hello.salam --output=a.js
try js-html "$SALAM_BIN" js hello.salam --output=a.html
try format-recursive "$SALAM_BIN" format "$ROOT/tests/en/basics" --check -r

# Error paths allocate too, and they are exactly where a cleanup is most
# likely to have been forgotten.
try err-missing-file "$SALAM_BIN" build "$W/does-not-exist.salam"
try err-bad-option "$SALAM_BIN" build hello.salam --not-a-real-option
try err-unknown-cmd "$SALAM_BIN" frobnicate
try err-bad-lang "$SALAM_BIN" build hello.salam --lang=nonesuch

# The two REPLs, driven off a here-document so they take a full line-editing
# path and then exit. Nothing else in CI ever enters them.
n=$((n + 1))
"$SALAM_BIN" cli >/dev/null 2>&1 <<'EOF'
1 + 2
:help
:quit
EOF
printf '  %-28s rc=%s\n' repl-cli "$?"

n=$((n + 1))
"$SALAM_BIN" layout >/dev/null 2>&1 <<'EOF'
:help
:quit
EOF
printf '  %-28s rc=%s\n' repl-layout "$?"

echo "  ($n invocations)"
