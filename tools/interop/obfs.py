import json
import os
import sys
import tempfile

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import matrix
from harness import Echo, Procs, check_tcp_via_socks, free_port

OBFS_SERVER = os.environ.get("INTEROP_OBFS_SERVER", "obfs-server")
OBFS_LOCAL = os.environ.get("INTEROP_OBFS_LOCAL", "obfs-local")
METHOD = "aes-256-gcm"


def ours_to_real(mode, work, echo):
    procs = Procs(work)
    try:
        sp, lp = free_port(), free_port()
        procs.start("server", [matrix.BIN["ssserver"], "-s", "127.0.0.1:%d" % sp, "-k", matrix.PASS, "-m", METHOD, "--plugin", OBFS_SERVER, "--plugin-opts", "obfs=%s" % mode])
        if not procs.wait_port(sp):
            return False, "server did not listen: " + procs.log("server")[-300:]
        cfg = os.path.join(work, "client.json")
        matrix.jdump(
            cfg,
            {
                "inbounds": [{"tag": "m", "listen": "127.0.0.1", "port": lp, "protocol": "mixed"}],
                "outbounds": [{"tag": "proxy", "protocol": "shadowsocks", "settings": {"servers": [{"address": "127.0.0.1", "port": sp, "method": METHOD, "password": matrix.PASS, "plugin": "obfs-local;obfs=%s;obfs-host=www.bing.com" % mode}]}}],
                "routing": {"defaultOutbound": "proxy", "rules": []},
            },
        )
        procs.start("client", matrix.argv_ours(cfg))
        if not procs.wait_port(lp):
            return False, "client did not listen: " + procs.log("client")[-300:]
        ok, d = check_tcp_via_socks(lp, echo.port)
        return ok, d + " | " + procs.log("server")[-150:].replace("\n", " ")
    finally:
        procs.stop_all()


def real_to_ours(mode, work, echo):
    procs = Procs(work)
    try:
        sp, lp = free_port(), free_port()
        cfg = os.path.join(work, "server.json")
        matrix.jdump(
            cfg,
            {
                "inbounds": [{"tag": "in", "port": sp, "protocol": "shadowsocks", "settings": {"method": METHOD, "password": matrix.PASS, "plugin": "obfs-server;obfs=%s" % mode}}],
                "outbounds": [{"tag": "direct", "protocol": "freedom"}],
            },
        )
        procs.start("server", matrix.argv_ours(cfg))
        if not procs.wait_port(sp):
            return False, "server did not listen: " + procs.log("server")[-300:]
        procs.start("client", [matrix.BIN["sslocal"], "-b", "127.0.0.1:%d" % lp, "-s", "127.0.0.1:%d" % sp, "-k", matrix.PASS, "-m", METHOD, "--protocol", "socks", "--plugin", OBFS_LOCAL, "--plugin-opts", "obfs=%s;obfs-host=www.bing.com" % mode])
        if not procs.wait_port(lp):
            return False, "client did not listen: " + procs.log("client")[-300:]
        ok, d = check_tcp_via_socks(lp, echo.port)
        return ok, d + " | " + procs.log("client")[-150:].replace("\n", " ")
    finally:
        procs.stop_all()


def main():
    work0 = os.environ.get("INTEROP_WORK") or tempfile.mkdtemp(prefix="interop-")
    echo = Echo()
    res = []
    for mode in ("http", "tls"):
        for name, fn in (("our client -> obfs-server + ssserver", ours_to_real), ("sslocal + obfs-local -> our server", real_to_ours)):
            work = os.path.join(work0, "obfs_%s_%s" % (mode, fn.__name__))
            os.makedirs(work, exist_ok=True)
            ok, d = fn(mode, work, echo)
            res.append(ok)
            print(("PASS " if ok else "FAIL ") + "simple-obfs %s: %s (1 MiB)" % (mode, name) + ("" if ok else "  " + d[:400]), flush=True)
    echo.close()
    sys.exit(0 if all(res) else 1)


if __name__ == "__main__":
    main()
