#!/usr/bin/env python3
"""Compile every code snippet marked class="snip" in the lessons.

The reference tables of the web lessons carry hundreds of short examples. Each one
sits in an element with class "snip" (<code class="snip"> in a table cell,
<pre class="code snip"> for a block). A snippet is an element expression: it is
wrapped in a page: block (and gets an import for dom when it calls dom) and built
with `salam page build`. A snippet that already starts a page: block is built as
it is.

usage: SALAM=path/to/salam python3 website/check-snippets.py [lesson.txt ...]
"""

import html
import os
import re
import shutil
import subprocess
import sys
import tempfile
from concurrent.futures import ThreadPoolExecutor

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
SALAM = os.environ.get("SALAM", "salam")
STD = os.environ.get("SALAM_STD", os.path.join(ROOT, "std"))

SNIP = re.compile(
    r'<(code|pre)\b[^>]*\bclass="[^"]*\bsnip\b[^"]*"[^>]*>(.*?)</\1>', re.DOTALL
)
BODY = re.compile(r"^lesson\.body <<\n(.*?)^>>\s*$", re.DOTALL | re.MULTILINE)

WORDS = {
    "en": ("page:", "end", "import dom"),
    "fa": ("صفحه:", "پایان", "واردسازی دام"),
}


def snippets(path):
    with open(path, encoding="utf-8") as f:
        text = f.read()
    m = BODY.search(text)
    if not m:
        return []
    out = []
    for s in SNIP.finditer(m.group(1)):
        inner = re.sub(r"^\s*<code>|</code>\s*$", "", s.group(2))
        inner = re.sub(r"<[^>]+>", "", inner)
        out.append(html.unescape(inner).strip("\n"))
    return out


def wrap(src, lang):
    page, end, imp = WORDS[lang]
    has_import = re.search(r"^(import|واردسازی)\s+(dom|دام)\b", src, re.MULTILINE)
    needs_dom = re.search(r"\b(dom|دام)\.", src)
    prefix = imp + "\n\n" if needs_dom and not has_import else ""
    if re.search(r"^(page|صفحه):\s*$", src, re.MULTILINE):
        return prefix + src + "\n"
    body = "\n".join(
        ("    " + line if line.strip() else line) for line in src.split("\n")
    )
    return f"{prefix}{page}\n{body}\n{end}\n"


def build(job):
    path, lang, src = job
    work = tempfile.mkdtemp(prefix="snip-")
    try:
        with open(os.path.join(work, "snip.salam"), "w", encoding="utf-8") as f:
            f.write(wrap(src, lang))
        env = dict(os.environ, SALAM_STD=STD)
        r = subprocess.run(
            [
                SALAM,
                "page",
                "build",
                "snip.salam",
                "--inline",
                "--no-color",
                "--log-level=warn",
            ],
            cwd=work,
            env=env,
            capture_output=True,
            check=False,
            text=True,
            timeout=180,
        )
        out = (r.stdout + r.stderr).strip()
        noise = out.lower().replace("0 error", "").replace("0 warning", "")
        ok = (
            r.returncode == 0
            and os.path.exists(os.path.join(work, "snip.html"))
            and not re.search(r"\b(error|warning)\b", noise)
        )
        return ok, path, src, out
    finally:
        shutil.rmtree(work, ignore_errors=True)


def main():
    files = sys.argv[1:]
    if not files:
        base = os.path.join(ROOT, "website", "content")
        for lang in ("en", "fa"):
            d = os.path.join(base, lang, "learn")
            files += [
                os.path.join(d, n) for n in sorted(os.listdir(d)) if n.endswith(".txt")
            ]
    jobs = []
    for path in files:
        lang = "fa" if "/fa/" in path.replace("\\", "/") else "en"
        jobs += [(path, lang, s) for s in snippets(path)]
    workers = int(os.environ.get("CHECK_JOBS", str(os.cpu_count() or 4)))
    with ThreadPoolExecutor(max_workers=workers) as pool:
        results = list(pool.map(build, jobs))
    bad = [r for r in results if not r[0]]
    for _, path, src, out in bad[:20]:
        print(f"FAIL {os.path.relpath(path, ROOT)}\n{src}\n--> {out[:300]}\n")
    print(f"snippets: {len(jobs)} checked, {len(bad)} failed")
    sys.exit(1 if bad else 0)


if __name__ == "__main__":
    main()
