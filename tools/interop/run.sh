#!/usr/bin/env bash
# Runs the interoperability suite against real xray, sing-box, shadowsocks-rust and mihomo.
# Usage: run.sh SALAM_BIN BIN_DIR WORK_DIR
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
export INTEROP_XRAY="$bins/xray"
export INTEROP_SINGBOX="$bins/sing-box"
export INTEROP_SSSERVER="$bins/ssserver"
export INTEROP_SSLOCAL="$bins/sslocal"
export INTEROP_MIHOMO="$bins/mihomo"
export INTEROP_WORK="$work/run"
mkdir -p "$INTEROP_WORK"
status=0
for t in matrix vmess_options ss_extra proxies fallback fuzz; do
    echo "== $t"
    python3 -I "$here/$t.py" || status=1
done
rm -rf "$work/build"
exit $status
