# Salam Tunnel

A macOS app, written entirely in Salam, that routes the whole Mac through an SSH
server with one Connect button. The window uses the page language and is drawn
natively (`--ui=native`), with no browser engine. It follows the system light
or dark appearance.

## Requirements

- macOS
- Salam 0.7.1 or newer
- An SSH account on a server that allows TCP forwarding
  (`AllowTcpForwarding yes`, the OpenSSH default)
- An administrator password on the Mac, asked on each connect

## Build and run

```sh
./build-app.sh
open "$HOME/Applications/Salam Tunnel.app"
```

`SALAM=/path/to/salam ./build-app.sh /some/folder` picks another compiler or
destination. The script uses the standard library of this repository
(`SALAM_STD` overrides it) and builds the Linux relay with the musl sysroot in
`~/.salam/sysroots` (`SALAM_MUSL_SYSROOT` overrides it).

For a quick run without a bundle, which has no UDP relay:
`salam run . --ui=native --stdlib-path=../../std`.

Enter the server, port, username and password, then press **Connect**. Leave
the password empty to use a key from `~/.ssh`. A key with a passphrase must be
loaded in `ssh-agent`. When connected, the window shows the public IP address,
which should be the server's.

## How it works

| Part                                                  | Runs as | Job                                                                                                                     |
| ----------------------------------------------------- | ------- | ----------------------------------------------------------------------------------------------------------------------- |
| Window (`main.salam`, `engine.salam`, `remote.salam`) | you     | starts `/usr/bin/ssh -N -D` on a loopback port, reconnects it when it drops                                             |
| Helper (`helper.salam`)                               | root    | creates a `utun` device with `std/net/tun`, sends every connection into the SSH SOCKS proxy, points DNS into the tunnel |

The same program is both; macOS asks for permission before the helper starts.
The helper restores the network when you disconnect, when the window process
dies, or when it receives a signal.

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
  no root and opens no port, stops with the connection, and the file stays there
  for next time. Only tick this for a server where you may run programs. It
  needs a Linux x86_64 server. UDP port 443 (QUIC) stays blocked so that
  browsers use TCP, which is faster through SSH.
- Not tunneled: ICMP (`ping` fails), IPv6, fragmented packets, and all UDP when
  the box is not ticked. These are blocked, not leaked. Addresses on your local
  network stay local.

While SSH is reconnecting, traffic is held back instead of going out directly.

Sites see the server's country. Whatever is blocked there, or refuses visitors
from there, will not load while you are connected.

## If the network stays broken

Routes disappear when the helper exits. DNS settings are saved in
`/var/db/salam-tunnel/dns.backup` first, so after a crash or power loss open the
app and press **Restore network settings**. By hand:

```sh
sudo networksetup -setdnsservers Wi-Fi Empty
```

Turn off other VPN apps before connecting.

## Files and security

- `~/.salam-tunnel/` holds the profile and the server's host key. The host key
  is trusted on first use, and a changed key stops the connection.
- The password is stored only if you tick the Keychain box. It reaches `ssh`
  through `SSH_ASKPASS` and never appears on a command line.
- The helper runs the app binary as root, so keep the app somewhere only you can
  write to.
