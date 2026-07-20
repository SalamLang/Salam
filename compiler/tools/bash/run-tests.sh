#!/bin/sh
# Salam end-to-end test suite.

set -u
. "$(dirname "$0")/lib.sh"
salam_ensure_compiler --quiet
WORK="${WORK:-${TMPDIR:-/tmp}/salam-run-tests-work.$$}"
mkdir -p "$WORK"
trap 'rm -rf "$WORK"' EXIT
pass=0
fail=0
LANGS="${LANGS:-en fa ar}"
NPROC="${NPROC:-$(command -v nproc >/dev/null 2>&1 && nproc || getconf _NPROCESSORS_ONLN 2>/dev/null || echo 4)}"
RUN_ONE="$(dirname "$0")/run-one-build.sh"
RUN_ONE_JS="$(dirname "$0")/run-one-js.sh"
case "$SALAM" in
/* | [A-Za-z]:*) SALAM_ABS="$SALAM" ;;
*) SALAM_ABS="$(pwd)/$SALAM" ;;
esac

if [ -z "${SALAM_STD:-}" ] && [ -d "$(pwd)/std" ]; then
    SALAM_STD="$(pwd)/std"
    export SALAM_STD
fi

run_batch() {
    jobs="$1"
    runner="${2:-$RUN_ONE}"
    [ -s "$jobs" ] || return 0
    results="$WORK/.batch-results.$$"
    # shellcheck disable=SC2016 # inner variables are expanded by the spawned sh, not here
    tr '\n' '\0' <"$jobs" | xargs -0 -n1 -P "$NPROC" sh -c '
        IFS="	"
        set -- $1
        label="$1"; f="$2"; lang="$3"; exp="$4"; extra="$5"
        unset IFS
        sh "'"$runner"'" "'"$SALAM_ABS"'" "'"$WORK"'" "$label" "$f" "$lang" "$exp" $extra
    ' _ >"$results" 2>&1
    cat "$results"
    p=$(grep -c '^PASS' "$results")
    fcount=$(grep -c '^FAIL' "$results")
    pass=$((pass + p))
    fail=$((fail + fcount))
    rm -f "$results" "$jobs"
}
check_out() {
    if [ "$3" = "$(tr -d '\r' <"$2")" ]; then
        echo "PASS $1"
        pass=$((pass + 1))
    else
        echo "FAIL $1"
        echo "  expected: $(tr '\n' '|' <"$2")"
        echo "  got:      $(echo "$3" | tr '\n' '|')"
        fail=$((fail + 1))
    fi
}

SECTIONS="$*"

want() {
    [ -z "$SECTIONS" ] && return 0
    for s in $SECTIONS; do
        case "$s" in c | codegen) s=general ;; interp | interpreter) s="exec" ;; esac
        [ "$s" = "$1" ] && return 0
    done
    return 1
}

if want general; then
    jobs="$WORK/.jobs-general.$$"
    : >"$jobs"
    for lang in $LANGS; do
        for f in tests/"$lang"/general/*.salam; do
            [ -e "$f" ] || continue
            name=$(basename "$f" .salam)
            case "$name" in _*) continue ;; esac
            exp="tests/$lang/general/$name.out"
            [ -f "$exp" ] || continue
            def=$(grep -o 'DEFINE: [A-Za-z0-9_]*' "$f" | sed 's/DEFINE: /-D/' | tr '\n' ' ')
            printf 'general/%s/%s\t%s\t%s\t%s\t%s\n' "$lang" "$name" "$f" "$lang" "$exp" "${def:--}" >>"$jobs"
        done
    done
    run_batch "$jobs"
fi
if want exec; then
    for lang in $LANGS; do
        for f in tests/"$lang"/exec/*.salam; do
            [ -e "$f" ] || continue
            name=$(basename "$f" .salam)
            case "$name" in _*) continue ;; esac
            exp="tests/$lang/exec/$name.out"
            [ -f "$exp" ] || continue
            got=$("$SALAM" exec "$f" --no-color --log-level=error --lang="$lang" 2>&1 | tr -d '\r')
            check_out "exec/$lang/$name" "$exp" "$got"
        done
    done
fi
if want js; then
    jobs="$WORK/.jobs-js.$$"
    : >"$jobs"
    for lang in $LANGS; do
        for f in tests/"$lang"/js/*.salam; do
            [ -e "$f" ] || continue
            name=$(basename "$f" .salam)
            case "$name" in _*) continue ;; esac
            exp="tests/$lang/js/$name.out"
            [ -f "$exp" ] || continue
            printf 'js/%s/%s\t%s\t%s\t%s\t-\n' "$lang" "$name" "$f" "$lang" "$exp" >>"$jobs"
        done
    done
    run_batch "$jobs" "$RUN_ONE_JS"
fi
if want errors; then
    for lang in $LANGS; do
        for f in tests/"$lang"/errors/*.salam; do
            [ -e "$f" ] || continue
            name=$(basename "$f" .salam)
            case "$name" in _*) continue ;; esac
            code=$(grep -oE '(EXPECT|انتظار|توقع): [^ ]*' "$f" | head -1 | sed -E 's/^(EXPECT|انتظار|توقع): //' | tr -d '\r')
            out=$("$SALAM" "$f" --emit-symbol-xml --no-color --log-level=error --lang="$lang" 2>&1 >/dev/null)
            if [ -n "$code" ] && echo "$out" | grep -qF "$code"; then
                echo "PASS errors/$lang/$name ($code)"
                pass=$((pass + 1))
            else
                echo "FAIL errors/$lang/$name (want $code)"
                echo "  $out"
                fail=$((fail + 1))
            fi
        done
    done
fi
if want layout; then
    for lang in $LANGS; do
        for f in tests/"$lang"/layout/*.salam; do
            [ -e "$f" ] || continue
            name=$(basename "$f" .salam)
            case "$name" in _*) continue ;; esac
            expect=$(grep -oE '(EXPECT|انتظار|توقع): .*' "$f" | head -1 | sed -E 's/^(EXPECT|انتظار|توقع): //' | tr -d '\r')
            "$SALAM" layout build "$f" --inline --output="$WORK/$name.html" --no-color --log-level=error --lang="$lang" >/dev/null 2>&1
            if [ -f "$WORK/$name.html" ] && grep -qF "$expect" "$WORK/$name.html"; then
                echo "PASS layout/$lang/$name (has '$expect')"
                pass=$((pass + 1))
            else
                echo "FAIL layout/$lang/$name (want '$expect')"
                fail=$((fail + 1))
            fi
            rm -f "$WORK/$name.html"
        done
    done
fi
if want fmt; then
    for lang in $LANGS; do
        for f in tests/"$lang"/fmt/*.salam; do
            [ -e "$f" ] || continue
            name=$(basename "$f" .salam)
            exp="tests/$lang/fmt/$name.out"
            [ -f "$exp" ] || continue
            cp "$f" "$WORK/$name.salam"
            "$SALAM" format "$WORK/$name.salam" --lang="$lang" --no-color --log-level=error >/dev/null 2>&1
            "$SALAM" format "$WORK/$name.salam" --check --lang="$lang" --no-color --log-level=error >/dev/null 2>&1
            idem=$?
            rm -f "$WORK/$name.exe"
            "$SALAM" build "$WORK/$name.salam" --output="$WORK/$name.exe" --no-color --log-level=error --lang="$lang" >/dev/null 2>&1
            if [ ! -x "$WORK/$name.exe" ]; then
                sleep 1
                "$SALAM" build "$WORK/$name.salam" --output="$WORK/$name.exe" --no-color --log-level=error --lang="$lang" >/dev/null 2>&1
            fi
            got=$([ -x "$WORK/$name.exe" ] && "$WORK/$name.exe" 2>&1 | tr -d '\r')

            TAB=$(printf '\t')
            cp "$f" "$WORK/${name}_tab.salam"
            "$SALAM" format "$WORK/${name}_tab.salam" --tabs --lang="$lang" --no-color --log-level=error >/dev/null 2>&1
            "$SALAM" format "$WORK/${name}_tab.salam" --tabs --check --lang="$lang" --no-color --log-level=error >/dev/null 2>&1
            tabidem=$?
            hastab=1
            grep -q "$TAB" "$WORK/${name}_tab.salam" && hastab=0
            rm -f "$WORK/${name}_tab.exe"
            "$SALAM" build "$WORK/${name}_tab.salam" --output="$WORK/${name}_tab.exe" --no-color --log-level=error --lang="$lang" >/dev/null 2>&1
            if [ ! -x "$WORK/${name}_tab.exe" ]; then
                sleep 1
                "$SALAM" build "$WORK/${name}_tab.salam" --output="$WORK/${name}_tab.exe" --no-color --log-level=error --lang="$lang" >/dev/null 2>&1
            fi
            gottab=$([ -x "$WORK/${name}_tab.exe" ] && "$WORK/${name}_tab.exe" 2>&1 | tr -d '\r')

            if [ "$idem" -eq 0 ] && [ "$got" = "$(tr -d '\r' <"$exp")" ] &&
                [ "$tabidem" -eq 0 ] && [ "$hastab" -eq 0 ] && [ "$gottab" = "$(tr -d '\r' <"$exp")" ]; then
                echo "PASS fmt/$lang/$name"
                pass=$((pass + 1))
            else
                echo "FAIL fmt/$lang/$name (idempotent=$idem tab-idem=$tabidem has-tab=$hastab)"
                echo "  got: $(echo "$got" | tr '\n' '|')"
                echo "  tab: $(echo "$gottab" | tr '\n' '|')"
                fail=$((fail + 1))
            fi
        done
    done
fi
if want db; then
    for lang in $LANGS; do
        [ -d "tests/$lang/db" ] || continue
        DBCC=""
        for c in tcc gcc cc clang; do command -v "$c" >/dev/null 2>&1 && {
            DBCC="$c"
            break
        }; done
        dbok=0
        if [ -n "$DBCC" ] && command -v ar >/dev/null 2>&1; then
            mkdir -p "$WORK/dbwork/.work"
            if "$DBCC" -c "tests/$lang/db/mysql_mock.c" -o "$WORK/dbwork/.work/mysql_mock.o" >/dev/null 2>&1 &&
                ar rcs "$WORK/dbwork/.work/libsalammock.a" "$WORK/dbwork/.work/mysql_mock.o" >/dev/null 2>&1; then
                dbok=1
            fi
        fi
        if [ "$dbok" = "1" ]; then
            jobs="$WORK/.jobs-db-$lang.$$"
            : >"$jobs"
            for f in tests/"$lang"/db/*.salam; do
                [ -e "$f" ] || continue
                name=$(basename "$f" .salam)
                case "$name" in _*) continue ;; esac
                exp="tests/$lang/db/$name.out"
                [ -f "$exp" ] || continue
                printf 'db/%s/%s\t%s\t%s\t%s\t%s\n' "$lang" "$name" "$f" "$lang" "$exp" \
                    "--cc=$DBCC -DSALAM_DB_MOCK" >>"$jobs"
            done
            run_batch "$jobs"
        else
            echo "SKIP db/$lang/* (no C compiler/ar to build the mysql mock)"
        fi
    done
fi
if want llvm; then
    for lang in $LANGS; do
        [ -d "tests/$lang/llvm" ] || continue
        probe_raw=$("$SALAM" llvm "tests/$lang/llvm/_probe.salam" --jit --no-color --log-level=error 2>/dev/null)
        prc=$?
        probe=$(printf '%s' "$probe_raw" | tr -d '\r')
        rm -f _probe.ll _probe.ll.run.sh 2>/dev/null
        if [ "$probe" = "OK" ]; then
            for f in tests/"$lang"/llvm/*.salam; do
                [ -e "$f" ] || continue
                name=$(basename "$f" .salam)
                case "$name" in _*) continue ;; esac
                exp="tests/$lang/llvm/$name.out"
                [ -f "$exp" ] || continue
                got=$("$SALAM" llvm "$f" --jit --no-color --log-level=error 2>/dev/null | tr -d '\r')
                rm -f "$name.ll" "$name.ll.run.sh" 2>/dev/null
                check_out "llvm/$lang/$name" "$exp" "$got"
            done
        elif [ "$prc" -ge 128 ]; then
            echo "FAIL llvm/$lang/* (salam crashed on probe, signal $((prc - 128)); rebuild salam via tools/build-compiler.sh)"
            fail=$((fail + 1))
        else
            echo "SKIP llvm/$lang/* (LLVM toolchain unavailable: 'salam llvm --jit' probe failed)"
        fi
    done
fi
echo "----------------------------------------"
echo "RESULT: $pass passed, $fail failed"
[ "$fail" -eq 0 ]
