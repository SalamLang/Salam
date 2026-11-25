#!/bin/sh
# Tests that salam upgrade and salam upgrade-std ask the cdn.salamlang.ir
# mirror first (a local python http.server stands in for it) and that a mirror
# copy with a wrong sha256 is rejected. The commit used is one GitHub does not
# have, so a rejected mirror copy always ends in the same failure.

set -u

root=$(CDPATH='' cd -- "$(dirname -- "$0")/../.." && pwd)
here="$root/tools/bash"
salam=${1:-}
[ -n "$salam" ] || salam="$root/salam"
case "$salam" in /* | [A-Za-z]:*) ;; *) salam="$root/$salam" ;; esac

command -v python3 >/dev/null 2>&1 || {
    echo "python3 is needed"
    exit 1
}

work=$(mktemp -d)
pid=""
cleanup() {
    [ -z "$pid" ] || kill "$pid" 2>/dev/null
    rm -rf "$work"
}
trap cleanup EXIT INT TERM
cd "$work" || exit 1

sha=aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa

mkdir -p bin install/std cdn/releases cdn/std stage/Salam-$sha
cp "$salam" bin/salam
cp -R "$root/std/." install/std/
cp -R "$root/std" stage/Salam-$sha/std
tar -czf "cdn/std/$sha.tar.gz" -C stage "Salam-$sha"
good=$(sha256sum "cdn/std/$sha.tar.gz" | cut -d' ' -f1)
printf '%s  %s.tar.gz\n' "$good" "$sha" >"cdn/std/$sha.tar.gz.sha256"
printf '%s\n' "$sha" >cdn/std/main.commit
printf 'v9.9.9\n' >cdn/releases/latest

python3 -I "$here/test-installer-mirror-server.py" "$work/cdn" "$work/port" "$work/log" files &
pid=$!
n=0
while [ ! -s "$work/port" ] && [ "$n" -lt 100 ]; do
    sleep 0.1
    n=$((n + 1))
done
port=$(cat "$work/port")

export SALAM_CDN="http://127.0.0.1:$port"
export SALAM_STD="$work/install"
export NO_COLOR=1

echo "== newest release comes from the mirror"
: >log
bin/salam upgrade --check 2>&1 | sed -n 's/^  latest  *\(9\.9\.9\)$/latest \1/p'
grep -q "GET /releases/latest 200" log && echo "mirror asked"

echo "== the standard library comes from the mirror"
: >log
bin/salam upgrade-std --check 2>&1 | grep -c "newer standard library is available" | sed 's/^1$/update found/'
grep -q "GET /std/main.commit 200" log && echo "commit read from the mirror"
rc=0
bin/salam upgrade-std --force >out.txt 2>&1 || rc=$?
echo "exit $rc"
grep -q "tar.gz.sha256 200" log && echo "checksum fetched"
grep -q "GET /std/$sha.tar.gz 200" log && echo "archive fetched from the mirror"
grep -q "standard library is now main aaaaaaaaaaaa" out.txt && echo "installed"
grep -q "commit=$sha" install/.salam-std && echo "receipt written"

echo "== a wrong checksum rejects the mirror copy"
printf '%s  %s.tar.gz\n' "0000000000000000000000000000000000000000000000000000000000000000" "$sha" >"cdn/std/$sha.tar.gz.sha256"
rc=0
bin/salam upgrade-std --force >out.txt 2>&1 || rc=$?
[ "$rc" != 0 ] && echo "failed as expected"
grep -q "did not work (the copy failed its sha256 check); trying GitHub instead" out.txt && echo "mirror copy rejected"
grep -q "could not download the standard library" out.txt && echo "nothing installed from it"
