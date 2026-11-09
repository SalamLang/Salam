# VPN and proxy interoperability

This page records what the `std/net` proxy packages (`vless`, `vmess`, `trojan`, `shadowsocks`, `socks`, `httpproxy`, `transport`) were tested against: real third party software on loopback, not our own vectors. Everything here is reproducible with `tools/interop/run.sh`.

## Software used

| Program                      | Version        | Source and check                                                                                   |
| ---------------------------- | -------------- | -------------------------------------------------------------------------------------------------- |
| Xray-core                    | 26.11.9        | XTLS/Xray-core release, SHA2-256 from the published `.dgst`                                        |
| sing-box                     | 1.14.4         | SagerNet/sing-box release, SHA-256 from the GitHub release digest                                  |
| shadowsocks-rust             | 1.25.0         | `ssserver` and `sslocal`, SHA-256 from the published `.sha256`                                     |
| Xray-core (legacy)           | 26.7.28        | XTLS/Xray-core release, SHA2-256 from the published `.dgst`; the last release before REALITY dropped hellos without X25519MLKEM768 (only for `chrome_120`) |
| simple-obfs                  | master 486bebd | built from source with libev 4.33 (SHA-256 507eb7b8d1015fbec5b935f34ebed15bf346bed04a11ab82b8eee848c4205aea); `obfs-server` and `obfs-local` |
| mihomo                       | 1.19.33        | MetaCubeX/mihomo release, SHA-256 from the GitHub release digest (only for simple-obfs, see below) |
| Loyalsoldier v2ray-rules-dat | 202611091318   | `geoip.dat`, `geosite.dat`, SHA-256 from the published `.sha256sum` (geo reader check)             |
| Chocolate4U Iran-v2ray-rules | 202611091318   | `geoip.dat`, `geosite.dat` with the `ir` and `category-ir` lists (geo reader check)                |
| v2fly domain-list-community  | 20261108011013 | `dlc.dat` (geo reader check)                                                                       |
| Go                           | 1.26.0         | `crypto/mlkem` and `crypto/tls`, only to produce ML-KEM vectors and to read the peers' source      |
| cURL                         | 8.18.0         | system                                                                                             |
| OpenSSL                      | 3.5.5          | system                                                                                             |
| Python                       | 3.14.4         | harness, echo servers, garbage senders                                                             |

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
4. **simple-obfs tls server reply** (commit "shadowsocks obfs: tls server reply carries the first payload in the handshake-typed record"). The real server sends ServerHello, ChangeCipherSpec, then the first payload inside a record of type 0x16; we sent an extra 37 byte fake handshake record, which a real client reads as payload and fails to decrypt. The client-side now also accepts the real server shape. Verified against the mihomo client and the simple-obfs source; a real `obfs-server` binary was not available (needs libev) so our client-side against a real obfs server is untested.

## Second round (gRPC, Vision, Mux.Cool, Reality, fragmenting, links, geo files, DoH)

Same method as the first round: the real programs on loopback, 1 MiB of random data through the proxy to an echo server, compared byte for byte, in both directions (our client to the real server, the real client to our server). Versions are the same as in the table above. The new cases are in `matrix2.py` (transports, Vision, Reality, fragmenting), `mux.py`, `ss2022.py`, `trojan_udp.py`, `stall.py`, `geo_check.py` and `doh_probe.salam`, and `run.sh` runs them.

Where the payload is HTTPS ("inner TLS" below) the echo server is a TLS 1.3 server and the test client does a TLS handshake through the proxy first. That is the case Vision is made for, and the one that failed (see the bugs).

| Case                                                                                                 | xray                                 | sing-box                             | Notes                                                                                                     |
| ---------------------------------------------------------------------------------------------------- | ------------------------------------ | ------------------------------------ | --------------------------------------------------------------------------------------------------------- |
| VLESS gRPC gun, plain (h2c) and TLS                                                                  | both ways pass                       | both ways pass                       |                                                                                                           |
| VLESS gRPC multi, plain and TLS                                                                      | both ways pass                       | not available                        | sing-box has no multi mode                                                                                |
| VMess gRPC gun + TLS                                                                                 | both ways pass                       | both ways pass                       |                                                                                                           |
| VLESS xtls-rprx-vision over TLS                                                                      | both ways pass                       | both ways pass                       |                                                                                                           |
| VLESS vision over TLS, inner TLS 1.3 payload (the peer sends `direct`)                               | both ways pass after the fix         | both ways pass after the fix         | our writer keeps sending padded frames and then plain data, our reader follows the peer to the raw socket |
| VLESS vision over Reality, plain and inner TLS payload                                               | both ways pass                       | both ways pass                       |                                                                                                           |
| VLESS over Reality, empty flow, plain and inner TLS payload                                          | both ways pass                       | both ways pass                       |                                                                                                           |
| VLESS over Reality + gRPC                                                                            | both ways pass                       | both ways pass                       |                                                                                                           |
| Reality client fingerprints against an Xray server: chrome, edge, firefox, safari, `ios`, randomized | authenticate, 1 MiB passes           | see round three                      | after the fix below; `chrome_120` does not (see Reality notes)                                            |
| Reality destination that picks X25519MLKEM768, and one that does not                                 | pass with both                       | pass with both                       | the first one failed before the fix                                                                       |
| ClientHello fragmenting (tcp default, 1 byte pieces, TLS records, both + SNI cut)                    | our client to the real server passes | our client to the real server passes | checked on the wire that the hello really arrives in pieces                                               |
| Mux.Cool, VLESS and VMess, 4 concurrent 1 MiB sessions per connection                                | both ways pass                       | not applicable                       | sing-box multiplexes with smux, which is a different protocol                                             |
| Shadowsocks 2022 UDP, three methods                                                                  | both ways pass                       | both ways pass                       | shadowsocks-rust 1.25.0 also both ways pass                                                               |
| Shadowsocks 2022 multi-user (iPSK:uPSK), aes-128 and aes-256, TCP and UDP                            | both ways pass                       | both ways pass                       | shadowsocks-rust also both ways pass; chacha20 has no multi-user form                                     |
| Trojan UDP                                                                                           | both ways pass                       | both ways pass                       |                                                                                                           |
| Round one matrix (`matrix.py`, `ss_extra.py`, ...) rerun on current main                             | unchanged                            | unchanged                            |                                                                                                           |

Reality notes:

- Xray drops a hello that has no X25519MLKEM768 key share before it looks at authentication (`REALITY: processed invalid connection ... authentication failed`). The `chrome_120` profile has none, so it cannot authenticate against Xray 26.x and the matrix records that as an expected failure. Every other profile now sends the share.
- Our own Reality server answers with a plain x25519 key share. Xray, sing-box and our client all accept that.
- A Reality client must also finish the handshake when the destination site picks the hybrid group. That needs real ML-KEM-768 (the bug below).

Link import (`tunnel.ParseLink`, `vless.ParseLink`, `vmess.ParseLink`) was fed the shapes v2rayN, Xray and sing-box produce: VLESS Reality with a percent encoded Persian remark and `spx=%2F`, IPv6 hosts in brackets, gRPC with `serviceName`, `mode=multi` and `authority`, ws with `path=%2Fws%3Fed%3D2048` and `allowInsecure=1`, httpupgrade, VMess JSON with a numeric or string port, with padded and unpadded base64 and with `net=grpc` and `type=multi`, Trojan with a percent encoded password, ws and gRPC, Shadowsocks SIP002 with a plugin, legacy base64 with an IPv6 host, and a 2022 method with a percent encoded key. Fields were compared by eye against the conventions; the two mistakes are in the bug list.

Geo files: `geo_check.py` reads the real `geoip.dat` and `geosite.dat` of Loyalsoldier and Chocolate4U (Iran) and `dlc.dat` of v2fly with our reader, and with a small protobuf reader in Python, and compares the number of entries of 15 categories by kind (full, suffix, regular expression; IPv4 and IPv6 networks). All 15 agree. Loading every category takes 0.28 s and 58 MB for the 11 MB `geosite.dat` (1562 categories) and 1.6 s and 56 MB for the 16 MB `geoip.dat` (260 categories).

DoH (`doh_probe.salam`, run from a normal network on 2026-11-09) against Cloudflare, Google and Quad9 over DoH, and the same three over DoT: A, AAAA, TXT, a CNAME chain, a NODATA answer and the cache all behave, and the answers agree between the servers. Quad9 over DoH failed before the fix below.

### Bugs found and fixed in round two

1. **Vision `direct` command** (commit "vless vision: honour the direct command and finish a frame at the end of the data"). When the inner traffic is TLS 1.3, Xray and sing-box send a frame with the `direct` command and then switch to the raw TCP socket, without the outer TLS. Our reader kept decrypting garbage, so every HTTPS session through a Vision server hung or died; the earlier tests used a plain echo payload, where Vision never goes direct. A frame that ended exactly at the end of a record was also only closed on the next read, which never comes after `direct`. Now `std/tls` can hand the rest of the stream to a raw reader (`tls.SwitchDirect`, bytes already read ahead are delivered first) and the Vision reader does it on `direct`. The server-side needed one more step because the `vpn` server sees the connection as a `tunnel.Stream`, hence `tunnel.DeadlineDirect`. Our writer never sends `direct`; it keeps the padded frames and then the plain data inside TLS, which the peers accept.
2. **Reality with a post quantum destination** (commit "utls, reality: a real X25519MLKEM768 key share ..."). The hybrid key share was a random ML-KEM key with no private part. Against a destination that picks X25519MLKEM768 (Google, Cloudflare, anything on OpenSSL 3.5) the handshake could not finish. `std/tls` now has SHA-3, SHAKE and ML-KEM-768 (commit "tls: sha3, shake and ml-kem-768"), checked against Python `hashlib` and Go `crypto/mlkem` (key generation from a seed, decapsulation of a foreign ciphertext, encapsulation read by Go, the implicit rejection key). The firefox and safari profiles had no hybrid share at all and so never authenticated with Xray; they send it now.
3. **Link import dropped the gRPC mode** (commit "links: keep the gRPC multi mode and decode reality query values"). `mode=multi` on VLESS and Trojan links and `type=multi` in VMess links were ignored, so a client imported from a link dialled gun against a multi server. `vless.ParseLink` also left `spx` and `alpn` of Reality links percent encoded.
4. **DoH over HTTP/2** (commit "doh: fall back to HTTP/2 when the server answers 505"). Quad9's DoH endpoint only speaks HTTP/2 and answers HTTP/1.1 with 505, so the quad9 preset resolved nothing. The client retries over h2 and remembers it.
5. **Pre-auth connections** (see the next section).
6. **Address space leak per connection** (commit "tunnel: per connection threads are detached", found by rerunning `fuzz.py` after the change above). The threads that run a handshake, a Trojan connection, a gRPC reader and a Mux reader were started with `spawn` and never joined, so each kept its 8 MB stack: 16 MB of address space per accepted connection (`VmSize` 0.1 GB, then 2.1 GB after 100 connections, 3.7 GB after 200). Under the 4 GB `ulimit -v` of the harness the server stopped accepting after about 220 connections, and the file descriptors of the stuck ones never came back; without the limit the same leak ends at the mapping limit of the kernel. They run through `tunnel.Go` (a detached thread) now; `VmSize` stays flat over 500 connections. `fuzz.py` has a check for it.
7. **Double close of a connection** (commit "transport: closing a connection twice is a no-op", found by `stall.py` once the accept path ran in its own thread). A failed inbound handshake closed the stream and the accept server closed it again. The second close hit the file descriptor number, which the accept thread had already given to the next client, so a silent client that connected right after a failed one (the probe a harness uses to wait for the port) was closed at once, in about half of the runs. The library is idempotent now, with a test that fails without the change.

Also found, and left alone: an outbound `streamSettings.fragment` in a config file is ignored on the client-side; fragmenting is switched on with `vpn.Options` (the `client -f` flag, or `tools/interop/vpn_frag`).

## Third round (UDP, Mux.Cool over TLS and Reality, fingerprints, simple-obfs, trojan transports)

Same method again. New or extended scripts: `udp.py` with the program `udp_peer`, `mux.py` (now over five transports), `obfs.py`, `matrix2.py` (fingerprints in both directions against Xray and sing-box), `matrix.py` (Trojan over more transports) and `fuzz.py` (more listeners, 500 sessions).

| Case | xray 26.11.9 | sing-box 1.14.4 | Notes |
| ---- | ------------ | --------------- | ----- |
| Mux.Cool, VLESS and VMess, 4 x 1 MiB, over tcp, TLS, ws+TLS, Reality, gRPC+TLS | both ways pass (20 cases) | not applicable | sing-box has no Mux.Cool |
| VLESS UDP (command 2), plain, TLS, ws+TLS, Reality, gRPC+TLS | ours to xray passes up to 8000 bytes; xray client to ours up to 4000 | both ways pass up to 16000 bytes | Xray itself loses datagrams over 4000 (client) and 8000 (server) in this mode, a run of Xray against Xray shows the same limit |
| VMess UDP (command 2), the same five transports | the same limits as VLESS | both ways pass up to 9000 bytes | sing-box VMess takes less than its VLESS |
| UDP inside Mux.Cool (VLESS XUDP and VMess), the same five transports | ours to xray up to 8192, xray client (with `mux.enabled`) to ours up to 8000 | VLESS only: ours to sing-box and sing-box with `packet_encoding: xudp` to ours, up to 16000 | sing-box has the XUDP form of VLESS Mux only |
| UDP to port 53 with DNS query packets and a redirect to the echo | both ways pass | both ways pass | Xray `freedom.redirect`, sing-box `route-options override_port`; our peer redirects itself |
| VLESS Reality fingerprints chrome, firefox, safari, edge, ios, randomized | both ways pass | both ways pass |
| `chrome_120` against Xray 26.7.28 | passes (needs the client version fix below) | not available (sing-box does not know the name) | against 26.11.9 it is refused, as expected; kept as a legacy profile, see below |
| Trojan over ws, ws+TLS, httpupgrade+TLS, gRPC+TLS (and tcp+TLS) | both ways pass after the fixes below | both ways pass | plain `trojan-go` style websocket is the same wire format as Xray's |
| Shadowsocks simple-obfs http and tls, real `obfs-server` with `ssserver` and `sslocal` with real `obfs-local` | our client to obfs-server + ssserver passes; sslocal + obfs-local to our server passes | | 1 MiB each, four cases |
| Link import shapes | | | expected values in `tests/{en,fa}/stdlib/tunnel_link_shapes` |

Datagram sizes tested: 1, 29, 512, 1200, 4000, 8000, 8192, 9000, 16000, 32000, 65000 bytes, two echoes per size on one session. A UDP session inside Mux.Cool carries at most 8192 bytes (`transport.MuxMaxPacket`): Xray closes the whole Mux connection on a larger packet, so a larger write is refused locally. The 65000 byte datagram did not pass anywhere.

### chrome_120

The profile has no hybrid X25519MLKEM768 key share. REALITY refuses such a hello since 2026-09-08 (REALITY commit 8cdf7bf9, in Xray-core 26.9.9 and later), and the Xray 26.7.28 test above shows that before that date it authenticates. Decision: it stays as an explicit legacy profile for older servers. `utls.ProfileNames()` and with it the documented default list no longer name it (`utls.LegacyProfileNames()` does), `randomized` is built on `chrome` and so has the share, and the profile table of the `net.utls` page says which servers refuse it.

### Bugs found and fixed in round three

1. **Reality client version** (`reality: send client version 26.3.27 ...`). The sealed proof carried the version 1.8.1. Xray 26.3.27 to 26.10 have a default `minClientVer` of 26.3.27 and refuse older clients ("authentication failed"), so every Salam Reality client (and sing-box, which sends the same) failed against those servers; 26.11.9 dropped the default, which hid it. We now send 26.3.27; the server side reports it (`Accepted.client_version`).
2. **Trojan on ws+TLS, httpupgrade and gRPC** (`vpn: trojan over ws with TLS ...`). The `vpn` trojan server only knew tcp+TLS and plain ws; everything else fell back to plain tcp, so real clients got a reset. Those transports now use the generic transport listener (TCP only; UDP and the fallback site stay on the tcp+TLS and plain ws paths).
3. **Trojan first payload with a NUL** (`trojan: the first payload after the request is kept as bytes ...`). The generic path carried the bytes after the request header in a string, which stops at the first NUL; a real client (Xray sends header and data together) lost the rest of the first packet and the session stalled at 1 MiB. `tunnel.Session` has `initial_data` and `initial_size` now.
4. **Mux UDP oversize** (`transport: a Mux UDP session refuses a datagram over 8192 bytes ...`). Writes over 8192 bytes in a UDP session were split into 16384 byte frames; Xray treats that as a protocol error and drops the whole multiplexed connection, including its TCP sessions.
5. **Shadowsocks plugin in `vpn`** (`vpn: shadowsocks plugin obfs-local/obfs-server ...`). The dialer ignored the `plugin` of an outbound and the shadowsocks inbound had no plugin setting, so simple-obfs worked in the library but not in the program.

### Not changed, found

- **File descriptors left in CLOSE-WAIT.** Opening 100 idle connections at once (half to a mixed listener, half to a VLESS one), then closing them, intermittently leaves between 1 and 36 accepted sockets open on the server (CLOSE-WAIT, no thread owns them; thread dumps show only idle workers). It does not reproduce under `strace`, with a single listener, or with 50 connections, and a second run may leave none. It is also present in the build from the start of this round. The descriptors are never reused. Reproduce: `fuzz.py` stall check, or 100 simultaneous idle connections to two listeners. Not fixed.
- **Growth over 500 sessions.** 500 sessions through the mixed listener, 20 at a time, twice in a row (`fuzz.py`): all pass, descriptors stay at 14 (the baseline). First batch: threads 40 to 64, resident memory 8.0 to 10.7 MB, virtual size +0.9 GB (pool threads with 8 MB stacks). Second batch: threads 64 to 76, resident memory 10.7 to 12.7 MB, virtual size +0.1 GB. The thread growth is the elastic pool filling up (the number levels off), the resident growth of about 4 KB per session matches the known closure environment leak of the accept path (about 2 KB per connection under valgrind in round one); it was not chased further.
- **Shadowsocks v2ray-plugin** is not implemented in our client or server (only simple-obfs), so no v2ray-plugin cases were run; use VLESS or VMess over ws for that transport.
- **Datagram sources.** An incoming UDP session in Mux.Cool answers from the requested target only: per-packet source addresses (full cone) and XUDP global ids are not implemented.

## Pre-auth connections (second look)

Measured with `stall.py` (silent connections to a `vpn run` server: plain tcp, ws, TLS for VLESS; TLS and ws for Trojan):

| Before                                                                                                                                                        | After                                                                                              |
| ------------------------------------------------------------------------------------------------------------------------------------------------------------- | -------------------------------------------------------------------------------------------------- |
| plain tcp: closed after 15 s (the handshake deadline)                                                                                                         | closed after 8 s                                                                                   |
| VLESS over plain ws: **never closed**; the read of the upgrade request had no deadline                                                                        | closed after 10 s (`handshake_timeout_ms`)                                                         |
| VLESS over TLS: the TLS handshake ran on the accept thread, so every silent connection stopped the listener for 15 s; three of them made a valid session fail | the handshake runs in its own worker with a 10 s limit; valid sessions are served at once          |
| Trojan over TLS: closed after 15 s (the TLS read limit of `std/tls`)                                                                                          | closed after 10 s (the TLS handshake read limit is now 10 s)                                       |
| Trojan over ws: the upgrade ran on the accept thread, one silent connection blocked the listener for good                                                     | the upgrade runs in the connection's thread with the handshake deadline (5 s)                      |
| no limit per source address                                                                                                                                   | at most 32 connections per address in their handshake, at both stages; the rest are closed at once |

`std/net/vpn` now takes every inbound through `transport.Accept` (the worker, the 128 handshake slots and the time limit) and sets `PENDING_PER_SOURCE` (32) on the accept server, the transport listener and the Trojan server. `tunnel.AcceptServer` and `tunnel.TcpInbound` have a `per_source` field (default 0, no limit, for library users) and an 8 second handshake default. The count is of handshakes in progress, not of open sessions, so a client with many parallel sessions is not limited. The source address comes from `getpeername`; on Windows there is no such call wired in yet, so the per source limit does nothing there (the deadlines still apply).

`fuzz.py` (35 kinds of bad input on 8 listeners, then 100 stalled connections) passes again with the new numbers: file descriptors back to the baseline of 12, resident memory 6.5 MB after the run, a valid session served while 100 connections stall, and all 20 silent connections of the last check closed after 8 s (15 s before).

Limits that remain: the deadlines are per read, so a client that sends one byte every few seconds keeps its handshake open for several reads; and several clients behind one address share the 32. The valgrind note about about 2 KB leaked per accepted connection (the closure environment) was not looked at again.

## Rerunning round two

```
tools/interop/run.sh /path/to/salam /path/to/bins /path/to/scratch
INTEROP_GEO_DIR=/path/to/geo tools/interop/run.sh ...
```

`bins` also needs Python 3 and `openssl`; the geo directory holds `ls/`, `ir/` and `v2/` with the downloaded `.dat` files. `matrix2.py` takes a substring filter like `matrix.py`, and `INTEROP_DEST_CURVE=` (empty) makes the Reality destination offer the post quantum group while `INTEROP_DEST_CURVE=X25519` limits it to x25519.

## Not covered or not possible here

- simple-obfs was run against the real `obfs-server` and `obfs-local` in round three (the mihomo run of round one stays).
- The `vpn` program does not serve VLESS or VMess UDP or Mux.Cool itself: those were run through `udp_peer` and `mux_peer`, small programs on the library. `vpn` forwards TCP only on those protocols.
- UDP: the 65000 byte datagram passed nowhere; the limits per peer are in the third round table.
- Reality: our Reality server never offers a hybrid share of its own; `chrome_120` is a legacy profile.
- Vision: our writer never sends the `direct` command, and a real Xray client does not send it on the uplink either, so the `direct` reader was exercised on the downlink of Xray and on both directions of sing-box.
- A `fragment` object inside an outbound `streamSettings` is ignored; only `vpn.Options` switches fragmenting on.
- The per source limit reads the peer address with `getpeername`, which is wired up for Linux and macOS (macOS was not run here) and not for Windows. The `vpn` program compiles to objects for `x86_64-windows`, `aarch64-macos` and `x86_64-macos`; nothing was run there.
- Slow clients: the handshake deadlines are per read, so a client that sends one byte every few seconds keeps its handshake open for several reads.
- The link import shapes now have expected values (`tunnel_link_shapes`); they were compared by eye against the conventions once when frozen.
- mihomo was not run in rounds two and three.
- xray no longer accepts `allowInsecure`; its clients pin the test certificate by SHA-256 instead (`pinnedPeerCertSha256`).
- xray 26.x `freedom` blocks loopback unless `finalRules` allows it; the harness sets that.

## Robustness (garbage, truncation, stalls)

`fuzz.py` starts one `vpn` server with vless, vmess, trojan (TLS), shadowsocks, mixed, socks, http and vless over ws+TLS listeners and sends 35 kinds of bad input to every listener: random data up to 200 KB, zero and 0xff runs, 1 MB and 100 000 line HTTP headers, negative and overflowing `Content-Length`, truncated or oversized SOCKS4/5 fields, wrong or truncated VLESS and VMess headers, TLS garbage, an HTTP/2 preface.

Results: the server stays up, a valid session works afterwards, the open file descriptor count returns to the baseline (12), resident memory grows from 3.6 MB to about 6.3 MB after the whole run and thread count returns to baseline. Silent connections are closed by the server after about 15 seconds (the handshake deadline). That is the round one state; the section "Pre-auth connections (second look)" has the current numbers.

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

`bins` holds `xray`, `sing-box`, `ssserver`, `sslocal` and `mihomo`, and for `obfs.py` `obfs-server` and `obfs-local` (`INTEROP_OBFS_SERVER`, `INTEROP_OBFS_LOCAL`). `udp.py` needs `INTEROP_UDP_PEER` (the `udp_peer` build); `INTEROP_XRAY_LEGACY=1` with `INTEROP_XRAY` set to Xray 26.7.28 makes `matrix2.py` expect `chrome_120` to pass. The compiler must have a `std/` directory next to it that points at this tree. Individual scripts (`matrix.py`, `vmess_options.py`, `ss_extra.py`, `proxies.py`, `fallback.py`, `fuzz.py`) take their inputs from the `INTEROP_*` environment variables listed in `run.sh`; `matrix.py` accepts a substring to select cases, for example `matrix.py vmess/`.
