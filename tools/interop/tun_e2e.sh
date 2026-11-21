#!/bin/sh
# End to end check of std/net/tun on Linux inside a container with /dev/net/tun and NET_ADMIN.
# Usage: tun_e2e.sh SALAM_BIN   (needs docker; set TUN_E2E_IMAGE to use another alpine based image)
set -eu
salam=$1
here=$(cd "$(dirname "$0")" && pwd)
root=$(cd "$here/../.." && pwd)
image=${TUN_E2E_IMAGE:-alpine:3.20}
work=$(mktemp -d)
name=salam-tun-e2e-$$
trap 'docker rm -f "$name" >/dev/null 2>&1 || true; rm -rf "$work"' EXIT
(cd "$work" && SALAM_STD="$root/std" "$salam" build "$here/tun_e2e.salam" --target=x86_64-linux-musl --output="$work/tun_e2e" --log-level=error)
docker run -d --name "$name" --cap-add NET_ADMIN --device /dev/net/tun "$image" sleep 600 >/dev/null
docker exec "$name" apk add -q iproute2
docker cp "$work/tun_e2e" "$name:/tun_e2e"
docker exec "$name" /tun_e2e
