#!/usr/bin/env python3
"""End-to-end test for `salam dap`.

Drives a real debug session: build tests/lsp/dap/hello.salam with debug
information, stop at a breakpoint inside it, read the stack and a local,
then run to the end. Needs gdb 14+ or lldb-dap on PATH.

    python3 tools/lsp/test_dap.py [path/to/salam]
"""

# pylint: disable=too-many-locals,too-many-statements,consider-using-with

import json
import os
import queue
import subprocess
import sys
import threading
import time

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
PROG = os.path.join(ROOT, "tests", "lsp", "dap", "hello.salam")


def read_text(path):
    """Read a UTF-8 file."""
    with open(path, encoding="utf-8") as f:
        return f.read()


class Dap:
    def __init__(self, exe):
        env = dict(os.environ)
        env.setdefault("SALAM_STD", os.path.join(ROOT, "std"))
        self.p = subprocess.Popen(
            [exe, "dap"],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            env=env,
        )
        self.q = queue.Queue()
        self.seen = []
        self.seq = 1
        threading.Thread(target=self._read, daemon=True).start()

    def _read(self):
        f = self.p.stdout
        while True:
            head = b""
            while not head.endswith(b"\r\n\r\n"):
                c = f.read(1)
                if not c:
                    self.q.put(None)
                    return
                head += c
            length = next(
                line
                for line in head.decode().split("\r\n")
                if line.lower().startswith("content-length")
            )
            n = int(length.split(":")[1])
            self.q.put(json.loads(f.read(n)))

    def send(self, command, args=None):
        seq = self.seq
        self.seq += 1
        body = json.dumps(
            {"seq": seq, "type": "request", "command": command, "arguments": args or {}}
        ).encode()
        self.p.stdin.write(b"Content-Length: %d\r\n\r\n" % len(body) + body)
        self.p.stdin.flush()
        return seq

    def wait(self, pred, timeout=120):
        for i, m in enumerate(self.seen):
            if pred(m):
                return self.seen.pop(i)
        end = time.time() + timeout
        while time.time() < end:
            m = self.q.get(timeout=max(0.05, end - time.time()))
            if m is None:
                raise RuntimeError("salam dap exited")
            if pred(m):
                return m
            self.seen.append(m)
        raise TimeoutError("no matching DAP message")

    def call(self, command, args=None):
        seq = self.send(command, args)
        return self.wait(
            lambda m: m.get("type") == "response" and m.get("request_seq") == seq
        )


def main():
    exe = (
        os.path.abspath(sys.argv[1])
        if len(sys.argv) > 1
        else os.path.join(ROOT, "salam")
    )
    lines = read_text(PROG).split("\n")
    bp_line = lines.index("    c := a + b") + 1
    failed = 0

    def check(name, ok, detail=""):
        nonlocal failed
        print(("PASS " if ok else "FAIL ") + name + ("" if ok else ": " + detail))
        if not ok:
            failed += 1

    d = Dap(exe)
    r = d.call(
        "initialize",
        {
            "clientID": "test",
            "adapterID": "salam",
            "linesStartAt1": True,
            "columnsStartAt1": True,
            "pathFormat": "path",
        },
    )
    check("initialize", r.get("success") is True, str(r)[:300])
    launch_seq = d.send("launch", {"program": PROG})
    d.wait(lambda m: m.get("type") == "event" and m.get("event") == "initialized")
    r = d.call(
        "setBreakpoints", {"source": {"path": PROG}, "breakpoints": [{"line": bp_line}]}
    )
    bps = r.get("body", {}).get("breakpoints", [])
    check(
        "a breakpoint in a .salam file is accepted",
        r.get("success") is True and len(bps) == 1,
        str(r)[:300],
    )
    d.call("configurationDone")
    lr = d.wait(
        lambda m: m.get("type") == "response" and m.get("request_seq") == launch_seq
    )
    check(
        "launch builds and starts the program", lr.get("success") is True, str(lr)[:300]
    )
    st = d.wait(lambda m: m.get("type") == "event" and m.get("event") == "stopped")
    tid = st["body"].get("threadId")
    if tid is None:
        tid = d.call("threads")["body"]["threads"][0]["id"]
    frames = d.call("stackTrace", {"threadId": tid})["body"]["stackFrames"]
    top = frames[0] if frames else {}
    src = top.get("source", {}).get("path", "") or top.get("source", {}).get("name", "")
    check(
        "stopped at the breakpoint in the Salam source",
        src.endswith("hello.salam") and top.get("line") == bp_line,
        json.dumps(top)[:300],
    )
    check(
        "the caller is main",
        any(f.get("name", "").endswith("main") for f in frames[1:]),
        str([f.get("name") for f in frames])[:300],
    )
    scopes = d.call("scopes", {"frameId": top.get("id")})["body"]["scopes"]
    names = {}
    for sc in scopes:
        for v in d.call("variables", {"variablesReference": sc["variablesReference"]})[
            "body"
        ].get("variables", []):
            names[v["name"]] = v.get("value")
    check(
        "locals are visible",
        names.get("a") == "2" and names.get("b") == "3",
        str(names)[:300],
    )
    d.call("continue", {"threadId": tid})

    def finished(m):
        return m.get("type") == "event" and m.get("event") in ("exited", "terminated")

    ev = d.wait(finished)
    check(
        "the program runs to the end",
        ev.get("event") in ("exited", "terminated"),
        str(ev)[:200],
    )
    try:
        if d.p.poll() is None:
            d.call(
                "disconnect",
                {"terminateDebuggee": True},
            )
    except (RuntimeError, TimeoutError):
        pass
    d.p.kill()
    print(f"RESULT: {failed} failed")
    sys.exit(1 if failed else 0)


if __name__ == "__main__":
    main()
