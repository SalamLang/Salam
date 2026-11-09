# Platform notes for the VPN and proxy stack

What was actually run on which platform, and what was only read. The
networking code is `std/net/internal/rawsock`, `tcp`, `udp`, `transport` and
the proxy packages on top of them; the hardware AES path is `std/cpu` and its
LLVM lowering in `compiler/llvmgen/llvm_hw.salam` (see `HARDWARE_CRYPTO.md`).

## Matrix

| Platform                           | How it was exercised                              | Result                                   |
| ---------------------------------- | ------------------------------------------------- | ---------------------------------------- |
| Linux x86-64                       | CI and local runs                                 | everything passes                        |
| AArch64 Linux (musl, static)       | LLVM backend, `qemu-aarch64 -cpu max`             | see below, all pass                      |
| Windows x86-64 (mingw)             | cross build, run under wine 11 on Linux           | see below, pass after three fixes; one flaky case    |
| macOS arm64 and x86-64             | `salam obj --target=...-apple-darwin`, no run     | objects build; BSD code read, not run    |
| FreeBSD, OpenBSD, NetBSD           | read only                                         | one struct size fixed, rest unverified   |
| Windows i686, 32-bit ARM, RISC-V   | not part of this round                            | not verified                             |

### AArch64 (qemu-user, Crypto Extensions)

The programs were built with the LLVM backend for `aarch64-linux-musl`, linked
with the built-in musl sysroot and `ld.lld`, and run with `qemu-aarch64 -cpu max`
(also `cortex-a35`, `cortex-a53`, `cortex-a710`, `neoverse-n2` and `a64fx` for the
small feature program).

- `cpu.Features()` reports 5 (hardware crypto plus the ARM bit) and
  `HasHardwareCrypto()` is true; `SALAM_NO_HWCRYPTO=1` turns it off.
- `HwAesEnc` and `HwClmul` give the same bytes through `aese`/`aesmc` and
  `pmull` as the software model.
- `aes_hw_gcm` (sizes 0 to 70000, both key sizes, tamper checks), `aes_fast_gcm`,
  `aead_demo` and `tls_selftest` pass. The disassembly of `aes_hw_gcm` holds 48
  `aese`/`aesmc` and 64 `pmull`/`pmull2` instructions.
- Loopback runs under user-mode emulation work: `socks_proxy`,
  `shadowsocks_stream`, `shadowsocks_udp`, `trojan_loopback`, `tunnel_relay`,
  `tunnel_sources`, `tunnel_halfclose`, `transport_loopback`, `vpn_registry`,
  `httpproxy_proxy`, and every `*_ipv6` test (`net_ipv6_basic`, `socks`,
  `trojan`, `tunnel`, `shadowsocks`, `vless_vmess`, `httpproxy`).
- Not covered: a CPU without the extensions. Every qemu CPU model offers them and
  there is no switch to turn them off, so the path where `AT_HWCAP` lacks
  `HWCAP_AES` or `HWCAP_PMULL` (software fallback) was not run on AArch64. It is
  the same code path as `SALAM_NO_HWCRYPTO=1`, which was run.
- Tests that read `tests/fixtures/` must run from the repository root.

### Windows (wine 11, x86_64-w64-windows-gnu)

Built with the LLVM backend and the mingw sysroot, run with
`WINEDEBUG=-all`, from the repository root. IPv6 loopback works under wine on a
host that has `::1`.

Passing: `net_ipv6_basic`, `socks_proxy`, `socks_ipv6`, `socks_tunnel`,
`socks_udp_source`, `httpproxy_proxy`, `httpproxy_ipv6`, `trojan_loopback`,
`trojan_ipv6`, `shadowsocks_stream`, `shadowsocks_udp`, `shadowsocks_ipv6`,
`tunnel_relay`, `tunnel_sources`, `tunnel_ipv6`, `tunnel_halfclose`,
`vpn_registry`, `transport_loopback`, `transport_accept`, `transport_close_twice`,
`vless_vmess_ipv6`, `vpn_over_stream`, `net_mux`, `tunnel_tls_duplex`,
`net_udp`, `net_udp_extra`, `net_loopback`, `tcp_structs`, and the other proxy
package tests (tunnel, shadowsocks, vmess, trojan, transport, socks, doh, nats).
`examples/vpn/selftest.salam` passes in most runs; its shadowsocks case fails
in roughly a third of the runs under wine, with and without hardware AES and
with main's `std/tls`, so it is a flake of the Windows (or wine) socket path and
not of the AES work. It is not explained yet. AES-NI (the x86 half
of the lowering) also runs there: `aes_hw_gcm`, `aes_fast_gcm`, `aead_demo` and
`tls_selftest` pass under wine.

Bugs found by this run and fixed:

- `rawsock._connected` called `getpeername` with an `int` and broke every
  Windows build (E012, `SOCKET` is `usize`). It is only used by the Linux
  connect race, so it now lives inside that block.
- `LocalPort` passed a 16 byte buffer to `getsockname`. Linux truncates, Winsock
  fails with `WSAEFAULT`, so every IPv6 listener reported port 0 and every
  connect to it was refused. It now uses a 128 byte `sockaddr_storage`.
- The transport handshake watchdog wakes a blocked read with `shutdown`. On
  Windows that does not interrupt a `recv` in another thread (seen under wine),
  so a stalled client held its handshake thread forever. The accepted socket
  now also gets a receive timeout slightly longer than the handshake limit on
  Windows, cleared after the handshake.

Checked by reading: every extern used by `rawsock` and the poller on Windows
(`socket`, `connect`, `bind`, `listen`, `accept`, `send`, `recv`, `sendto`,
`recvfrom`, `closesocket`, `shutdown`, `setsockopt`, `getsockname`,
`getpeername`, `ioctlsocket`, `getaddrinfo`, `freeaddrinfo`, `gethostbyname`,
`WSAPoll`, `WSAStartup`, `htons`, `ntohs`, `inet_ntoa`) is in the stdcall list
in `compiler/sal_core.salam` and `compiler/codegen/codegen_header.salam`. No
`inet_pton` or `WSAGetLastError` is used by the library.

Not verified: real Windows (wine is a reimplementation: `WSAPoll`, shutdown
and UDP connection reset behaviour can differ), i686 Windows, Windows on Arm.

### macOS (not run)

`salam obj` builds the socks proxy for `aarch64-apple-darwin` and
`x86_64-apple-darwin` with the LLVM backend. That found one real error:
`_sa_family` ended a block with `ret` and then more code, which the
condition-folding made unreachable (E070) on every macOS and BSD build.

Checked against the Darwin headers from memory:

| Item                         | Code                             | Darwin                              |
| ---------------------------- | -------------------------------- | ----------------------------------- |
| `AF_INET6`                   | 30                               | 30                                  |
| `IPV6_V6ONLY`, `IPPROTO_IPV6`| 27, 41                           | 27, 41                              |
| `sockaddr_in`/`in6`          | length byte, then family         | `sin_len`, `sin_family`; 16 / 28    |
| `sin6_scope_id` offset       | 24                               | 24                                  |
| `SOL_SOCKET`, `SO_REUSEADDR` | 65535, 4                         | 0xffff, 4                           |
| `SO_NOSIGPIPE`               | 4130 on every new socket         | 0x1022; no `MSG_NOSIGNAL`           |
| `SO_RCVTIMEO`, `SO_SNDTIMEO` | 4102, 4101                       | 0x1006, 0x1005                      |
| `SO_KEEPALIVE`, `SO_BROADCAST` | 8, 32                          | 8, 32                               |
| `O_NONBLOCK`, `F_GETFL/SETFL`| 4, 3, 4                          | 4, 3, 4                             |
| `addrinfo` layout            | canonname before addr            | `ai_canonname` before `ai_addr`     |
| `IP_ADD_MEMBERSHIP`          | 12 / 13                          | 12 / 13                             |
| `kevent`                     | 32 bytes, filter -1/-2, add 1    | 32 bytes on 64-bit, `EV_ADD` 1      |

`fcntl` is declared variadic, which the LLVM backend passes through as a
variadic call; that matters on arm64 Darwin where variadic arguments go on the
stack.

Unverified without a Mac: that the objects link and run, `kqueue` behaviour,
`SO_NOSIGPIPE` on accepted sockets, `getaddrinfo` with `AI_ADDRCONFIG` style
defaults, and the Network framework quirks (local network permission prompts
for non-loopback addresses on recent macOS).

### BSD

`poller` read a `kevent` as 32 bytes. FreeBSD (12 and later) has four extra
`ext` words, 64 bytes, so the stride and the change record are now 64 on
FreeBSD. NetBSD's `kevent` has a different layout (32 bit filter, 40 bytes) and
is still wrong; OpenBSD is 32 bytes. `MSG_NOSIGNAL` values (FreeBSD 0x20000,
OpenBSD and NetBSD 0x400) and the `AF_INET6` numbers (28, 24, 24) match the
headers. None of this was run.
