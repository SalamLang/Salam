#!/bin/sh
# Salam end-to-end test suite - fully parallel.
#
# Every test category is first collected into one flat job list (so the
# grand total is known up front), then ALL tests - including the sections
# that used to run sequentially (exec, errors, layout, fmt, llvm, cross,
# and the example .expect/.buildonly passes) - execute concurrently on
# $NPROC workers with a live "STATUS [index/total] label" progress line
# per test. Each test runs in its own isolated scratch directory, so no
# two jobs ever collide on generated files (salam_mod_*.c, *.ll, html,
# js bundles, ...). Heavy build-based sections are queued first so the
# worker pool stays saturated until the end.
#
# Usage:
#   sh tools/bash/run-tests.sh [-j N] [section ...]
# Sections: general exec js errors layout fmt ssl db opencv llvm cross timereport
#           examples apps basics data editor-selected features games
#           interop stdlib types webframework
# Env: SALAM, SALAM_STD, LANGS, NPROC, SALAM_TEST_TIMEOUT,
#      SALAM_TEST_NETWORK=1, SALAM_TEST_INTERACTIVE=1
#
# Portability: plain POSIX sh, matching every host compiler-release.yml
# runs it on - Linux x64/i686/aarch64/arm/armhf, macOS, and Windows
# (MSYS2/Git Bash), 64-bit and 32-bit alike. Parallel dispatch prefers
# `xargs -0 -P` (GNU, BSD/macOS and MSYS2 all support it) and falls back
# to a plain background-job pool when a host's xargs cannot do -0/-P.

set -u

case "$0" in /* | [A-Za-z]:*) SELF="$0" ;; *) SELF="$(pwd)/$0" ;; esac
TOOLS_DIR=$(dirname "$SELF")
TAB=$(printf '\t')

# ---------------------------------------------------------------------------
# Worker mode: `sh run-tests.sh --worker "<tab-separated job line>"`.
# Runs exactly one test, writes its full result block to $WORK/results/,
# and prints the block live with an "[index/total]" progress marker on
# the first line. Spawned only by the dispatcher below, never by hand.
# ---------------------------------------------------------------------------
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

    # Portable `timeout` (absent on stock macOS): drop the limit if missing.
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
        # produced: 1 once something exists to inspect. Without it a build
        # that never emitted a binary was reported as "missing expected:
        # <last line>", i.e. blamed on the program's output, which sent
        # anyone reading CI after the wrong bug - a compile or link error
        # and a genuinely wrong line are not the same failure.
        produced=0
        if [ -x "$exe" ]; then
            produced=1
            got=$(tmo 20 "$exe" </dev/null 2>&1 | tr -d '\r')
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
                # Every unmatched line, not just the last one to fail. With
                # only the last kept, a two-line expectation where both
                # lines were absent looked identical to one where only the
                # second was, which is exactly the ambiguity that made
                # interop/en/mysql_demo unreadable from a CI log alone.
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
        if ! "$SALAM_ABS" build "$f" --output="$outbin" --no-color --log-level=error \
            --lang="$lang" --target="$target" >/dev/null 2>&1; then
            echo "SKIP $label (cross build unavailable for $target - no embedded static libs, or self-hosted/non-flagship salam)"
            return
        fi
        if [ -z "$runner" ]; then
            # An empty runner means "this host executes the target natively",
            # which only holds when the target OS matches the host OS. A
            # Windows host cannot exec an ELF (and a Linux host cannot exec a
            # PE), so report the same SKIP the qemu/wine paths use instead of
            # letting the exec fail with a bare "Exec format error".
            hostos=$(uname -s 2>/dev/null || echo unknown)
            native=1
            case "$hostos" in
            MINGW* | MSYS* | CYGWIN*) case "$target" in *windows*) ;; *) native=0 ;; esac ;;
            Linux) case "$target" in *linux*) ;; *) native=0 ;; esac ;;
            Darwin) case "$target" in *darwin* | *apple* | *macos*) ;; *) native=0 ;; esac ;;
            esac
            if [ "$native" -eq 0 ]; then
                echo "SKIP $label (build OK; a $hostos host cannot run a $target binary and no emulator is configured for it)"
                rm -f "$outbin"
                return
            fi
        fi
        if [ -n "$runner" ] && ! command -v "$runner" >/dev/null 2>&1; then
            alt="${runner%-static}"
            if [ "$alt" != "$runner" ] && command -v "$alt" >/dev/null 2>&1; then
                runner="$alt"
            else
                echo "SKIP $label (build OK; no $runner on this host to run the $target binary)"
                rm -f "$outbin"
                return
            fi
        fi
        got=$($runner "$outbin" 2>&1 | tr -d '\r')
        wk_check "$expabs" "$got"
        rm -f "$outbin"
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
            out=$("$SALAM_ABS" inspect "$f" --emit-symbol-xml --no-color --log-level=error --lang="$lang" 2>&1 >/dev/null)
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
            # `salam llvm --jit` drops <name>.ll next to the process cwd,
            # so each job gets a private cwd to run from.
            jobdir="$WORK/llvmjob_${jobid}_$$"
            mkdir -p "$jobdir"
            got=$( (cd "$jobdir" && "$SALAM_ABS" llvm "$fabs" --jit --no-color --log-level=error 2>/dev/null) | tr -d '\r')
            rm -rf "$jobdir"
            wk_check "$expabs" "$got"
            ;;
        fmt) wk_fmt ;;
        expect) wk_expect ;;
        buildonly) wk_buildonly ;;
        cross) wk_cross ;;
        esac
    }

    block=$(run_worker)
    [ -n "$block" ] || block="FAIL $label (worker produced no output)"
    # Exact progress index: a noclobber-created lockfile serializes the
    # read-increment-write of $WORK/.counter. `set -C; : > file` is an
    # atomic O_EXCL create in every POSIX sh (pure builtins, no external
    # process) and behaves correctly on Linux, macOS and MSYS2/NTFS, so
    # no two workers can ever claim the same index. The spin is bounded:
    # if a worker is killed while holding the lock, the stale lockfile
    # is stolen after ~5s instead of hanging the whole suite.
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

# ---------------------------------------------------------------------------
# Main mode: collect every requested test into $JOBS, then dispatch.
# ---------------------------------------------------------------------------
. "$TOOLS_DIR/lib.sh"
salam_ensure_compiler --quiet
WORK="${WORK:-${TMPDIR:-/tmp}/salam-run-tests-work.$$}"
mkdir -p "$WORK/results"
printf '0\n' >"$WORK/.counter"
trap 'rm -rf "$WORK"' EXIT
LANGS="${LANGS:-en fa ar}"
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

if [ -z "${SALAM_STD:-}" ] && [ -d "$(pwd)/std" ]; then
    SALAM_STD="$(pwd)/std"
    export SALAM_STD
fi

# Generated C must compile without a single diagnostic from the C compiler,
# and a build that produces one fails the test that produced it. On by
# default here, not left to the caller: a warning that only fails when
# somebody remembers to ask is a warning that comes back, which is exactly
# how the "assignment makes pointer from integer without a cast" family kept
# reappearing in one module after being fixed in another. Set
# SALAM_C_STRICT=0 to inspect a corpus-wide regression without the suite
# stopping on the first one.
: "${SALAM_C_STRICT:=1}"
export SALAM_C_STRICT

# Some tests (e.g. stdlib/os_detect) legitimately produce different, all
# "correct", output depending on the host OS and/or CPU architecture -
# rather than skip them off the primary (linux/x64) CI host, an optional
# more-specific sibling is preferred over the plain $name.out when one is
# present for this host, checked most-specific first:
#   $name.$HOST_OS.$HOST_ARCH.out   (e.g. os_detect.windows.arm64.out)
#   $name.$HOST_OS.out              (e.g. os_detect.windows.out)
#   $name.out                       (default/fallback - every existing
#                                     test is unaffected unless it actually
#                                     ships one of the more specific files)
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

EXAMPLE_DIRS="apps basics data editor-selected features games interop stdlib types webframework"

want_example() {
    want "$1" && return 0
    for s in $SECTIONS; do [ "$s" = examples ] && return 0; done
    return 1
}

# The redis_* tests talk to a real server on 127.0.0.1:6379 instead of the
# mysql-style mock, so they are only runnable where one is listening. Probe
# once and skip (not fail) when it is absent, the same way the llvm/opencv
# sections degrade - a missing optional service is not a test failure.
# Start one with e.g. `redis-server --port 6379` (under WSL on Windows: WSL2
# forwards localhost, so a server bound in the distro is reachable here).
# `nc` and `redis-cli` are the portable probes and are tried first; bash's
# /dev/tcp is only a last resort for the common Git-Bash-on-Windows case where
# neither is installed. It is a bash extension (undefined in POSIX sh, hence
# the SC3025 suppression), so it is reached only when this really is bash.
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
            if [ -f "$exp" ]; then
                add_job build "$dir/$lang/$name" "$f" "$lang" "$exp"
            fi
            if [ -f "$base.expect" ]; then
                skip=""
                if [ -f "$base.redis" ] && [ "${REDIS_OK:-0}" != "1" ]; then
                    skip="requires a Redis server on 127.0.0.1:6379"
                elif [ -f "$base.network" ] && [ "${SALAM_TEST_NETWORK:-0}" != "1" ]; then
                    skip="requires live network; set SALAM_TEST_NETWORK=1"
                elif [ -f "$base.interactive" ] && [ "${SALAM_TEST_INTERACTIVE:-0}" != "1" ]; then
                    skip="opens a modal window; set SALAM_TEST_INTERACTIVE=1 on a desktop session"
                fi
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
    for lang in $LANGS; do
        [ -f "tests/$lang/db/mysql_mock.c" ] && {
            mockc="tests/$lang/db/mysql_mock.c"
            break
        }
    done
    dbok=0
    if [ -n "$DBCC" ] && [ -n "$mockc" ] && command -v ar >/dev/null 2>&1; then
        mkdir -p "$WORK/dbwork/.work"
        if "$DBCC" -c "$mockc" -o "$WORK/dbwork/.work/mysql_mock.o" >/dev/null 2>&1 &&
            ar rcs "$WORK/dbwork/.work/libsalammock.a" "$WORK/dbwork/.work/mysql_mock.o" >/dev/null 2>&1; then
            dbok=1
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
        probe_raw=$("$SALAM" llvm "tests/$lang/llvm/_probe.salam" --jit --no-color --log-level=error 2>/dev/null)
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
            note_result "FAIL llvm/$lang/* (salam crashed on probe, signal $((prc - 128)); rebuild salam via tools/build-compiler.sh)" "llvm/$lang/all"
        else
            note_result "SKIP llvm/$lang/* (LLVM toolchain unavailable: 'salam llvm --jit' probe failed)" "llvm/$lang/all"
        fi
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

# The self-hosted compiler's own unit tests. Every one of them links a large
# slice of compiler/, so this section costs minutes of build time where the
# others cost seconds - it is opt-in via SALAM_TEST_PORT=1 or by naming the
# section, and skipped (never silently dropped) otherwise.
#
# It is wired here at all because being wired nowhere is what happened last
# time: the dir-per-stage restructure moved lexer.salam to lexer/lexer.salam,
# every file here kept importing the flat paths, and with no runner
# referencing them the whole suite sat unbuildable without failing anything.
#
# *_test.salam only. The *_run.salam files are differential runners that take
# a source path and print output for byte-comparison against a reference
# compiler; they are not self-contained suites and have nothing to assert.
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

# ---------------------------------------------------------------------------
# timereport: --time-report / --time-trace, and the C-vs-Salam parity of the
# JSON they emit. Timings are machine-dependent, so nothing here asserts a
# duration; it checks the report's SHAPE (schema tag, phase keys, self <= wall)
# and that the two implementations agree on it.
# ---------------------------------------------------------------------------
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

    # Extract a top-level key's value from the one-line report JSON.
    tr_field() {
        tr -d ' ' <"$1" | grep -o "\"$2\":[0-9]*" | head -1 | cut -d: -f2
    }

    tr_json="$tr_dir/report.json"
    (cd "$tr_dir" && "$SALAM" build --time-report=json tiny.salam >/dev/null 2>"$tr_json")
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
        # Sum of per-phase self time can never exceed wall time (5% slack for
        # the clock reads the profiler itself performs).
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

    # --time-trace writes a Chrome Trace Event array the same run.
    (cd "$tr_dir" && "$SALAM" build --time-trace=trace.json tiny.salam >/dev/null 2>&1)
    if [ -s "$tr_dir/trace.json" ] && grep -q '"ph":"X"' "$tr_dir/trace.json"; then
        note_result "PASS timereport/trace" "timereport/trace"
    else
        note_result "FAIL timereport/trace (no trace events written)" "timereport/trace"
    fi

    # Parity: the self-hosted compiler must report the same phase key set. Only
    # runs when a second binary is pointed at by SALAM_SELFHOST; skips (not
    # fails) otherwise, the same way the llvm/opencv sections degrade.
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

# A job that produced no result at all is not a pass. The tally above counts
# result files, so a worker that died before writing one simply vanished from
# the arithmetic: a run where 42 of 302 workers failed to start still printed
# "260 passed, 0 failed" and every CI gate grepping for "0 failed" accepted
# it. Compare what was queued against what came back and account for the
# difference as failure, which is the only honest reading of "the test never
# ran".
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
