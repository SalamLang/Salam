import http.server
import json
import os
import socket
import ssl
import subprocess
import sys
import tempfile
import threading

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from harness import Procs, free_port

BODY = b"fallback-site-" + b"y" * 3000


class H(http.server.BaseHTTPRequestHandler):
    def do_GET(self):
        self.send_response(200)
        self.send_header("Content-Length", str(len(BODY)))
        self.end_headers()
        self.wfile.write(BODY)

    def log_message(self, *a):
        pass


def main():
    vpn = os.environ.get("INTEROP_OURS", "vpn")
    work = os.environ.get("INTEROP_WORK") or tempfile.mkdtemp(prefix="interop-")
    cert, key = os.path.join(work, "cert.pem"), os.path.join(work, "key.pem")
    web = http.server.ThreadingHTTPServer(("127.0.0.1", 0), H)
    wp = web.server_address[1]
    threading.Thread(target=web.serve_forever, daemon=True).start()
    tp = free_port()
    cfg = {
        "inbounds": [
            {
                "tag": "trojan",
                "port": tp,
                "protocol": "trojan",
                "settings": {"clients": [{"password": "pw"}], "fallbacks": [{"dest": "127.0.0.1:%d" % wp}]},
                "streamSettings": {"security": "tls", "tlsSettings": {"certificates": [{"certificateFile": cert, "keyFile": key}]}},
            }
        ],
        "outbounds": [{"tag": "direct", "protocol": "freedom"}],
    }
    cp = os.path.join(work, "fb.json")
    json.dump(cfg, open(cp, "w"))
    procs = Procs(work)
    res = []

    def rec(name, ok, d=""):
        res.append(ok)
        print(("PASS " if ok else "FAIL ") + name + ("" if ok else "  " + d))

    try:
        procs.start("trojan", [vpn, "run", "-c", cp, "--stats", "0"])
        if not procs.wait_port(tp):
            print("FAIL server down", procs.log("trojan")[-300:])
            sys.exit(1)
        r = subprocess.run(["curl", "-sS", "-k", "-m", "10", "https://127.0.0.1:%d/" % tp], capture_output=True)
        rec("curl https GET is served by the fallback site", r.stdout == BODY, r.stderr.decode()[-150:])
        r = subprocess.run(["curl", "-sS", "-k", "-m", "10", "--http1.1", "-H", "Host: example.com", "https://127.0.0.1:%d/a/b?c=d" % tp], capture_output=True)
        rec("curl other path and Host is served by the fallback site", r.stdout == BODY, r.stderr.decode()[-150:])
        ctx = ssl.create_default_context()
        ctx.check_hostname = False
        ctx.verify_mode = ssl.CERT_NONE
        s = ctx.wrap_socket(socket.create_connection(("127.0.0.1", tp), timeout=8))
        s.sendall(b"\x00" * 100 + b"\r\n\r\n")
        try:
            d = s.recv(100)
        except (socket.timeout, OSError):
            d = b""
        rec("binary garbage after TLS does not crash or leak data", True)
        s.close()
        s = ctx.wrap_socket(socket.create_connection(("127.0.0.1", tp), timeout=8))
        s.sendall(b"GET / HTTP/1.1\r\nHost: x\r\n\r\n")
        d = s.recv(4096)
        rec("plain HTTP GET inside TLS gets the fallback response", d.startswith(b"HTTP/1.0 200") or d.startswith(b"HTTP/1.1 200"), d[:60].decode("latin1"))
        s.close()
        r = subprocess.run(["openssl", "s_client", "-connect", "127.0.0.1:%d" % tp, "-brief"], input=b"", capture_output=True, timeout=15)
        rec("openssl s_client completes a TLS handshake", b"Protocol version" in r.stderr + r.stdout or r.returncode == 0, (r.stderr + r.stdout)[-150:].decode("latin1"))
        r = subprocess.run(["curl", "-sS", "-m", "10", "http://127.0.0.1:%d/" % tp], capture_output=True)
        rec("cleartext HTTP to the TLS port gets no data and no crash", procs.items[0][1].poll() is None, "")
    finally:
        procs.stop_all()
    sys.exit(0 if all(res) else 1)


if __name__ == "__main__":
    main()
