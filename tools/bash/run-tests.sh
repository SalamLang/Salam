#!/bin/sh

set -u

case "$0" in /* | [A-Za-z]:*) SELF="$0" ;; *) SELF="$(pwd)/$0" ;; esac
TOOLS_DIR=$(dirname "$SELF")
TAB=$(printf '\t')

if [ "${1:-}" = "--worker" ]; then
    cd "$TOOLS_DIR/../.." || exit 1
    line="${2:-}"
    [ -n "$line" ] || exit 0
    oIFS=$IFS
    IFS=$TAB
    set -f
    # shellcheck disable=SC2086
    set -- $line
    set +f
    IFS=$oIFS
    [ "$#" -ge 6 ] || exit 0
    kind="$1"
    label="$2"
    f="$3"
    lang="$4"
    exp="$5"
    extra="$6"
    [ "$extra" = "-" ] && extra=""
    TOTAL="${TOTAL:-0}"
    jobid=$(printf '%s' "$label" | tr '/ .' '___')
    case "$f" in /* | [A-Za-z]:* | "") fabs="$f" ;; *) fabs="$(pwd)/$f" ;; esac
    case "$exp" in /* | [A-Za-z]:* | "" | -) expabs="$exp" ;; *) expabs="$(pwd)/$exp" ;; esac

    tmo() {
        if command -v timeout >/dev/null 2>&1; then timeout "$@"; else
            shift
            "$@"
        fi
    }
    wk_check() { # <expected-file> <got>
        want=$(tr -d '\r' <"$1")
        if [ "$2" = "$want" ]; then
            echo "PASS $label"
        else
            echo "FAIL $label"
            echo "  expected: $(printf '%s' "$want" | tr '\n' '|')"
            echo "  got:      $(printf '%s' "$2" | tr '\n' '|')"
        fi
    }
    wk_repl() {
        jobdir="$WORK/repljob_${jobid}_$$"
        mkdir -p "$jobdir"
        got=$( (cd "$jobdir" && tmo "${SALAM_TEST_TIMEOUT:-600}" "$SALAM_ABS" cli --lang="$lang" --no-color --log-level=error <"$fabs" 2>&1) | tr -d '\r')
        rm -rf "$jobdir"
        wk_check "$expabs" "$got"
    }

    wk_fmt() {
        name=$(basename "$f" .salam)
        jobdir="$WORK/fmtjob_${jobid}_$$"
        mkdir -p "$jobdir"
        cp "$f" "$jobdir/$name.salam"
        "$SALAM_ABS" format "$jobdir/$name.salam" --lang="$lang" --no-color --log-level=error >/dev/null 2>&1
        "$SALAM_ABS" format "$jobdir/$name.salam" --check --lang="$lang" --no-color --log-level=error >/dev/null 2>&1
        idem=$?
        exe="$jobdir/$name.exe"
        btry=1
        while [ ! -x "$exe" ] && [ "$btry" -le 2 ]; do
            [ "$btry" -gt 1 ] && sleep 1
            (cd "$jobdir" && "$SALAM_ABS" build "$jobdir/$name.salam" --output="$exe" --no-color --log-level=error --lang="$lang") >/dev/null 2>&1
            btry=$((btry + 1))
        done
        got=$([ -x "$exe" ] && "$exe" 2>&1 | tr -d '\r')

        cp "$f" "$jobdir/${name}_tab.salam"
        "$SALAM_ABS" format "$jobdir/${name}_tab.salam" --tabs --lang="$lang" --no-color --log-level=error >/dev/null 2>&1
        "$SALAM_ABS" format "$jobdir/${name}_tab.salam" --tabs --check --lang="$lang" --no-color --log-level=error >/dev/null 2>&1
        tabidem=$?
        hastab=1
        grep -q "$TAB" "$jobdir/${name}_tab.salam" && hastab=0
        texe="$jobdir/${name}_tab.exe"
        btry=1
        while [ ! -x "$texe" ] && [ "$btry" -le 2 ]; do
            [ "$btry" -gt 1 ] && sleep 1
            (cd "$jobdir" && "$SALAM_ABS" build "$jobdir/${name}_tab.salam" --output="$texe" --no-color --log-level=error --lang="$lang") >/dev/null 2>&1
            btry=$((btry + 1))
        done
        gottab=$([ -x "$texe" ] && "$texe" 2>&1 | tr -d '\r')

        want=$(tr -d '\r' <"$expabs")
        if [ "$idem" -eq 0 ] && [ "$got" = "$want" ] &&
            [ "$tabidem" -eq 0 ] && [ "$hastab" -eq 0 ] && [ "$gottab" = "$want" ]; then
            echo "PASS $label"
        else
            echo "FAIL $label (idempotent=$idem tab-idem=$tabidem has-tab=$hastab)"
            echo "  got: $(printf '%s' "$got" | tr '\n' '|')"
            echo "  tab: $(printf '%s' "$gottab" | tr '\n' '|')"
        fi
        rm -rf "$jobdir"
    }
    wk_expect() {
        case "$label" in
        port/*) exp_tmo="${SALAM_TEST_TIMEOUT:-300}" ;;
        *) exp_tmo="${SALAM_TEST_TIMEOUT:-20}" ;;
        esac
        jobdir="$WORK/exjob_${jobid}_$$"
        mkdir -p "$jobdir"
        exe="$jobdir/a.exe"
        buildlog="$jobdir/build.log"
        btry=1
        while [ ! -x "$exe" ] && [ "$btry" -le 3 ]; do
            [ "$btry" -gt 1 ] && sleep "$btry"
            (cd "$jobdir" && "$SALAM_ABS" build "$fabs" --output="$exe" --no-color --log-level=error --lang="$lang") >"$buildlog" 2>&1
            btry=$((btry + 1))
        done
        produced=0
        if [ -x "$exe" ]; then
            produced=1
            got=$(tmo "$exp_tmo" "$exe" </dev/null 2>&1 | tr -d '\r')
        else
            html="$jobdir/a.html"
            wtry=1
            while [ ! -f "$html" ] && [ "$wtry" -le 3 ]; do
                [ "$wtry" -gt 1 ] && sleep "$wtry"
                (cd "$jobdir" && "$SALAM_ABS" web "$fabs" --output="$html" --no-color --log-level=error --lang="$lang") >"$buildlog" 2>&1
                wtry=$((wtry + 1))
            done
            if [ -f "$html" ]; then
                produced=1
                got=$(tr -d '\r' <"$html")
            else
                got=""
            fi
        fi
        if [ "$produced" -eq 0 ]; then
            echo "FAIL $label (build failed)"
            sed 's/^/  /' "$buildlog" 2>/dev/null | head -20
            rm -rf "$jobdir"
            return
        fi
        rm -rf "$jobdir"
        ok=1
        missing=""
        while IFS= read -r l || [ -n "$l" ]; do
            [ -z "$l" ] && continue
            case "$got" in
            *"$l"*) ;;
            *)
                ok=0
                if [ -z "$missing" ]; then
                    missing="$l"
                else
                    missing="$missing | $l"
                fi
                ;;
            esac
        done <"$expabs"
        if [ "$ok" -eq 1 ]; then
            echo "PASS $label"
        else
            echo "FAIL $label (missing expected: $missing)"
        fi
    }
    wk_buildonly() {
        jobdir="$WORK/bojob_${jobid}_$$"
        mkdir -p "$jobdir"
        exe="$jobdir/a.exe"
        buildlog="$jobdir/build.log"
        btry=1
        while [ ! -x "$exe" ] && [ "$btry" -le 3 ]; do
            [ "$btry" -gt 1 ] && sleep "$btry"
            (cd "$jobdir" && "$SALAM_ABS" build "$fabs" --output="$exe" --no-color --log-level=error --lang="$lang") >"$buildlog" 2>&1
            btry=$((btry + 1))
        done
        if [ -x "$exe" ]; then
            echo "PASS $label (build)"
        else
            echo "FAIL $label (build failed)"
            sed 's/^/  /' "$buildlog" 2>/dev/null | head -20
        fi
        rm -rf "$jobdir"
    }
    wk_cross() {
        target="${extra%%:*}"
        runner="${extra#*:}"
        outbin="$WORK/cross_${jobid}_$$"
        case "$target" in *windows*) outbin="$outbin.exe" ;; esac
        crosslog="$WORK/cross_${jobid}_$$.log"
        if ! "$SALAM_ABS" build "$f" --output="$outbin" --no-color --log-level=info \
            --lang="$lang" --target="$target" >"$crosslog" 2>&1; then
            echo "SKIP $label (cross build unavailable for $target - no embedded static libs, or self-hosted/non-flagship salam)"
            rm -f "$crosslog"
            return
        fi
        if [ -z "$runner" ]; then
            hostos=$(uname -s 2>/dev/null || echo unknown)
            native=1
            case "$hostos" in
            MINGW* | MSYS* | CYGWIN*) case "$target" in *windows*) ;; *) native=0 ;; esac ;;
            Linux) case "$target" in *linux*) ;; *) native=0 ;; esac ;;
            Darwin) case "$target" in *darwin* | *apple* | *macos*) ;; *) native=0 ;; esac ;;
            esac
            if [ "$native" -eq 0 ]; then
                echo "SKIP $label (build OK; a $hostos host cannot run a $target binary and no emulator is configured for it)"
                rm -f "$outbin" "$crosslog"
                return
            fi
        fi
        if [ -n "$runner" ] && ! command -v "$runner" >/dev/null 2>&1; then
            alt="${runner%-static}"
            if [ "$alt" != "$runner" ] && command -v "$alt" >/dev/null 2>&1; then
                runner="$alt"
            else
                echo "SKIP $label (build OK; no $runner on this host to run the $target binary)"
                rm -f "$outbin" "$crosslog"
                return
            fi
        fi
        got=$($runner "$outbin" 2>&1 | tr -d '\r')
        case "$got" in
        *"Error loading shared library"* | *"error while loading shared libraries"* | \
            *"cannot open shared object file"*)
            if ! grep -q "static third-party libs" "$crosslog" 2>/dev/null; then
                missing_lib=$(printf '%s\n' "$got" | tr ' ' '\n' | grep '\.so' | sed 1q | tr -d ':')
                echo "SKIP $label (build OK; the $target binary wants ${missing_lib:-a shared library} at runtime - this salam carries no embedded static third-party libs for $target)"
                rm -f "$outbin" "$crosslog"
                return
            fi
            ;;
        esac
        wk_check "$expabs" "$got"
        rm -f "$outbin" "$crosslog"
    }
    run_worker() {
        case "$kind" in
        build)
            sh "$TOOLS_DIR/run-one-build.sh" "$SALAM_ABS" "$WORK" "$label" "$f" "$lang" "$exp" ${extra:--} 2>&1
            ;;
        js)
            sh "$TOOLS_DIR/run-one-js.sh" "$SALAM_ABS" "$WORK" "$label" "$f" "$lang" "$exp" 2>&1
            ;;
        exec)
            got=$("$SALAM_ABS" exec "$f" --no-color --log-level=error --lang="$lang" 2>&1 | tr -d '\r')
            wk_check "$expabs" "$got"
            ;;
        errors)
            code=$(grep -oE '(EXPECT|انتظار|توقع): [^ ]*' "$f" | head -1 | sed -E 's/^(EXPECT|انتظار|توقع): //' | tr -d '\r')
            out=$("$SALAM_ABS" inspect "$f" --emit-symbol --no-color --log-level=error --lang="$lang" 2>&1 >/dev/null)
            if [ -n "$code" ] && printf '%s\n' "$out" | grep -qF "$code"; then
                echo "PASS $label ($code)"
            else
                echo "FAIL $label (want $code)"
                echo "  $out"
            fi
            ;;
        layout)
            expect=$(grep -oE '(EXPECT|انتظار|توقع): .*' "$f" | head -1 | sed -E 's/^(EXPECT|انتظار|توقع): //' | tr -d '\r')
            html="$WORK/layout_${jobid}_$$.html"
            "$SALAM_ABS" layout build "$f" --inline --no-minify --output="$html" --no-color --log-level=error --lang="$lang" >/dev/null 2>&1
            if [ -f "$html" ] && grep -qF "$expect" "$html"; then
                echo "PASS $label (has '$expect')"
            else
                echo "FAIL $label (want '$expect')"
            fi
            rm -f "$html"
            ;;
        llvm)
            jobdir="$WORK/llvmjob_${jobid}_$$"
            mkdir -p "$jobdir"
            got=$( (cd "$jobdir" && "$SALAM_ABS" llvm "$fabs" --jit --no-color --log-level=error 2>/dev/null) | tr -d '\r')
            rm -rf "$jobdir"
            wk_check "$expabs" "$got"
            ;;
        fmt) wk_fmt ;;
        repl) wk_repl ;;
        expect) wk_expect ;;
        buildonly) wk_buildonly ;;
        cross) wk_cross ;;
        esac
    }

    block=$(run_worker)
    [ -n "$block" ] || block="FAIL $label (worker produced no output)"
    spins=0
    while ! (set -C && : >"$WORK/.counter.lock") 2>/dev/null; do
        spins=$((spins + 1))
        case "$spins" in
        500 | 1000 | 1500 | 2000 | 2500) sleep 1 ;;
        3000) break ;;
        esac
    done
    idx=0
    [ -f "$WORK/.counter" ] && read -r idx <"$WORK/.counter"
    idx=$((idx + 1))
    printf '%s\n' "$idx" >"$WORK/.counter"
    rm -f "$WORK/.counter.lock"
    printf '%s\n' "$block" >"$WORK/results/$(printf 'r%06d' "$idx").$$"
    first=$(printf '%s\n' "$block" | sed 1q)
    rest=$(printf '%s\n' "$block" | sed 1d)
    status="${first%% *}"
    detail="${first#* }"
    W=${#TOTAL}
    head_line=$(printf '%s [%'"$W"'d/%s] %s' "$status" "$idx" "$TOTAL" "$detail")
    if [ -n "$rest" ]; then
        printf '%s\n%s\n' "$head_line" "$rest"
    else
        printf '%s\n' "$head_line"
    fi
    exit 0
fi

. "$TOOLS_DIR/lib.sh"
salam_ensure_compiler --quiet
WORK="${WORK:-${TMPDIR:-/tmp}/salam-run-tests-work.$$}"
mkdir -p "$WORK/results"
printf '0\n' >"$WORK/.counter"
trap 'rm -rf "$WORK"' EXIT
LANGS="${LANGS:-en fa}"
NPROC="${NPROC:-}"
if [ -z "$NPROC" ]; then
    command -v nproc >/dev/null 2>&1 && NPROC=$(nproc 2>/dev/null)
    [ -z "$NPROC" ] && command -v getconf >/dev/null 2>&1 && NPROC=$(getconf _NPROCESSORS_ONLN 2>/dev/null)
    [ -z "$NPROC" ] && command -v sysctl >/dev/null 2>&1 && NPROC=$(sysctl -n hw.ncpu 2>/dev/null)
    [ -z "$NPROC" ] && [ -n "${NUMBER_OF_PROCESSORS:-}" ] && NPROC="$NUMBER_OF_PROCESSORS"
    [ -z "$NPROC" ] && [ -r /proc/cpuinfo ] && NPROC=$(grep -c '^processor' /proc/cpuinfo 2>/dev/null)
    NPROC="${NPROC:-4}"
fi
while [ $# -gt 0 ]; do
    case "$1" in
    -j)
        NPROC="$2"
        shift 2
        ;;
    -j*)
        NPROC="${1#-j}"
        shift
        ;;
    --jobs=*)
        NPROC="${1#--jobs=}"
        shift
        ;;
    *) break ;;
    esac
done
case "$SALAM" in
/* | [A-Za-z]:*) SALAM_ABS="$SALAM" ;;
*) SALAM_ABS="$(pwd)/$SALAM" ;;
esac

# Programs that `import llvm` link libsalam_llvm.a. Putting the archive on
# LIBRARY_PATH here is what lets the llvmapi examples build; without it that
# section probes false and skips.
case "${SALAM_LLVM_LIBDIR:-}" in
"") ;;
*)
    LIBRARY_PATH="$SALAM_LLVM_LIBDIR${LIBRARY_PATH:+:$LIBRARY_PATH}"
    export LIBRARY_PATH
    ;;
esac

if [ -z "${SALAM_STD:-}" ] && [ -d "$(pwd)/std" ]; then
    SALAM_STD="$(pwd)/std"
    export SALAM_STD
fi

: "${SALAM_C_STRICT:=1}"
export SALAM_C_STRICT

case "$(uname -s 2>/dev/null)" in
Linux) HOST_OS=linux ;;
Darwin) HOST_OS=mac ;;
MINGW* | MSYS* | CYGWIN*) HOST_OS=windows ;;
*) HOST_OS="" ;;
esac
[ "${OS:-}" = "Windows_NT" ] && HOST_OS=windows
case "$(uname -m 2>/dev/null)" in
x86_64 | amd64) HOST_ARCH=x64 ;;
aarch64 | arm64) HOST_ARCH=arm64 ;;
i386 | i486 | i586 | i686 | x86) HOST_ARCH=x86 ;;
armv6l | armv7l | armv7 | arm) HOST_ARCH=arm ;;
*) HOST_ARCH="" ;;
esac
pick_expect() {
    if [ -n "$HOST_OS" ] && [ -n "$HOST_ARCH" ] && [ -f "$1.$HOST_OS.$HOST_ARCH.out" ]; then
        printf '%s\n' "$1.$HOST_OS.$HOST_ARCH.out"
    elif [ -n "$HOST_OS" ] && [ -f "$1.$HOST_OS.out" ]; then
        printf '%s\n' "$1.$HOST_OS.out"
    else
        printf '%s\n' "$1.out"
    fi
}

JOBS="$WORK/jobs.tsv"
: >"$JOBS"
add_job() {
    printf '%s\t%s\t%s\t%s\t%s\t%s\n' "$1" "$2" "$3" "$4" "$5" "${6:--}" >>"$JOBS"
}
note_result() {
    printf '%s\n' "$1" >"$WORK/results/s_$(printf '%s' "$2" | tr '/ .*' '____')"
    printf '%s\n' "$1"
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

EXAMPLE_DIRS="apps basics data editor-selected features games interop stdlib types webframework switch match"

want_example() {
    want "$1" && return 0
    for s in $SECTIONS; do [ "$s" = examples ] && return 0; done
    return 1
}

redis_listening() {
    if command -v nc >/dev/null 2>&1 && nc -z 127.0.0.1 6379 >/dev/null 2>&1; then
        return 0
    fi
    if command -v redis-cli >/dev/null 2>&1 &&
        redis-cli -h 127.0.0.1 -p 6379 ping >/dev/null 2>&1; then
        return 0
    fi
    [ -n "${BASH_VERSION:-}" ] || return 1
    # shellcheck disable=SC3025
    (exec 3<>/dev/tcp/127.0.0.1/6379) 2>/dev/null
}
REDIS_OK=0
if redis_listening; then
    REDIS_OK=1
fi
export REDIS_OK
REDIS_SKIP_MSG="requires a Redis server on 127.0.0.1:6379"

if want fmt; then
    for lang in $LANGS; do
        for f in tests/"$lang"/fmt/*.salam; do
            [ -e "$f" ] || continue
            name=$(basename "$f" .salam)
            exp="$(pick_expect "tests/$lang/fmt/$name")"
            [ -f "$exp" ] || continue
            add_job fmt "fmt/$lang/$name" "$f" "$lang" "$exp"
        done
    done
fi

if want repl; then
    for lang in $LANGS; do
        for f in tests/"$lang"/repl/*.in; do
            [ -e "$f" ] || continue
            name=$(basename "$f" .in)
            exp="tests/$lang/repl/$name.out"
            [ -f "$exp" ] || continue
            add_job repl "repl/$lang/$name" "$f" "$lang" "$exp"
        done
    done
fi

collect_example_dir() {
    dir="$1"
    for lang in $LANGS; do
        [ -d "tests/$lang/$dir" ] || continue
        find "tests/$lang/$dir" -name '*.salam' | sort | while IFS= read -r f; do
            case "$(basename "$f")" in _*) continue ;; esac
            rel="${f#tests/"$lang"/"$dir"/}"
            name="${rel%.salam}"
            base="tests/$lang/$dir/$name"
            exp="$(pick_expect "$base")"
            skip=""
            if [ -f "$base.redis" ] && [ "${REDIS_OK:-0}" != "1" ]; then
                skip="requires a Redis server on 127.0.0.1:6379"
            elif [ -f "$base.network" ] && [ "${SALAM_TEST_NETWORK:-0}" != "1" ]; then
                skip="requires live network; set SALAM_TEST_NETWORK=1"
            elif [ -f "$base.interactive" ] && [ "${SALAM_TEST_INTERACTIVE:-0}" != "1" ]; then
                skip="opens a modal window; set SALAM_TEST_INTERACTIVE=1 on a desktop session"
            fi
            if [ -f "$exp" ]; then
                if [ -n "$skip" ]; then
                    note_result "SKIP $dir/$lang/$name ($skip)" "$dir/$lang/$name"
                else
                    add_job build "$dir/$lang/$name" "$f" "$lang" "$exp"
                fi
            fi
            if [ -f "$base.expect" ]; then
                if [ -n "$skip" ]; then
                    note_result "SKIP $dir/$lang/$name ($skip)" "$dir/$lang/$name"
                else
                    add_job expect "$dir/$lang/$name" "$f" "$lang" "$base.expect"
                fi
            fi
            if [ -f "$base.buildonly" ]; then
                add_job buildonly "$dir/$lang/${name}#build" "$f" "$lang" -
            fi
        done
    done
}
for dir in $EXAMPLE_DIRS; do
    want_example "$dir" && collect_example_dir "$dir"
done

if want general; then
    for lang in $LANGS; do
        for f in tests/"$lang"/general/*.salam; do
            [ -e "$f" ] || continue
            name=$(basename "$f" .salam)
            case "$name" in _*) continue ;; esac
            exp="$(pick_expect "tests/$lang/general/$name")"
            [ -f "$exp" ] || continue
            def=$(grep -o 'DEFINE: [A-Za-z0-9_]*' "$f" | sed 's/DEFINE: /-D/' | tr '\n' ' ')
            def="$def$(grep -o 'CONST: [!-~]*' "$f" | sed 's/CONST: /-d/' | tr '\n' ' ')"
            add_job build "general/$lang/$name" "$f" "$lang" "$exp" "${def:--}"
        done
    done
fi

if want db; then
    DBCC=""
    for c in tcc gcc cc clang; do
        command -v "$c" >/dev/null 2>&1 && {
            DBCC="$c"
            break
        }
    done
    mockc=""
    pgmockc=""
    for lang in $LANGS; do
        [ -f "tests/$lang/db/mysql_mock.c" ] && {
            mockc="tests/$lang/db/mysql_mock.c"
            break
        }
    done
    for lang in $LANGS; do
        [ -f "tests/$lang/db/postgres_mock.c" ] && {
            pgmockc="tests/$lang/db/postgres_mock.c"
            break
        }
    done
    dbok=0
    if [ -n "$DBCC" ] && [ -n "$mockc" ] && command -v ar >/dev/null 2>&1; then
        mkdir -p "$WORK/dbwork/.work"
        mockobjs="$WORK/dbwork/.work/mysql_mock.o"
        if "$DBCC" -c "$mockc" -o "$WORK/dbwork/.work/mysql_mock.o" >/dev/null 2>&1; then
            if [ -n "$pgmockc" ] &&
                "$DBCC" -c "$pgmockc" -o "$WORK/dbwork/.work/postgres_mock.o" \
                    >/dev/null 2>&1; then
                mockobjs="$mockobjs $WORK/dbwork/.work/postgres_mock.o"
            fi
            # shellcheck disable=SC2086
            if ar rcs "$WORK/dbwork/.work/libsalammock.a" $mockobjs >/dev/null 2>&1; then
                dbok=1
            fi
        fi
    fi
    for lang in $LANGS; do
        [ -d "tests/$lang/db" ] || continue
        if [ "$dbok" = "1" ]; then
            for f in tests/"$lang"/db/*.salam; do
                [ -e "$f" ] || continue
                name=$(basename "$f" .salam)
                case "$name" in _*) continue ;; esac
                exp="$(pick_expect "tests/$lang/db/$name")"
                [ -f "$exp" ] || continue
                case "$name" in
                redis_*)
                    if [ "$REDIS_OK" != "1" ]; then
                        note_result "SKIP db/$lang/$name ($REDIS_SKIP_MSG)" "db/$lang/$name"
                        continue
                    fi
                    ;;
                esac
                add_job build "db/$lang/$name" "$f" "$lang" "$exp" "--cc=$DBCC -DSALAM_DB_MOCK"
            done
        else
            note_result "SKIP db/$lang/* (no C compiler/ar to build the mysql mock)" "db/$lang/all"
        fi
    done
fi

if want opencv; then
    OCVCC=""
    for c in tcc gcc cc clang; do
        command -v "$c" >/dev/null 2>&1 && {
            OCVCC="$c"
            break
        }
    done
    ocvmockc=""
    for lang in $LANGS; do
        [ -f "std/opencv/native/mock/opencv_mock.c" ] && {
            ocvmockc="std/opencv/native/mock/opencv_mock.c"
            break
        }
    done
    ocvok=0
    if [ -n "$OCVCC" ] && [ -n "$ocvmockc" ] && command -v ar >/dev/null 2>&1; then
        mkdir -p "$WORK/opencvwork/.work"
        if "$OCVCC" -c -std=c11 "$ocvmockc" -o "$WORK/opencvwork/.work/opencv_mock.o" >/dev/null 2>&1 &&
            ar rcs "$WORK/opencvwork/.work/libsalam_opencv_mock.a" "$WORK/opencvwork/.work/opencv_mock.o" >/dev/null 2>&1; then
            ocvok=1
        fi
    fi
    for lang in $LANGS; do
        [ -d "tests/$lang/opencv" ] || continue
        if [ "$ocvok" = "1" ]; then
            for f in tests/"$lang"/opencv/*.salam; do
                [ -e "$f" ] || continue
                name=$(basename "$f" .salam)
                case "$name" in _*) continue ;; esac
                exp="$(pick_expect "tests/$lang/opencv/$name")"
                [ -f "$exp" ] || continue
                add_job build "opencv/$lang/$name" "$f" "$lang" "$exp" "--cc=$OCVCC -DSALAM_OPENCV_MOCK"
            done
        else
            note_result "SKIP opencv/$lang/* (no C compiler/ar to build the OpenCV mock shim)" "opencv/$lang/all"
        fi
    done
fi

if want webview_cef; then
    CEFCC=""
    for c in tcc gcc cc clang; do
        command -v "$c" >/dev/null 2>&1 && {
            CEFCC="$c"
            break
        }
    done
    cefmockc=""
    [ -f "std/webview/native/mock/cef_mock.c" ] && cefmockc="std/webview/native/mock/cef_mock.c"
    cefok=0
    if [ -n "$CEFCC" ] && [ -n "$cefmockc" ] && command -v ar >/dev/null 2>&1; then
        mkdir -p "$WORK/cefwork/.work"
        if "$CEFCC" -c -std=c11 "$cefmockc" -o "$WORK/cefwork/.work/cef_mock.o" >/dev/null 2>&1 &&
            ar rcs "$WORK/cefwork/.work/libsalam_webview_cef_mock.a" "$WORK/cefwork/.work/cef_mock.o" >/dev/null 2>&1; then
            cefok=1
        fi
    fi
    for lang in $LANGS; do
        [ -d "tests/$lang/webview_cef" ] || continue
        if [ "$cefok" = "1" ]; then
            for f in tests/"$lang"/webview_cef/*.salam; do
                [ -e "$f" ] || continue
                name=$(basename "$f" .salam)
                case "$name" in _*) continue ;; esac
                exp="$(pick_expect "tests/$lang/webview_cef/$name")"
                [ -f "$exp" ] || continue
                add_job build "webview_cef/$lang/$name" "$f" "$lang" "$exp" "--cc=$CEFCC -DSALAM_WEBVIEW_CEF -DSALAM_WEBVIEW_CEF_MOCK"
            done
        else
            note_result "SKIP webview_cef/$lang/* (no C compiler/ar to build the CEF mock shim)" "webview_cef/$lang/all"
        fi
    done
fi

if want ssl; then
    for lang in $LANGS; do
        [ -d "tests/$lang/ssl" ] || continue
        for f in tests/"$lang"/ssl/*.salam; do
            [ -e "$f" ] || continue
            name=$(basename "$f" .salam)
            case "$name" in _*) continue ;; esac
            exp="$(pick_expect "tests/$lang/ssl/$name")"
            [ -f "$exp" ] || continue
            add_job build "ssl/$lang/$name" "$f" "$lang" "$exp"
        done
    done
fi

if want js; then
    for lang in $LANGS; do
        for f in tests/"$lang"/js/*.salam; do
            [ -e "$f" ] || continue
            name=$(basename "$f" .salam)
            case "$name" in _*) continue ;; esac
            exp="$(pick_expect "tests/$lang/js/$name")"
            [ -f "$exp" ] || continue
            add_job js "js/$lang/$name" "$f" "$lang" "$exp"
        done
    done
fi

if want cross; then
    for lang in $LANGS; do
        [ -d "tests/$lang/cross" ] || continue
        for f in tests/"$lang"/cross/*.salam; do
            [ -e "$f" ] || continue
            name=$(basename "$f" .salam)
            case "$name" in _*) continue ;; esac
            exp="$(pick_expect "tests/$lang/cross/$name")"
            [ -f "$exp" ] || continue
            for pair in x86_64-linux-musl: aarch64-linux-musl:qemu-aarch64-static \
                i686-linux-musl:qemu-i386-static arm-linux-musleabihf:qemu-arm-static \
                x86_64-w64-windows-gnu:wine; do
                target="${pair%%:*}"
                add_job cross "cross/$lang/$name/$target" "$f" "$lang" "$exp" "$pair"
            done
        done
    done
fi

if want llvm; then
    for lang in $LANGS; do
        [ -d "tests/$lang/llvm" ] || continue
        probe_err="$WORK/llvm_probe_$lang.err"
        probe_raw=$("$SALAM" llvm "tests/$lang/llvm/_probe.salam" --jit --no-color --log-level=error 2>"$probe_err")
        prc=$?
        probe=$(printf '%s' "$probe_raw" | tr -d '\r')
        rm -f _probe.ll _probe.ll.run.sh 2>/dev/null
        if [ "$probe" = "OK" ]; then
            for f in tests/"$lang"/llvm/*.salam; do
                [ -e "$f" ] || continue
                name=$(basename "$f" .salam)
                case "$name" in _*) continue ;; esac
                exp="$(pick_expect "tests/$lang/llvm/$name")"
                [ -f "$exp" ] || continue
                add_job llvm "llvm/$lang/$name" "$f" "$lang" "$exp"
            done
        elif [ "$prc" -ge 128 ]; then
            note_result "FAIL llvm/$lang/* (salam crashed on probe, signal $((prc - 128)); rebuild salam via tools/bash/build-selfhost.sh)" "llvm/$lang/all"
        else
            why=$(tr -d '\r' <"$probe_err" 2>/dev/null | grep -i 'error' | head -1)
            [ -n "$why" ] || why="no error output; got '$probe'"
            note_result "SKIP llvm/$lang/* ('salam llvm --jit' probe failed: $why)" "llvm/$lang/all"
        fi
    done
fi

if want llvmapi; then
    for lang in $LANGS; do
        [ -d "tests/$lang/llvmapi" ] || continue
        probe_dir="$WORK/llvmapi_probe_$lang"
        mkdir -p "$probe_dir"
        probe_exe="$probe_dir/probe"
        probe_err="$probe_dir/probe.err"
        probe_src="$(pwd)/tests/$lang/llvmapi/_probe.salam"
        (cd "$probe_dir" && "$SALAM_ABS" build "$probe_src" \
            --output="$probe_exe" --no-color --log-level=error) >"$probe_err" 2>&1
        probe=""
        [ -x "$probe_exe" ] && probe=$("$probe_exe" 2>/dev/null | tr -d '\r')
        if [ "$probe" = "OK" ]; then
            for f in tests/"$lang"/llvmapi/*.salam; do
                [ -e "$f" ] || continue
                name=$(basename "$f" .salam)
                case "$name" in _*) continue ;; esac
                exp="$(pick_expect "tests/$lang/llvmapi/$name")"
                [ -f "$exp" ] || continue
                add_job expect "llvmapi/$lang/$name" "$f" "$lang" "$exp"
            done
        else
            why=$(grep -i 'error\|not found' "$probe_err" 2>/dev/null | head -1)
            [ -n "$why" ] || why="probe produced '$probe'"
            note_result "SKIP llvmapi/$lang/* (needs libsalam_llvm.a on the link path; set SALAM_LLVM_LIBDIR: $why)" "llvmapi/$lang/all"
        fi
        rm -rf "$probe_dir"
    done
fi

if want exec; then
    for lang in $LANGS; do
        for f in tests/"$lang"/exec/*.salam; do
            [ -e "$f" ] || continue
            name=$(basename "$f" .salam)
            case "$name" in _*) continue ;; esac
            exp="$(pick_expect "tests/$lang/exec/$name")"
            [ -f "$exp" ] || continue
            add_job exec "exec/$lang/$name" "$f" "$lang" "$exp"
        done
    done
fi

if want errors; then
    for lang in $LANGS; do
        for f in tests/"$lang"/errors/*.salam; do
            [ -e "$f" ] || continue
            name=$(basename "$f" .salam)
            case "$name" in _*) continue ;; esac
            add_job errors "errors/$lang/$name" "$f" "$lang" -
        done
    done
fi

if want port; then
    if [ -z "$SECTIONS" ] && [ "${SALAM_TEST_PORT:-0}" != "1" ]; then
        note_result "SKIP port/* (heavy; set SALAM_TEST_PORT=1 or run the 'port' section)" "port/all"
    else
        port_expect="$WORK/port.expect"
        printf '0 failed\n' >"$port_expect"
        for f in compiler/tests_port/*_test.salam; do
            [ -e "$f" ] || continue
            name=$(basename "$f" .salam)
            add_job expect "port/$name" "$f" en "$port_expect"
        done
    fi
fi

if want layout; then
    for lang in $LANGS; do
        for f in tests/"$lang"/layout/*.salam; do
            [ -e "$f" ] || continue
            name=$(basename "$f" .salam)
            case "$name" in _*) continue ;; esac
            add_job layout "layout/$lang/$name" "$f" "$lang" -
        done
    done
fi

TOTAL=$(wc -l <"$JOBS")
TOTAL=$((TOTAL + 0))
START=$(date +%s 2>/dev/null || echo 0)
if [ "$TOTAL" -gt 0 ]; then
    echo "== running $TOTAL tests on $NPROC parallel workers =="
    export SALAM SALAM_ABS WORK TOTAL
    if printf 'x\0' | xargs -0 -n 1 -P 2 sh -c ':' probe >/dev/null 2>&1; then
        tr '\n' '\000' <"$JOBS" | xargs -0 -n 1 -P "$NPROC" sh "$SELF" --worker
    else
        pids=""
        n=0
        while IFS= read -r line; do
            [ -n "$line" ] || continue
            sh "$SELF" --worker "$line" &
            pids="$pids $!"
            n=$((n + 1))
            if [ "$n" -ge "$NPROC" ]; then
                # shellcheck disable=SC2046
                set -- $pids
                wait "$1" 2>/dev/null
                shift
                pids="$*"
                n=$#
            fi
        done <"$JOBS"
        wait
    fi
fi

if want timereport; then
    tr_dir="$WORK/timereport"
    mkdir -p "$tr_dir"
    cat >"$tr_dir/tiny.salam" <<'TIMEREPORT_EOF'
func main:
    mut i := 0
    mut s := 0
    until i < 10:
        s += i
        i += 1
    end
    println s
    ret 0
end
TIMEREPORT_EOF

    tr_field() {
        tr -d ' ' <"$1" | grep -o "\"$2\":[0-9]*" | head -1 | cut -d: -f2
    }

    tr_json="$tr_dir/report.json"
    (cd "$tr_dir" && "$SALAM_ABS" build --time-report=json tiny.salam >/dev/null 2>"$tr_json")
    tr_rc=$?
    tr_line=$(grep '"schema":"salam.timereport.v1"' "$tr_json" | head -1)
    if [ "$tr_rc" -ne 0 ]; then
        note_result "FAIL timereport/json (build exited $tr_rc)" "timereport/json"
    elif [ -z "$tr_line" ]; then
        note_result "FAIL timereport/json (no salam.timereport.v1 object on stderr)" "timereport/json"
    else
        printf '%s\n' "$tr_line" >"$tr_json"
        tr_wall=$(tr_field "$tr_json" wall_ns)
        tr_bad=""
        [ -n "$tr_wall" ] && [ "$tr_wall" -gt 0 ] || tr_bad="wall_ns not positive"
        for k in source lexer parser semantic codegen write; do
            grep -q "\"$k\":{" "$tr_json" || tr_bad="missing phase '$k'"
        done
        tr_sum=$(tr -d ' ' <"$tr_json" | grep -o '"self_ns":[0-9]*' | cut -d: -f2 |
            awk '{ t += $1 } END { print t + 0 }')
        [ -n "$tr_wall" ] && [ "$tr_sum" -le $((tr_wall + tr_wall / 20)) ] ||
            tr_bad="self_ns sum $tr_sum exceeds wall $tr_wall"
        if [ -n "$tr_bad" ]; then
            note_result "FAIL timereport/json ($tr_bad)" "timereport/json"
        else
            note_result "PASS timereport/json" "timereport/json"
        fi
    fi

    (cd "$tr_dir" && "$SALAM_ABS" build --time-trace=trace.json tiny.salam >/dev/null 2>&1)
    if [ -s "$tr_dir/trace.json" ] && grep -q '"ph":"X"' "$tr_dir/trace.json"; then
        note_result "PASS timereport/trace" "timereport/trace"
    else
        note_result "FAIL timereport/trace (no trace events written)" "timereport/trace"
    fi

    if [ -n "${SALAM_SELFHOST:-}" ] && [ -x "$SALAM_SELFHOST" ]; then
        rm -rf "$tr_dir/.salam-build"
        (cd "$tr_dir" && "$SALAM_SELFHOST" build --time-report=json tiny.salam >/dev/null 2>"$tr_dir/self.json")
        grep '"schema":"salam.timereport.v1"' "$tr_dir/self.json" | head -1 >"$tr_dir/self1.json"
        if [ ! -s "$tr_dir/self1.json" ]; then
            note_result "FAIL timereport/parity (self-hosted emitted no report)" "timereport/parity"
        else
            keys_c=$(grep -o '"[a-z]*":{"self_ns"' "$tr_json" | sort | tr -d '
')
            keys_s=$(grep -o '"[a-z]*":{"self_ns"' "$tr_dir/self1.json" | sort | tr -d '
')
            if [ "$keys_c" = "$keys_s" ]; then
                note_result "PASS timereport/parity" "timereport/parity"
            else
                note_result "FAIL timereport/parity (C phases [$keys_c] vs Salam [$keys_s])" "timereport/parity"
            fi
        fi
    else
        note_result "SKIP timereport/parity (set SALAM_SELFHOST to the self-hosted binary)" "timereport/parity"
    fi
fi

ALL="$WORK/all-results.txt"
cat "$WORK/results"/* >"$ALL" 2>/dev/null || true
pass=$(grep -c '^PASS' "$ALL")
fail=$(grep -c '^FAIL' "$ALL")
skip=$(grep -c '^SKIP' "$ALL")

njobs=$(wc -l <"$JOBS" 2>/dev/null | tr -d ' ')
ngot=$(find "$WORK/results" -name 'r*' -type f 2>/dev/null | wc -l | tr -d ' ')
: "${njobs:=0}"
: "${ngot:=0}"
if [ "$njobs" -gt 0 ] && [ "$ngot" -lt "$njobs" ]; then
    missing=$((njobs - ngot))
    echo ""
    echo "FAIL harness/incomplete ($missing of $njobs jobs produced no result)"
    echo "  A worker that cannot start writes nothing, so those tests were"
    echo "  neither run nor reported. Counting them as failures rather than"
    echo "  omitting them keeps the summary honest."
    fail=$((fail + missing))
fi
if [ "$fail" -gt 0 ]; then
    echo ""
    echo "==== failed tests ===="
    awk '/^FAIL/ { p = 1 } /^(PASS|SKIP)/ { p = 0 } p' "$ALL"
fi
END=$(date +%s 2>/dev/null || echo 0)
echo "----------------------------------------"
if [ "$START" -gt 0 ] && [ "$END" -ge "$START" ]; then
    echo "RESULT: $pass passed, $fail failed, $skip skipped in $((END - START))s"
else
    echo "RESULT: $pass passed, $fail failed, $skip skipped"
fi
[ "$fail" -eq 0 ]
