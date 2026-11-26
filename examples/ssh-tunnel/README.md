# Salam Tunnel

An app, written entirely in Salam, that routes a whole computer through an SSH
server with one Connect button. The window uses the page language and is drawn
natively (`--ui=native`), with no browser engine. It follows the system light
or dark appearance.

## Platforms

| System  | State                                                                                                |
| ------- | ---------------------------------------------------------------------------------------------------- |
| Linux   | The whole tunnel is tested on arm64 containers; the window is tested on X11                          |
| macOS   | Builds and signs in; the privileged tunnel part has not been run on a real Mac                       |
| Windows | Builds from macOS; never run. Needs the OpenSSH client 8.4 or newer and `wintun.dll` next to the app |

## Requirements

- Salam newer than 0.7.1, or 0.7.1 with the standard library of this repository
- An SSH account on a server that allows TCP forwarding
  (`AllowTcpForwarding yes`, the OpenSSH default)
- Administrator rights, asked on each connect
- Linux: `libx11-dev` to build, `iproute2`, `curl`, and polkit (`pkexec`)
  unless you start the app as root

## Build and run

```sh
./build.sh                 # for the system you are on
./build.sh linux /opt/st   # on Linux, into a folder
./build.sh windows         # a cross build
```

On macOS the app is written to `~/Applications/Salam Tunnel.app`.
`SALAM=/path/to/salam` picks another compiler. The script uses the standard
library of this repository (`SALAM_STD` overrides it) and also builds the two
Linux relay programs with the musl sysroots of the compiler.

Enter the server, port, username and password, then press **Connect**. Leave
the password empty to use a key from `~/.ssh`. A key with a passphrase must be
loaded in `ssh-agent`. When connected, the window shows the public IP address,
which should be the server's.

## How it works

| Part                                      | Runs as                    | Job                                                                                                               |
| ----------------------------------------- | -------------------------- | ----------------------------------------------------------------------------------------------------------------- |
| Window (`main`, `engine`, `remote.salam`) | you                        | runs the OpenSSH client as a SOCKS proxy on a loopback port and reconnects it when it drops                       |
| Helper (`helper.salam`)                   | administrator              | creates a tunnel device with `std/net/tun`, sends every connection into the SSH proxy, points DNS into the tunnel |
| `host.salam`                              |                            | the parts that differ between systems: paths, DNS settings and routes                                             |
| Relay (`relay/main.salam`)                | your account on the server | sends UDP packets for you, when you tick **UDP too**                                                              |

The window and the helper are the same program; the system asks for permission
before the helper starts (a password prompt on macOS, polkit on Linux, UAC on
Windows). The helper restores the network when you disconnect, when the window
process dies, or when it receives a signal.

## What goes through the tunnel

- All IPv4 TCP connections.
- DNS, sent as DNS over TCP. The app uses the first of these that answers
  through your server: the server's own resolver (`127.0.0.53`), Google
  (`8.8.8.8`, `8.8.4.4`), Cloudflare (`1.1.1.1`, `1.0.0.1`), Quad9 (`9.9.9.9`),
  `4.2.2.4` and OpenDNS (`208.67.222.222`). While connected it checks every 20
  seconds and switches if the current one stops answering. The window shows
  which one is in use.
- UDP, if you tick **UDP too**. SSH cannot carry UDP, so the app copies a small
  relay program (`relay/main.salam`, built on `std/net/udprelay`) to
  `~/.salam-tunnel/` on the server and runs it over the SSH connection. It needs
  no root, listens only on the server's loopback address for a random token,
  stops with the connection, and the file stays there for next time. Only tick
  this for a server where you may run programs. It needs a Linux server on
  x86_64 or aarch64. UDP port 443 (QUIC) stays blocked so that browsers use TCP,
  which is faster through SSH.
- Not tunneled: ICMP (`ping` fails), IPv6, fragmented packets, and all UDP when
  the box is not ticked. These are blocked, not leaked. Addresses on your local
  network stay local.

While SSH is reconnecting, traffic is held back instead of going out directly.

Sites see the server's country. Whatever is blocked there, or refuses visitors
from there, will not load while you are connected.

## If the network stays broken

Routes disappear when the helper exits. On macOS and Linux the DNS settings are
saved first (`dns.backup` in `/var/db/salam-tunnel` or
`/var/lib/salam-tunnel`), so after a crash or power loss open the app and press
**Restore network settings**. By hand:

```sh
sudo networksetup -setdnsservers Wi-Fi Empty                   # macOS
sudo cp /var/lib/salam-tunnel/dns.backup /etc/resolv.conf      # Linux
```

On Windows, remove a leftover rule in an administrator PowerShell:

```powershell
Get-DnsClientNrptRule | Where-Object Comment -like 'salam-tun-*' | Remove-DnsClientNrptRule -Force
```

Turn off other VPN apps before connecting. The tunnel uses the addresses
`172.19.0.0/30`; a local network in that range will not work.

## Files and security

- `.salam-tunnel/` in your home folder holds the profile and the server's host key. The host key
  is trusted on first use, and a changed key stops the connection.
- The password is stored only on macOS, and only if you tick the Keychain box.
  It reaches `ssh` through `SSH_ASKPASS` and never appears on a command line.
- The helper runs the app binary as root, so keep the app somewhere only you can
  write to.
