import hashlib
import json
import os
import subprocess
import sys
import tempfile

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import matrix
from harness import Echo, Procs, check_tcp_via_socks, free_port

OPTS = [
    ("stream", 1),
    ("stream+mask", 5),
    ("stream+mask+padding", 13),
    ("stream+authlen", 17),
    ("stream+mask+authlen", 21),
    ("stream+mask+padding+authlen", 29),
]


def combos():
    out = []
    for sec in ("aes-128-gcm", "chacha20-poly1305"):
        for name, o in OPTS:
            out.append((sec, name, o))
    out.append(("none", "stream", 1))
    out.append(("none", "raw", 0))
    out.append(("zero", "raw", 0))
    return out


def main():
    peer = os.environ.get("INTEROP_VMESS_CLIENT", "vmess_client")
    work0 = os.environ.get("INTEROP_WORK") or tempfile.mkdtemp(prefix="interop-")
    os.makedirs(work0, exist_ok=True)
    cert, key = os.path.join(work0, "cert.pem"), os.path.join(work0, "key.pem")
    echo = Echo()
    results = []
    for server in ("ours", "xray", "sing-box"):
        for sec, name, o in combos():
            work = os.path.join(work0, "vmo_%s_%s_%s" % (server, sec, name))
            os.makedirs(work, exist_ok=True)
            procs = Procs(work)
            label = "vmess our-client %s %s -> %s" % (sec, name, server)
            try:
                sp = free_port()
                case = {"name": "vmess", "proto": "vmess"}
                spath = os.path.join(work, "server.json")
                matrix.IMPLS[server][0](case, sp, cert, key, spath)
                procs.start("server", matrix.IMPLS[server][2](spath))
                if not procs.wait_port(sp):
                    results.append((label, False, "server down"))
                    print("FAIL", label, "server down")
                    continue
                r = subprocess.run(
                    [peer, str(sp), matrix.UUID, sec, str(o), "127.0.0.1", str(echo.port), str(1 << 20)],
                    capture_output=True,
                    timeout=60,
                )
                out = r.stdout.decode().strip()
                ok = r.returncode == 0 and out.startswith("OK")
                results.append((label, ok, out))
                print(("PASS " if ok else "FAIL ") + label + "  " + out)
            except subprocess.TimeoutExpired:
                results.append((label, False, "timeout"))
                print("FAIL", label, "timeout")
            finally:
                procs.stop_all()
    echo.close()
    json.dump(results, open(os.path.join(work0, "vmess_options.json"), "w"), indent=1)
    bad = [r for r in results if not r[1]]
    print("%d/%d passed" % (len(results) - len(bad), len(results)))
    sys.exit(1 if bad else 0)


if __name__ == "__main__":
    main()
