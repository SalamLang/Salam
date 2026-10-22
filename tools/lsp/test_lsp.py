#!/usr/bin/env python3
"""End-to-end tests for `salam lsp`.

Starts the server over stdio, drives it like an editor would, and checks the
answers against the fixtures in tests/lsp. Uses only the standard library.

    python3 tools/lsp/test_lsp.py [path/to/salam]
"""

# pylint: disable=too-many-locals,too-many-statements,too-few-public-methods,consider-using-with

import json
import os
import queue
import subprocess
import sys
import threading
import time

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
STD = os.path.join(ROOT, "std")
EN = os.path.join(ROOT, "tests", "lsp", "en", "shapes.salam")
FA = os.path.join(ROOT, "tests", "lsp", "fa", "binary.salam")
PKG = os.path.join(ROOT, "std", "str", "basic.salam")
PROJ = os.path.join(ROOT, "tests", "lsp", "proj")
IFACE = os.path.join(ROOT, "tests", "lsp", "en", "iface.salam")


def read_text(path):
    """Read a UTF-8 file."""
    with open(path, encoding="utf-8") as f:
        return f.read()


class Server:
    def __init__(self, exe, extra):
        self.proc = subprocess.Popen(
            [exe, "lsp"] + extra,
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.DEVNULL,
        )
        self.inbox = queue.Queue()
        self.notes = []
        self.next_id = 1
        threading.Thread(target=self._read, daemon=True).start()

    def _read(self):
        out = self.proc.stdout
        while True:
            head = b""
            while not head.endswith(b"\r\n\r\n"):
                c = out.read(1)
                if not c:
                    self.inbox.put(None)
                    return
                head += c
            n = 0
            for line in head.decode("ascii").split("\r\n"):
                if line.lower().startswith("content-length:"):
                    n = int(line.split(":", 1)[1])
            self.inbox.put(json.loads(out.read(n)))

    def _send(self, msg):
        body = json.dumps(msg).encode("utf-8")
        self.proc.stdin.write(b"Content-Length: %d\r\n\r\n" % len(body) + body)
        self.proc.stdin.flush()

    def _next(self, deadline):
        m = self.inbox.get(timeout=max(0.05, deadline - time.time()))
        if m is None:
            raise RuntimeError("the server exited")
        return m

    def notify(self, method, params):
        self._send({"jsonrpc": "2.0", "method": method, "params": params})

    def request(self, method, params, timeout=120):
        rid = self.next_id
        self.next_id += 1
        self._send({"jsonrpc": "2.0", "id": rid, "method": method, "params": params})
        deadline = time.time() + timeout
        while True:
            m = self._next(deadline)
            if m.get("id") == rid and "method" not in m:
                return m
            self.notes.append(m)

    def diagnostics(self, uri, timeout=120):
        deadline = time.time() + timeout
        while True:
            for i, m in enumerate(self.notes):
                if (
                    m.get("method") == "textDocument/publishDiagnostics"
                    and m["params"]["uri"] == uri
                ):
                    return self.notes.pop(i)["params"]
            self.notes.append(self._next(deadline))


def uri_of(path):
    return "file://" + path


def utf16_len(s):
    return len(s.encode("utf-16-le")) // 2


def pos(text, needle, shift=0, nth=0):
    found = -1
    for _ in range(nth + 1):
        found = text.index(needle, found + 1)
    found += shift
    line = text.count("\n", 0, found)
    start = text.rfind("\n", 0, found) + 1
    return {"line": line, "character": utf16_len(text[start:found])}


class Run:
    def __init__(self):
        self.failed = 0
        self.passed = 0

    def check(self, name, ok, detail=""):
        if ok:
            self.passed += 1
            print("PASS " + name)
        else:
            self.failed += 1
            print("FAIL " + name + (": " + detail if detail else ""))


def at(uri, p):
    return {"textDocument": {"uri": uri}, "position": p}


def labels(result):
    items = result["items"] if isinstance(result, dict) else result
    return [i["label"] for i in items]


def english(t, s):
    text = read_text(EN)
    uri = uri_of(EN)
    s.notify(
        "textDocument/didOpen",
        {
            "textDocument": {
                "uri": uri,
                "languageId": "salam",
                "version": 1,
                "text": text,
            }
        },
    )
    d = s.diagnostics(uri)
    t.check(
        "en: a clean file has no diagnostics",
        d["diagnostics"] == [],
        json.dumps(d["diagnostics"])[:300],
    )

    h = s.request("textDocument/hover", at(uri, pos(text, "make(3)", 1)))["result"]
    t.check(
        "en: hover shows the signature",
        h and "func make(n: int): Point" in h["contents"]["value"],
        str(h)[:300],
    )

    h = s.request("textDocument/hover", at(uri, pos(text, "FromInt", 2)))["result"]
    v = h["contents"]["value"] if h else ""
    t.check(
        "en: hover on a std function",
        "FromInt" in v and "standard library" in v,
        v[:300],
    )

    d = s.request("textDocument/definition", at(uri, pos(text, "make(3)", 1)))["result"]
    want = pos(text, "func make", 5)
    t.check(
        "en: definition in the same file",
        d and d["uri"] == uri and d["range"]["start"] == want,
        str(d),
    )

    d = s.request("textDocument/definition", at(uri, pos(text, "FromInt", 2)))["result"]
    t.check(
        "en: definition into std",
        d and d["uri"].endswith("/std/str/basic.salam"),
        str(d),
    )

    d = s.request("textDocument/typeDefinition", at(uri, pos(text, "p := make", 0)))[
        "result"
    ]
    t.check(
        "en: type definition of a local",
        d and d["range"]["start"] == pos(text, "struct Point", 7),
        str(d),
    )

    r = s.request(
        "textDocument/references",
        {**at(uri, pos(text, "x: int", 0)), "context": {"includeDeclaration": True}},
    )["result"]
    t.check("en: references of a field", len(r) == 4, str(len(r)))

    hl = s.request(
        "textDocument/documentHighlight", at(uri, pos(text, "p := make", 0))
    )["result"]
    t.check("en: highlight of a local", len(hl) == 3, str(len(hl)))

    sy = s.request("textDocument/documentSymbol", {"textDocument": {"uri": uri}})[
        "result"
    ]
    names = [x["name"] for x in sy]
    kids = [c["name"] for c in sy[0].get("children", [])] if sy else []
    t.check(
        "en: outline",
        names == ["Point", "make", "label", "main"] and kids == ["x", "y", "sum"],
        str(names) + str(kids),
    )

    f = s.request("textDocument/foldingRange", {"textDocument": {"uri": uri}})["result"]
    t.check("en: folding ranges", len(f) >= 5, str(len(f)))

    st = s.request("textDocument/semanticTokens/full", {"textDocument": {"uri": uri}})[
        "result"
    ]["data"]
    t.check("en: semantic tokens", len(st) > 0 and len(st) % 5 == 0, str(len(st)))

    sig = s.request("textDocument/signatureHelp", at(uri, pos(text, '"sum="', 0)))[
        "result"
    ]
    ok = (
        sig
        and sig["signatures"][0]["label"].startswith("func label(")
        and sig["activeParameter"] == 1
    )
    t.check("en: signature help", ok, str(sig)[:300])

    lines = text.split("\n")
    li = lines.index('    println label(p, "sum="), p.x')
    edit = {
        "range": {
            "start": {"line": li, "character": 4},
            "end": {"line": li, "character": 4},
        },
        "text": "p.\n    ",
    }
    s.notify(
        "textDocument/didChange",
        {"textDocument": {"uri": uri, "version": 2}, "contentChanges": [edit]},
    )
    d = s.diagnostics(uri)
    t.check(
        "en: an edit is analyzed and versioned",
        d["version"] == 2 and len(d["diagnostics"]) > 0,
        json.dumps(d)[:300],
    )
    c = s.request("textDocument/completion", at(uri, {"line": li, "character": 6}))[
        "result"
    ]
    t.check(
        "en: member completion",
        {"x", "y", "sum"} <= set(labels(c)),
        str(labels(c))[:300],
    )

    undo = {
        "range": {
            "start": {"line": li, "character": 4},
            "end": {"line": li + 1, "character": 4},
        },
        "text": "",
    }
    s.notify(
        "textDocument/didChange",
        {"textDocument": {"uri": uri, "version": 3}, "contentChanges": [undo]},
    )
    d = s.diagnostics(uri)
    t.check(
        "en: undoing the edit clears the errors",
        d["version"] == 3 and d["diagnostics"] == [],
        json.dumps(d)[:300],
    )

    c = s.request("textDocument/completion", at(uri, pos(text, "FromInt", 0)))["result"]
    t.check(
        "en: package member completion",
        "FromInt" in labels(c) and "Len" in labels(c),
        str(labels(c))[:200],
    )

    c = s.request("textDocument/completion", at(uri, pos(text, "p := make", 0)))[
        "result"
    ]
    got = set(labels(c))
    t.check(
        "en: scope completion",
        {"make", "label", "Point", "each", "until"} <= got,
        str(sorted(got))[:300],
    )

    r = s.request(
        "textDocument/rename",
        {**at(uri, pos(text, "func make", 6)), "newName": "build"},
    )["result"]
    edits = r["changes"][uri] if r else []
    t.check(
        "en: rename",
        len(edits) == 2 and all(e["newText"] == "build" for e in edits),
        str(r)[:300],
    )

    r = s.request(
        "textDocument/rename", {**at(uri, pos(text, "func make", 6)), "newName": "each"}
    )
    t.check("en: rename refuses a keyword", "error" in r, str(r)[:200])

    bad = text.replace("p.sum()", "p.summ()")
    s.notify(
        "textDocument/didChange",
        {"textDocument": {"uri": uri, "version": 4}, "contentChanges": [{"text": bad}]},
    )
    d = s.diagnostics(uri)
    codes = [x.get("code") for x in d["diagnostics"]]
    t.check(
        "en: a type error is reported with its code",
        "E017" in codes,
        json.dumps(d["diagnostics"])[:300],
    )
    s.notify(
        "textDocument/didChange",
        {
            "textDocument": {"uri": uri, "version": 5},
            "contentChanges": [{"text": text}],
        },
    )
    s.diagnostics(uri)

    messy = text.replace("    ret p\n", "        ret p\n")
    s.notify(
        "textDocument/didChange",
        {
            "textDocument": {"uri": uri, "version": 6},
            "contentChanges": [{"text": messy}],
        },
    )
    s.diagnostics(uri)
    fm = s.request(
        "textDocument/formatting",
        {"textDocument": {"uri": uri}, "options": {"tabSize": 4, "insertSpaces": True}},
    )["result"]
    t.check(
        "en: formatting returns a minimal edit",
        fm and len(fm) == 1 and fm[0]["newText"] == "    ret p\n",
        str(fm)[:300],
    )

    s.notify(
        "textDocument/didChange",
        {
            "textDocument": {"uri": uri, "version": 7},
            "contentChanges": [{"text": text}],
        },
    )
    s.diagnostics(uri)
    ih = s.request(
        "textDocument/inlayHint",
        {
            "textDocument": {"uri": uri},
            "range": {
                "start": {"line": 0, "character": 0},
                "end": {"line": 999, "character": 0},
            },
        },
    )["result"]
    got = sorted((h["position"]["line"], h["label"]) for h in ih)
    want = sorted(
        [
            (pos(text, "p := Point")["line"], ": Point"),
            (pos(text, "p := make")["line"], ": Point"),
        ]
    )
    t.check("en: inlay hints show inferred types", got == want, str(got))

    w = s.request("workspace/symbol", {"query": "lab"})["result"]
    t.check("en: workspace symbols", [x["name"] for x in w] == ["label"], str(w)[:200])


def persian(t, s):
    text = read_text(FA)
    uri = uri_of(FA)
    s.notify(
        "textDocument/didOpen",
        {
            "textDocument": {
                "uri": uri,
                "languageId": "salam",
                "version": 1,
                "text": text,
            }
        },
    )
    d = s.diagnostics(uri)
    t.check(
        "fa: a clean file has no diagnostics",
        d["diagnostics"] == [],
        json.dumps(d["diagnostics"], ensure_ascii=False)[:300],
    )

    p = pos(text, "دودویی(عدد", 2)
    h = s.request("textDocument/hover", at(uri, p))["result"]
    t.check(
        "fa: hover after Persian text",
        h and "تابع دودویی(ن: صحیح, پهنا: صحیح): رشته" in h["contents"]["value"],
        str(h)[:300],
    )
    want = pos(text, "دودویی(ن", 0)
    t.check(
        "fa: hover range in UTF-16",
        h and h["range"]["start"] == pos(text, "دودویی(عدد", 0),
        str(h and h["range"]),
    )

    d = s.request("textDocument/definition", at(uri, p))["result"]
    t.check("fa: definition", d and d["range"]["start"] == want, str(d))

    r = s.request(
        "textDocument/references",
        {
            **at(uri, pos(text, "عدد اول :=", 1)),
            "context": {"includeDeclaration": True},
        },
    )["result"]
    t.check("fa: references of a two-word name", len(r) == 3, str(r)[:300])
    if r:
        rg = r[-1]["range"]
        t.check(
            "fa: a two-word name has one range",
            rg["end"]["character"] - rg["start"]["character"] == utf16_len("عدد اول"),
            str(rg),
        )

    sy = s.request("textDocument/documentSymbol", {"textDocument": {"uri": uri}})[
        "result"
    ]
    t.check("fa: outline", [x["name"] for x in sy] == ["دودویی", "ریشه"], str(sy)[:200])

    c = s.request("textDocument/completion", at(uri, pos(text, "سرچاپ", 0)))["result"]
    got = set(labels(c))
    t.check(
        "fa: Persian keywords and names in completion",
        {"اگر", "تابع", "دودویی", "عدد اول"} <= got,
        str(sorted(got))[:300],
    )

    bad = text.replace("دودویی(عدد اول, 8)", "دودوی(عدد اول, 8)")
    s.notify(
        "textDocument/didChange",
        {"textDocument": {"uri": uri, "version": 2}, "contentChanges": [{"text": bad}]},
    )
    d = s.diagnostics(uri)
    msgs = [x["message"] for x in d["diagnostics"]]
    t.check(
        "fa: Persian diagnostics",
        any("دودوی" in m and "تابع" in m for m in msgs),
        json.dumps(msgs, ensure_ascii=False)[:300],
    )


def interfaces(t, s):
    text = read_text(IFACE)
    uri = uri_of(IFACE)
    s.notify(
        "textDocument/didOpen",
        {
            "textDocument": {
                "uri": uri,
                "languageId": "salam",
                "version": 1,
                "text": text,
            }
        },
    )
    s.diagnostics(uri)
    impl = (
        s.request(
            "textDocument/implementation", at(uri, pos(text, "interface Shape", 11))
        )["result"]
        or []
    )
    lines = sorted(x["range"]["start"]["line"] for x in impl)
    want = sorted([pos(text, "struct Sq")["line"], pos(text, "struct Rect")["line"]])
    t.check("iface: implementations of an interface", lines == want, str(impl)[:300])
    impl = (
        s.request("textDocument/implementation", at(uri, pos(text, "func Area", 6)))[
            "result"
        ]
        or []
    )
    lines = sorted(x["range"]["start"]["line"] for x in impl)
    want = sorted(
        [pos(text, "func Area", 0, 1)["line"], pos(text, "func Area", 0, 2)["line"]]
    )
    t.check(
        "iface: implementations of an interface method", lines == want, str(impl)[:300]
    )
    item = s.request(
        "textDocument/prepareTypeHierarchy", at(uri, pos(text, "interface Shape", 11))
    )["result"]
    subs = (
        s.request("typeHierarchy/subtypes", {"item": item[0]})["result"] if item else []
    )
    t.check(
        "iface: subtypes",
        sorted(x["name"] for x in subs) == ["Rect", "Sq"],
        str(subs)[:300],
    )
    item = s.request(
        "textDocument/prepareTypeHierarchy", at(uri, pos(text, "struct Sq", 8))
    )["result"]
    sups = (
        s.request("typeHierarchy/supertypes", {"item": item[0]})["result"]
        if item
        else []
    )
    t.check(
        "iface: supertypes", [x["name"] for x in sups] == ["Shape"], str(sups)[:300]
    )


def package(t, s):
    text = read_text(PKG)
    uri = uri_of(PKG)
    s.notify(
        "textDocument/didOpen",
        {
            "textDocument": {
                "uri": uri,
                "languageId": "salam",
                "version": 1,
                "text": text,
            }
        },
    )
    d = s.diagnostics(uri)
    t.check(
        "pkg: a file of a folder package checks with its siblings",
        d["diagnostics"] == [],
        json.dumps(d["diagnostics"])[:300],
    )
    sy = s.request("textDocument/documentSymbol", {"textDocument": {"uri": uri}})[
        "result"
    ]
    t.check("pkg: outline", "Len" in [x["name"] for x in sy], str(sy)[:200])


def project(t, exe):
    s = Server(exe, ["--stdlib-path=" + STD])
    s.request(
        "initialize",
        {
            "processId": None,
            "rootUri": uri_of(PROJ),
            "capabilities": {},
            "clientInfo": {"name": "test"},
        },
    )
    s.notify("initialized", {})
    upath = os.path.join(PROJ, "util.salam")
    mpath = os.path.join(PROJ, "main.salam")
    utext = read_text(upath)
    mtext = read_text(mpath)
    uuri, muri = uri_of(upath), uri_of(mpath)
    s.notify(
        "textDocument/didOpen",
        {
            "textDocument": {
                "uri": uuri,
                "languageId": "salam",
                "version": 1,
                "text": utext,
            }
        },
    )
    s.diagnostics(uuri)
    twice = at(uuri, pos(utext, "Twice", 1))
    refs = []
    deadline = time.time() + 120
    while time.time() < deadline:
        refs = s.request(
            "textDocument/references",
            {**twice, "context": {"includeDeclaration": True}},
        )["result"]
        if len(refs) >= 3:
            break
        time.sleep(0.3)
    t.check(
        "proj: references reach a file that is not open",
        len(refs) == 3 and muri in [r["uri"] for r in refs],
        str(refs)[:300],
    )

    r = s.request("textDocument/rename", {**twice, "newName": "Double"})["result"]
    ch = r["changes"] if r else {}
    t.check(
        "proj: rename edits every file",
        len(ch.get(uuri, [])) == 1 and len(ch.get(muri, [])) == 2,
        str(ch)[:300],
    )

    lens = s.request("textDocument/codeLens", {"textDocument": {"uri": uuri}})["result"]
    t.check(
        "proj: code lens counts references",
        [lens_item["command"]["title"] for lens_item in lens] == ["2 references"],
        str(lens)[:300],
    )

    s.notify(
        "textDocument/didOpen",
        {
            "textDocument": {
                "uri": muri,
                "languageId": "salam",
                "version": 1,
                "text": mtext,
            }
        },
    )
    s.diagnostics(muri)
    links = s.request("textDocument/documentLink", {"textDocument": {"uri": muri}})[
        "result"
    ]
    t.check(
        "proj: include paths are links",
        [link["target"] for link in links]
        == [uuri, uri_of(os.path.join(PROJ, "geo", "geo.salam"))],
        str(links)[:300],
    )

    item = s.request(
        "textDocument/prepareCallHierarchy", at(muri, pos(mtext, "Twice(n)", 1))
    )["result"]
    t.check(
        "proj: call hierarchy item",
        item and item[0]["name"] == "Twice" and item[0]["uri"] == uuri,
        str(item)[:300],
    )
    if item:
        inc = s.request("callHierarchy/incomingCalls", {"item": item[0]})["result"]
        t.check(
            "proj: incoming calls",
            [(c["from"]["name"], len(c["fromRanges"])) for c in inc] == [("run", 2)],
            str(inc)[:300],
        )
    run_item = s.request(
        "textDocument/prepareCallHierarchy", at(muri, pos(mtext, "run(2)", 1))
    )["result"]
    if run_item:
        out = s.request("callHierarchy/outgoingCalls", {"item": run_item[0]})["result"]
        t.check(
            "proj: outgoing calls",
            [(c["to"]["name"], len(c["fromRanges"])) for c in out] == [("Twice", 2)],
            str(out)[:300],
        )
    else:
        t.check("proj: outgoing calls", False, "no item for run")

    inside = set(
        labels(
            s.request(
                "textDocument/completion", at(muri, pos(mtext, "println inner", 0))
            )["result"]
        )
    )
    after = set(
        labels(
            s.request("textDocument/completion", at(muri, pos(mtext, "println x", 0)))[
                "result"
            ]
        )
    )
    t.check(
        "proj: completion sees block scope",
        "inner" in inside and "x" in after and "inner" not in after,
        str(sorted(after))[:200],
    )

    bad = mtext.replace("    if x > 0:", "    while x > 0:")
    s.notify(
        "textDocument/didChange",
        {
            "textDocument": {"uri": muri, "version": 2},
            "contentChanges": [{"text": bad}],
        },
    )
    d = s.diagnostics(muri)
    rng = {"start": pos(bad, "while"), "end": pos(bad, "while", 5)}
    acts = s.request(
        "textDocument/codeAction",
        {
            "textDocument": {"uri": muri},
            "range": rng,
            "context": {"diagnostics": d["diagnostics"]},
        },
    )["result"]
    titles = [a["title"] for a in acts]
    t.check(
        "proj: quick fix for a foreign keyword",
        "Replace 'while' with 'until'" in titles,
        str(titles) + json.dumps(d["diagnostics"])[:200],
    )

    epath = os.path.join(PROJ, "geo", "extra.salam")
    etext = read_text(epath)
    euri = uri_of(epath)
    s.notify(
        "textDocument/didOpen",
        {
            "textDocument": {
                "uri": euri,
                "languageId": "salam",
                "version": 1,
                "text": etext,
            }
        },
    )
    s.diagnostics(euri)
    area = at(euri, pos(etext, "Area", 1))
    deadline = time.time() + 120
    while True:
        r = s.request(
            "textDocument/references", {**area, "context": {"includeDeclaration": True}}
        )["result"]
        if len(r) >= 3 or time.time() > deadline:
            break
        time.sleep(0.3)
    got = sorted(os.path.relpath(uri_to_path(x["uri"]), PROJ) for x in r)
    t.check(
        "proj: references across a folder package",
        got == ["geo/extra.salam", "geo/geo.salam", "main.salam"],
        str(got),
    )

    s.request("shutdown", None)
    s.notify("exit", None)
    s.proc.wait(10)


def uri_to_path(u):
    return u.removeprefix("file://")


def pull_and_colors(t, exe):
    s = Server(exe, ["--stdlib-path=" + STD])
    caps = {"textDocument": {"diagnostic": {"dynamicRegistration": False}}}
    init = s.request(
        "initialize", {"processId": None, "rootUri": None, "capabilities": caps}
    )
    t.check(
        "pull: the server offers pull diagnostics",
        "diagnosticProvider" in init["result"]["capabilities"],
    )
    s.notify("initialized", {})
    text = (
        read_text(EN).replace("p.sum()", "p.summ()") + '\nconst ACCENT := "#ff8800"\n'
    )
    uri = uri_of(os.path.join(ROOT, "tests", "lsp", "en", "pull_probe.salam"))
    s.notify(
        "textDocument/didOpen",
        {
            "textDocument": {
                "uri": uri,
                "languageId": "salam",
                "version": 1,
                "text": text,
            }
        },
    )
    r = s.request("textDocument/diagnostic", {"textDocument": {"uri": uri}})["result"]
    codes = [d.get("code") for d in r.get("items", [])]
    t.check(
        "pull: textDocument/diagnostic reports the error", "E017" in codes, str(r)[:300]
    )
    pushed = [
        m
        for m in s.notes
        if m.get("method") == "textDocument/publishDiagnostics"
        and m["params"]["uri"] == uri
    ]
    t.check("pull: nothing is pushed for an open file", not pushed, str(pushed)[:200])
    cols = s.request("textDocument/documentColor", {"textDocument": {"uri": uri}})[
        "result"
    ]
    ok = len(cols) == 1 and abs(cols[0]["color"]["red"] - 1.0) < 0.01
    t.check("colors: a hex colour is found", ok, str(cols)[:300])
    if cols:
        pres = s.request(
            "textDocument/colorPresentation",
            {
                "textDocument": {"uri": uri},
                "color": cols[0]["color"],
                "range": cols[0]["range"],
            },
        )["result"]
        t.check(
            "colors: presentation", pres and pres[0]["label"] == "#ff8800", str(pres)
        )
    s.request("shutdown", None)
    s.notify("exit", None)
    s.proc.wait(10)


def main():
    exe = (
        os.path.abspath(sys.argv[1])
        if len(sys.argv) > 1
        else os.path.join(ROOT, "salam")
    )
    t = Run()
    s = Server(exe, ["--stdlib-path=" + STD])
    init = s.request(
        "initialize", {"processId": None, "rootUri": uri_of(ROOT), "capabilities": {}}
    )
    caps = init["result"]["capabilities"]
    want = [
        "hoverProvider",
        "definitionProvider",
        "referencesProvider",
        "renameProvider",
        "completionProvider",
        "documentSymbolProvider",
        "semanticTokensProvider",
        "documentFormattingProvider",
        "signatureHelpProvider",
        "inlayHintProvider",
    ]
    t.check(
        "initialize advertises the features",
        all(k in caps for k in want),
        str(sorted(caps)),
    )
    s.notify("initialized", {})
    english(t, s)
    interfaces(t, s)
    package(t, s)
    s.request("shutdown", None)
    s.notify("exit", None)
    t.check("shutdown then exit ends with status 0", s.proc.wait(10) == 0)

    f = Server(exe, ["--stdlib-path=" + STD, "--messages=fa"])
    f.request(
        "initialize", {"processId": None, "rootUri": uri_of(ROOT), "capabilities": {}}
    )
    f.notify("initialized", {})
    persian(t, f)
    f.request("shutdown", None)
    f.notify("exit", None)
    f.proc.wait(10)

    project(t, exe)
    pull_and_colors(t, exe)

    print(f"RESULT: {t.passed} passed, {t.failed} failed")
    sys.exit(1 if t.failed else 0)


if __name__ == "__main__":
    main()
