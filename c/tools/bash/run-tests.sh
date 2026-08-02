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
# HOST_OS/HOST_ARCH use the same normalized vocabulary as install.sh's
# platform detection (linux/mac/windows, x64/arm64/x86/arm) so a test's
# variant filenames read the same way across the codebase.
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
    # $1 = path without extension (e.g. ../tests/en/stdlib/os_detect)
    if [ -n "$HOST_OS" ] && [ -n "$HOST_ARCH" ] && [ -f "$1.$HOST_OS.$HOST_ARCH.out" ]; then
        printf '%s\n' "$1.$HOST_OS.$HOST_ARCH.out"
    elif [ -n "$HOST_OS" ] && [ -f "$1.$HOST_OS.out" ]; then
        printf '%s\n' "$1.$HOST_OS.out"
    else
        printf '%s\n' "$1.out"
    fi
}

run_batch() {
    jobs="$1"
    runner="${2:-$RUN_ONE}"
    [ -s "$jobs" ] || return 0
    outdir="$WORK/.batch-out.$$"
    mkdir -p "$outdir"
    n=0
    running=0
    while IFS= read -r line; do
        [ -z "$line" ] && continue
        n=$((n + 1))
        out="$outdir/$(printf '%06d' "$n")"
        (
            IFS="	"
            set -- $line
            label="$1"
            f="$2"
            lang="$3"
            exp="$4"
            extra="$5"
            unset IFS
            sh "$runner" "$SALAM_ABS" "$WORK" "$label" "$f" "$lang" "$exp" $extra
        ) >"$out" 2>&1 &
        running=$((running + 1))
        if [ "$running" -ge "$NPROC" ]; then
            wait
            running=0
        fi
    done <"$jobs"
    wait
    results="$WORK/.batch-results.$$"
    cat "$outdir"/* >"$results" 2>/dev/null
    rm -rf "$outdir"
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
        for f in ../tests/"$lang"/general/*.salam; do
            [ -e "$f" ] || continue
            name=$(basename "$f" .salam)
            case "$name" in _*) continue ;; esac
            exp="$(pick_expect "../tests/$lang/general/$name")"
            [ -f "$exp" ] || continue
            def=$(grep -o 'DEFINE: [A-Za-z0-9_]*' "$f" | sed 's/DEFINE: /-D/' | tr '\n' ' ')
            printf 'general/%s/%s\t%s\t%s\t%s\t%s\n' "$lang" "$name" "$f" "$lang" "$exp" "${def:--}" >>"$jobs"
        done
    done
    run_batch "$jobs"
fi
if want exec; then
    for lang in $LANGS; do
        for f in ../tests/"$lang"/exec/*.salam; do
            [ -e "$f" ] || continue
            name=$(basename "$f" .salam)
            case "$name" in _*) continue ;; esac
            exp="$(pick_expect "../tests/$lang/exec/$name")"
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
        for f in ../tests/"$lang"/js/*.salam; do
            [ -e "$f" ] || continue
            name=$(basename "$f" .salam)
            case "$name" in _*) continue ;; esac
            exp="$(pick_expect "../tests/$lang/js/$name")"
            [ -f "$exp" ] || continue
            printf 'js/%s/%s\t%s\t%s\t%s\t-\n' "$lang" "$name" "$f" "$lang" "$exp" >>"$jobs"
        done
    done
    run_batch "$jobs" "$RUN_ONE_JS"
fi
if want errors; then
    for lang in $LANGS; do
        for f in ../tests/"$lang"/errors/*.salam; do
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
        for f in ../tests/"$lang"/layout/*.salam; do
            [ -e "$f" ] || continue
            name=$(basename "$f" .salam)
            case "$name" in _*) continue ;; esac
            expect=$(grep -oE '(EXPECT|انتظار|توقع): .*' "$f" | head -1 | sed -E 's/^(EXPECT|انتظار|توقع): //' | tr -d '\r')
            "$SALAM" layout build "$f" --inline --no-minify --output="$WORK/$name.html" --no-color --log-level=error --lang="$lang" >/dev/null 2>&1
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
        for f in ../tests/"$lang"/fmt/*.salam; do
            [ -e "$f" ] || continue
            name=$(basename "$f" .salam)
            exp="$(pick_expect "../tests/$lang/fmt/$name")"
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
if want ssl; then
    jobs="$WORK/.jobs-ssl.$$"
    : >"$jobs"
    for lang in $LANGS; do
        [ -d "../tests/$lang/ssl" ] || continue
        for f in ../tests/"$lang"/ssl/*.salam; do
            [ -e "$f" ] || continue
            name=$(basename "$f" .salam)
            case "$name" in _*) continue ;; esac
            exp="$(pick_expect "../tests/$lang/ssl/$name")"
            [ -f "$exp" ] || continue
            printf 'ssl/%s/%s\t%s\t%s\t%s\t-\n' "$lang" "$name" "$f" "$lang" "$exp" >>"$jobs"
        done
    done
    run_batch "$jobs"
fi
if want db; then
    for lang in $LANGS; do
        [ -d "../tests/$lang/db" ] || continue
        DBCC=""
        for c in tcc gcc cc clang; do command -v "$c" >/dev/null 2>&1 && {
            DBCC="$c"
            break
        }; done
        dbok=0
        if [ -n "$DBCC" ] && command -v ar >/dev/null 2>&1; then
            mkdir -p "$WORK/dbwork/.work"
            if "$DBCC" -c "../tests/$lang/db/mysql_mock.c" -o "$WORK/dbwork/.work/mysql_mock.o" >/dev/null 2>&1 &&
                ar rcs "$WORK/dbwork/.work/libsalammock.a" "$WORK/dbwork/.work/mysql_mock.o" >/dev/null 2>&1; then
                dbok=1
            fi
        fi
        if [ "$dbok" = "1" ]; then
            jobs="$WORK/.jobs-db-$lang.$$"
            : >"$jobs"
            for f in ../tests/"$lang"/db/*.salam; do
                [ -e "$f" ] || continue
                name=$(basename "$f" .salam)
                case "$name" in _*) continue ;; esac
                exp="$(pick_expect "../tests/$lang/db/$name")"
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
        [ -d "../tests/$lang/llvm" ] || continue
        probe_raw=$("$SALAM" llvm "../tests/$lang/llvm/_probe.salam" --jit --no-color --log-level=error 2>/dev/null)
        prc=$?
        probe=$(printf '%s' "$probe_raw" | tr -d '\r')
        rm -f _probe.ll _probe.ll.run.sh 2>/dev/null
        if [ "$probe" = "OK" ]; then
            for f in ../tests/"$lang"/llvm/*.salam; do
                [ -e "$f" ] || continue
                name=$(basename "$f" .salam)
                case "$name" in _*) continue ;; esac
                exp="$(pick_expect "../tests/$lang/llvm/$name")"
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

# Cross-compiled builds against the statically-embedded third-party libs
# (SALAM_EMBED_EXTRALIBS_*_DIR - see c/Makefile and
# c/src/llvm/llvm_native.c). Only meaningful on a "flagship self-contained"
# salam build; SKIPs (not FAILs) per-target rather than erroring when a
# target wasn't embedded (e.g. a plain non-release build) or this host has
# no way to execute that target's binary (e.g. no qemu-*-static, or the
# windows target on a runner with no Wine) - building without running is
# still useful signal (confirms the static link itself succeeded), but
# without a way to check output it can only SKIP, not PASS/FAIL.
if want cross; then
    for lang in $LANGS; do
        [ -d "../tests/$lang/cross" ] || continue
        for f in ../tests/"$lang"/cross/*.salam; do
            [ -e "$f" ] || continue
            name=$(basename "$f" .salam)
            case "$name" in _*) continue ;; esac
            exp="$(pick_expect "../tests/$lang/cross/$name")"
            [ -f "$exp" ] || continue
            for pair in x86_64-linux-musl: aarch64-linux-musl:qemu-aarch64-static \
                i686-linux-musl:qemu-i386-static arm-linux-musleabihf:qemu-arm-static \
                x86_64-w64-windows-gnu:wine; do
                target="${pair%%:*}"
                runner="${pair#*:}"
                label="cross/$lang/$name/$target"
                outbin="$WORK/cross_$$_$(echo "$name-$target" | tr '/.' '__')"
                case "$target" in *windows*) outbin="$outbin.exe" ;; esac
                if ! "$SALAM_ABS" build "$f" --output="$outbin" --no-color --log-level=error \
                    --lang="$lang" --target="$target" >/dev/null 2>&1; then
                    echo "SKIP $label (cross build unavailable for $target - no embedded static libs, or self-hosted/non-flagship salam)"
                    continue
                fi
                if [ -n "$runner" ] && ! command -v "$runner" >/dev/null 2>&1; then
                    echo "SKIP $label (build OK; no $runner on this host to run the $target binary)"
                    rm -f "$outbin"
                    continue
                fi
                got=$($runner "$outbin" 2>&1 | tr -d '\r')
                check_out "$label" "$exp" "$got"
                rm -f "$outbin"
            done
        done
    done
fi
# Categories that hold example projects (possibly nested, e.g. apps/webview/*
# or games/pacman/src/*): discovered recursively and matched by sibling
# .out/.expect/.buildonly files, unlike the flat sections above.
EXAMPLE_DIRS="apps basics data editor-selected features games interop stdlib types webframework"

want_example() {
    want "$1" && return 0
    for s in $SECTIONS; do [ "$s" = examples ] && return 0; done
    return 1
}

run_example_dir() {
    dir="$1"
    jobs="$WORK/.jobs-$dir.$$"
    : >"$jobs"
    for lang in $LANGS; do
        [ -d "../tests/$lang/$dir" ] || continue
        while IFS= read -r f; do
            case "$(basename "$f")" in _*) continue ;; esac
            rel="${f#../tests/"$lang"/"$dir"/}"
            name="${rel%.salam}"
            exp="$(pick_expect "../tests/$lang/$dir/$name")"
            [ -f "$exp" ] || continue
            printf '%s/%s/%s\t%s\t%s\t%s\t-\n' "$dir" "$lang" "$name" "$f" "$lang" "$exp" >>"$jobs"
        done <<EOF
$(find "../tests/$lang/$dir" -name '*.salam' | sort)
EOF
    done
    run_batch "$jobs"

    for lang in $LANGS; do
        [ -d "../tests/$lang/$dir" ] || continue
        while IFS= read -r f; do
            case "$(basename "$f")" in _*) continue ;; esac
            rel="${f#../tests/"$lang"/"$dir"/}"
            name="${rel%.salam}"
            base="../tests/$lang/$dir/$name"
            expf="$base.expect"
            [ -f "$expf" ] || continue
            if [ -f "$base.network" ] && [ "${SALAM_TEST_NETWORK:-0}" != "1" ]; then
                echo "SKIP $dir/$lang/$name (requires live network; set SALAM_TEST_NETWORK=1 to run - no need inside GitHub Actions)"
                continue
            fi
            id=$(echo "$name" | tr '/ ' '__')
            jobdir="$WORK/exjob_${dir}_${id}"
            mkdir -p "$jobdir"
            fabs="$(pwd)/$f"
            exe="$jobdir/a.exe"
            btry=1
            while [ ! -x "$exe" ] && [ "$btry" -le 3 ]; do
                [ "$btry" -gt 1 ] && sleep "$btry"
                (cd "$jobdir" && "$SALAM_ABS" build "$fabs" --output="$exe" --no-color --log-level=error --lang="$lang") >/dev/null 2>&1
                btry=$((btry + 1))
            done
            if [ -x "$exe" ]; then
                got=$(timeout 20 "$exe" </dev/null 2>&1 | tr -d '\r')
            else
                html="$jobdir/a.html"
                wtry=1
                while [ ! -f "$html" ] && [ "$wtry" -le 3 ]; do
                    [ "$wtry" -gt 1 ] && sleep "$wtry"
                    (cd "$jobdir" && "$SALAM_ABS" web "$fabs" --output="$html" --no-color --log-level=error --lang="$lang") >/dev/null 2>&1
                    wtry=$((wtry + 1))
                done
                got=$([ -f "$html" ] && tr -d '\r' <"$html")
            fi
            rm -rf "$jobdir"
            ok=1
            missing=""
            while IFS= read -r line || [ -n "$line" ]; do
                [ -z "$line" ] && continue
                case "$got" in
                *"$line"*) ;;
                *)
                    ok=0
                    missing="$line"
                    ;;
                esac
            done <"$expf"
            if [ "$ok" -eq 1 ]; then
                echo "PASS $dir/$lang/$name"
                pass=$((pass + 1))
            else
                echo "FAIL $dir/$lang/$name (missing expected: $missing)"
                fail=$((fail + 1))
            fi
        done <<EOF
$(find "../tests/$lang/$dir" -name '*.salam' | sort)
EOF
    done

    for lang in $LANGS; do
        [ -d "../tests/$lang/$dir" ] || continue
        while IFS= read -r f; do
            case "$(basename "$f")" in _*) continue ;; esac
            rel="${f#../tests/"$lang"/"$dir"/}"
            name="${rel%.salam}"
            base="../tests/$lang/$dir/$name"
            [ -f "$base.buildonly" ] || continue
            id=$(echo "$name" | tr '/ ' '__')
            jobdir="$WORK/exjob_${dir}_${id}"
            mkdir -p "$jobdir"
            fabs="$(pwd)/$f"
            exe="$jobdir/a.exe"
            buildlog="$jobdir/build.log"
            btry=1
            while [ ! -x "$exe" ] && [ "$btry" -le 3 ]; do
                [ "$btry" -gt 1 ] && sleep "$btry"
                (cd "$jobdir" && "$SALAM_ABS" build "$fabs" --output="$exe" --no-color --log-level=error --lang="$lang") >"$buildlog" 2>&1
                btry=$((btry + 1))
            done
            if [ -x "$exe" ]; then
                echo "PASS $dir/$lang/$name (build)"
                pass=$((pass + 1))
            else
                msg="FAIL $dir/$lang/$name (build failed)
$(sed 's/^/  /' "$buildlog" 2>/dev/null | head -20)"
                printf '%s\n' "$msg"
                fail=$((fail + 1))
            fi
            rm -rf "$jobdir"
        done <<EOF
$(find "../tests/$lang/$dir" -name '*.salam' | sort)
EOF
    done
}

for dir in $EXAMPLE_DIRS; do
    want_example "$dir" && run_example_dir "$dir"
done
echo "----------------------------------------"
echo "RESULT: $pass passed, $fail failed"
[ "$fail" -eq 0 ]
