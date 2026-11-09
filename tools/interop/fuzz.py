import json
import os
import random
import socket
import ssl
import subprocess
import sys
import tempfile
import time

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from harness import Echo, Procs, check_tcp_via_socks, free_port

UUID = "b831381d-6324-4d53-ad4f-8cda48b30811"


def fds(pid):
    return len(os.listdir("/proc/%d/fd" % pid))


def vmsize(pid):
    for line in open("/proc/%d/status" % pid):
        if line.startswith("VmSize"):
            return int(line.split()[1])
    return 0


def rss(pid):
    for line in open("/proc/%d/status" % pid):
        if line.startswith("VmRSS"):
            return int(line.split()[1])
    return 0


def threads(pid):
    return len(os.listdir("/proc/%d/task" % pid))


def garbage_cases():
    r = random.Random(7)
    yield "empty close", b""
    yield "one byte", b"\x00"
    yield "random 64", bytes(r.randrange(256) for _ in range(64))
    yield "random 4096", bytes(r.randrange(256) for _ in range(4096))
    yield "random 200000", bytes(r.randrange(256) for _ in range(200000))
    yield "all zero 70000", b"\x00" * 70000
    yield "all ff 70000", b"\xff" * 70000
    yield "http header 1 MB", b"GET / HTTP/1.1\r\nX: " + b"a" * (1 << 20)
    yield "http 100k headers", b"GET / HTTP/1.1\r\n" + b"X-A: b\r\n" * 100000
    yield "http bad request line", b"GET\r\n\r\n"
    yield "http negative content-length", b"POST http://127.0.0.1:1/ HTTP/1.1\r\nContent-Length: -5\r\n\r\n"
    yield "http huge content-length", b"POST http://127.0.0.1:1/ HTTP/1.1\r\nContent-Length: 99999999999999999999\r\n\r\n"
    yield "http CONNECT no port", b"CONNECT example.com HTTP/1.1\r\n\r\n"
    yield "http CONNECT long host", b"CONNECT " + b"a" * 100000 + b":80 HTTP/1.1\r\n\r\n"
    yield "socks5 truncated greeting", b"\x05\x05"
    yield "socks5 zero methods", b"\x05\x00"
    yield "socks5 request 0xff domain len", b"\x05\x01\x00\x05\x01\x00\x03\xff" + b"a" * 10
    yield "socks5 bad atyp", b"\x05\x01\x00\x05\x01\x00\x09" + b"\x00" * 20
    yield "socks5 unknown command", b"\x05\x01\x00\x05\x09\x00\x01\x7f\x00\x00\x01\x00\x50"
    yield "socks4 truncated", b"\x04\x01\x00\x50\x7f"
    yield "socks4a overlong userid", b"\x04\x01\x00\x50\x00\x00\x00\x01" + b"u" * 70000
    yield "vless short", b"\x00" + b"\x11" * 8
    yield "vless wrong uuid", b"\x00" + b"\x11" * 16 + b"\x00\x01\x00\x50\x01\x7f\x00\x00\x01"
    yield "vless addons len 255", b"\x00" + bytes.fromhex(UUID.replace("-", "")) + b"\xff" + b"\x00" * 10
    yield "vless domain len 255 truncated", b"\x00" + bytes.fromhex(UUID.replace("-", "")) + b"\x00\x01\x00\x50\x02\xff" + b"a" * 5
    yield "vmess 16 bytes then stall", b"\x01" * 16
    yield "vmess 100 random", bytes(r.randrange(256) for _ in range(100))
    yield "vmess 70000 random", bytes(r.randrange(256) for _ in range(70000))
    yield "ss 16 random", bytes(r.randrange(256) for _ in range(16))
    yield "ss 50 random", bytes(r.randrange(256) for _ in range(50))
    yield "ss 65536 random", bytes(r.randrange(256) for _ in range(65536))
    yield "tls client hello garbage", b"\x16\x03\x01\x00\x10" + b"\x01" * 16
    yield "tls record length 65535 only header", b"\x16\x03\x03\xff\xff"
    yield "tls alert", b"\x15\x03\x03\x00\x02\x02\x28"
    yield "http2 preface", b"PRI * HTTP/2.0\r\n\r\nSM\r\n\r\n" + b"\x00" * 100


def hit(port, data, tls=False, linger=1.0):
    try:
        s = socket.create_connection(("127.0.0.1", port), timeout=5)
        if tls:
            ctx = ssl.create_default_context()
            ctx.check_hostname = False
            ctx.verify_mode = ssl.CERT_NONE
            s = ctx.wrap_socket(s)
        s.settimeout(linger)
        try:
            s.sendall(data)
        except OSError:
            pass
        try:
            s.recv(4096)
        except (socket.timeout, OSError, ssl.SSLError):
            pass
        s.close()
    except (OSError, ssl.SSLError):
        pass


def main():
    vpn = os.environ.get("INTEROP_OURS", "vpn")
    work = os.environ.get("INTEROP_WORK") or tempfile.mkdtemp(prefix="interop-")
    cert, key = os.path.join(work, "cert.pem"), os.path.join(work, "key.pem")
    names = ["vless", "vmess", "trojan", "ss", "mixed", "socks", "http", "grpc", "reality"]
    ports = {n: free_port() for n in names}
    tlsport = free_port()
    if not os.path.exists(cert):
        subprocess.check_call(["openssl", "req", "-x509", "-newkey", "rsa:2048", "-nodes", "-keyout", key, "-out", cert, "-days", "30", "-subj", "/CN=localhost", "-addext", "subjectAltName=DNS:localhost,IP:127.0.0.1"], stderr=subprocess.DEVNULL)
    echo = Echo(cert, key, "X25519")
    cfg = {
        "inbounds": [
            {"tag": "vless", "port": ports["vless"], "protocol": "vless", "settings": {"clients": [{"id": UUID}]}},
            {"tag": "vmess", "port": ports["vmess"], "protocol": "vmess", "settings": {"clients": [{"id": UUID}]}},
            {
                "tag": "trojan",
                "port": ports["trojan"],
                "protocol": "trojan",
                "settings": {"clients": [{"password": "pw"}]},
                "streamSettings": {"security": "tls", "tlsSettings": {"certificates": [{"certificateFile": cert, "keyFile": key}]}},
            },
            {"tag": "ss", "port": ports["ss"], "protocol": "shadowsocks", "settings": {"method": "aes-256-gcm", "password": "pw"}},
            {"tag": "mixed", "listen": "127.0.0.1", "port": ports["mixed"], "protocol": "mixed"},
            {"tag": "socks", "listen": "127.0.0.1", "port": ports["socks"], "protocol": "socks"},
            {"tag": "http", "listen": "127.0.0.1", "port": ports["http"], "protocol": "http"},
            {
                "tag": "grpc",
                "port": ports["grpc"],
                "protocol": "vless",
                "settings": {"clients": [{"id": UUID}]},
                "streamSettings": {"network": "grpc", "security": "tls", "grpcSettings": {"serviceName": "gsvc"}, "tlsSettings": {"certificates": [{"certificateFile": cert, "keyFile": key}]}},
            },
            {
                "tag": "reality",
                "port": ports["reality"],
                "protocol": "vless",
                "settings": {"clients": [{"id": UUID}]},
                "streamSettings": {"security": "reality", "realitySettings": {"dest": "127.0.0.1:%d" % echo.tls_port, "serverNames": ["localhost"], "privateKey": "QI3DpbA326sqI7QAAz13iSj6Jk9Q8UUvgkjlKZu2u2I", "shortIds": ["0123abcd"]}},
            },
            {
                "tag": "vlesstls",
                "port": tlsport,
                "protocol": "vless",
                "settings": {"clients": [{"id": UUID}]},
                "streamSettings": {"network": "ws", "security": "tls", "tlsSettings": {"certificates": [{"certificateFile": cert, "keyFile": key}]}},
            },
        ],
        "outbounds": [{"tag": "direct", "protocol": "freedom"}],
        "routing": {"defaultOutbound": "direct", "rules": []},
    }
    cp = os.path.join(work, "fuzz.json")
    json.dump(cfg, open(cp, "w"))
    procs = Procs(work)
    problems = []
    try:
        p = procs.start("server", [vpn, "run", "-c", cp, "--stats", "0"])
        for n in list(ports.values()) + [tlsport]:
            if not procs.wait_port(n):
                print("FAIL server did not start", procs.log("server")[-300:])
                sys.exit(1)
        time.sleep(1)
        base_fd, base_rss, base_threads = fds(p.pid), rss(p.pid), threads(p.pid)
        print("baseline fds=%d rss=%dkB threads=%d" % (base_fd, base_rss, base_threads))
        for name, data in garbage_cases():
            for n in names:
                hit(ports[n], data, tls=(n in ("trojan", "grpc")), linger=0.3)
            hit(tlsport, data, tls=True, linger=0.3)
            if p.poll() is not None:
                problems.append("server died after case: " + name)
                print("FAIL server died after", name, "rc", p.returncode)
                break
        else:
            print("PASS %d garbage cases x %d listeners: server alive" % (len(list(garbage_cases())), len(names) + 1))
            for _ in range(30):
                time.sleep(1)
                if fds(p.pid) <= base_fd + 3:
                    break
            f = fds(p.pid)
            ok = f <= base_fd + 3
            print(("PASS " if ok else "FAIL ") + "fds back to baseline: %d (baseline %d)" % (f, base_fd))
            if not ok:
                problems.append("fd leak %d vs %d" % (f, base_fd))
            ok, d = check_tcp_via_socks(ports["mixed"], echo.port, 100000)
            print(("PASS " if ok else "FAIL ") + "valid socks5 session still works after the garbage: " + d)
            if not ok:
                problems.append("service broken")
            print("rss now %dkB (baseline %dkB), threads %d (baseline %d)" % (rss(p.pid), base_rss, threads(p.pid), base_threads))
            vm0 = vmsize(p.pid)
            import concurrent.futures

            def one(_i):
                ok_, _d = check_tcp_via_socks(ports["mixed"], echo.port, 2000)
                return ok_

            for rnd in (1, 2):
                vm1 = vmsize(p.pid)
                f0, t0_, r0 = fds(p.pid), threads(p.pid), rss(p.pid)
                with concurrent.futures.ThreadPoolExecutor(20) as ex:
                    good = sum(1 for x in ex.map(one, range(500)) if x)
                time.sleep(2)
                print("500 valid sessions, round %d: %d ok; fds %d -> %d, threads %d -> %d, rss %d -> %d kB, vmsize %d -> %d kB" % (rnd, good, f0, fds(p.pid), t0_, threads(p.pid), r0, rss(p.pid), vm1, vmsize(p.pid)))
            if good < 495 or fds(p.pid) > f0 + 3 or vmsize(p.pid) - vm1 > 200000:
                problems.append("500 valid sessions leave growth")
            vm0 = vmsize(p.pid)
            for _ in range(300):
                try:
                    c = socket.create_connection(("127.0.0.1", ports["mixed"]), timeout=5)
                    c.sendall(b"\x05\x01\x00")
                    c.recv(2)
                    c.close()
                except OSError:
                    break
            time.sleep(1)
            grown = vmsize(p.pid) - vm0
            ok = grown < 200000
            print(("PASS " if ok else "FAIL ") + "address space after 300 more connections grew by %dkB" % grown)
            if not ok:
                problems.append("address space grows per connection")

        socks = []
        for i in range(100):
            try:
                socks.append(socket.create_connection(("127.0.0.1", ports["vless"] if i % 2 else ports["mixed"]), timeout=5))
            except OSError:
                break
        print("opened %d idle connections (slowloris)" % len(socks))
        time.sleep(1)
        t0 = time.time()
        ok, d = check_tcp_via_socks(ports["socks"], echo.port, 100000)
        print(("PASS " if ok else "FAIL ") + "service answers during 100 stalled connections (%.1fs): %s" % (time.time() - t0, d))
        if not ok:
            problems.append("service starved by stalled connections")
        trickle = socket.create_connection(("127.0.0.1", ports["http"]), timeout=5)
        for ch in b"GET / HTTP/1.1\r\nX-Slow: ":
            trickle.send(bytes([ch]))
            time.sleep(0.05)
        for s in socks:
            s.close()
        trickle.close()
        for _ in range(45):
            time.sleep(1)
            if fds(p.pid) <= base_fd + 3:
                break
        f = fds(p.pid)
        waited = 0
        ok = f <= base_fd + 3
        print(("PASS " if ok else "FAIL ") + "fds back to baseline after closing stalled connections: %d (baseline %d)" % (f, base_fd))
        if not ok:
            problems.append("fd leak after stall")
        socks = [socket.create_connection(("127.0.0.1", ports[k]), timeout=5) for k in ("vmess", "vless", "ss", "mixed", "http") for _ in range(4)]
        t0 = time.time()
        closed = 0
        for c in socks:
            c.settimeout(max(0.1, 40 - (time.time() - t0)))
            try:
                if c.recv(1) == b"":
                    closed += 1
            except OSError:
                pass
        ok = closed == len(socks)
        print(("PASS " if ok else "FAIL ") + "server closed %d of %d silent connections after %.0fs" % (closed, len(socks), time.time() - t0))
        if not ok:
            problems.append("silent connections never dropped")
        for c in socks:
            c.close()
    finally:
        procs.stop_all()
        echo.close()
    sys.exit(1 if problems else 0)


if __name__ == "__main__":
    main()
