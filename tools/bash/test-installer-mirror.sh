#!/bin/sh
# Tests the mirror rule of install.sh: the CDN is tried first, and GitHub is the
# fallback on a missing file, a bad checksum, a stalled server or a dead port.
# A local python http.server stands in for the CDN and for GitHub; a curl
# wrapper points github.com at the second one.

set -u

root=$(CDPATH='' cd -- "$(dirname -- "$0")/../.." && pwd)
here="$root/tools/bash"

command -v python3 >/dev/null 2>&1 || {
    echo "python3 is needed"
    exit 1
}
command -v curl >/dev/null 2>&1 || {
    echo "curl is needed"
    exit 1
}

work=$(mktemp -d)
pids=""
cleanup() {
    for p in $pids; do
        kill "$p" 2>/dev/null
    done
    rm -rf "$work"
}
trap cleanup EXIT INT TERM
cd "$work" || exit 1

real_curl=$(command -v curl)

mkdir bin
cat >bin/curl <<EOF
#!/bin/sh
for a in "\$@"; do
    shift
    case "\$a" in
    https://github.com/SalamLang/Salam/releases*)
        a="http://127.0.0.1:\${TEST_GH_PORT}/releases\${a#https://github.com/SalamLang/Salam/releases}"
        ;;
    https://api.github.com/* | https://raw.githubusercontent.com/*)
        a="http://127.0.0.1:\${TEST_GH_PORT}/dead"
        ;;
    esac
    set -- "\$@" "\$a"
done
exec "$real_curl" "\$@"
EOF
chmod +x bin/curl

cat >mkzip.py <<'EOF'
import sys, zipfile
out, mark = sys.argv[1], sys.argv[2]
with zipfile.ZipFile(out, "w") as z:
    zi = zipfile.ZipInfo("salam-linux/salam")
    zi.external_attr = 0o755 << 16
    z.writestr(zi, "#!/bin/sh\necho " + mark + "\n" + "# pad\n" * 400)
EOF

version=9.9.9
asset="salam-$version-linux.zip"

mkrelease() {
    dir=$1
    mark=$2
    mkdir -p "$dir/releases/download/v$version"
    python3 -I mkzip.py "$dir/releases/download/v$version/$asset" "$mark"
    sum=$(sha256sum "$dir/releases/download/v$version/$asset" | cut -d' ' -f1)
    printf '%s  %s\n' "$sum" "$asset" >"$dir/releases/download/v$version/$asset.sha256"
}

start() {
    name=$1
    dir=$2
    mode=$3
    mkdir -p "$dir"
    : >"$work/$name.log"
    python3 -I "$here/test-installer-mirror-server.py" "$dir" "$work/$name.port" "$work/$name.log" "$mode" &
    pids="$pids $!"
    n=0
    while [ ! -s "$work/$name.port" ] && [ "$n" -lt 100 ]; do
        sleep 0.1
        n=$((n + 1))
    done
}

deadport() {
    python3 -I -c 'import socket; s = socket.socket(); s.bind(("127.0.0.1", 0)); print(s.getsockname()[1])'
}

mkrelease gh gh-copy
start gh "$work/gh" files
GH=$(cat gh.port)

mkrelease cdn cdn-copy
printf 'v%s\n' "$version" >cdn/releases/latest
printf 'v%s\n' "$version" >cdn/releases/tags.txt
start cdn "$work/cdn" files
CDN=$(cat cdn.port)

mkdir empty
start empty "$work/empty" files
EMPTY=$(cat empty.port)

mkrelease bad bad-copy
printf 'v%s\n' "$version" >bad/releases/latest
python3 -I mkzip.py "bad/releases/download/v$version/$asset" tampered
start bad "$work/bad" files
BAD=$(cat bad.port)

start hole "$work/hole" blackhole
HOLE=$(cat hole.port)

DEAD=$(deadport)

run() {
    cdnport=$1
    ghport=$2
    xenv=${3:-NOTHING=1}

    rm -rf inst home
    mkdir home
    : >gh.log
    : >cdn.log
    : >empty.log
    : >bad.log
    : >hole.log
    env -i HOME="$work/home" PATH="$work/bin:$PATH" TEST_GH_PORT="$ghport" \
        SALAM_CDN="http://127.0.0.1:$cdnport" SALAM_CDN_TIMEOUT=1 NO_COLOR=1 \
        $xenv sh "$root/install.sh" --platform linux --dir "$work/inst" --no-modify-path $ARGS >out.txt 2>&1
    rc=$?
}

installed() {
    "$work/inst/salam" 2>/dev/null
}

echo "== mirror first"
ARGS="--version $version"
run "$CDN" "$DEAD"
echo "exit $rc"
installed
grep -q "(mirror)" out.txt && echo "checksum verified on the mirror copy"
echo "github requests: $(grep -c . gh.log)"

echo "== newest release comes from the mirror"
ARGS=""
run "$CDN" "$DEAD"
echo "exit $rc"
installed

ARGS="--version $version"
echo "== missing on the mirror falls back"
run "$EMPTY" "$GH"
echo "exit $rc"
installed
grep -c . empty.log | sed 's/^[1-9][0-9]*$/mirror asked/'
grep -q "sha256 verified" out.txt && echo "github copy verified"

echo "== bad checksum on the mirror falls back"
run "$BAD" "$GH"
echo "exit $rc"
installed
grep -q "does not match its checksum" out.txt && echo "mirror copy rejected"

echo "== stalled mirror falls back"
run "$HOLE" "$GH"
echo "exit $rc"
installed
grep -q CONNECT hole.log && echo "mirror was asked"
grep -q "trying GitHub instead" out.txt && echo "fallback announced"

echo "== dead mirror port falls back"
run "$DEAD" "$GH"
echo "exit $rc"
installed
grep -q "trying GitHub instead" out.txt && echo "fallback announced"

echo "== SALAM_NO_CDN skips the mirror"
run "$CDN" "$GH" SALAM_NO_CDN=1
echo "exit $rc"
installed
echo "mirror requests: $(grep -c . cdn.log)"

echo "== empty SALAM_CDN skips the mirror"
run "$CDN" "$GH" SALAM_CDN=
echo "exit $rc"
installed
echo "mirror requests: $(grep -c . cdn.log)"

echo "== both fail"
run "$DEAD" "$DEAD"
if [ "$rc" != 0 ]; then echo "failed as expected"; fi
grep -q "tried http://127.0.0.1:$DEAD/releases/download/v$version/$asset (mirror) and" out.txt && echo "message names both sources"
grep -q "download it by hand" out.txt && echo "manual hint printed"
