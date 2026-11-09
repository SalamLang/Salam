# VPN and proxy interoperability

This page records what the `std/net` proxy packages (`vless`, `vmess`, `trojan`, `shadowsocks`, `socks`, `httpproxy`, `transport`) were tested against: real third party software on loopback, not our own vectors. Everything here is reproducible with `tools/interop/run.sh`.

## Software used

| Program          | Version | Source and check                                                                                   |
| ---------------- | ------- | -------------------------------------------------------------------------------------------------- |
| Xray-core        | 26.11.9 | XTLS/Xray-core release, SHA2-256 from the published `.dgst`                                        |
| sing-box         | 1.14.4  | SagerNet/sing-box release, SHA-256 from the GitHub release digest                                  |
| shadowsocks-rust | 1.25.0  | `ssserver` and `sslocal`, SHA-256 from the published `.sha256`                                     |
| mihomo           | 1.19.33 | MetaCubeX/mihomo release, SHA-256 from the GitHub release digest (only for simple-obfs, see below) |
| cURL             | 8.18.0  | system                                                                                             |
| OpenSSL          | 3.5.5   | system                                                                                             |
| Python           | 3.14.4  | harness, echo servers, garbage senders                                                             |

All third party programs run only against 127.0.0.1 with generated configs and a virtual memory cap (`ulimit -v`). Our side is `examples/vpn` (`vpn run -c config.json`) plus two small programs in `tools/interop` (`vmess_client`, `ss_peer`) for options the example does not expose.

Each case sends 1 MiB of random data through the proxy to an echo server and compares it byte for byte (full duplex, so a deadlock fails the case).

## Result matrix

"ours" is the Salam implementation. Each row was run in both directions: our client against the real server, and the real client against our server.

| Protocol and transport                                             | xray                                                    | sing-box                            | shadowsocks-rust | Notes                                                                                                     |
| ------------------------------------------------------------------ | ------------------------------------------------------- | ----------------------------------- | ---------------- | --------------------------------------------------------------------------------------------------------- |
| VLESS tcp                                                          | both ways pass                                          | both ways pass                      |                  |                                                                                                           |
| VLESS tcp + TLS                                                    | both ways pass                                          | both ways pass                      |                  |                                                                                                           |
| VLESS ws                                                           | both ways pass                                          | both ways pass                      |                  |                                                                                                           |
| VLESS ws + TLS                                                     | both ways pass                                          | both ways pass                      |                  |                                                                                                           |
| VLESS httpupgrade                                                  | both ways pass                                          | both ways pass                      |                  | gRPC not implemented                                                                                      |
| VMess AEAD aes-128-gcm: tcp, tcp+TLS, ws, ws+TLS, httpupgrade      | both ways pass                                          | both ways pass                      |                  | after the fixes below                                                                                     |
| VMess AEAD chacha20-poly1305 (tcp)                                 | both ways pass                                          | both ways pass                      |                  |                                                                                                           |
| VMess security none (tcp)                                          | our client rejected by xray, xray client to ours passes | both ways pass                      |                  | Xray 26.x removed `none` and `zero` from its server                                                       |
| VMess zero                                                         | not supported by xray 26.x                              | sing-box server does not support it |                  | ours to ours passes                                                                                       |
| Trojan tcp + TLS                                                   | both ways pass                                          | both ways pass                      |                  |                                                                                                           |
| Trojan fallback                                                    |                                                         |                                     |                  | cURL, OpenSSL and raw TLS probes all get the fallback site                                                |
| Shadowsocks TCP aes-128-gcm, aes-256-gcm, chacha20-ietf-poly1305   | both ways pass                                          | both ways pass                      | both ways pass   |                                                                                                           |
| Shadowsocks 2022 TCP (aes-128-gcm, aes-256-gcm, chacha20-poly1305) | both ways pass                                          | both ways pass                      | both ways pass   | UDP for 2022 is not implemented                                                                           |
| Shadowsocks UDP, the 3 legacy methods                              | both ways pass                                          | both ways pass                      | both ways pass   | our client to real server, and real client (through its SOCKS5 UDP ASSOCIATE) to our UDP server           |
| Shadowsocks simple-obfs http                                       |                                                         |                                     |                  | mihomo client to our server passes                                                                        |
| Shadowsocks simple-obfs tls                                        |                                                         |                                     |                  | mihomo client to our server passes after the fix below                                                    |
| SOCKS5 inbound                                                     |                                                         |                                     |                  | cURL `--socks5`, `--socks5-hostname`, user/pass offer pass                                                |
| SOCKS4 and SOCKS4a inbound                                         |                                                         |                                     |                  | cURL `--socks4`, `--socks4a` pass on the mixed port; the dedicated socks inbound is SOCKS5 only by design |
| HTTP proxy inbound                                                 |                                                         |                                     |                  | cURL `-x` plain GET, CONNECT, keep-alive pass                                                             |
| HTTPS proxy inbound                                                |                                                         |                                     |                  | cURL `--proxy https://` plain and CONNECT pass                                                            |
| SOCKS5 and HTTP outbound                                           | pass                                                    |                                     |                  | our outbound to xray socks and http inbounds                                                              |

VMess option matrix (our client with a chosen option byte, `vmess_options.py`), against our server, xray and sing-box:

| Options                                 | aes-128-gcm              | chacha20-poly1305 |
| --------------------------------------- | ------------------------ | ----------------- |
| chunk stream                            | pass (all three servers) | pass              |
| + chunk mask                            | pass                     | pass              |
| + global padding                        | pass                     | pass              |
| + authenticated length                  | pass                     | pass              |
| + mask + padding + authenticated length | pass                     | pass              |

Real clients: Xray sends chunk stream, mask and padding by default and authenticated length when the user has `"experiments": "AuthenticatedLength"`; both were accepted by our server.

## Bugs found and fixed

1. **VMess response header IVs** (commit "vmess: derive the response header IVs from the response IV"). The response header length and payload IVs were derived from the response key; the spec derives them from the response IV. Every VMess session failed against xray and sing-box in both directions. Only our own vectors, written with the same mistake, passed. `SealResponseHeader`, `OpenResponseLength` and `OpenResponseHeader` now take the response IV too.
2. **VMess security none without chunk stream** (commit "vmess: security none without the chunk stream option is a raw stream"). sing-box sends `none` with option 0; we always framed chunks, so the stream hung.
3. **VMess authenticated length** (commit "vmess: authenticated length uses the request key and IV in both directions"). Both Xray and sing-box derive the length AEAD key (`auth_len`) and nonce from the request key and IV for both directions, not the response ones; and the chacha20 length AEAD uses the chacha key expansion. We used AES-GCM for chacha and the response key for responses.
4. **simple-obfs tls server reply** (commit "shadowsocks obfs: tls server reply carries the first payload in the handshake-typed record"). The real server sends ServerHello, ChangeCipherSpec, then the first payload inside a record of type 0x16; we sent an extra 37 byte fake handshake record, which a real client reads as payload and fails to decrypt. The client side now also accepts the real server shape. Verified against the mihomo client and the simple-obfs source; a real `obfs-server` binary was not available (needs libev) so our client side against a real obfs server is untested.

## Not covered or not possible here

- simple-obfs: no `obfs-local` or `obfs-server` binary was available; only the mihomo client against our server was tested.
- Reality, uTLS, gRPC, XTLS vision, Mux, VLESS and VMess UDP, Trojan UDP, Shadowsocks 2022 UDP: not part of this base or not run.
- xray no longer accepts `allowInsecure`; its clients pin the test certificate by SHA-256 instead (`pinnedPeerCertSha256`).
- xray 26.x `freedom` blocks loopback unless `finalRules` allows it; the harness sets that.

## Robustness (garbage, truncation, stalls)

`fuzz.py` starts one `vpn` server with vless, vmess, trojan (TLS), shadowsocks, mixed, socks, http and vless over ws+TLS listeners and sends 35 kinds of bad input to every listener: random data up to 200 KB, zero and 0xff runs, 1 MB and 100 000 line HTTP headers, negative and overflowing `Content-Length`, truncated or oversized SOCKS4/5 fields, wrong or truncated VLESS and VMess headers, TLS garbage, an HTTP/2 preface.

Results: the server stays up, a valid session works afterwards, the open file descriptor count returns to the baseline (12), resident memory grows from 3.6 MB to about 6.3 MB after the whole run and thread count returns to baseline. Silent connections are closed by the server after about 15 seconds (the handshake deadline).

Finding: every accepted connection occupies a pool thread until its handshake finishes. With the harness `ulimit -v 4000000` about 100 stalled connections already exhaust address space (each thread reserves a large stack) and a new valid session times out until the 15 second deadline closes them; without the cap, 300 stalled connections did not affect service (a valid session completed immediately). An attacker holding 512 idle connections (the pool maximum) can still delay new sessions for up to 15 seconds, repeatedly. A lower handshake deadline for pre-auth connections, or a per-IP limit, would reduce this.

Under valgrind (`--leak-check=full`, definite leaks, same `fuzz.py` run) the server did not crash, produced no invalid read or write reports and also kept file descriptors at baseline. It does leak memory: 276 definite loss records at exit, the largest being about 430 KB in 227 blocks (roughly 1.9 KB per accepted connection) allocated in a compiler generated lambda (`salam_lambda_6`, the per-connection closure submitted to the worker pool, the known closure environment leak), plus small `str.split` results (1 to 90 bytes). A long-running server therefore grows by about 2 KB per connection until the closure leak is fixed in the compiler or the accept loop avoids a capturing closure.

The stalled connection check is not fully explained: in the plain `fuzz.py` run, 100 stalled connections starved a new session for the whole 30 second probe, while the same run under valgrind and a separate script (`stall.py` style, 10 / 100 / 300 stalled connections without prior garbage) served the new session immediately. So the starvation depends on state left by the earlier garbage cases or on the virtual memory cap; reproduce with `INTEROP_OURS=/path/to/vpn python3 -I tools/interop/fuzz.py`.

## DoH and DNS preset check (std/net/doh)

Checked on 2026-11-09 from a normal (non Iranian) network.

| Preset                                                      | Result                                                   |
| ----------------------------------------------------------- | -------------------------------------------------------- |
| Cloudflare 1.1.1.1, 1.0.0.1, `cloudflare-dns.com/dns-query` | alive, valid certificate, HTTP 200                       |
| Google 8.8.8.8, 8.8.4.4, `dns.google/dns-query`             | alive, valid certificate                                 |
| Quad9 9.9.9.9, 149.112.112.112, `dns.quad9.net/dns-query`   | alive, valid certificate                                 |
| Shecan 178.22.122.100                                       | answers; matches the community lists                     |
| Shecan 185.51.200.2                                         | listed by community sources, no answer from outside Iran |
| Electro 78.157.42.100, 78.157.42.101                        | both answer                                              |
| 403.online 10.202.10.202, 10.202.10.102                     | private range, unreachable here; match the Persian lists |
| Radar 10.202.10.10, 10.202.10.11                            | private range, unreachable here; match the Persian lists |

No wrong value was found. The Shecan, 403 and Radar addresses are confirmed only by third party lists (no official page could be fetched), and the 10.202.x.x addresses can only be tested from inside Iran.

## Rerunning

```
tools/interop/run.sh /path/to/salam /path/to/bins /path/to/scratch
```

`bins` holds `xray`, `sing-box`, `ssserver`, `sslocal` and `mihomo`. The compiler must have a `std/` directory next to it that points at this tree. Individual scripts (`matrix.py`, `vmess_options.py`, `ss_extra.py`, `proxies.py`, `fallback.py`, `fuzz.py`) take their inputs from the `INTEROP_*` environment variables listed in `run.sh`; `matrix.py` accepts a substring to select cases, for example `matrix.py vmess/`.
