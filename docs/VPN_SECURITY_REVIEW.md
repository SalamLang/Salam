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
| 10 | Medium | trojan, socks, httpproxy servers | One detached thread per connection and no global cap; the per-read handshake timeout was idle, not total, so a slow sender could hold a slot. | Fixed 80f26c003, 296c8d701, b0bd16300: `max_connections` (default 1024, socks closes at once, httpproxy answers 503, trojan closes), `max_per_source` for pending handshakes (default 32), and a total `handshake_deadline_ms` (default 10000) enforced by a watcher that shuts the socket down. Tests proxy_limits, tunnel_deadline. The VPN runner and tunnel acceptors keep their own pool and per source limit; the watcher helper is public (`tunnel.WatchHandshake`) but is not wired into the transport accept path |
| 11 | Medium | open proxy defaults | `socks`, `httpproxy` and the server-side `vpn` dialer allowed loopback and private destinations; hostname rules did not match and the later dial resolved again (rebinding). | Fixed 8c2d3a72d, 296c8d701, b0bd16300: every destination is checked against `tunnel.DestPolicy` (loopback, private, CGNAT, link local and metadata, ULA, multicast, broadcast, unspecified, IPv4-mapped, NAT64 and 6to4 forms) unless `allow_private` is set AND the listener is bound to loopback. Names are resolved once, every answer is checked and the checked address is dialed. ACL lists: allow and deny CIDR, allow and deny domain suffix (deny wins). SOCKS reply 02, HTTP 403. Tests tunnel_policy, socks_policy, httpproxy_policy. Servers given a custom connector or dialer only get name and literal checks unless the dialer is wrapped with `tunnel.Guard` |
| 12 | Low | trojan UDP (`trojan/server.salam`) | UDP associate sent straight from an unbound socket, bypassing the dialer ACL. | Fixed b0bd16300: every UDP target goes through the destination policy (resolved once, checked address used), is dropped when the router picks a block outbound, and `allow_udp` is honoured. UDP still ignores the router for choosing an outbound, but never the ACL |
| 13 | Low | `reality`: `max_time_diff` default 0 | No time check by default. | Fixed 4d7eabd2c: default is now 60000 ms; 0 stays an explicit off (Xray itself defaults to 0). Clients with a clock off by more than a minute are sent to `dest`. VPN config key `maxTimeDiff`. Test reality_time_diff |
| 14 | Low | VMess | 16-bit chunk counter wraps after 65536 chunks (about 1 GiB per direction), repeating AEAD nonces. This is how the VMess spec defines it. Failed auth closes immediately without draining. | Inherent, documented. Test vmess_counter_wrap streams 70016 chunks each way through our own server with both AEAD ciphers |
| 15 | Low | `vless Accept` | UUID compare. | Re-checked: `transport.Equal` has no early exit and `Accept` visits every user, so it was already constant time over the table. VMess auth id matching and the Reality short id list had early exits and were made full scans in 76ad7fd16. Trojan and the other servers already used `SecretEquals` |
| 16 | Low | `doh` resolver | Response question not compared with the query. | Fixed 0f0ca29fe: plain UDP replies must match id, QR bit, one question with the same name (case insensitive) type and class, and come from the server address and port; anything else is skipped until the timeout. Source port is random in 20000 to 59999. The question check also applies to DoT and DoH. Test doh_udp_validate |
| 17 | Info | all servers | Authentication by SecretEquals/constant-time compare, crypto/random for salts, IDs and padding, and `allow_insecure=false` defaults were checked and look right. | OK |

## Fuzzing

`tools/fuzz/proxy_parsers.salam`: deterministic xorshift mutation (bit flips, truncation, insertion, duplication) of 14 binary seeds and 14 text seeds. Targets: SOCKS4/5 request, reply, address and UDP header decoders, trojan request/UDP/address, shadowsocks target, tunnel address decoders, VLESS and VMess request decoders, transport address, ClientHello sniff/parse/SNI, DNS message decode, geosite/geoip .dat, and the link, subscription, config, rule, CIDR, URL, percent, UUID, Reality parameter and HTTP head parsers. Run:

    salam build --asan --output=/scratch/fz tools/fuzz/proxy_parsers.salam
    ASAN_OPTIONS=detect_leaks=0 /scratch/fz 100000 SEED

Result under ASAN: after the geo fix, seeds 1 to 3 at 20000 rounds and seeds 11 to 16 at 100000 rounds (about 700000 binary and 700000 text inputs each) ran with no crash or hang. The fuzzer now also drives stateful stream parsers through an in-memory `tunnel.Stream`: the Shadowsocks server stream reader (`AcceptOver` then chunk reads), the VMess server (`Accept` then chunk reads), the Mux.Cool server frame reader and the HPACK header block decoder, each seeded from a valid captured client stream. Seeds 1 to 3 at 20000 rounds (1000 stream inputs per parser per seed) ran clean under ASAN. Still not covered: the gRPC frame reader (`_g_deliver` is private), HTTP/2 frame reading, and TLS record reassembly, which need a socket harness.

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
- The servers now cap connections (`max_connections`, `max_per_source`) and enforce a total handshake deadline; still keep a per source limit at the firewall for floods that never reach the accept loop.
- Private and loopback destinations are refused by default. Leave `allow_private` off on public servers (it is ignored unless the listener is on loopback) and use `allowCidr`/`denyCidr`/`allowDomain`/`denyDomain` in VPN inbound settings, or `policy` on the Go-style servers, for exceptions.
- Clock: VMess, Reality and SS2022 need a correct clock; run NTP on servers.
- This stack has been checked against xray, sing-box and shadowsocks-rust in the interop rounds, but has had one independent review only. Treat it as unproven against a well-resourced censor.

## Second pass

Read again after the fixes above: Vision padding (frame headers are bounded by 16 bit lengths and the command byte is not trusted beyond continue/end/direct; no problem found), the VLESS and VMess user lookup, Reality short id and time handling (found: the timestamp was assembled in 32 bit `int`, which overflows in 2038; fixed in 76ad7fd16), the DoH UDP path (found: replies were accepted without a question check, fixed), Trojan UDP (fixed). The Mux.Cool reader and the SS and VMess stream readers were fuzzed. Not yet done: a line by line read of the utls ClientHello builder, the Reality certificate construction, `std/tls` record reassembly and the new crypto files, HPACK Huffman tables, SS obfs, and the geosite `ir` list; they remain unreviewed beyond tests and interop.

## Defaults that changed

- `socks`, `httpproxy`, `trojan` servers and every VPN inbound refuse loopback, private and special destinations. Programs that proxy to local services must set `allow_private` (and bind to loopback), or add `allowCidr` entries. The VPN client example has `--allow-private`.
- New caps: `max_connections` 1024, `max_per_source` 32 pending handshakes, `handshake_deadline_ms` 10000 on socks, httpproxy and trojan. Trojan `max_per_source` was 0.
- Reality `max_time_diff` is 60000 ms.

## Not reviewed in depth

utls ClientHello builder, Reality certificate construction, Vision padding, mux, `std/tls` record reassembly and the additive crypto files (the r3/aes branch is rewriting AES), IPv6 paths (IPv6 branch), HPACK, `shadowsocks` obfs, `vpn.salam` JSON option parsing, the full geosite `ir` list.
