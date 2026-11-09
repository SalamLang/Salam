import json
import os
import socket
import struct
import subprocess
import sys
import tempfile
import time

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import matrix
from harness import Echo, Procs, free_port

PEER = os.environ.get("INTEROP_UDP_PEER", "udp_peer")
SIZES = [1, 29, 512, 1200, 4000, 8000, 8192, 9000, 16000, 32000, 65000]
MUST = 8000
UDP_ALL = os.environ.get("INTEROP_UDP_ALL") == "1"


def dns_query(i):
    name = b"".join(bytes([len(p)]) + p for p in b"www.example.com".split(b".")) + b"\x00"
    return struct.pack(">HHHHHH", 0x1000 + i, 0x0100, 1, 0, 0, 0) + name + struct.pack(">HH", 1, 1)


TRANSPORTS = [
    ("tcp", {}),
    ("tcp+tls", {"tls": True}),
    ("ws+tls", {"net": "ws", "tls": True}),
    ("reality", {"reality": True}),
    ("grpc+tls", {"net": "grpc", "tls": True}),
]


def patch_redirect(real, path, port):
    j = json.load(open(path))
    if real == "xray":
        for o in j["outbounds"]:
            if o["protocol"] == "freedom":
                o["settings"]["redirect"] = "127.0.0.1:%d" % port
    else:
        j.setdefault("route", {})["rules"] = [{"action": "route-options", "override_port": port}]
    matrix.jdump(path, j)


def dump_stream(case, cert, key, server, path):
    matrix.jdump(path, matrix.stream_ours(case, cert, key, server))


def ours_client(case, real, kind, work, cert, key, echo, target_port, redirect):
    procs = Procs(work)
    try:
        sp = free_port()
        spath = os.path.join(work, "server.json")
        matrix.IMPLS[real][0](case, sp, cert, key, spath)
        if redirect:
            patch_redirect(real, spath, echo.port)
        procs.start("server", matrix.IMPLS[real][2](spath))
        if not procs.wait_port(sp):
            return False, "server did not listen: " + procs.log("server")[-200:].replace("\n", " | ")
        st = os.path.join(work, "stream.json")
        dump_stream(case, cert, key, False, st)
        r = subprocess.run([PEER, "client", case["proto"], str(sp), matrix.UUID, st, kind, "127.0.0.1", str(target_port), ",".join(str(s) for s in SIZES)], capture_output=True, timeout=240)
        out = r.stdout.decode().strip()
        return parse(out), out + " | " + procs.log("server")[-160:].replace("\n", " ")
    except subprocess.TimeoutExpired:
        return None, "timeout"
    finally:
        procs.stop_all()


def parse(out):
    res = {}
    for tok in out.split():
        if ":" in tok:
            a, b = tok.split(":", 1)
            if a.isdigit():
                res[int(a)] = b == "ok"
    return res


def assoc(proxy_port, host, port, sizes, dns=False):
    ctl = socket.create_connection(("127.0.0.1", proxy_port), timeout=10)
    ctl.sendall(b"\x05\x01\x00")
    ctl.recv(2)
    ctl.sendall(b"\x05\x03\x00\x01\x00\x00\x00\x00\x00\x00")
    r = ctl.recv(10)
    if len(r) < 10 or r[1] != 0:
        raise RuntimeError("udp associate refused %r" % r)
    bport = struct.unpack(">H", r[8:10])[0]
    u = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    u.settimeout(3)
    hdr = b"\x00\x00\x00\x01" + socket.inet_aton(host) + struct.pack(">H", port)
    res = {}
    for i, sz in enumerate(sizes):
        good = 0
        for k in range(2):
            payload = dns_query(i + k) if dns else os.urandom(sz)
            u.sendto(hdr + payload, ("127.0.0.1", bport))
            try:
                d, _ = u.recvfrom(70000)
            except socket.timeout:
                continue
            if d[10:] == payload:
                good += 1
        res[sz] = good == 2
    u.close()
    ctl.close()
    return res


def ours_server(case, real, kind, work, cert, key, echo, target_port, redirect, dns=False):
    procs = Procs(work)
    try:
        sp, lp = free_port(), free_port()
        st = os.path.join(work, "stream_s.json")
        dump_stream(case, cert, key, True, st)
        argv = [PEER, "server", case["proto"], str(sp), matrix.UUID, st]
        if redirect:
            argv.append(str(echo.port))
        procs.start("server", argv)
        if not procs.wait_port(sp):
            return None, "server did not listen: " + procs.log("server")[-200:]
        cpath = os.path.join(work, "client.json")
        c = dict(case)
        if kind == "mux":
            c["mux"] = True
            c["packet_encoding"] = "xudp"
        matrix.IMPLS[real][1](c, sp, lp, cert, key, cpath)
        procs.start("client", matrix.IMPLS[real][3](cpath))
        if not procs.wait_port(lp):
            return None, "client did not listen: " + procs.log("client")[-300:].replace("\n", " | ")
        sizes = [29] if dns else SIZES
        res = assoc(lp, "127.0.0.1", target_port, sizes, dns)
        return res, procs.log("client")[-160:].replace("\n", " ") + " | " + procs.log("server")[-160:].replace("\n", " ")
    except Exception as e:
        return None, "%s: %s" % (type(e).__name__, e)
    finally:
        procs.stop_all()


def verdict(res, must_upto=MUST):
    if res is None:
        return False, "no result"
    if not res:
        return False, "nothing came back"
    ok_upto = max([s for s, g in res.items() if g] or [0])
    must = all(res.get(s, False) for s in SIZES if s <= must_upto and s in res)
    lost = [s for s, g in sorted(res.items()) if not g]
    return must, "max ok %d, lost %s" % (ok_upto, lost)


def main():
    only = sys.argv[1] if len(sys.argv) > 1 else ""
    dns53 = only == "dns53"
    if dns53:
        only = "/tcp/"
    work0 = os.environ.get("INTEROP_WORK") or tempfile.mkdtemp(prefix="interop-")
    cert, key = matrix.prepare(work0)
    matrix.reality_keys()
    echo = Echo(cert, key, "X25519")
    matrix.REALITY["dest"] = echo.tls_port
    results = []
    cases = []
    for proto in ("vless", "vmess"):
        for tname, extra in TRANSPORTS:
            for kind in ("direct", "mux"):
                for real in ("xray", "sing-box"):
                    if kind == "mux" and (real == "sing-box" and proto == "vmess"):
                        continue
                    if kind == "mux" and not UDP_ALL and tname in ("grpc+tls",):
                        continue
                    case = {"name": "%s/%s/%s" % (proto, tname, kind), "proto": proto}
                    case.update(extra)
                    cases.append((case, real, kind))
    for case, real, kind in cases:
        if only and only not in case["name"] + " " + real:
            continue
        for direction in ("ours->real", "real->ours"):
            work = os.path.join(work0, "udp", (case["name"] + "_" + real + "_" + direction).replace("/", "_").replace(">", ""))
            os.makedirs(work, exist_ok=True)
            if direction == "ours->real":
                res, d = ours_client(case, real, kind, work, cert, key, echo, 53 if dns53 else echo.port, dns53)
            else:
                res, d = ours_server(case, real, kind, work, cert, key, echo, 53 if dns53 else echo.port, dns53, dns53)
            upto = MUST
            if dns53:
                upto = 1200
            if real == "xray" and direction == "real->ours" and kind == "direct":
                upto = 4000
            ok, summary = verdict(res, upto)
            name = "%s %s %s%s" % (case["name"], real, direction, " port 53 with DNS packets" if dns53 else "")
            results.append((name, ok, summary))
            print(("PASS " if ok else "FAIL ") + name + "  " + summary + ("" if ok else " | " + d[:300]), flush=True)
    echo.close()
    bad = [r for r in results if not r[1]]
    print("\n%d/%d passed" % (len(results) - len(bad), len(results)))
    with open(os.path.join(work0, "results_udp.json"), "w") as f:
        json.dump(results, f, indent=1)
    sys.exit(1 if bad else 0)


if __name__ == "__main__":
    main()
