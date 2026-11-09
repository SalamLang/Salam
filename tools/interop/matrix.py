import base64
import json
import os
import subprocess
import sys
import tempfile

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from harness import Echo, Procs, check_tcp_via_socks, free_port, socks5_udp_associate

UUID = "b831381d-6324-4d53-ad4f-8cda48b30811"
PASS = "interop-secret-pw"
PATH = "/ray"

BIN = {}


def need(name, env, default):
    BIN[name] = os.environ.get(env, default)


need("ours", "INTEROP_OURS", "vpn")
need("xray", "INTEROP_XRAY", "xray")
need("sing-box", "INTEROP_SINGBOX", "sing-box")
need("ssserver", "INTEROP_SSSERVER", "ssserver")
need("sslocal", "INTEROP_SSLOCAL", "sslocal")


def jdump(path, obj):
    with open(path, "w") as f:
        json.dump(obj, f, indent=1)


PIN = [""]
REALITY = {"priv": "", "pub": "", "sid": "0123abcd", "dest": 0}
GRPC_SERVICE = "gunsvc"


def stream_xray(c, cert, key, server, server_is_xray_client=False):
    s = {"network": c.get("net", "tcp")}
    if c.get("net") == "ws":
        s["wsSettings"] = {"path": PATH}
    if c.get("net") == "httpupgrade":
        s["httpupgradeSettings"] = {"path": PATH}
    if c.get("net") == "grpc":
        s["grpcSettings"] = {"serviceName": GRPC_SERVICE, "multiMode": bool(c.get("multi"))}
    if c.get("reality"):
        s["security"] = "reality"
        if server:
            s["realitySettings"] = {"dest": "127.0.0.1:%d" % REALITY["dest"], "serverNames": ["localhost"], "privateKey": REALITY["priv"], "shortIds": [REALITY["sid"]]}
        else:
            s["realitySettings"] = {"serverName": "localhost", "fingerprint": c.get("fp", "chrome"), "publicKey": REALITY["pub"], "shortId": REALITY["sid"]}
    elif c.get("tls"):
        s["security"] = "tls"
        if server:
            s["tlsSettings"] = {"certificates": [{"certificateFile": cert, "keyFile": key}]}
        else:
            if server_is_xray_client:
                s["tlsSettings"] = {"serverName": "localhost", "pinnedPeerCertSha256": PIN[0]}
            else:
                s["tlsSettings"] = {"serverName": "localhost", "allowInsecure": True}
    return s


def stream_ours(c, cert, key, server):
    return stream_xray(c, cert, key, server)


def stream_sb(c, cert, key, server):
    out = {}
    n = c.get("net", "tcp")
    if n == "ws":
        out["transport"] = {"type": "ws", "path": PATH}
    if n == "httpupgrade":
        out["transport"] = {"type": "httpupgrade", "path": PATH}
    if n == "grpc":
        out["transport"] = {"type": "grpc", "service_name": GRPC_SERVICE}
    if c.get("reality"):
        if server:
            out["tls"] = {"enabled": True, "server_name": "localhost", "reality": {"enabled": True, "handshake": {"server": "127.0.0.1", "server_port": REALITY["dest"]}, "private_key": REALITY["priv"], "short_id": [REALITY["sid"]]}}
        else:
            out["tls"] = {"enabled": True, "server_name": "localhost", "utls": {"enabled": True, "fingerprint": c.get("fp", "chrome")}, "reality": {"enabled": True, "public_key": REALITY["pub"], "short_id": REALITY["sid"]}}
    elif c.get("tls"):
        if server:
            out["tls"] = {"enabled": True, "certificate_path": cert, "key_path": key}
        else:
            out["tls"] = {"enabled": True, "insecure": True, "server_name": "localhost"}
    return out


def inbound_settings(c):
    p = c["proto"]
    if p == "vless":
        u = {"id": UUID}
        if c.get("flow"):
            u["flow"] = c["flow"]
        return {"clients": [u], "decryption": "none"}
    if p == "vmess":
        return {"clients": [{"id": UUID}]}
    if p == "trojan":
        return {"clients": [{"password": PASS}]}
    if p == "ss":
        st = {"method": c["method"], "password": c["password"], "network": "tcp,udp"}
        if c.get("user_psk"):
            st["clients"] = [{"password": c["user_psk"], "email": "u1@example.com"}]
        return st


def _client_pw(c):
    if c.get("user_psk"):
        return c["password"] + ":" + c["user_psk"]
    return c["password"]


def outbound_xray(c, port, stream):
    p = c["proto"]
    if p in ("vless", "vmess"):
        u = {"id": UUID}
        if p == "vless":
            u["encryption"] = "none"
            if c.get("flow"):
                u["flow"] = c["flow"]
        else:
            u["security"] = c.get("sec", "aes-128-gcm")
        st = {"vnext": [{"address": "127.0.0.1", "port": port, "users": [u]}]}
    elif p == "trojan":
        st = {"servers": [{"address": "127.0.0.1", "port": port, "password": PASS}]}
    else:
        st = {"servers": [{"address": "127.0.0.1", "port": port, "method": c["method"], "password": _client_pw(c)}]}
    o = {"tag": "proxy", "protocol": "shadowsocks" if p == "ss" else p, "settings": st}
    if p != "ss":
        o["streamSettings"] = stream
    if c.get("mux"):
        o["mux"] = {"enabled": True, "concurrency": 8, "xudpConcurrency": 8}
    return o


def gen_xray_server(c, port, cert, key, path):
    ib = {"listen": "127.0.0.1", "port": port, "protocol": "shadowsocks" if c["proto"] == "ss" else c["proto"], "settings": inbound_settings(c)}
    if c["proto"] != "ss":
        ib["streamSettings"] = stream_xray(c, cert, key, True)
    jdump(path, {"log": {"loglevel": "info"}, "inbounds": [ib], "outbounds": [{"protocol": "freedom", "settings": {"finalRules": [{"action": "allow"}]}}]})


def gen_xray_client(c, port, local, cert, key, path):
    jdump(
        path,
        {
            "log": {"loglevel": "info"},
            "inbounds": [{"listen": "127.0.0.1", "port": local, "protocol": "socks", "settings": {"udp": True, "ip": "127.0.0.1"}}],
            "outbounds": [outbound_xray(c, port, stream_xray(c, cert, key, False, True))],
        },
    )


def gen_ours_server(c, port, cert, key, path):
    ib = {"tag": "in", "port": port, "protocol": "shadowsocks" if c["proto"] == "ss" else c["proto"], "settings": inbound_settings(c)}
    if c["proto"] == "ss":
        ib["settings"] = {"method": c["method"], "password": c["password"]}
    else:
        ib["streamSettings"] = stream_ours(c, cert, key, True)
    if c["proto"] == "trojan" and c.get("fallback"):
        ib["settings"]["fallbacks"] = [{"dest": "127.0.0.1:%d" % c["fallback"]}]
    jdump(path, {"log": {"loglevel": "warning"}, "inbounds": [ib], "outbounds": [{"tag": "direct", "protocol": "freedom"}]})


def gen_ours_client(c, port, local, cert, key, path):
    o = outbound_xray(c, port, stream_ours(c, cert, key, False))
    jdump(
        path,
        {
            "inbounds": [{"tag": "mixed-in", "listen": "127.0.0.1", "port": local, "protocol": "mixed"}],
            "outbounds": [o, {"tag": "direct", "protocol": "freedom"}],
            "routing": {"defaultOutbound": "proxy", "rules": []},
        },
    )


def gen_sb_server(c, port, cert, key, path):
    p = c["proto"]
    ib = {"type": "shadowsocks" if p == "ss" else p, "tag": "in", "listen": "127.0.0.1", "listen_port": port}
    if p == "vless":
        ib["users"] = [{"uuid": UUID}]
        if c.get("flow"):
            ib["users"][0]["flow"] = c["flow"]
    elif p == "vmess":
        ib["users"] = [{"uuid": UUID, "alterId": 0}]
    elif p == "trojan":
        ib["users"] = [{"password": PASS}]
    else:
        ib["method"] = c["method"]
        ib["password"] = c["password"]
        if c.get("user_psk"):
            ib["users"] = [{"name": "u1", "password": c["user_psk"]}]
    if p != "ss":
        ib.update(stream_sb(c, cert, key, True))
    jdump(path, {"log": {"level": "info"}, "inbounds": [ib], "outbounds": [{"type": "direct", "tag": "direct"}]})


def gen_sb_client(c, port, local, cert, key, path):
    p = c["proto"]
    ob = {"type": "shadowsocks" if p == "ss" else p, "tag": "proxy", "server": "127.0.0.1", "server_port": port}
    if p == "vless":
        ob["uuid"] = UUID
        if c.get("flow"):
            ob["flow"] = c["flow"]
        if c.get("packet_encoding"):
            ob["packet_encoding"] = c["packet_encoding"]
    elif p == "vmess":
        ob["uuid"] = UUID
        ob["security"] = c.get("sec", "aes-128-gcm")
        ob["alter_id"] = 0
    elif p == "trojan":
        ob["password"] = PASS
    else:
        ob["method"] = c["method"]
        ob["password"] = _client_pw(c)
    if p != "ss":
        ob.update(stream_sb(c, cert, key, False))
    jdump(
        path,
        {
            "log": {"level": "info"},
            "inbounds": [{"type": "socks", "tag": "in", "listen": "127.0.0.1", "listen_port": local}],
            "outbounds": [ob],
            "route": {"final": "proxy"},
        },
    )


def gen_ssr_server(c, port, cert, key, path):
    jdump(path, {"port": port, "method": c["method"], "password": c["password"], "user": c.get("user_psk", "")})


def gen_ssr_client(c, port, local, cert, key, path):
    jdump(path, {"port": port, "local": local, "method": c["method"], "password": _client_pw(c)})


def argv_ssr_server(path):
    j = json.load(open(path))
    if j.get("user"):
        cfg = path + ".full.json"
        jdump(cfg, {"servers": [{"server": "127.0.0.1", "server_port": j["port"], "method": j["method"], "password": j["password"], "users": [{"name": "u1", "password": j["user"]}], "mode": "tcp_and_udp"}]})
        return [BIN["ssserver"], "-c", cfg]
    return [BIN["ssserver"], "-s", "127.0.0.1:%d" % j["port"], "-k", j["password"], "-m", j["method"], "-U"]


def argv_ssr_client(path):
    j = json.load(open(path))
    return [BIN["sslocal"], "-b", "127.0.0.1:%d" % j["local"], "-s", "127.0.0.1:%d" % j["port"], "-k", j["password"], "-m", j["method"], "--protocol", "socks", "-U"]


def argv_xray(path):
    return [BIN["xray"], "run", "-c", path]


def argv_sb(path):
    return [BIN["sing-box"], "run", "-c", path]


def argv_ours(path):
    return [BIN["ours"], "run", "-c", path, "--stats", "0"]


def ours_exe(role):
    return BIN["ours"]


IMPLS = {
    "ssrust": (gen_ssr_server, gen_ssr_client, argv_ssr_server, argv_ssr_client),
    "xray": (gen_xray_server, gen_xray_client, argv_xray, argv_xray),
    "sing-box": (gen_sb_server, gen_sb_client, argv_sb, argv_sb),
    "ours": (gen_ours_server, gen_ours_client, argv_ours, argv_ours),
}


def case_list():
    cs = []
    for proto in ("vless", "trojan", "vmess"):
        nets = [("tcp", False), ("tcp", True), ("ws", False), ("ws", True), ("httpupgrade", False)]
        if proto == "trojan":
            nets = [("tcp", True), ("ws", False), ("ws", True), ("httpupgrade", True), ("grpc", True)]
        for net, tls in nets:
            if proto == "vmess":
                for sec in ("aes-128-gcm", "chacha20-poly1305", "none"):
                    if net == "tcp" and not tls:
                        cs.append({"name": "vmess/%s/tcp" % sec, "proto": proto, "net": net, "tls": tls, "sec": sec})
                if net != "tcp" or tls:
                    cs.append({"name": "vmess/aes-128-gcm/%s%s" % (net, "+tls" if tls else ""), "proto": proto, "net": net, "tls": tls})
            else:
                cs.append({"name": "%s/%s%s" % (proto, net, "+tls" if tls else ""), "proto": proto, "net": net, "tls": tls})
    for m in ("aes-128-gcm", "aes-256-gcm", "chacha20-ietf-poly1305"):
        cs.append({"name": "ss/" + m, "proto": "ss", "method": m, "password": PASS})
    for m, n in (("2022-blake3-aes-128-gcm", 16), ("2022-blake3-aes-256-gcm", 32), ("2022-blake3-chacha20-poly1305", 32)):
        cs.append({"name": "ss/" + m, "proto": "ss", "method": m, "password": base64.b64encode(bytes(range(1, n + 1))).decode()})
    return cs


def run_pair(case, server_impl, client_impl, cert, key, work, echo, results, udp=False):
    procs = Procs(work)
    name = "%s %s->%s" % (case["name"], client_impl, server_impl)
    try:
        sp = free_port()
        lp = free_port()
        sg = IMPLS[server_impl][0]
        cg = IMPLS[client_impl][1]
        spath = os.path.join(work, "server.json")
        cpath = os.path.join(work, "client.json")
        sg(case, sp, cert, key, spath)
        cg(case, sp, lp, cert, key, cpath)
        procs.start("server", IMPLS[server_impl][2](spath))
        if not procs.wait_port(sp):
            results.append((name, False, "server did not listen: " + procs.log("server")[-300:].replace("\n", " | ")))
            return
        cargv = IMPLS[client_impl][3](cpath)
        if client_impl == "ours" and case.get("frag_args"):
            cargv = [os.environ.get("INTEROP_VPN_FRAG", "vpn_frag"), "--config", cpath] + case["frag_args"]
        procs.start("client", cargv)
        if not procs.wait_port(lp):
            results.append((name, False, "client did not listen: " + procs.log("client")[-300:].replace("\n", " | ")))
            return
        ok, detail = check_tcp_via_socks(lp, echo.tls_port if case.get("inner_tls") else echo.port, inner_tls=bool(case.get("inner_tls")))
        if not ok:
            detail += " | srv: " + procs.log("server")[-200:].replace("\n", " ") + " | cli: " + procs.log("client")[-200:].replace("\n", " ")
        results.append((name + " tcp", ok, detail))
        if udp and case.get("udp"):
            try:
                ok2, d2 = socks5_udp_associate(lp, "127.0.0.1", echo.port)
            except Exception as e:
                ok2, d2 = False, "%s: %s" % (type(e).__name__, e)
            results.append((name + " udp", ok2, d2))
    finally:
        procs.stop_all()


def prepare(work0):
    os.makedirs(work0, exist_ok=True)
    cert = os.path.join(work0, "cert.pem")
    key = os.path.join(work0, "key.pem")
    if not os.path.exists(cert):
        subprocess.check_call(
            ["openssl", "req", "-x509", "-newkey", "rsa:2048", "-nodes", "-keyout", key, "-out", cert, "-days", "30", "-subj", "/CN=localhost", "-addext", "subjectAltName=DNS:localhost,IP:127.0.0.1"],
            stderr=subprocess.DEVNULL,
        )
    der = subprocess.check_output(["openssl", "x509", "-in", cert, "-outform", "DER"])
    import hashlib

    PIN[0] = hashlib.sha256(der).hexdigest()
    return cert, key


def reality_keys():
    out = subprocess.check_output([BIN["xray"], "x25519"]).decode()
    vals = [l.split(":", 1)[1].strip() for l in out.splitlines() if ":" in l]
    REALITY["priv"] = vals[0]
    REALITY["pub"] = vals[1]


def run_cases(cases, work0, cert, key, echo, reals=("xray", "sing-box"), only=""):
    results = []
    for case in cases:
        if only and only not in case["name"]:
            continue
        for real in case.get("reals", reals):
            for (srv, cli) in (("ours", real), (real, "ours")):
                if case.get("only_ours_client") and cli != "ours":
                    continue
                if srv == "ours" and case.get("no_ours_server") or cli == "ours" and case.get("no_ours_client"):
                    continue
                work = os.path.join(work0, case["name"].replace("/", "_").replace(" ", "_") + "_" + cli + "_" + srv)
                os.makedirs(work, exist_ok=True)
                run_pair(case, srv, cli, cert, key, work, echo, results)
                if case.get("expect_fail"):
                    results[-1] = (results[-1][0] + " (expected to fail: " + case["expect_fail"] + ")", not results[-1][1], results[-1][2])
                last = results[-1]
                print(("PASS " if last[1] else "FAIL ") + last[0] + "  " + last[2][:300], flush=True)
    return results


def main():
    only = sys.argv[1] if len(sys.argv) > 1 else ""
    work0 = os.environ.get("INTEROP_WORK") or tempfile.mkdtemp(prefix="interop-")
    cert, key = prepare(work0)
    echo = Echo()
    results = []
    for case in case_list():
        if only and only not in case["name"]:
            continue
        reals = ("xray", "sing-box", "ssrust") if case["proto"] == "ss" else ("xray", "sing-box")
        results += run_cases([case], work0, cert, key, echo, reals)
    echo.close()
    bad = [r for r in results if not r[1]]
    print("\n%d/%d passed" % (len(results) - len(bad), len(results)))
    with open(os.path.join(work0, "results.json"), "w") as f:
        json.dump(results, f, indent=1)
    sys.exit(1 if bad else 0)


if __name__ == "__main__":
    main()
