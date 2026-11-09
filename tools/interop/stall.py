import os
import socket
import sys
import tempfile
import time

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import matrix
from harness import Echo, Procs, check_tcp_via_socks, free_port

LIMIT = 32
DEADLINE = 11.0


def run(kind, proto, silent, cert, key, work0, echo):
    work = os.path.join(work0, "stall_%s_%s_%d" % (proto, kind, silent))
    os.makedirs(work, exist_ok=True)
    procs = Procs(work)
    case = {"name": "stall", "proto": proto, "net": "ws" if kind == "ws" else "tcp", "tls": kind == "tls" or proto == "trojan" and kind == "tcp"}
    try:
        sp, lp = free_port(), free_port()
        matrix.gen_ours_server(case, sp, cert, key, os.path.join(work, "s.json"))
        matrix.gen_ours_client(case, sp, lp, cert, key, os.path.join(work, "c.json"))
        procs.start("server", matrix.argv_ours(os.path.join(work, "s.json")))
        if not procs.wait_port(sp):
            return False, "server did not listen"
        t_conn = time.time()
        socks = [socket.create_connection(("127.0.0.1", sp)) for _ in range(silent)]
        time.sleep(0.5)
        refused = 0
        for s in socks:
            try:
                if s.recv(1, socket.MSG_DONTWAIT) == b"":
                    refused += 1
            except BlockingIOError:
                pass
            except OSError:
                refused += 1
        procs.start("client", matrix.argv_ours(os.path.join(work, "c.json")))
        procs.wait_port(lp)
        ok, d = check_tcp_via_socks(lp, echo.port, 1 << 16)
        last = 0.0
        closed = 0
        for s in socks:
            s.settimeout(max(0.1, 30 - (time.time() - t_conn)))
            try:
                if s.recv(1) == b"":
                    closed += 1
                    last = max(last, time.time() - t_conn)
            except OSError:
                pass
        good = closed == silent and last <= DEADLINE
        if silent > LIMIT:
            good = good and refused >= silent - LIMIT
        else:
            good = good and ok and refused == 0
        return good, "%d silent: %d refused at once, %d closed, the last after %.1fs, valid session %s" % (silent, refused, closed, last, ok)
    finally:
        procs.stop_all()


def main():
    work0 = os.environ.get("INTEROP_WORK") or tempfile.mkdtemp(prefix="interop-")
    cert, key = matrix.prepare(work0)
    echo = Echo()
    res = []
    for proto, kinds in (("vless", ("tcp", "ws", "tls")), ("trojan", ("tcp", "ws"))):
        for kind in kinds:
            for n in (10, 40):
                ok, d = run(kind, proto, n, cert, key, work0, echo)
                res.append(ok)
                print(("PASS " if ok else "FAIL ") + "%s %s: %s" % (proto, kind, d), flush=True)
    echo.close()
    sys.exit(0 if all(res) else 1)


if __name__ == "__main__":
    main()
