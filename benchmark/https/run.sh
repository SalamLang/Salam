#!/bin/bash
# Compares Salam HTTP and HTTPS with Go and nginx on this machine.
# usage: benchmark/https/run.sh [salam binary] [work dir]
# Set BASE_STD to a std/ directory to add the same server built from older code.
set -u
HERE=$(cd "$(dirname "$0")" && pwd)
SALAM=${1:-salam}
WORK=${2:-$(mktemp -d)}
ROUNDS=${ROUNDS:-3}
SECS=${SECS:-3}
CONNS=${CONNS:-16}
FLAGS=${SALAM_BUILD_FLAGS:---release --backend=llvm}
mkdir -p "$WORK"
cd "$WORK" || exit 1

openssl req -x509 -newkey rsa:2048 -nodes -keyout rsa.key -out rsa.crt -days 30 -subj /CN=localhost 2>/dev/null
openssl req -x509 -newkey ec -pkeyopt ec_paramgen_curve:P-256 -nodes -keyout ec.key -out ec.crt -days 30 -subj /CN=localhost 2>/dev/null
head -c 10485760 /dev/urandom > 10m.bin

go build -o loadgen "$HERE/loadgen.go" || exit 1
go build -o refgo "$HERE/refserver.go" || exit 1
# shellcheck disable=SC2086
"$SALAM" build "$HERE/server.salam" --output=salam-new $FLAGS --log-level=error 2>&1 | grep -v "override\|generated\|^$"
if [ -n "${BASE_STD:-}" ]; then
    # shellcheck disable=SC2086
    SALAM_STD=$BASE_STD "$SALAM" build "$HERE/server.salam" --output=salam-old $FLAGS --log-level=error 2>&1 | grep -v "override\|generated\|^$"
fi

mkdir -p nginx-s nginx-tmp
ln -sf ../10m.bin nginx-s/10m.bin
printf '{"message":"Hello, World!"}' > nginx-s/json
head -c 102400 /dev/zero | tr '\0' x > nginx-s/100k
cat > nginx.conf <<CONF
worker_processes auto;
pid $WORK/nginx.pid;
error_log /dev/null;
events { worker_connections 4096; }
http {
    access_log off;
    client_body_temp_path $WORK/nginx-tmp/body;
    proxy_temp_path $WORK/nginx-tmp/proxy;
    fastcgi_temp_path $WORK/nginx-tmp/fastcgi;
    uwsgi_temp_path $WORK/nginx-tmp/uwsgi;
    scgi_temp_path $WORK/nginx-tmp/scgi;
    keepalive_requests 100000000;
    default_type application/json;
    server {
        listen 18480;
        listen 18443 ssl;
        ssl_certificate $WORK/CERT.crt;
        ssl_certificate_key $WORK/CERT.key;
        ssl_protocols TLSv1.2 TLSv1.3;
        ssl_session_tickets off;
        root $WORK/nginx-s;
        location /s/ { alias $WORK/nginx-s/; }
        location = /json { default_type application/json; }
    }
}
CONF

PIDS=()
start() {
    "$@" > /dev/null 2>&1 &
    PIDS+=($!)
}
stop_all() {
    for p in "${PIDS[@]}"; do kill "$p" 2>/dev/null; done
    PIDS=()
    sleep 0.3
}
trap stop_all EXIT

best() {
    local best_line="" best_v=0 line v
    for _ in $(seq "$ROUNDS"); do
        line=$(./loadgen "$@" -d "$SECS" -c "$CONNS")
        v=${line%% *}
        if [ "$v" -gt "$best_v" ]; then best_v=$v; best_line=$line; fi
    done
    echo "$best_line"
}

printf '%-10s %-5s %-12s %-10s %10s %9s %8s %8s %5s\n' server cert scenario path 'req/s' 'MB/s' 'p50 ms' 'p99 ms' errs
row() {
    local server=$1 cert=$2 scen=$3 path=$4 addr=$5 secure=$6 keep=$7 out
    out=$(best -addr "$addr" -path "$path" -tls="$secure" -keep="$keep")
    [ -n "$out" ] || out="- - - - failed"
    # shellcheck disable=SC2086
    printf '%-10s %-5s %-12s %-10s %10s %9s %8s %8s %5s\n' "$server" "$cert" "$scen" "$path" $out
}

matrix() {
    local server=$1 plain=$2 tlsaddr=$3 cert=$4
    if [ "$cert" = ec ]; then
        row "$server" - http-keep /json "$plain" false true
        row "$server" - http-close /json "$plain" false false
        row "$server" - http-keep /s/10m.bin "$plain" false true
    fi
    row "$server" "$cert" https-keep /json "$tlsaddr" true true
    row "$server" "$cert" https-close /json "$tlsaddr" true false
    row "$server" "$cert" https-keep /100k "$tlsaddr" true true
    row "$server" "$cert" https-keep /s/10m.bin "$tlsaddr" true true
}

for cert in ec rsa; do
    for bin in salam-old salam-new; do
        [ -x "./$bin" ] || continue
        start "./$bin" http 18080
        start "./$bin" https 18081 "$cert.crt" "$cert.key"
        sleep 0.5
        matrix "$bin" 127.0.0.1:18080 127.0.0.1:18081 "$cert"
        stop_all
    done
    start ./refgo http 18082
    start ./refgo https 18083 "$cert.crt" "$cert.key"
    sleep 0.5
    matrix go 127.0.0.1:18082 127.0.0.1:18083 "$cert"
    stop_all
    if command -v nginx > /dev/null; then
        sed "s/CERT/$cert/" nginx.conf > "nginx-$cert.conf"
        start nginx -c "$WORK/nginx-$cert.conf" -g 'daemon off;'
        sleep 0.5
        matrix nginx 127.0.0.1:18480 127.0.0.1:18443 "$cert"
        stop_all
    fi
done
uptime
