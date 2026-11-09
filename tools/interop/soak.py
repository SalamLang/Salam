import concurrent.futures
import json
import os
import socket
import sys
import tempfile
import time

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import matrix
from harness import Echo, Procs, check_tcp_via_socks, free_port

SESSIONS = int(os.environ.get("SOAK_SESSIONS", "500"))
WORKERS = int(os.environ.get("SOAK_WORKERS", "10"))
ROUNDS = int(os.environ.get("SOAK_ROUNDS", "1"))
SIZE = int(os.environ.get("SOAK_SIZE", "2000"))
WRAP = os.environ.get("SOAK_SERVER_WRAP", "")
PLAIN_CLIENT = os.environ.get("SOAK_CLIENT", "")

CASES = [
    {"name": "vless/tcp", "proto": "vless", "net": "tcp", "tls": False},
    {"name": "vless/ws", "proto": "vless", "net": "ws", "tls": False},
    {"name": "vless/tcp+tls", "proto": "vless", "net": "tcp", "tls": True},
    {"name": "vless/grpc+tls", "proto": "vless", "net": "grpc", "tls": True},
    {"name": "trojan/tcp+tls", "proto": "trojan", "net": "tcp", "tls": True},
    {"name": "vmess/tcp", "proto": "vmess", "net": "tcp", "tls": False, "sec": "aes-128-gcm"},
    {"name": "ss/aes-128-gcm", "proto": "ss", "method": "aes-128-gcm", "password": matrix.PASS},
]


def stat(pid, key):
    for line in open("/proc/%d/status" % pid):
        if line.startswith(key):
            return int(line.split()[1])
    return 0


def snap(pid):
    return {"fds": len(os.listdir("/proc/%d/fd" % pid)), "threads": stat(pid, "Threads"), "rss": stat(pid, "VmRSS"), "vm": stat(pid, "VmSize")}


def direct_socks(port, echo_port):
    return check_tcp_via_socks(port, echo_port, SIZE)[0]


def direct_http(port, echo_port):
    try:
        s = socket.create_connection(("127.0.0.1", port), timeout=20)
        s.sendall(("CONNECT 127.0.0.1:%d HTTP/1.1\r\nHost: 127.0.0.1:%d\r\n\r\n" % (echo_port, echo_port)).encode())
        buf = b""
        while b"\r\n\r\n" not in buf:
            d = s.recv(1024)
            if not d:
                return False
            buf += d
        if b" 200" not in buf.split(b"\r\n")[0]:
            return False
        s.sendall(b"ping")
        got = b""
        while len(got) < 4:
            d = s.recv(4 - len(got))
            if not d:
                return False
            got += d
        s.close()
        return got == b"ping"
    except OSError:
        return False


def drive(label, fn, pid):
    first = None
    for r in range(ROUNDS):
        res = drive_once("%s #%d" % (label, r + 1) if ROUNDS > 1 else label, fn, pid)
        if first is None:
            first = res
    return res[0], first[1], res[2]


def drive_once(label, fn, pid):
    time.sleep(1)
    before = snap(pid)
    t0 = time.time()
    with concurrent.futures.ThreadPoolExecutor(WORKERS) as ex:
        good = sum(1 for x in ex.map(lambda _i: fn(), range(SESSIONS)) if x)
    after = snap(pid)
    for _ in range(30):
        time.sleep(1)
        after = snap(pid)
        if after["fds"] <= before["fds"] + 1:
            break
    print(
        "%-20s %d/%d ok in %.0fs; fds %d -> %d, threads %d -> %d, rss %d -> %d kB, vmsize %d -> %d kB"
        % (label, good, SESSIONS, time.time() - t0, before["fds"], after["fds"], before["threads"], after["threads"], before["rss"], after["rss"], before["vm"], after["vm"]),
        flush=True,
    )
    return good, before, after


def main():
    work0 = os.environ.get("INTEROP_WORK") or tempfile.mkdtemp(prefix="interop-")
    cert, key = matrix.prepare(work0)
    echo = Echo()
    vpn = matrix.BIN["ours"]
    inbounds = []
    ports = {}
    for c in CASES:
        port = free_port()
        ports[c["name"]] = port
        tmp = os.path.join(work0, "soak_in.json")
        matrix.gen_ours_server(c, port, cert, key, tmp)
        ib = json.load(open(tmp))["inbounds"][0]
        ib["tag"] = c["name"]
        inbounds.append(ib)
    mixed, socks, http = free_port(), free_port(), free_port()
    inbounds.append({"tag": "mixed", "listen": "127.0.0.1", "port": mixed, "protocol": "mixed"})
    inbounds.append({"tag": "socks", "listen": "127.0.0.1", "port": socks, "protocol": "socks"})
    inbounds.append({"tag": "http", "listen": "127.0.0.1", "port": http, "protocol": "http"})
    cfg = os.path.join(work0, "soak.json")
    json.dump({"inbounds": inbounds, "outbounds": [{"tag": "direct", "protocol": "freedom"}], "routing": {"defaultOutbound": "direct", "rules": []}}, open(cfg, "w"))
    procs = Procs(work0)
    bad = []
    try:
        argv = (WRAP.split() if WRAP else []) + [vpn, "run", "-c", cfg, "--stats", "0"]
        sp = procs.start("server", argv, mem_kb=int(os.environ.get("SOAK_SERVER_KB", "4000000")))
        for port in list(ports.values()) + [mixed, socks, http]:
            if not procs.wait_port(port, timeout=int(os.environ.get("SOAK_START", "60"))):
                print("server did not start", procs.log("server")[-400:])
                sys.exit(1)
        time.sleep(2)
        base = snap(sp.pid)
        print("server baseline: fds %d, threads %d, rss %d kB, vmsize %d kB" % (base["fds"], base["threads"], base["rss"], base["vm"]), flush=True)
        for c in CASES:
            local = free_port()
            ccfg = os.path.join(work0, "soak_c.json")
            matrix.gen_ours_client(c, ports[c["name"]], local, cert, key, ccfg)
            cp = procs.start("client", [PLAIN_CLIENT or vpn, "run", "-c", ccfg, "--stats", "0"])
            if not procs.wait_port(local, timeout=30):
                print("client for", c["name"], "did not start")
                bad.append(c["name"])
                continue
            good, b, a = drive(c["name"], lambda: check_tcp_via_socks(local, echo.port, SIZE)[0], sp.pid)
            if good < SESSIONS * 0.99 or a["fds"] > b["fds"] + 3:
                bad.append(c["name"])
            cp.terminate()
            cp.wait()
        for label, fn in (("socks5/mixed", lambda: direct_socks(mixed, echo.port)), ("socks5/socks", lambda: direct_socks(socks, echo.port)), ("http/connect", lambda: direct_http(http, echo.port))):
            good, b, a = drive(label, fn, sp.pid)
            if good < SESSIONS * 0.99 or a["fds"] > b["fds"] + 3:
                bad.append(label)
        end = snap(sp.pid)
        print("server end: fds %d, threads %d, rss %d kB, vmsize %d kB" % (end["fds"], end["threads"], end["rss"], end["vm"]), flush=True)
    finally:
        procs.stop_all()
        echo.close()
    print("FAIL: " + ", ".join(bad) if bad else "PASS")
    sys.exit(1 if bad else 0)


if __name__ == "__main__":
    main()
