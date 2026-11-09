import json
import os
import subprocess
import sys
import tempfile

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import matrix
from harness import Echo, Procs, check_tcp_via_socks, free_port, socks5_udp_associate

import base64

METHODS = ("aes-128-gcm", "aes-256-gcm", "chacha20-ietf-poly1305")
M2022 = (("2022-blake3-aes-128-gcm", 16), ("2022-blake3-aes-256-gcm", 32), ("2022-blake3-chacha20-poly1305", 32))


def key2022(n, salt):
    return base64.b64encode(bytes((i * 7 + salt) & 255 for i in range(n))).decode()
PW = matrix.PASS


def main():
    peer = os.environ.get("INTEROP_SS_PEER", "ss_peer")
    mihomo = os.environ.get("INTEROP_MIHOMO", "")
    work0 = os.environ.get("INTEROP_WORK") or tempfile.mkdtemp(prefix="interop-")
    cert, key = os.path.join(work0, "cert.pem"), os.path.join(work0, "key.pem")
    echo = Echo()
    res = []

    def rec(name, ok, d=""):
        res.append(ok)
        print(("PASS " if ok else "FAIL ") + name + ("" if ok else "  " + d), flush=True)

    for m in METHODS:
        case = {"name": "ss/" + m, "proto": "ss", "method": m, "password": PW}
        for real in ("xray", "sing-box", "ssrust"):
            work = os.path.join(work0, "ssu_%s_%s" % (m, real))
            os.makedirs(work, exist_ok=True)
            procs = Procs(work)
            try:
                sp = free_port()
                sg, _, sa, _ = matrix.IMPLS[real]
                spath = os.path.join(work, "s.json")
                sg(case, sp, cert, key, spath)
                procs.start("server", sa(spath))
                procs.wait_port(sp)
                r = subprocess.run([peer, "client-udp", str(sp), m, PW, "127.0.0.1", str(echo.port)], capture_output=True, timeout=60)
                out = r.stdout.decode().strip()
                rec("ss udp %s our client -> %s server" % (m, real), r.returncode == 0 and out.startswith("OK"), out + procs.log("server")[-150:])
            except subprocess.TimeoutExpired:
                rec("ss udp %s our client -> %s server" % (m, real), False, "timeout")
            finally:
                procs.stop_all()
        for real in ("xray", "sing-box", "ssrust"):
            work = os.path.join(work0, "ssu2_%s_%s" % (m, real))
            os.makedirs(work, exist_ok=True)
            procs = Procs(work)
            try:
                sp, lp = free_port(), free_port()
                procs.start("server", [peer, "server-udp", str(sp), m, PW])
                import time

                time.sleep(0.5)
                _, cg, _, ca = matrix.IMPLS[real]
                cpath = os.path.join(work, "c.json")
                cg(case, sp, lp, cert, key, cpath)
                procs.start("client", ca(cpath))
                procs.wait_port(lp)
                ok, d = socks5_udp_associate(lp, "127.0.0.1", echo.port)
                rec("ss udp %s %s client -> our server" % (m, real), ok, d + procs.log("client")[-150:])
            except Exception as e:
                rec("ss udp %s %s client -> our server" % (m, real), False, str(e))
            finally:
                procs.stop_all()
    if mihomo:
        for mode in ("http", "tls"):
            work = os.path.join(work0, "obfs_" + mode)
            os.makedirs(work, exist_ok=True)
            procs = Procs(work)
            try:
                sp, lp = free_port(), free_port()
                procs.start("server", [peer, "server-tcp", str(sp), "aes-256-gcm", PW, mode])
                procs.wait_port(sp)
                cfg = (
                    "mixed-port: %d\nbind-address: 127.0.0.1\nallow-lan: false\nmode: rule\nlog-level: info\n"
                    "proxies:\n  - {name: s, type: ss, server: 127.0.0.1, port: %d, cipher: aes-256-gcm, password: %s, plugin: obfs, plugin-opts: {mode: %s, host: www.bing.com}}\n"
                    "rules:\n  - MATCH,s\n" % (lp, sp, PW, mode)
                )
                cd = os.path.join(work, "mh")
                os.makedirs(cd, exist_ok=True)
                open(os.path.join(cd, "config.yaml"), "w").write(cfg)
                procs.start("client", [mihomo, "-d", cd])
                procs.wait_port(lp)
                ok, d = check_tcp_via_socks(lp, echo.port)
                rec("ss simple-obfs %s: mihomo client -> our server (1 MB)" % mode, ok, d + procs.log("client")[-200:])
            finally:
                procs.stop_all()
    echo.close()
    sys.exit(0 if all(res) else 1)


if __name__ == "__main__":
    main()
