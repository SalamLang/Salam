#!/usr/bin/env bash
# Runs the interoperability suite against real xray, sing-box, shadowsocks-rust and mihomo.
# Usage: run.sh SALAM_BIN BIN_DIR WORK_DIR   (set INTEROP_GEO_DIR to also check the geo dat reader)
#   SALAM_BIN  a salam compiler with a std/ directory next to it
#   BIN_DIR    holds xray, sing-box, ssserver, sslocal, mihomo (see docs/VPN_INTEROP.md for versions)
#   WORK_DIR   scratch space, created if missing; build outputs land here and are removed at the end
set -u
salam=$1
bins=$2
work=$3
here=$(cd "$(dirname "$0")" && pwd)
mkdir -p "$work/build"
"$here/build.sh" "$salam" "$work/build" || exit 1
export INTEROP_OURS="$work/build/vpn"
export INTEROP_VMESS_CLIENT="$work/build/vmess_client"
export INTEROP_SS_PEER="$work/build/ss_peer"
export INTEROP_MUX_PEER="$work/build/mux_peer"
export INTEROP_UDP_PEER="$work/build/udp_peer"
export INTEROP_TROJAN_PEER="$work/build/trojan_peer"
export INTEROP_VPN_FRAG="$work/build/vpn_frag"
export INTEROP_GEO_DUMP="$work/build/geo_dump"
export INTEROP_XRAY="$bins/xray"
export INTEROP_SINGBOX="$bins/sing-box"
export INTEROP_SSSERVER="$bins/ssserver"
export INTEROP_SSLOCAL="$bins/sslocal"
if [ -x "$bins/mihomo" ]; then export INTEROP_MIHOMO="$bins/mihomo"; fi
if [ -x "$bins/obfs-server" ]; then export INTEROP_OBFS_SERVER="$bins/obfs-server" INTEROP_OBFS_LOCAL="$bins/obfs-local"; fi
export INTEROP_WORK="$work/run"
mkdir -p "$INTEROP_WORK"
status=0
for t in matrix vmess_options ss_extra ss2022 proxies fallback mux udp trojan_udp stall fuzz; do
    echo "== $t"
    python3 -I "$here/$t.py" || status=1
done
if [ -n "${INTEROP_OBFS_SERVER:-}" ]; then
    echo "== obfs"
    python3 -I "$here/obfs.py" || status=1
fi
echo "== matrix2 (Reality destination without a post quantum group)"
INTEROP_DEST_CURVE=X25519 python3 -I "$here/matrix2.py" || status=1
echo "== matrix2 (Reality destination that picks X25519MLKEM768)"
INTEROP_DEST_CURVE='' python3 -I "$here/matrix2.py" reality || status=1
if [ -n "${INTEROP_GEO_DIR:-}" ]; then
    echo "== geo_check"
    python3 -I "$here/geo_check.py" || status=1
fi
rm -rf "$work/build"
exit $status
