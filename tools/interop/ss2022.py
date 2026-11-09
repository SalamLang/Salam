import os
import subprocess
import sys
import tempfile

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import matrix
from harness import Echo, Procs, check_tcp_via_socks, free_port, socks5_udp_associate
from ss_extra import M2022, key2022

PEER = os.environ.get("INTEROP_SS_PEER", "ss_peer")
REALS = ("xray", "sing-box", "ssrust")


def case_for(method, n, user):
    c = {"name": "ss/" + method, "proto": "ss", "method": method, "password": key2022(n, 1)}
    if user:
        c["user_psk"] = key2022(n, 9)
    return c


def our_udp_to_real(case, real, work0, echo, cert, key):
    work = os.path.join(work0, "ssu_%s_%s_%s" % (case["method"], real, "mu" if case.get("user_psk") else "su"))
    os.makedirs(work, exist_ok=True)
    procs = Procs(work)
    try:
        sp = free_port()
        spath = os.path.join(work, "s.json")
        matrix.IMPLS[real][0](case, sp, cert, key, spath)
        procs.start("server", matrix.IMPLS[real][2](spath))
        procs.wait_port(sp)
        r = subprocess.run([PEER, "client-udp", str(sp), case["method"], matrix._client_pw(case), "127.0.0.1", str(echo.port)], capture_output=True, timeout=60)
        out = r.stdout.decode().strip()
        return r.returncode == 0 and out.startswith("OK"), out + " " + procs.log("server")[-150:].replace("\n", " ")
    except subprocess.TimeoutExpired:
        return False, "timeout"
    finally:
        procs.stop_all()


def real_udp_to_our(case, real, work0, echo, cert, key):
    work = os.path.join(work0, "ssu2_%s_%s_%s" % (case["method"], real, "mu" if case.get("user_psk") else "su"))
    os.makedirs(work, exist_ok=True)
    procs = Procs(work)
    try:
        sp, lp = free_port(), free_port()
        argv = [PEER, "server-udp", str(sp), case["method"], case["password"]]
        if case.get("user_psk"):
            argv.append(case["user_psk"])
        procs.start("server", argv)
        import time

        time.sleep(0.5)
        cpath = os.path.join(work, "c.json")
        matrix.IMPLS[real][1](case, sp, lp, cert, key, cpath)
        procs.start("client", matrix.IMPLS[real][3](cpath))
        procs.wait_port(lp)
        ok, d = socks5_udp_associate(lp, "127.0.0.1", echo.port)
        return ok, d + " " + procs.log("client")[-150:].replace("\n", " ")
    except Exception as e:
        return False, str(e)
    finally:
        procs.stop_all()


def real_tcp_to_our(case, real, work0, echo, cert, key):
    work = os.path.join(work0, "sst_%s_%s" % (case["method"], real))
    os.makedirs(work, exist_ok=True)
    procs = Procs(work)
    try:
        sp, lp = free_port(), free_port()
        procs.start("server", [PEER, "server-tcp", str(sp), case["method"], case["password"], "-", case["user_psk"]])
        procs.wait_port(sp)
        cpath = os.path.join(work, "c.json")
        matrix.IMPLS[real][1](case, sp, lp, cert, key, cpath)
        procs.start("client", matrix.IMPLS[real][3](cpath))
        procs.wait_port(lp)
        ok, d = check_tcp_via_socks(lp, echo.port)
        return ok, d + " " + procs.log("server")[-150:].replace("\n", " ")
    finally:
        procs.stop_all()


def main():
    only = sys.argv[1] if len(sys.argv) > 1 else ""
    work0 = os.environ.get("INTEROP_WORK") or tempfile.mkdtemp(prefix="interop-")
    cert, key = matrix.prepare(work0)
    echo = Echo()
    res = []

    def rec(name, ok, d):
        res.append(ok)
        print(("PASS " if ok else "FAIL ") + name + ("" if ok else "  " + d[:300]), flush=True)

    for method, n in M2022:
        for user in (False, True):
            if user and "chacha" in method:
                continue
            case = case_for(method, n, user)
            tag = "multi-user " if user else ""
            for real in REALS:
                if only and only not in method + " " + tag:
                    continue
                ok, d = our_udp_to_real(case, real, work0, echo, cert, key)
                rec("ss %s%s udp: our client -> %s server" % (tag, method, real), ok, d)
                ok, d = real_udp_to_our(case, real, work0, echo, cert, key)
                rec("ss %s%s udp: %s client -> our server" % (tag, method, real), ok, d)
                if user:
                    work = os.path.join(work0, "mu_" + method + "_" + real)
                    os.makedirs(work, exist_ok=True)
                    out = []
                    matrix.run_pair(case, real, "ours", cert, key, work, echo, out)
                    rec("ss %s%s tcp: our client -> %s server" % (tag, method, real), out[-1][1], out[-1][2])
                    ok, d = real_tcp_to_our(case, real, work0, echo, cert, key)
                    rec("ss %s%s tcp: %s client -> our server" % (tag, method, real), ok, d)
    echo.close()
    sys.exit(0 if all(res) else 1)


if __name__ == "__main__":
    main()
