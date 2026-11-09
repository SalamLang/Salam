# VPN and proxy stack: security review

Scope: `std/net/{tunnel,socks,httpproxy,shadowsocks,vless,vmess,transport,trojan,doh,censor,reality,utls,vpn}`, the additive `std/tls` files, `examples/vpn` and `tools/interop`. This was one reviewer reading the code and fuzzing the parsers; it is not an audit and it is not a guarantee. Areas that were read only partly are listed at the end.

## Threat model

- Remote unauthenticated attacker who can open TCP/UDP connections to a listener (internet scanners, the censor's active probes).
- Authenticated user of a server who tries to reach things the operator did not intend (loopback, private ranges, other users).
- On-path or off-path network attacker against plain DNS and against the client-side.
- A malicious or compromised upstream (DoH server, subscription, rule or geo file) feeding hostile data to a client.
- Out of scope: a local attacker on the client machine, traffic analysis by volume and timing, protocol weaknesses inherited from VMess/Shadowsocks/VLESS specs.

## Findings

| # | Severity | Where | Impact | Status |
|---|----------|-------|--------|--------|
| 1 | High | `vless/vless.salam` `ReadRequest` | Pre-auth heap overflow of about 21 bytes: the buffer was sized 300 (rounded to 512) but a legal header with a 255 byte addon block and a 255 byte domain is 533 bytes. Reproduced under ASAN. | Fixed 3a5d305fa, test 5e1937454 |
| 2 | High | `transport/grpc.salam` `_g_deliver` | `int` is 32-bit; a 5 byte protobuf varint wraps negative or makes `i + ln` overflow, so the bounds check passes and the slice reads far out of range. Reachable before authentication on a gRPC listener (crash or out-of-bounds read). Found by reading, not exploited. | Fixed 747088817 (no test: needs a raw HTTP/2 client) |
| 3 | Medium | `transport/conn.salam` WebSocket reader | Control frame (ping/pong) with a length up to 1 GiB allocated that much before any authentication; a negative 64-bit length also slipped past the cap. | Fixed 856234cea (no test; behavior is a timing/memory difference) |
| 4 | Medium | `shadowsocks/server.salam`, `udp.salam` | Salt was recorded in the replay filter before the first chunk authenticated, so anyone could flood random salts and evict real ones, re-enabling replay of a captured legacy AEAD stream or datagram. Now checked early, recorded only after authentication (this also closes the concurrent-replay race). | Fixed 1426cb9ec |
| 5 | Medium | `httpproxy/httpproxy.salam`, `reader.salam` | A bare LF or CR inside a request value, target, response head or chunk extension was forwarded; upstreams that treat bare LF as a line end see injected headers (request smuggling / header injection). Heads and chunk lines with such bytes are refused. | Fixed c5c4e4687 |
| 6 | Medium | `socks/server.salam` UDP ASSOCIATE | When the client announced 0.0.0.0:0, the relay adopted the first datagram from any host, so a third party could use the association. Now only the TCP peer's address is accepted. Reproduced before the fix. | Fixed (socks commit, test socks_udp_source) |
| 7 | Medium | `doh/client.salam` | A hostile or compromised DoH server (or a non-verifying config) could send `Content-Length: -5`, a wrapping chunk size, or an endless body: negative copy lengths and unbounded memory. Now validated and capped at 1 MiB. | Fixed 839b54c13 |
| 8 | Medium | `tunnel/geo.salam` `_dl_line` | A rule or domain-list line made only of odd whitespace (for example vertical tab) panicked (`Fields` empty, `parts[0]`). Found by the fuzzer. | Fixed 14ffbfffb |
| 9 | Low | `tunnel/geo.salam` `_pb_next` | `pos + l` in i64 could wrap for a huge length in a geosite/geoip .dat. Now compared against the bytes left. | Fixed f9c693124 |
| 10 | Medium | trojan, socks, httpproxy servers | One detached thread per connection and no global cap. `max_per_source` is 0 by default for trojan (the `vpn` runner sets 32). Many sources can exhaust threads. The per-read handshake timeout is idle, not total, so a slow sender can hold a slot. | Open. Proposed: `max_connections` in each server config (default a few thousand), decrement in the connection thread, and an absolute handshake deadline. |
| 11 | Medium | open proxy defaults | `socks`, `httpproxy` and the server-side `vpn` dialer allow loopback and private destinations by default. Routing rules like `geoip:private` do not match hostnames unless a resolver is set, and the later dial resolves again (rebinding). Safe only because listeners default to 127.0.0.1. | Open. Proposed: opt-out `block_private` in `DirectDialer` that checks the addresses it actually connects to. Until then see hardening below. |
| 12 | Low | trojan UDP (`trojan/server.salam`) | UDP associate sends straight from an unbound socket, bypassing the configured dialer/router ACL. | Open |
| 13 | Low | `reality`: `max_time_diff` default 0 | No time check on the ClientHello auth by default, so a captured hello can be replayed to reach the Reality-signed certificate path (distinguishable from the real site). Set it (milliseconds, for example 60000). | Open (config) |
| 14 | Low | VMess | 16-bit chunk counter wraps after 65536 chunks (about 1 GiB per direction), repeating AEAD nonces. This is how the VMess spec defines it. Failed auth closes immediately without draining. | Inherent, documented |
| 15 | Low | `vless Accept` | UUID is compared with `transport.Equal`, probably not constant time. Remote exploitation is impractical but cheap to fix. | Open |
| 16 | Low | `doh` resolver | The response question (qname/qtype) is not compared with the query. TLS-protected DoH/DoT are unaffected; plain UDP resolvers (all Iran presets) accept any datagram with the right id from the right address. | Open |
| 17 | Info | all servers | Authentication by SecretEquals/constant-time compare, crypto/random for salts, IDs and padding, and `allow_insecure=false` defaults were checked and look right. | OK |

## Fuzzing

`tools/fuzz/proxy_parsers.salam`: deterministic xorshift mutation (bit flips, truncation, insertion, duplication) of 14 binary seeds and 14 text seeds. Targets: SOCKS4/5 request, reply, address and UDP header decoders, trojan request/UDP/address, shadowsocks target, tunnel address decoders, VLESS and VMess request decoders, transport address, ClientHello sniff/parse/SNI, DNS message decode, geosite/geoip .dat, and the link, subscription, config, rule, CIDR, URL, percent, UUID, Reality parameter and HTTP head parsers. Run:

    salam build --asan --output=/scratch/fz tools/fuzz/proxy_parsers.salam
    ASAN_OPTIONS=detect_leaks=0 /scratch/fz 100000 SEED

Result under ASAN: after the geo fix, seeds 1 to 3 at 20000 rounds and seeds 11 to 16 at 100000 rounds (about 700000 binary and 700000 text inputs each) ran with no crash or hang. Not covered: stateful stream parsers (VMess/SS chunk readers, HTTP/2 and HPACK in gRPC, TLS record reassembly), which need socket or pipe harnesses.

## Iran DNS presets

Official sites could not be fetched from the review machine (shecan.ir redirect loop, 403.online and radar.game did not resolve, electrotm.org lists no addresses). Values were checked against a Persian tech outlet's comparison table (zoomit.ir, "best DNS for gaming") and community listings. All of them agree with `std/net/doh`:

| Preset | Addresses | Confirmed by |
|--------|-----------|--------------|
| Shecan (free) | 178.22.122.100, 185.51.200.2 | zoomit.ir table, several GitHub tools; third party only |
| 403.online | 10.202.10.202, 10.202.10.102 | zoomit.ir table, one SEO page; private range, works only inside Iran |
| Radar | 10.202.10.10, 10.202.10.11 | zoomit.ir and zoomg.ir (same underlying table); private range |
| Electro | 78.157.42.100, 78.157.42.101 | zoomit.ir table; also answered in the earlier interop round |
| Begzar (added) | 185.55.226.26, 185.55.225.25 | zoomit.ir table only |

Nothing could be confirmed about DoH/DoT hostnames for the Iranian services (one low quality page mentions `https://dns.403.online/dns-query`; it is not in the presets). Cloudflare, Google and Quad9 addresses are the well known public ones. These are plain UDP resolvers: they give no integrity, and in Iran encrypted DNS to the big public resolvers is often blocked, so treat resolver choice as something to test from the target network.

The bundled Iran direct domain list and `IranRouter` (suffix `ir`, `geoip:private` direct, everything else through the proxy) were read; no global service is routed direct. The geosite `ir` data is the bundled list; a full source audit of every domain in it was not done.

## Recommended deployment hardening for operators in Iran

- Bind management and SOCKS/HTTP inbounds to 127.0.0.1. Never expose an unauthenticated mixed inbound.
- On a public server add rules that send `geoip:private` and loopback to `block`, and run it in a network namespace or firewall that denies those ranges from the service user. Do not rely on hostname rules alone (finding 11).
- Probe resistance: use Reality with a real, popular, TLS 1.3 site as `dest` that is reachable from Iran, set `max_time_diff`, and keep `shortIds` secret and unique per user. For Trojan, set a real fallback site on the same host that answers like an ordinary site, with a valid certificate for the SNI you serve. For VLESS/VMess/Shadowsocks, expect a plain close on failure (Shadowsocks drains with random delay).
- Certificates: use a real certificate whose name matches the SNI, renew before expiry, and do not set `allow_insecure` on clients.
- Ports: 443 blends in best; other ports on foreign hosts are cheaper to block. Avoid well-known proxy ports.
- CDN fronting (WebSocket/gRPC through a CDN) hides the server IP but ties you to the CDN's availability and policy, adds a hop that sees metadata, and some CDN ranges are throttled from Iran. It is not a substitute for the points above.
- Keep a connection limit at the firewall (per source) until finding 10 is closed.
- Clock: VMess, Reality and SS2022 need a correct clock; run NTP on servers.
- This stack has been checked against xray, sing-box and shadowsocks-rust in the interop rounds, but has had one independent review only. Treat it as unproven against a well-resourced censor.

## Not reviewed in depth

utls ClientHello builder, Reality certificate construction, Vision padding, mux, `std/tls` record reassembly and the additive crypto files (the r3/aes branch is rewriting AES), IPv6 paths (IPv6 branch), HPACK, `shadowsocks` obfs, `vpn.salam` JSON option parsing, the full geosite `ir` list.
