#!/usr/bin/env bash

set -uo pipefail

HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
APP="$(cd "$HERE/.." && pwd)"
ROOT="$(cd "$APP/../../../.." && pwd)"
RUNDIR="${RUNDIR:-/tmp/httpbench}"
ASSETS="$APP/public"

DURATION="${DURATION:-10}"
WARMUP="${WARMUP:-3}"
CONNS="${CONNS:-64}"
THREADS="${THREADS:-2}"
SERVERS="${SERVERS:-salam nginx apache node php}"
ROUTES="${ROUTES:-plaintext json cached file users search compute headers static home echo}"

SALAM_PORT=8099
NGINX_PORT=8100
PHP_PORT=8101
NODE_PORT=8102
APACHE_PORT=8103

NPROC="$(nproc)"
SERVER_CORES="$((NPROC > 2 ? NPROC - THREADS : 1))"
[ "$SERVER_CORES" -lt 1 ] && SERVER_CORES=1
SERVER_CPUS="0-$((SERVER_CORES - 1))"
GEN_CPUS="$SERVER_CORES-$((NPROC - 1))"
[ "$SERVER_CORES" -ge "$NPROC" ] && GEN_CPUS="$SERVER_CPUS"

APACHE_THREADS=64
APACHE_MAX_WORKERS=$((SERVER_CORES * APACHE_THREADS))
[ "$APACHE_MAX_WORKERS" -lt "$CONNS" ] && APACHE_MAX_WORKERS="$CONNS"

mkdir -p "$RUNDIR"
RESULTS="$RUNDIR/results.json"
: >"$RESULTS"

say() { printf '\033[1m%s\033[0m\n' "$*" >&2; }
note() { printf '  %s\n' "$*" >&2; }

build_loadgen() {
    if [ ! -x "$RUNDIR/loadgen" ] || [ "$HERE/loadgen.c" -nt "$RUNDIR/loadgen" ]; then
        say "building loadgen"
        cc -O2 -pthread -o "$RUNDIR/loadgen" "$HERE/loadgen.c" || exit 1
    fi
}

resolve_salam_bin() {
    if [ -z "${SALAM_BIN:-}" ]; then
        for cand in "$ROOT/salam" "$ROOT/c/salam"; do
            [ -x "$cand" ] && {
                SALAM_BIN="$cand"
                break
            }
        done
    fi
}

build_salam() {
    if [ -n "${SALAM_HTTPBENCH_BIN:-}" ]; then
        say "using prebuilt $SALAM_HTTPBENCH_BIN"
        cp "$SALAM_HTTPBENCH_BIN" "$RUNDIR/httpbench"
        return 0
    fi
    resolve_salam_bin
    [ -n "${SALAM_BIN:-}" ] && [ -x "$SALAM_BIN" ] ||
        {
            echo "no salam binary (run: sh tools/bash/build-selfhost.sh)" >&2
            exit 1
        }
    say "building httpbench with $SALAM_BIN"
    (cd "$RUNDIR" && "$SALAM_BIN" build "$APP/main.salam" --output="$RUNDIR/httpbench" \
        ${SALAM_BUILD_FLAGS:-} >"$RUNDIR/salam-build.log" 2>&1) ||
        {
            tail -20 "$RUNDIR/salam-build.log" >&2
            exit 1
        }
}

ensure_assets() {
    if [ ! -s "$ASSETS/style.css" ]; then
        say "regenerating $ASSETS/style.css (gitignored)"
        cat >"$ASSETS/style.css" <<'CSS'
:root {
    color-scheme: light dark;
    --bg:#fff; --fg:#1b1b1f; --muted:#5b5b66;
    --line:#e2e2e8; --accent:#2f6f4f;
}
@media (prefers-color-scheme: dark) {
    :root {
        --bg:#16161a; --fg:#eceef2; --muted:#a0a0ad;
        --line:#2c2c34; --accent:#7fd1a5;
    }
}
body {
    margin:0; background:var(--bg); color:var(--fg);
    font:16px/1.55 system-ui, -apple-system, "Segoe UI", sans-serif;
}
main { max-width:46rem; margin:0 auto; padding:3rem 1.25rem; }
h1 { margin:0 0 .35rem; font-size:1.65rem; letter-spacing:-.01em; }
p { margin:0 0 1.75rem; color:var(--muted); }
table { width:100%; border-collapse:collapse; font-size:.94rem; }
th, td {
    text-align:left; padding:.55rem .7rem;
    border-bottom:1px solid var(--line); vertical-align:top;
}
th {
    font-weight:600; color:var(--muted); font-size:.8rem;
    text-transform:uppercase; letter-spacing:.06em;
}
td:first-child {
    white-space:nowrap;
    font-family:ui-monospace, SFMono-Regular, Menlo, monospace;
}
a { color:var(--accent); text-decoration:none; }
a:hover { text-decoration:underline; }
CSS
    fi

    if [ ! -s "$ASSETS/index.html" ]; then
        say "regenerating $ASSETS/index.html from the Salam server's own / route"
        if start_salam; then
            curl -s "http://127.0.0.1:$SALAM_PORT/" -o "$ASSETS/index.html"
        fi
        stop_server
        if [ ! -s "$ASSETS/index.html" ]; then
            rm -f "$ASSETS/index.html"
            note "could not render index.html (see $RUNDIR/salam.log); nginx will skip /"
        fi
    fi
}

SERVER_PID=""
stop_server() {
    [ -n "$SERVER_PID" ] && kill "$SERVER_PID" 2>/dev/null
    pkill -f "$RUNDIR/httpbench" 2>/dev/null
    pkill -f "nginx: master.*$RUNDIR" 2>/dev/null
    pkill -f "apache2 -f $RUNDIR/apache/httpd.conf" 2>/dev/null
    pkill -f "bench/node/server.js" 2>/dev/null
    pkill -f "php -S 127.0.0.1:$PHP_PORT" 2>/dev/null
    SERVER_PID=""
    sleep 0.4
}

wait_up() {
    local port="$1" tries=100
    while [ $tries -gt 0 ]; do
        curl -fs -o /dev/null "http://127.0.0.1:$port/health" && return 0
        curl -fs -o /dev/null "http://127.0.0.1:$port/plaintext" && return 0
        sleep 0.1
        tries=$((tries - 1))
    done
    return 1
}

start_salam() {
    HTTPBENCH_ASSETS="$ASSETS" PORT="$SALAM_PORT" \
        taskset -c "$SERVER_CPUS" "$RUNDIR/httpbench" >"$RUNDIR/salam.log" 2>&1 &
    SERVER_PID=$!
    wait_up "$SALAM_PORT"
}

start_nginx() {
    mkdir -p "$RUNDIR/nginx" "$RUNDIR/nginx/logs"
    sed -e "s|@RUNDIR@|$RUNDIR/nginx|g" \
        -e "s|@ASSETS@|$ASSETS|g" \
        -e "s|@PORT@|$NGINX_PORT|g" \
        -e "s|@WORKERS@|$SERVER_CORES|g" \
        "$HERE/nginx/nginx.conf.template" >"$RUNDIR/nginx/nginx.conf"
    taskset -c "$SERVER_CPUS" \
        nginx -p "$RUNDIR/nginx" -c "$RUNDIR/nginx/nginx.conf" >"$RUNDIR/nginx.log" 2>&1 &
    SERVER_PID=$!
    wait_up "$NGINX_PORT"
}

apache_canned() {
    mkdir -p "$RUNDIR/apache/canned"
    printf 'Hello, World!' >"$RUNDIR/apache/canned/plaintext.txt"
    printf '{"message":"Hello, World!","server":"apache","routes":13,"ok":true}' \
        >"$RUNDIR/apache/canned/json.json"
    printf '{"status":"ok","service":"httpbench"}' >"$RUNDIR/apache/canned/health.json"
    cp "$ASSETS/data.json" "$RUNDIR/apache/canned/cached.json"
}

start_apache() {
    local moddir
    moddir="$(apxs -q LIBEXECDIR 2>/dev/null)"
    [ -d "${moddir:-}" ] || moddir=/usr/lib/apache2/modules
    apache_canned
    sed -e "s|@RUNDIR@|$RUNDIR/apache|g" \
        -e "s|@ASSETS@|$ASSETS|g" \
        -e "s|@PORT@|$APACHE_PORT|g" \
        -e "s|@WORKERS@|$SERVER_CORES|g" \
        -e "s|@MAXWORKERS@|$APACHE_MAX_WORKERS|g" \
        -e "s|@MODDIR@|$moddir|g" \
        "$HERE/apache/httpd.conf.template" >"$RUNDIR/apache/httpd.conf"
    taskset -c "$SERVER_CPUS" \
        apache2 -f "$RUNDIR/apache/httpd.conf" -DFOREGROUND >"$RUNDIR/apache.log" 2>&1 &
    SERVER_PID=$!
    wait_up "$APACHE_PORT"
}

start_node() {
    [ -d "$HERE/node/node_modules" ] || npm --prefix "$HERE/node" install --silent >/dev/null 2>&1
    HTTPBENCH_ASSETS="$ASSETS" PORT="$NODE_PORT" CLUSTER="$SERVER_CORES" \
        taskset -c "$SERVER_CPUS" node "$HERE/node/server.js" >"$RUNDIR/node.log" 2>&1 &
    SERVER_PID=$!
    wait_up "$NODE_PORT"
}

start_php() {
    HTTPBENCH_ASSETS="$ASSETS" PHP_CLI_SERVER_WORKERS="$SERVER_CORES" \
        taskset -c "$SERVER_CPUS" \
        php -d opcache.enable_cli=1 -S "127.0.0.1:$PHP_PORT" -t "$HERE/php" "$HERE/php/index.php" \
        >"$RUNDIR/php.log" 2>&1 &
    SERVER_PID=$!
    wait_up "$PHP_PORT"
}

start_server() {
    case "$1" in
    salam) start_salam ;;
    nginx) start_nginx ;;
    apache) start_apache ;;
    node) start_node ;;
    php) start_php ;;
    *) return 1 ;;
    esac
}

server_alive() { [ -n "$SERVER_PID" ] && kill -0 "$SERVER_PID" 2>/dev/null; }

CRASHED_ROUTES=""
ensure_up() {
    local server="$1" route="$2"
    if server_alive; then return 0; fi
    note "$server is not running before /$route; restarting"
    CRASHED_ROUTES="$CRASHED_ROUTES $server:$route"
    start_server "$server"
}

port_of() {
    case "$1" in
    salam) echo "$SALAM_PORT" ;;
    nginx) echo "$NGINX_PORT" ;;
    apache) echo "$APACHE_PORT" ;;
    php) echo "$PHP_PORT" ;;
    node) echo "$NODE_PORT" ;;
    esac
}

path_of() {
    case "$1" in
    plaintext) echo "/plaintext" ;;
    json) echo "/json" ;;
    cached) echo "/cached" ;;
    file) echo "/file" ;;
    users) echo "/users/42" ;;
    search) echo "/search?q=salam&n=5" ;;
    compute) echo "/compute?n=1000" ;;
    headers) echo "/headers" ;;
    static) echo "/static/style.css" ;;
    home) echo "/" ;;
    echo) echo "/echo" ;;
    esac
}

nginx_skips() {
    case "$1" in
    search | compute | echo | headers) return 0 ;;
    home) [ ! -s "$ASSETS/index.html" ] && return 0 ;;
    esac
    return 1
}

apache_skips() {
    case "$1" in
    users) return 0 ;;
    esac
    nginx_skips "$1"
}

server_skips() {
    case "$1" in
    nginx) nginx_skips "$2" ;;
    apache) apache_skips "$2" ;;
    *) return 1 ;;
    esac
}

drain_time_wait() {
    local limit=2000 waited=0
    while [ "$(ss -tan state time-wait 2>/dev/null | wc -l)" -gt "$limit" ] && [ "$waited" -lt 120 ]; do
        sleep 5
        waited=$((waited + 5))
    done
    [ "$waited" -gt 0 ] && note "waited ${waited}s for TIME_WAIT sockets to expire"
    return 0
}

prewarm() {
    local port="$1"
    taskset -c "$GEN_CPUS" "$RUNDIR/loadgen" -j -c "$CONNS" -t "$THREADS" -d 3 -w 0 \
        "http://127.0.0.1:$port/health" >/dev/null 2>&1
    drain_time_wait
}

measure() {
    local server="$1" route="$2" port path extra=()
    port="$(port_of "$server")"
    path="$(path_of "$route")"
    [ "$route" = "echo" ] && extra=(-m POST -b '{"hello":"world","n":42}')

    local out
    out="$(taskset -c "$GEN_CPUS" "$RUNDIR/loadgen" -j -c "$CONNS" -t "$THREADS" \
        -d "$DURATION" -w "$WARMUP" "${extra[@]}" \
        "http://127.0.0.1:$port$path" 2>/dev/null)"
    [ -z "$out" ] && return 1

    local died=false
    if ! server_alive; then
        died=true
        CRASHED_ROUTES="$CRASHED_ROUTES $server:$route"
    fi
    printf '%s\n' "{\"server\":\"$server\",\"route\":\"$route\",\"died\":$died,${out#\{}" >>"$RESULTS"
    printf '  %-6s %-10s %s%s\n' "$server" "$route" \
        "$(printf '%s' "$out" | sed -n 's/.*"rps":\([0-9.]*\).*"p99_us":\([0-9]*\).*/\1 req\/s, p99 \2us/p')" \
        "$($died && printf '   <-- SERVER DIED')" >&2
}

build_loadgen
build_salam
ensure_assets

say "machine: $NPROC cores; servers on cpu $SERVER_CPUS, generator on cpu $GEN_CPUS ($THREADS threads)"
say "each route: ${WARMUP}s warmup discarded + ${DURATION}s measured, $CONNS connections"

for server in $SERVERS; do
    say "== $server"
    if ! start_server "$server"; then
        note "$server did not come up; see $RUNDIR/$server.log"
        stop_server
        continue
    fi
    prewarm "$(port_of "$server")"
    ensure_up "$server" "prewarm"
    for route in $ROUTES; do
        if server_skips "$server" "$route"; then
            note "$server skips /$route (needs an application server)"
            continue
        fi
        drain_time_wait
        ensure_up "$server" "$route"
        measure "$server" "$route"
    done
    stop_server
done

[ -n "$CRASHED_ROUTES" ] && say "servers that died during a run:$CRASHED_ROUTES"
say "results: $RESULTS"
resolve_salam_bin
if [ -n "${SALAM_BIN:-}" ] && [ -x "$SALAM_BIN" ]; then
    "$SALAM_BIN" run "$HERE/report.salam" "$RESULTS" >"$RUNDIR/results.md" &&
        say "table: $RUNDIR/results.md"
else
    say "no salam binary for the report; raw results stay at $RESULTS"
fi
