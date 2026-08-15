#!/usr/bin/env bash
# Runs the whole comparison and writes results.json + results.md.
#
#   tests/en/apps/httpbench/bench/run.sh                 # everything
#   SERVERS="salam nginx" ROUTES="plaintext json" run.sh # a subset
#
# Method, and why each piece of it is there:
#
#   * One server runs at a time. Four servers sharing four cores would measure
#     scheduling, not serving.
#   * Every server is given every core it can use (nginx workers, node cluster
#     workers, Salam threads). `php -S` is single-request-at-a-time by design;
#     that is reported as what it is rather than worked around.
#   * Each route gets a warmup that is measured and thrown away, then a timed
#     run. Without the warmup, JIT-warmup in node and first-touch page faults
#     everywhere land inside the sample.
#   * The load generator and the servers share a machine. That costs every
#     server something, and it costs the fastest server the most, because it
#     is the one whose throughput is closest to what the generator itself can
#     drive. The generator's own CPU share is reported so the reader can see
#     how much room is left.
#   * Nothing here is a "who wins" number. The routes are chosen so that
#     differences can be attributed: /plaintext isolates the server loop,
#     /cached isolates response writing, /file adds one disk read, /compute
#     adds work that is identical in every language only in shape, never in
#     cost.

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
SERVERS="${SERVERS:-salam nginx node php}"
ROUTES="${ROUTES:-plaintext json cached file users search compute headers static home echo}"

SALAM_PORT=8099
NGINX_PORT=8100
PHP_PORT=8101
NODE_PORT=8102

NPROC="$(nproc)"
# The generator needs cores of its own, or it becomes the bottleneck and every
# server converges on the same wrong number. Servers get the low cores, the
# generator gets the rest, and neither is ever scheduled onto the other's.
SERVER_CORES="$(( NPROC > 2 ? NPROC - THREADS : 1 ))"
[ "$SERVER_CORES" -lt 1 ] && SERVER_CORES=1
SERVER_CPUS="0-$((SERVER_CORES - 1))"
GEN_CPUS="$SERVER_CORES-$((NPROC - 1))"
[ "$SERVER_CORES" -ge "$NPROC" ] && GEN_CPUS="$SERVER_CPUS"

mkdir -p "$RUNDIR"
RESULTS="$RUNDIR/results.json"
: > "$RESULTS"

say() { printf '\033[1m%s\033[0m\n' "$*" >&2; }
note() { printf '  %s\n' "$*" >&2; }

# ------------------------------------------------------------------ build

build_loadgen() {
    if [ ! -x "$RUNDIR/loadgen" ] || [ "$HERE/loadgen.c" -nt "$RUNDIR/loadgen" ]; then
        say "building loadgen"
        cc -O2 -pthread -o "$RUNDIR/loadgen" "$HERE/loadgen.c" || exit 1
    fi
}

build_salam() {
    # Point SALAM_HTTPBENCH_BIN at an already-built server to skip the build.
    # This is what makes a before/after comparison possible: build the two
    # versions once each, then measure them without the compiler in the loop.
    if [ -n "${SALAM_HTTPBENCH_BIN:-}" ]; then
        say "using prebuilt $SALAM_HTTPBENCH_BIN"
        cp "$SALAM_HTTPBENCH_BIN" "$RUNDIR/httpbench"
        return 0
    fi
    # `make -C c` drops the binary at the repository root; an installed or
    # cross-built one may sit under c/. Either is fine, SALAM_BIN overrides.
    if [ -z "${SALAM_BIN:-}" ]; then
        for cand in "$ROOT/salam" "$ROOT/c/salam"; do
            [ -x "$cand" ] && { SALAM_BIN="$cand"; break; }
        done
    fi
    [ -n "${SALAM_BIN:-}" ] && [ -x "$SALAM_BIN" ] \
        || { echo "no salam binary (run: make -C c)" >&2; exit 1; }
    say "building httpbench with $SALAM_BIN"
    ( cd "$RUNDIR" && "$SALAM_BIN" build "$APP/main.salam" --output="$RUNDIR/httpbench" \
        ${SALAM_BUILD_FLAGS:-} >"$RUNDIR/salam-build.log" 2>&1 ) \
        || { tail -20 "$RUNDIR/salam-build.log" >&2; exit 1; }
}

# The repository's .gitignore drops *.css and *.html under tests/, so neither
# asset survives a clean clone even though the routes that serve them are
# measured. Both are regenerated here rather than force-added, so the ignore
# rule stays as the repository intends and the benchmark still reproduces from
# nothing but a checkout.
ensure_assets() {
    if [ ! -s "$ASSETS/style.css" ]; then
        say "regenerating $ASSETS/style.css (gitignored)"
        cat > "$ASSETS/style.css" <<'CSS'
:root { color-scheme: light dark; --bg:#fff; --fg:#1b1b1f; --muted:#5b5b66;
        --line:#e2e2e8; --accent:#2f6f4f; }
@media (prefers-color-scheme: dark) {
  :root { --bg:#16161a; --fg:#eceef2; --muted:#a0a0ad; --line:#2c2c34;
          --accent:#7fd1a5; }
}
body { margin:0; background:var(--bg); color:var(--fg);
       font:16px/1.55 system-ui, -apple-system, "Segoe UI", sans-serif; }
main { max-width:46rem; margin:0 auto; padding:3rem 1.25rem; }
h1 { margin:0 0 .35rem; font-size:1.65rem; letter-spacing:-.01em; }
p { margin:0 0 1.75rem; color:var(--muted); }
table { width:100%; border-collapse:collapse; font-size:.94rem; }
th, td { text-align:left; padding:.55rem .7rem; border-bottom:1px solid var(--line);
         vertical-align:top; }
th { font-weight:600; color:var(--muted); font-size:.8rem; text-transform:uppercase;
     letter-spacing:.06em; }
td:first-child { white-space:nowrap;
                 font-family:ui-monospace, SFMono-Regular, Menlo, monospace; }
a { color:var(--accent); text-decoration:none; }
a:hover { text-decoration:underline; }
CSS
    fi

    # nginx serves / from a file, since it cannot render one. The file is the
    # byte-identical output of the Salam server's own / route, fetched once, so
    # the response sizes match across all four columns.
    if [ ! -s "$ASSETS/index.html" ]; then
        say "regenerating $ASSETS/index.html from the Salam server's own / route"
        if start_salam; then
            curl -s "http://127.0.0.1:$SALAM_PORT/" -o "$ASSETS/index.html"
        fi
        stop_server
        # An empty or missing index.html does not fail loudly on its own: nginx
        # happily serves 404s at several tens of thousands a second, and the
        # column reads like a result. Delete the stub and say so instead.
        if [ ! -s "$ASSETS/index.html" ]; then
            rm -f "$ASSETS/index.html"
            note "could not render index.html (see $RUNDIR/salam.log); nginx will skip /"
        fi
    fi
}

# ------------------------------------------------------------------ servers

SERVER_PID=""
stop_server() {
    [ -n "$SERVER_PID" ] && kill "$SERVER_PID" 2>/dev/null
    pkill -f "$RUNDIR/httpbench" 2>/dev/null
    pkill -f "nginx: master.*$RUNDIR" 2>/dev/null
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
        "$HERE/nginx/nginx.conf.template" > "$RUNDIR/nginx/nginx.conf"
    taskset -c "$SERVER_CPUS" \
        nginx -p "$RUNDIR/nginx" -c "$RUNDIR/nginx/nginx.conf" >"$RUNDIR/nginx.log" 2>&1 &
    SERVER_PID=$!
    wait_up "$NGINX_PORT"
}

start_node() {
    [ -d "$HERE/node/node_modules" ] || npm --prefix "$HERE/node" install --silent >/dev/null 2>&1
    HTTPBENCH_ASSETS="$ASSETS" PORT="$NODE_PORT" CLUSTER="$SERVER_CORES" \
        taskset -c "$SERVER_CPUS" node "$HERE/node/server.js" >"$RUNDIR/node.log" 2>&1 &
    SERVER_PID=$!
    wait_up "$NODE_PORT"
}

start_php() {
    # php -S handles one request at a time. PHP_CLI_SERVER_WORKERS is an
    # undocumented but real knob that forks N of them; it is set to the same
    # core budget the others get, so the column is "php -S at its best on this
    # machine" and not a strawman.
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
        node)  start_node ;;
        php)   start_php ;;
        *)     return 1 ;;
    esac
}

server_alive() { [ -n "$SERVER_PID" ] && kill -0 "$SERVER_PID" 2>/dev/null; }

# A server that dies mid-run would otherwise score zero on every route after
# the one that killed it, which reads as "this server is slow" when what
# happened is "this server crashed". Restarting and counting it separates the
# two, and the count goes in the results so it cannot be lost.
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
        php)   echo "$PHP_PORT" ;;
        node)  echo "$NODE_PORT" ;;
    esac
}

# ------------------------------------------------------------------ routes

path_of() {
    case "$1" in
        plaintext) echo "/plaintext" ;;
        json)      echo "/json" ;;
        cached)    echo "/cached" ;;
        file)      echo "/file" ;;
        users)     echo "/users/42" ;;
        search)    echo "/search?q=salam&n=5" ;;
        compute)   echo "/compute?n=1000" ;;
        headers)   echo "/headers" ;;
        static)    echo "/static/style.css" ;;
        home)      echo "/" ;;
        echo)      echo "/echo" ;;
    esac
}

# Routes nginx cannot answer without an application behind it. Skipped rather
# than faked; see nginx.conf.template. `home` joins them only when the
# pre-rendered page is missing, which nginx cannot produce for itself.
nginx_skips() {
    case "$1" in
        search | compute | echo | headers) return 0 ;;
        home) [ ! -s "$ASSETS/index.html" ] && return 0 ;;
    esac
    return 1
}

# Waits for the kernel's TIME_WAIT table to drain before the next route.
#
# A server that closes after every response leaves one socket in TIME_WAIT per
# request, and each of those holds an ephemeral port on loopback for a minute.
# The generator closes hard to avoid most of this, but a server that closes
# first still puts its own end into TIME_WAIT, and enough of them starve the
# next route of ports. Without this pause the symptom is a route that scores
# zero for no visible reason - which is exactly how the first run of this
# harness failed, and it looked like a bug in the server rather than in the
# measurement.
# The threshold is low on purpose. A close-per-request server can leave
# 25,000 sockets in TIME_WAIT after a single eight-second route, and measuring
# the next route against a half-full port table charges it for the previous
# one's mess. Every route therefore starts from a table that is close to
# empty, whichever server filled it.
drain_time_wait() {
    local limit=2000 waited=0
    while [ "$(ss -tan state time-wait 2>/dev/null | wc -l)" -gt "$limit" ] && [ "$waited" -lt 120 ]; do
        sleep 5
        waited=$((waited + 5))
    done
    [ "$waited" -gt 0 ] && note "waited ${waited}s for TIME_WAIT sockets to expire"
    return 0
}

# The first route measured against a freshly started server reads low even
# with a per-route warmup: allocator arenas, thread stacks and page tables are
# all cold, and node has a JIT that has not seen the code yet. This drives a
# short burst before any route is measured, so the first route is not
# systematically punished for being first.
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
    printf '%s\n' "{\"server\":\"$server\",\"route\":\"$route\",\"died\":$died,${out#\{}" >> "$RESULTS"
    printf '  %-6s %-10s %s%s\n' "$server" "$route" \
        "$(printf '%s' "$out" | sed -n 's/.*"rps":\([0-9.]*\).*"p99_us":\([0-9]*\).*/\1 req\/s, p99 \2us/p')" \
        "$($died && printf '   <-- SERVER DIED')" >&2
}

# ------------------------------------------------------------------ main

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
        if [ "$server" = "nginx" ] && nginx_skips "$route"; then
            note "nginx skips /$route (needs an application server)"
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
python3 "$HERE/report.py" "$RESULTS" > "$RUNDIR/results.md" && say "table: $RUNDIR/results.md"
