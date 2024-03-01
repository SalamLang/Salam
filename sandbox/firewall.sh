#!/usr/bin/env bash
set -euo pipefail

subnet="${1:?usage: firewall.sh SUBNET}"

iptables -N SALAM-SANDBOX 2>/dev/null || true
iptables -F SALAM-SANDBOX
for range in 10.0.0.0/8 172.16.0.0/12 192.168.0.0/16 169.254.0.0/16 100.64.0.0/10; do
    iptables -A SALAM-SANDBOX -d "$range" -j DROP
done
for address in $(hostname -I); do
    case "$address" in
    *:*) ;;
    *) iptables -A SALAM-SANDBOX -d "$address/32" -j DROP ;;
    esac
done
iptables -A SALAM-SANDBOX -j RETURN

iptables -C DOCKER-USER -s "$subnet" -j SALAM-SANDBOX 2>/dev/null || iptables -I DOCKER-USER 1 -s "$subnet" -j SALAM-SANDBOX
iptables -C INPUT -s "$subnet" -j DROP 2>/dev/null || iptables -I INPUT 1 -s "$subnet" -j DROP
