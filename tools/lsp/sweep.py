#!/usr/bin/env python3
"""Robustness sweep for the `salam lsp --analyze` worker.

Runs the worker on every .salam file under the given directories (default:
tests/ and std/) and reports each file where it crashes, times out or prints
a snapshot that is not valid JSON. It does not judge the answers; the end to
end tests do that.

    python3 tools/lsp/sweep.py path/to/salam [dir ...] [-j N] [--limit N]
"""

import argparse
import concurrent.futures
import json
import os
import subprocess
import sys
import tempfile

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))


def files_under(dirs, limit):
    out = []
    for d in dirs:
        if os.path.isfile(d):
            out.append(os.path.abspath(d))
            continue
        for base, subdirs, names in os.walk(d):
            subdirs[:] = sorted(s for s in subdirs if not s.startswith("."))
            for n in sorted(names):
                if n.endswith(".salam"):
                    out.append(os.path.join(base, n))
    if limit and len(out) > limit:
        step = len(out) / float(limit)
        out = [out[int(i * step)] for i in range(limit)]
    return out


def run_one(exe, path, timeout):
    with tempfile.TemporaryDirectory() as tmp:
        req = os.path.join(tmp, "req.json")
        snap = os.path.join(tmp, "snap.json")
        try:
            text = open(path, encoding="utf-8").read()
        except (OSError, UnicodeDecodeError) as e:
            return path, "unreadable: %s" % e
        with open(req, "w", encoding="utf-8") as f:
            json.dump({"path": path, "text": text, "stdlib": os.path.join(ROOT, "std"), "out": snap}, f)
        try:
            p = subprocess.run([exe, "lsp", "--analyze", req], capture_output=True, timeout=timeout)
        except subprocess.TimeoutExpired:
            return path, "timeout after %ds" % timeout
        if p.returncode != 0:
            err = (p.stderr.decode("utf-8", "replace").strip().splitlines() or [""])[-1]
            return path, "exit %d: %s" % (p.returncode, err[:200])
        try:
            data = json.load(open(snap, encoding="utf-8"))
        except (OSError, ValueError) as e:
            return path, "bad snapshot: %s" % e
        wrong = mismatched_refs(text, data)
        if wrong:
            return path, "%d reference(s) point at a name they do not spell, first: %s" % (len(wrong), wrong[0])
        return path, ""


def squash(s):
    return s.replace("\u200c", "").replace(" ", "").replace("\u064a", "\u06cc").replace("\u0643", "\u06a9")


def mismatched_refs(text, data):
    lines = text.split("\n")
    out = []
    decls = data.get("decls", [])
    for r in data.get("refs", []):
        l1, c1, l2, c2 = r["r"]
        if l1 != l2 or l1 >= len(lines):
            continue
        units = lines[l1].encode("utf-16-le")
        spelled = units[c1 * 2:c2 * 2].decode("utf-16-le", "replace")
        d = decls[r["d"]] if r["d"] < len(decls) else {}
        name = d.get("name", "")
        doc = d.get("doc", "")
        if squash(spelled) == squash(name) or ("`" + squash(spelled) + "`") in squash(doc):
            continue
        if name.startswith(spelled + "_") or name.endswith("_" + spelled):
            continue
        out.append("%d:%d '%s' -> %s" % (l1 + 1, c1 + 1, spelled, name))
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("salam")
    ap.add_argument("dirs", nargs="*")
    ap.add_argument("-j", type=int, default=os.cpu_count() or 4)
    ap.add_argument("--limit", type=int, default=0)
    ap.add_argument("--timeout", type=int, default=120)
    a = ap.parse_args()
    exe = os.path.abspath(a.salam)
    dirs = a.dirs or [os.path.join(ROOT, "tests"), os.path.join(ROOT, "std")]
    files = files_under(dirs, a.limit)
    bad = []
    with concurrent.futures.ThreadPoolExecutor(max_workers=a.j) as pool:
        for path, problem in pool.map(lambda f: run_one(exe, f, a.timeout), files):
            if problem:
                bad.append((path, problem))
                print("FAIL %s: %s" % (os.path.relpath(path, ROOT), problem), flush=True)
    print("RESULT: %d files, %d failed" % (len(files), len(bad)))
    sys.exit(1 if bad else 0)


if __name__ == "__main__":
    main()
