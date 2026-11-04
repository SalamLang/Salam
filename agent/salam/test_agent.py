#!/usr/bin/env python3
"""End-to-end test of the Salam build of Salam Agent against a fake backend.

Usage: test_agent.py <agent binary>
Needs a display, for example under xvfb-run.
"""

import json
import os
import shutil
import subprocess
import sys
import tempfile
import threading
import time
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer

SITE = """<!doctype html><meta charset=utf-8><title>site</title>
<h1>Site heading</h1><p id=copy>سلام from the site</p>
<a id=next href="/site2">Second page</a>
<button id=btn onclick="document.getElementById('copy').textContent='clicked';console.warn('warned')">Press me</button>
<input id=name name=name placeholder="your name"><select id=pick><option value=a>Apple</option><option value=b>Berry</option></select>
<img src="/missing.png"><div id=late></div><div style="height:3000px"></div>
<script>console.log("hello log");setTimeout(()=>{document.getElementById('late').textContent='ready now'},300);</script>"""

SITE2 = """<!doctype html><meta charset=utf-8><title>second</title><h2>Second</h2>"""

PAGE = """<!doctype html><meta charset=utf-8><title>fake app</title><script>
(async () => {
  const report = (name, value) => fetch("/report", {method: "POST", body: JSON.stringify({name, value})});
  try {
    salam.listen("agent://call", (c) => {
      report("call:" + c.tool + ":" + c.state, c.output || c.summary);
      if (c.state === "running" && c.call_id === "stopme") setTimeout(() => salam.invoke("agent_stop", {callId: "stopme"}), 600);
      if (c.state === "pending") salam.invoke("agent_decide", {callId: c.call_id, answer: c.tool === "shell" || c.tool === "browser" ? "allow" : "deny"});
    });
    salam.listen("agent://output", (o) => report("output", o.chunk));
    salam.listen("agent://connection", (v) => report("connection", v));
    salam.listen("agent://device", () => report("device", "gone"));
    await report("info", await salam.invoke("agent_info"));
    await report("link", await salam.invoke("agent_link", {token: "user-token"}));
    await report("prefs", await salam.invoke("agent_notify_prefs"));
    await report("setprefs", await salam.invoke("agent_set_notify_prefs", {prefs: {enabled: false, sound: false, preview: false, lang: "en"}}));
    await report("badwatch", await salam.invoke("agent_watch_reply", {chat: "bad id", mid: 1, token: "t"}).catch((e) => e.message));
    await report("context", await salam.invoke("agent_project_context", {workspace: WS}));
    await report("ready", true);
  } catch (e) {
    await report("exception", String(e));
  }
})();
const WS = %WS%;
</script>"""


class State:
    def __init__(self, workspace):
        self.workspace = workspace
        self.reports = []
        self.results = {}
        self.calls = []
        self.lock = threading.Lock()
        self.ready = threading.Event()
        self.done = threading.Event()
        self.linked = False


def make_handler(state):
    class Handler(BaseHTTPRequestHandler):
        def log_message(self, *args):
            pass

        def send_json(self, code, value=None):
            body = b"" if value is None else json.dumps(value).encode()
            self.send_response(code)
            self.send_header("Content-Type", "application/json")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)

        def do_GET(self):
            if self.path == "/":
                body = PAGE.replace("%WS%", json.dumps(state.workspace)).encode()
                self.send_response(200)
                self.send_header("Content-Type", "text/html; charset=utf-8")
                self.send_header("Content-Length", str(len(body)))
                self.end_headers()
                self.wfile.write(body)
            elif self.path == "/site" or self.path == "/site2":
                body = (SITE if self.path == "/site" else SITE2).encode()
                self.send_response(200)
                self.send_header("Content-Type", "text/html; charset=utf-8")
                self.send_header("Content-Length", str(len(body)))
                self.end_headers()
                self.wfile.write(body)
            elif self.path == "/bridge/poll":
                if self.headers.get("Authorization") != "Bearer device-secret":
                    return self.send_json(401, {})
                deadline = time.time() + 3
                while time.time() < deadline:
                    with state.lock:
                        if state.calls and state.ready.is_set():
                            return self.send_json(200, {"call": state.calls.pop(0)})
                    time.sleep(0.05)
                self.send_json(204)
            else:
                self.send_json(404, {})

        def do_POST(self):
            size = int(self.headers.get("Content-Length", 0))
            data = json.loads(self.rfile.read(size) or b"{}")
            if self.path == "/report":
                with state.lock:
                    state.reports.append((data["name"], data["value"]))
                if data["name"] == "ready":
                    state.ready.set()
                self.send_json(200, {})
            elif self.path == "/bridge/devices":
                if self.headers.get("Authorization") != "Bearer user-token":
                    return self.send_json(401, {"message": "bad token"})
                state.linked = True
                self.send_json(200, {"device": {"id": 7}, "secret": "device-secret"})
            elif self.path == "/bridge/result":
                with state.lock:
                    state.results[data["call_id"]] = data
                    if len(state.results) == state.expected:
                        state.done.set()
                self.send_json(200, {})
            else:
                self.send_json(404, {})

    return Handler


def call(cid, tool, args, ws):
    return {"id": cid, "tool": tool, "args": args, "chat": "chat1", "workspace": ws}


def main():
    binary = os.path.abspath(sys.argv[1])
    root = tempfile.mkdtemp(prefix="salam-agent-test-")
    workspace = os.path.realpath(os.path.join(root, "work"))
    home = os.path.join(root, "home")
    os.makedirs(workspace)
    os.makedirs(home)
    with open(os.path.join(workspace, "a.txt"), "w") as f:
        f.write("hello world\nسلام\n")
    with open(os.path.join(workspace, "AGENTS.md"), "w") as f:
        f.write("be kind\n")
    with open(os.path.join(workspace, ".gitignore"), "w") as f:
        f.write("skipped/\n*.log\n")
    os.makedirs(os.path.join(workspace, "skipped"))
    with open(os.path.join(workspace, "skipped", "b.txt"), "w") as f:
        f.write("hello hidden\n")
    with open(os.path.join(workspace, "x.log"), "w") as f:
        f.write("hello log\n")
    outside = os.path.join(root, "outside.txt")
    with open(outside, "w") as f:
        f.write("secret\n")
    os.symlink(root, os.path.join(workspace, "escape"))
    with open(os.path.join(workspace, "Makefile"), "w") as f:
        f.write("test:\n\techo made-test\n")
    with open(os.path.join(workspace, "big.bin"), "wb") as f:
        f.write(b"\0\1\2binary")

    state = State(workspace)
    state.calls = [
        call("read", "read_file", {"path": "a.txt"}, workspace),
        call("list", "list_dir", {"path": "."}, workspace),
        call("glob", "glob", {"pattern": "**/*.txt"}, workspace),
        call("grep", "grep", {"pattern": "HELLO", "ignore_case": True}, workspace),
        call("shell", "shell", {"command": "echo out; echo err >&2; pwd", "shell": "sh"}, workspace),
        call("write", "write_file", {"path": "new/dir/b.txt", "content": "made\n"}, workspace),
        call("edit", "edit_file", {"path": "a.txt", "old_text": "world", "new_text": "salam"}, workspace),
        call("git", "write_file", {"path": ".git/config", "content": ""}, workspace),
        call("up", "read_file", {"path": "../outside.txt"}, workspace),
        call("save", "save_memory", {"title": "Build steps", "description": "how", "content": "run it"}, workspace),
        call("forget", "forget_memory", {"slug": "build-steps"}, workspace),
        call("badslug", "forget_memory", {"slug": "../x"}, workspace),
        call("fail", "shell", {"command": "exit 3", "shell": "sh"}, workspace),
        call("missing", "nope", {}, workspace),
        call("symlink", "read_file", {"path": "escape/outside.txt"}, workspace),
        call("binary", "read_file", {"path": "big.bin"}, workspace),
        call("nofolder", "read_file", {"path": "a.txt"}, os.path.join(root, "no-such-folder")),
        call("tests", "run_tests", {"shell": "sh"}, workspace),
        call("loud", "shell", {"command": "yes 0123456789 | head -c 200000", "shell": "sh"}, workspace),
        call("slow", "shell", {"command": "sleep 30 & sleep 30; echo never", "shell": "sh", "timeout": 1}, workspace),
        call("stopme", "shell", {"command": "echo started-stopme; sleep 30", "shell": "sh"}, workspace),
        call("noprog", "shell", {"command": "exit 0", "shell": "no-such-shell-xyz"}, workspace),
        call("docker", "docker", {"args": ["ps", "a b"], "cwd": "."}, workspace),
        call("wsl", "wsl", {"command": "ls"}, workspace),
    ]
    server = ThreadingHTTPServer(("127.0.0.1", 0), make_handler(state))
    port = server.server_address[1]
    threading.Thread(target=server.serve_forever, daemon=True).start()
    url = f"http://127.0.0.1:{port}"
    has_browser = bool(os.environ.get("SALAM_AGENT_BROWSER"))
    if has_browser:

        def browse(cid, **args):
            state.calls.append(call("b_" + cid, "browser", args, workspace))

        browse("badact", action="dance")
        browse("badkey", action="press", key="Hyper+x")
        browse("badurl", action="open", url="javascript:alert(1)")
        browse("open", action="open", url=url + "/site")
        browse("text", action="text")
        browse("click", action="click", selector="#btn")
        browse("eval", action="eval", script="document.getElementById('copy').textContent")
        browse("evalret", action="eval", script="return 40 + 2;")
        browse("evalbad", action="eval", script="throw new Error('boom')")
        browse("type", action="type", selector="#name", text="سارا")
        browse("select", action="select", selector="#pick", text="Berry")
        browse("selectbad", action="select", selector="#pick", text="Nope")
        browse("wait", action="wait", text="ready now")
        browse("missing", action="click", selector="#none")
        browse("hover", action="hover", selector="#btn")
        browse("scroll", action="scroll", y=500)
        browse("shot", action="screenshot", full_page=True)
        browse("console", action="console")
        browse("network", action="network")
        browse("next", action="click", selector="#next")
        browse("back", action="back")
        browse("reload", action="reload")
        browse("small", action="screenshot", width=400, height=600)
        browse("close", action="close")
        browse("closed", action="close")
    state.expected = len(state.calls)
    env = dict(os.environ, HOME=home, XDG_DATA_HOME=os.path.join(home, "data"), SALAM_AGENT_DEBUG="1", SALAM_APP_URL=url + "/", SALAM_API_URL=url)
    log_path = os.path.join(root, "agent.log")
    with open(log_path, "wb") as log:
        proc = subprocess.Popen([binary], env=env, stdout=log, stderr=log)
    failures = []

    def check(name, ok, detail=""):
        print(("ok   " if ok else "FAIL ") + name + ("" if ok else ": " + str(detail)[:400]))
        if not ok:
            failures.append(name)

    try:
        finished = state.done.wait(240)
        check("every call answered", finished, sorted(state.results))
        reports = dict(state.reports)
        info = reports.get("info", {})
        check("info", info.get("device") is None and info.get("os") and "version" in info, info)
        link = reports.get("link", {})
        check("link", state.linked and link.get("device") and link["device"]["id"] == "7", link)
        check("prefs default", reports.get("prefs", {}).get("lang") == "fa", reports.get("prefs"))
        check("prefs set", reports.get("setprefs", {}).get("lang") == "en", reports.get("setprefs"))
        check("watch rejects a bad chat", reports.get("badwatch") == "invalid chat id", reports.get("badwatch"))
        ctx = reports.get("context", {})
        check("project context", ctx.get("instructions_file") == "AGENTS.md" and "be kind" in ctx.get("instructions", ""), ctx)
        check("no page exception", "exception" not in reports, reports.get("exception"))
        r = state.results
        get = lambda k: r.get(k, {})
        check("read", get("read").get("status") == "ok" and "سلام" in get("read").get("output", ""), get("read"))
        check("list", "a.txt" in get("list").get("output", ""), get("list"))
        check("glob skips gitignored", "a.txt" in get("glob").get("output", "") and "skipped" not in get("glob").get("output", ""), get("glob"))
        grep = get("grep").get("output", "")
        check("grep ignores case and gitignore", "a.txt:1:" in grep and "hidden" not in grep and "x.log" not in grep, grep)
        shell = get("shell")
        check("shell runs after approval", shell.get("status") == "ok" and "out" in shell.get("output", "") and "err" in shell.get("output", "") and workspace in shell.get("output", ""), shell)
        check("write denied when asked", get("write").get("status") == "denied", get("write"))
        check("edit denied, diff previewed", get("edit").get("status") == "denied" and "+hello salam" in get("edit").get("detail", "") and "-hello world" in get("edit").get("detail", ""), get("edit"))
        check("git refused", get("git").get("status") == "error", get("git"))
        check("outside asks and is denied", get("up").get("status") == "denied" and get("up").get("outside") is True, get("up"))
        check("memory saved without asking", get("save").get("status") == "ok" and "build-steps" in get("save").get("output", ""), get("save"))
        check("memory forgotten", get("forget").get("status") == "ok", get("forget"))
        check("bad slug refused", get("badslug").get("status") == "error", get("badslug"))
        check("exit code reported", get("fail").get("status") == "error" and "exit code 3" in get("fail").get("output", ""), get("fail"))
        check("unknown tool", get("missing").get("status") == "error", get("missing"))
        names = [n for n, _ in state.reports]
        check("call events", "call:shell:pending" in names and "call:shell:done" in names, names)
        check("output streamed", "output" in names, names)
        check("connected", any(n == "connection" and v is True for n, v in state.reports), names)
        if has_browser:
            out = lambda k: get("b_" + k).get("output", "")
            st = lambda k: get("b_" + k).get("status")
            check("browser refuses a bad action", st("badact") == "error", get("b_badact"))
            check("browser refuses a bad key", st("badkey") == "error", get("b_badkey"))
            check("browser refuses javascript links", st("badurl") == "error", get("b_badurl"))
            check("browser open", st("open") == "ok" and "(200)" in out("open") and '"site"' in out("open"), out("open"))
            check("browser open sends a screenshot", len(get("b_open").get("image", "")) > 1000 and get("b_open").get("image_mime") == "image/jpeg", get("b_open").get("image_mime"))
            check("browser reports the 404 image", "missing.png" in out("open") or "missing.png" in out("text"), out("open"))
            check("browser text outline", "سلام from the site" in out("text") and "Site heading" in out("text") and "Second page" in out("text") and "Press me" in out("text"), out("text"))
            check("browser click", st("click") == "ok" and "button" in out("click"), out("click"))
            check("browser click ran the page", "clicked" in out("eval"), out("eval"))
            check("browser eval asks first", "call:browser:pending" in names, names)
            check("browser eval accepts return", "42" in out("evalret"), out("evalret"))
            check("browser eval reports errors", st("evalbad") == "error" and "boom" in out("evalbad"), out("evalbad"))
            check("browser type", st("type") == "ok" and "سارا" in out("type"), out("type"))
            check("browser select", "Berry" in out("select"), out("select"))
            check("browser select lists options", st("selectbad") == "error" and "Apple" in out("selectbad"), out("selectbad"))
            check("browser wait", "Found" in out("wait"), out("wait"))
            check("browser missing selector", st("missing") == "error" and "#none" in out("missing"), out("missing"))
            check("browser hover", st("hover") == "ok", out("hover"))
            check("browser scroll", "y=500" in out("scroll"), out("scroll"))
            check("browser full page shot", "x" in out("shot") and len(get("b_shot").get("image", "")) > 1000, out("shot"))
            check("browser console", "hello log" in out("console") and "warned" in out("console"), out("console"))
            check("browser network", "/site" in out("network") and "GET 200" in out("network"), out("network"))
            check("browser follows a link", '"second"' in out("next"), out("next"))
            check("browser back", '"site"' in out("back"), out("back"))
            check("browser reload", st("reload") == "ok", out("reload"))
            check("browser viewport", "400x600" in out("small"), out("small"))
            check("browser close", "Closed" in out("close"), out("close"))
            check("browser closes once", "No browser" in out("closed"), out("closed"))
            left = subprocess.run(["sh", "-c", "ls -d /tmp/salam-agent-browser-* 2>/dev/null | wc -l"], capture_output=True, text=True).stdout.strip()
            check("browser profile removed", left == "0", left)
        check("symlink out is treated as outside", get("symlink").get("status") == "denied" and get("symlink").get("outside") is True, get("symlink"))
        check("binary refused", get("binary").get("status") == "error" and "binary" in get("binary").get("output", ""), get("binary"))
        check("missing workspace explained", get("nofolder").get("status") == "error" and "does not exist" in get("nofolder").get("output", ""), get("nofolder"))
        check("run_tests finds make", "make test" in get("tests").get("detail", ""), get("tests"))
        loud = get("loud").get("output", "")
        check("output capped head and tail", "bytes cut" in loud and len(loud) < 32000, len(loud))
        check("timeout kills the tree", get("slow").get("status") == "error" and "timed out" in get("slow").get("output", "") and "never" not in get("slow").get("output", ""), get("slow"))
        check("stop kills a running command", "stopped by the user" in get("stopme").get("output", "") and "started-stopme" in get("stopme").get("output", ""), get("stopme"))
        check("docker quotes its preview", "'a b'" in get("docker").get("detail", ""), get("docker"))
        check("wsl refused off Windows", sys.platform == "win32" or get("wsl").get("status") == "error", get("wsl"))
        check("files untouched by denied calls", not os.path.exists(os.path.join(workspace, "new")) and "hello world" in open(os.path.join(workspace, "a.txt"), encoding="utf-8").read())
    finally:
        proc.terminate()
        try:
            proc.wait(5)
        except subprocess.TimeoutExpired:
            proc.kill()
        server.shutdown()
    if failures:
        print(open(log_path, errors="replace").read()[-2000:])
    shutil.rmtree(root, ignore_errors=True)
    print(f"{len(failures)} failed")
    return 1 if failures else 0


if __name__ == "__main__":
    sys.exit(main())
