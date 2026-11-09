import os
import subprocess
import sys
import tempfile
import threading
import time

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import matrix
from harness import Echo, Procs, check_tcp_via_socks, free_port

PEER = os.environ.get("INTEROP_MUX_PEER", "mux_peer")
STREAMS = 4
SIZE = 1 << 20


def ours_client_to_xray(proto, work0, echo, cert, key):
    work = os.path.join(work0, "mux_%s_ours_xray" % proto)
    os.makedirs(work, exist_ok=True)
    procs = Procs(work)
    name = "%s mux.cool: our client -> xray server, %d streams of 1 MiB" % (proto, STREAMS)
    try:
        case = {"name": "mux", "proto": proto}
        sp = free_port()
        spath = os.path.join(work, "server.json")
        matrix.gen_xray_server(case, sp, cert, key, spath)
        procs.start("server", matrix.argv_xray(spath))
        if not procs.wait_port(sp):
            return name, False, "server did not listen"
        r = subprocess.run([PEER, "client", proto, str(sp), matrix.UUID, "127.0.0.1", str(echo.port), str(STREAMS), str(SIZE)], capture_output=True, timeout=120)
        out = r.stdout.decode().strip()
        return name, r.returncode == 0 and out.startswith("OK"), out + " " + procs.log("server")[-200:].replace("\n", " ")
    except subprocess.TimeoutExpired:
        return name, False, "timeout"
    finally:
        procs.stop_all()


def xray_client_to_ours(proto, work0, echo, cert, key):
    work = os.path.join(work0, "mux_%s_xray_ours" % proto)
    os.makedirs(work, exist_ok=True)
    procs = Procs(work)
    name = "%s mux.cool: xray client -> our server, %d parallel streams of 1 MiB" % (proto, STREAMS)
    try:
        case = {"name": "mux", "proto": proto, "mux": True}
        sp, lp = free_port(), free_port()
        procs.start("server", [PEER, "server", proto, str(sp), matrix.UUID])
        if not procs.wait_port(sp):
            return name, False, "server did not listen"
        cpath = os.path.join(work, "client.json")
        matrix.gen_xray_client(case, sp, lp, cert, key, cpath)
        procs.start("client", matrix.argv_xray(cpath))
        if not procs.wait_port(lp):
            return name, False, "client did not listen"
        res = [None] * STREAMS

        def one(i):
            res[i] = check_tcp_via_socks(lp, echo.port, SIZE)

        ts = [threading.Thread(target=one, args=(i,)) for i in range(STREAMS)]
        for t in ts:
            t.start()
        for t in ts:
            t.join(90)
        ok = all(r and r[0] for r in res)
        return name, ok, str(res) + " " + procs.log("client")[-200:].replace("\n", " ")
    finally:
        procs.stop_all()


def main():
    work0 = os.environ.get("INTEROP_WORK") or tempfile.mkdtemp(prefix="interop-")
    cert, key = matrix.prepare(work0)
    echo = Echo()
    results = []
    for proto in ("vless", "vmess"):
        for fn in (ours_client_to_xray, xray_client_to_ours):
            n, ok, d = fn(proto, work0, echo, cert, key)
            results.append(ok)
            print(("PASS " if ok else "FAIL ") + n + ("" if ok else "  " + d[:400]), flush=True)
    echo.close()
    sys.exit(0 if all(results) else 1)


if __name__ == "__main__":
    main()
