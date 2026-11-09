import json
import os
import sys
import tempfile

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import matrix
from harness import Echo

FINGERPRINTS = ("chrome", "firefox", "safari", "edge", "ios", "randomized", "chrome_120")


def case_list():
    cs = []
    for multi in (False, True):
        for tls in (False, True):
            name = "vless/grpc-%s%s" % ("multi" if multi else "gun", "+tls" if tls else "")
            cs.append({"name": name, "proto": "vless", "net": "grpc", "multi": multi, "tls": tls, "reals": ("xray",) if multi else ("xray", "sing-box")})
    cs.append({"name": "vmess/grpc-gun+tls", "proto": "vmess", "net": "grpc", "tls": True})
    vision = "xtls-rprx-vision"
    cs.append({"name": "vless/vision+tls", "proto": "vless", "tls": True, "flow": vision})
    cs.append({"name": "vless/vision+tls inner-tls", "proto": "vless", "tls": True, "flow": vision, "inner_tls": True})
    cs.append({"name": "vless/vision+reality", "proto": "vless", "reality": True, "flow": vision})
    cs.append({"name": "vless/vision+reality inner-tls", "proto": "vless", "reality": True, "flow": vision, "inner_tls": True})
    cs.append({"name": "vless/reality", "proto": "vless", "reality": True})
    cs.append({"name": "vless/reality inner-tls", "proto": "vless", "reality": True, "inner_tls": True})
    cs.append({"name": "vless/reality grpc", "proto": "vless", "reality": True, "net": "grpc"})
    for name, args in (
        ("tcp default", ["--mode", "tcp"]),
        ("tcp 1 byte", ["--mode", "tcp", "--min", "1", "--max", "1", "--delay", "1"]),
        ("tls records", ["--mode", "tls", "--min", "10", "--max", "40"]),
        ("both split at the sni", ["--mode", "both", "--sni", "--min", "3", "--max", "9", "--delay", "1"]),
    ):
        cs.append({"name": "vless/tls fragment %s" % name, "proto": "vless", "tls": True, "frag_args": args, "only_ours_client": True})
    for fp in FINGERPRINTS:
        c = {"name": "vless/reality fp=%s" % fp, "proto": "vless", "reality": True, "fp": fp, "reals": ("xray",), "no_ours_server": True}
        if fp == "chrome_120":
            c["expect_fail"] = "no hybrid key share, Xray drops the hello"
        cs.append(c)
    return cs


def main():
    only = sys.argv[1] if len(sys.argv) > 1 else ""
    work0 = os.environ.get("INTEROP_WORK") or tempfile.mkdtemp(prefix="interop-")
    cert, key = matrix.prepare(work0)
    matrix.reality_keys()
    echo = Echo(cert, key, os.environ.get("INTEROP_DEST_CURVE", "X25519") or None)
    matrix.REALITY["dest"] = echo.tls_port
    results = matrix.run_cases(case_list(), os.path.join(work0, "m2"), cert, key, echo, only=only)
    echo.close()
    bad = [r for r in results if not r[1]]
    print("\n%d/%d passed" % (len(results) - len(bad), len(results)))
    with open(os.path.join(work0, "results2.json"), "w") as f:
        json.dump(results, f, indent=1)
    sys.exit(1 if bad else 0)


if __name__ == "__main__":
    main()
