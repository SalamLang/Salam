import hashlib
import json
import os
import signal
import socket
import struct
import subprocess
import sys
import threading
import time

PAYLOAD = 1 << 20


def free_port():
    s = socket.socket()
    s.bind(("127.0.0.1", 0))
    p = s.getsockname()[1]
    s.close()
    return p


class Echo:
    def __init__(self):
        self.tcp = socket.socket()
        self.tcp.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.tcp.bind(("127.0.0.1", 0))
        self.tcp.listen(64)
        self.port = self.tcp.getsockname()[1]
        self.udp = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.udp.bind(("127.0.0.1", self.port))
        self.alive = True
        self.conns = 0
        threading.Thread(target=self._accept, daemon=True).start()
        threading.Thread(target=self._udp, daemon=True).start()

    def _accept(self):
        while self.alive:
            try:
                c, _ = self.tcp.accept()
            except OSError:
                return
            self.conns += 1
            threading.Thread(target=self._serve, args=(c,), daemon=True).start()

    def _serve(self, c):
        try:
            while True:
                d = c.recv(65536)
                if not d:
                    break
                c.sendall(d)
        except OSError:
            pass
        finally:
            self.conns -= 1
            c.close()

    def _udp(self):
        while self.alive:
            try:
                d, a = self.udp.recvfrom(65535)
                self.udp.sendto(d, a)
            except OSError:
                return

    def close(self):
        self.alive = False
        self.tcp.close()
        self.udp.close()


def socks5_connect(proxy_port, host, port, timeout=20):
    s = socket.create_connection(("127.0.0.1", proxy_port), timeout=timeout)
    s.sendall(b"\x05\x01\x00")
    if s.recv(2) != b"\x05\x00":
        raise RuntimeError("socks5 method refused")
    h = host.encode()
    s.sendall(b"\x05\x01\x00\x03" + bytes([len(h)]) + h + struct.pack(">H", port))
    r = b""
    while len(r) < 4:
        x = s.recv(4 - len(r))
        if not x:
            raise RuntimeError("socks5 closed in reply")
        r += x
    if r[1] != 0:
        raise RuntimeError("socks5 reply code %d" % r[1])
    need = {1: 6, 4: 18, 3: 0}[r[3]]
    if r[3] == 3:
        need = s.recv(1)[0] + 2
    got = 0
    while got < need:
        x = s.recv(need - got)
        if not x:
            raise RuntimeError("socks5 short reply")
        got += len(x)
    return s


def roundtrip(sock, size=PAYLOAD, chunk=16384):
    data = os.urandom(size)
    want = hashlib.sha256(data).hexdigest()
    err = []

    def tx():
        try:
            for i in range(0, size, chunk):
                sock.sendall(data[i : i + chunk])
        except OSError as e:
            err.append(e)

    t = threading.Thread(target=tx, daemon=True)
    t.start()
    h = hashlib.sha256()
    got = 0
    while got < size:
        d = sock.recv(min(65536, size - got))
        if not d:
            break
        h.update(d)
        got += len(d)
    t.join(5)
    return got == size and h.hexdigest() == want, got


def check_tcp_via_socks(proxy_port, echo_port, size=PAYLOAD):
    try:
        s = socks5_connect(proxy_port, "127.0.0.1", echo_port)
        s.settimeout(30)
        ok, got = roundtrip(s, size)
        s.close()
        return ok, "%d/%d bytes" % (got, size)
    except Exception as e:
        return False, "%s: %s" % (type(e).__name__, e)


def socks5_udp_associate(proxy_port, host, port, count=5):
    ctl = socket.create_connection(("127.0.0.1", proxy_port), timeout=10)
    ctl.sendall(b"\x05\x01\x00")
    ctl.recv(2)
    ctl.sendall(b"\x05\x03\x00\x01\x00\x00\x00\x00\x00\x00")
    r = ctl.recv(10)
    if len(r) < 10 or r[1] != 0:
        raise RuntimeError("udp associate refused %r" % r)
    bport = struct.unpack(">H", r[8:10])[0]
    bhost = socket.inet_ntoa(r[4:8])
    if bhost == "0.0.0.0":
        bhost = "127.0.0.1"
    u = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    u.settimeout(8)
    hdr = b"\x00\x00\x00\x01" + socket.inet_aton(host) + struct.pack(">H", port)
    good = 0
    for i in range(count):
        payload = os.urandom(100 + i * 200)
        u.sendto(hdr + payload, (bhost, bport))
        try:
            d, _ = u.recvfrom(65535)
        except socket.timeout:
            continue
        if d[10:] == payload:
            good += 1
    u.close()
    ctl.close()
    return good == count, "%d/%d datagrams" % (good, count)


class Procs:
    def __init__(self, logdir):
        self.items = []
        self.logdir = logdir

    def start(self, name, argv, mem_kb=int(os.environ.get("INTEROP_ULIMIT_KB", "4000000")), env=None):
        log = open(os.path.join(self.logdir, name + ".log"), "wb")
        e = dict(os.environ)
        if env:
            e.update(env)
        p = subprocess.Popen(
            ["bash", "-c", "ulimit -v %d; exec \"$@\"" % mem_kb, "x"] + argv,
            stdout=log,
            stderr=subprocess.STDOUT,
            env=e,
            start_new_session=False,
        )
        self.items.append((name, p, log))
        return p

    def wait_port(self, port, timeout=15):
        end = time.time() + timeout
        while time.time() < end:
            try:
                socket.create_connection(("127.0.0.1", port), timeout=1).close()
                return True
            except OSError:
                time.sleep(0.15)
        return False

    def stop_all(self):
        for name, p, log in self.items:
            if p.poll() is None:
                p.send_signal(signal.SIGTERM)
        end = time.time() + 5
        for name, p, log in self.items:
            try:
                p.wait(max(0.1, end - time.time()))
            except subprocess.TimeoutExpired:
                p.kill()
                p.wait()
            log.close()
        self.items = []

    def log(self, name):
        try:
            with open(os.path.join(self.logdir, name + ".log"), "rb") as f:
                return f.read().decode("utf8", "replace")
        except OSError:
            return ""
