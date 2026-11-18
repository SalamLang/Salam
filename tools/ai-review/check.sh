#!/usr/bin/env bash
set -u
here=$(cd "$(dirname "$0")" && pwd)
root=$(cd "$here/../.." && pwd)
salam=${SALAM:-salam}
port=${PORT:-18120}
fail=0
work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT
(cd "$root" && "$salam" build .github/scripts/ai_pr_review.salam -o "$work/review" >"$work/build.log" 2>&1) || { tail -20 "$work/build.log"; echo "build failed"; exit 1; }

run_case() {
    local mode=$1 want_exit=$2 want_post=$3 want_text=$4
    MODE=$mode PORT=$port node "$here/mock.mjs" >/dev/null 2>&1 &
    local pid=$!
    for _ in $(seq 1 50); do
        curl -s "http://127.0.0.1:$port/stats" >/dev/null 2>&1 && break
        sleep 0.1
    done
    local out
    out=$(cd "$root" && GITHUB_TOKEN=t GITHUB_REPOSITORY=o/r PR_NUMBER=1 SALAM_AI_API_KEY=k \
        GITHUB_API_URL="http://127.0.0.1:$port" SALAM_AI_API_URL="http://127.0.0.1:$port" \
        SALAM_AI_RETRY_WAIT=1 GITHUB_STEP_SUMMARY=/dev/null "$work/review" 2>&1)
    local code=$?
    local stats
    stats=$(curl -s "http://127.0.0.1:$port/stats")
    kill "$pid" 2>/dev/null
    wait "$pid" 2>/dev/null
    local posted=0
    case "$stats" in *'"posted":1'*) posted=1 ;; esac
    local ok=1
    [ "$code" = "$want_exit" ] || ok=0
    [ "$posted" = "$want_post" ] || ok=0
    case "$out" in *"$want_text"*) ;; *) ok=0 ;; esac
    if [ "$ok" = 1 ]; then
        echo "ok   $mode"
    else
        echo "FAIL $mode (exit $code, posted $posted, wanted $want_exit/$want_post/'$want_text')"
        printf "%s\n" "${out//$'\n'/$'\n     '}"
        fail=1
    fi
}

run_case good 0 1 "posted review comment"
run_case padding 0 1 "posted review comment"
run_case flaky 0 1 "retrying"
run_case html502 0 0 "HTTP 502"
run_case empty 0 0 "empty response body"
run_case blank 0 0 "empty response body"
run_case truncated 0 0 "invalid JSON"
run_case errorobject 0 0 "engine_failed"
run_case text 0 0 "upstream connect error"
run_case unauthorized 1 0 "rejected SALAM_AI_API_KEY"
exit $fail
