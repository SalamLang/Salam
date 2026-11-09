import http.server
import json
import os
import subprocess
import sys
import tempfile
import threading

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from harness import Procs, free_port

BODY = b"interop-ok-" + b"x" * 5000


class H(http.server.BaseHTTPRequestHandler):
    def do_GET(self):
        self.send_response(200)
        self.send_header("Content-Length", str(len(BODY)))
        self.end_headers()
        self.wfile.write(BODY)

    def log_message(self, *a):
        pass


def curl(args):
    r = subprocess.run(["curl", "-sS", "-m", "15", "-o", "-"] + args, capture_output=True)
    return r.returncode == 0 and r.stdout == BODY, (r.stderr.decode()[-200:] or "rc=%d" % r.returncode)


def main():
    vpn = os.environ.get("INTEROP_OURS", "vpn")
    work = os.environ.get("INTEROP_WORK") or tempfile.mkdtemp(prefix="interop-")
    cert, key = os.path.join(work, "cert.pem"), os.path.join(work, "key.pem")
    web = http.server.ThreadingHTTPServer(("127.0.0.1", 0), H)
    wp = web.server_address[1]
    threading.Thread(target=web.serve_forever, daemon=True).start()
    url = "http://127.0.0.1:%d/" % wp
    res = []
    procs = Procs(work)
    try:
        out = [("freedom", "direct")]
        cases = []
        mp, sp, hp, hs, tp = (free_port() for _ in range(5))
        cfg = {
            "inbounds": [
                {"tag": "mixed", "listen": "127.0.0.1", "port": mp, "protocol": "mixed"},
                {"tag": "socks", "listen": "127.0.0.1", "port": sp, "protocol": "socks"},
                {"tag": "http", "listen": "127.0.0.1", "port": hp, "protocol": "http"},
                {
                    "tag": "https",
                    "listen": "127.0.0.1",
                    "port": hs,
                    "protocol": "http",
                    "streamSettings": {"network": "tcp", "security": "tls", "tlsSettings": {"certificates": [{"certificateFile": cert, "keyFile": key}]}},
                },
            ],
            "outbounds": [{"tag": "direct", "protocol": "freedom"}],
            "routing": {"defaultOutbound": "direct", "rules": []},
        }
        cp = os.path.join(work, "proxies.json")
        json.dump(cfg, open(cp, "w"))
        procs.start("proxies", [vpn, "run", "-c", cp, "--stats", "0"])
        if not (procs.wait_port(mp) and procs.wait_port(hs)):
            print("FAIL proxies did not start", procs.log("proxies")[-300:])
            sys.exit(1)
        tests = [
            ("socks5 (curl --socks5)", ["--socks5", "127.0.0.1:%d" % sp, url]),
            ("socks5h domain (curl --socks5-hostname, localhost)", ["--socks5-hostname", "127.0.0.1:%d" % sp, "http://localhost:%d/" % wp]),
            ("socks5 user/pass offered (curl --proxy-user)", ["--socks5", "127.0.0.1:%d" % sp, "--proxy-user", "u:p", url]),
            ("mixed port as socks4 (curl --socks4)", ["--socks4", "127.0.0.1:%d" % mp, url]),
            ("mixed port as socks5", ["--socks5", "127.0.0.1:%d" % mp, url]),
            ("mixed port as socks4a", ["--socks4a", "127.0.0.1:%d" % mp, "http://localhost:%d/" % wp]),
            ("mixed port as http proxy", ["-x", "http://127.0.0.1:%d" % mp, url]),
            ("http proxy plain GET (curl -x)", ["-x", "http://127.0.0.1:%d" % hp, url]),
            ("http proxy CONNECT (curl -p -x)", ["-p", "-x", "http://127.0.0.1:%d" % hp, url]),
            ("https proxy plain GET (curl --proxy https)", ["--proxy-insecure", "-x", "https://127.0.0.1:%d" % hs, url]),
            ("https proxy CONNECT (curl -p --proxy https)", ["--proxy-insecure", "-p", "-x", "https://127.0.0.1:%d" % hs, url]),
            ("http proxy keep-alive two requests", ["-x", "http://127.0.0.1:%d" % hp, url, url]),
        ]
        for name, args in tests:
            ok, d = curl(args)
            if "two requests" in name:
                r = subprocess.run(["curl", "-sS", "-m", "15", "-o", "/dev/null", "-w", "%{http_code} %{num_connects}", "-x", "http://127.0.0.1:%d" % hp, url, url], capture_output=True)
                ok = r.returncode == 0 and r.stdout.decode().startswith("200 1")
                d = r.stdout.decode() + r.stderr.decode()[-100:]
            res.append((name, ok, d))
            print(("PASS " if ok else "FAIL ") + name + ("" if ok else "  " + d))
    finally:
        procs.stop_all()
    xray = os.environ.get("INTEROP_XRAY")
    if xray:
        from harness import Echo, check_tcp_via_socks

        echo = Echo()
        for proto in ("socks", "http"):
            procs = Procs(work)
            try:
                rp, lp = free_port(), free_port()
                sc = os.path.join(work, "x_%s.json" % proto)
                json.dump({"inbounds": [{"listen": "127.0.0.1", "port": rp, "protocol": proto, "settings": {"auth": "noauth", "udp": False}}], "outbounds": [{"protocol": "freedom", "settings": {"finalRules": [{"action": "allow"}]}}]}, open(sc, "w"))
                cc = os.path.join(work, "o_%s.json" % proto)
                json.dump({"inbounds": [{"tag": "m", "listen": "127.0.0.1", "port": lp, "protocol": "mixed"}], "outbounds": [{"tag": "proxy", "protocol": proto, "settings": {"servers": [{"address": "127.0.0.1", "port": rp}]}}], "routing": {"defaultOutbound": "proxy", "rules": []}}, open(cc, "w"))
                procs.start("xs", [xray, "run", "-c", sc])
                procs.wait_port(rp)
                procs.start("oc", [vpn, "run", "-c", cc, "--stats", "0"])
                procs.wait_port(lp)
                ok, d = check_tcp_via_socks(lp, echo.port)
                name = "ours %s outbound -> xray %s inbound (1 MB)" % (proto, proto)
                res.append((name, ok, d))
                print(("PASS " if ok else "FAIL ") + name + ("" if ok else "  " + d))
            finally:
                procs.stop_all()
        echo.close()
    bad = [r for r in res if not r[1]]
    print("%d/%d passed" % (len(res) - len(bad), len(res)))
    sys.exit(1 if bad else 0)


if __name__ == "__main__":
    main()
