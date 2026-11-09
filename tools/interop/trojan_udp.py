import os
import subprocess
import sys
import tempfile
import time

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import matrix
from harness import Echo, Procs, free_port, socks5_udp_associate

PEER = os.environ.get("INTEROP_TROJAN_PEER", "trojan_peer")
CASE = {"name": "trojan/udp", "proto": "trojan", "net": "tcp", "tls": True}


def main():
    work0 = os.environ.get("INTEROP_WORK") or tempfile.mkdtemp(prefix="interop-")
    cert, key = matrix.prepare(work0)
    echo = Echo()
    res = []

    def rec(name, ok, d):
        res.append(ok)
        print(("PASS " if ok else "FAIL ") + name + ("" if ok else "  " + d[:300]), flush=True)

    for real in ("xray", "sing-box"):
        work = os.path.join(work0, "tu1_" + real)
        os.makedirs(work, exist_ok=True)
        procs = Procs(work)
        try:
            sp = free_port()
            spath = os.path.join(work, "s.json")
            matrix.IMPLS[real][0](CASE, sp, cert, key, spath)
            procs.start("server", matrix.IMPLS[real][2](spath))
            procs.wait_port(sp)
            r = subprocess.run([PEER, "client-udp", str(sp), matrix.PASS, "127.0.0.1", str(echo.port)], capture_output=True, timeout=60)
            out = r.stdout.decode().strip()
            rec("trojan udp: our client -> %s server" % real, r.returncode == 0 and out.startswith("OK"), out + " " + procs.log("server")[-200:].replace("\n", " "))
        except subprocess.TimeoutExpired:
            rec("trojan udp: our client -> %s server" % real, False, "timeout")
        finally:
            procs.stop_all()
        work = os.path.join(work0, "tu2_" + real)
        os.makedirs(work, exist_ok=True)
        procs = Procs(work)
        try:
            sp, lp = free_port(), free_port()
            procs.start("server", [PEER, "server", str(sp), matrix.PASS, cert, key])
            procs.wait_port(sp)
            cpath = os.path.join(work, "c.json")
            matrix.IMPLS[real][1](CASE, sp, lp, cert, key, cpath)
            procs.start("client", matrix.IMPLS[real][3](cpath))
            procs.wait_port(lp)
            ok, d = socks5_udp_associate(lp, "127.0.0.1", echo.port)
            rec("trojan udp: %s client -> our server" % real, ok, d + " " + procs.log("client")[-200:].replace("\n", " "))
        except Exception as e:
            rec("trojan udp: %s client -> our server" % real, False, str(e))
        finally:
            procs.stop_all()
    echo.close()
    sys.exit(0 if all(res) else 1)


if __name__ == "__main__":
    main()
