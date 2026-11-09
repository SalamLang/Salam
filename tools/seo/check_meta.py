#!/usr/bin/env python3
"""Check the SEO and link-preview tags of a built page.

Usage: check_meta.py PAGE.html SITE_ROOT BASE_URL
Every URL under BASE_URL that the page references must exist under SITE_ROOT.
"""
import json
import re
import struct
import sys
from html.parser import HTMLParser
from pathlib import Path


class P(HTMLParser):
    def __init__(self):
        super().__init__()
        self.meta, self.links, self.ld, self.title, self._t, self._l = {}, [], [], "", False, False

    def handle_starttag(self, tag, a):
        a = dict(a)
        if tag == "meta":
            k = a.get("name") or a.get("property")
            if k:
                self.meta.setdefault(k, []).append(a.get("content", ""))
        elif tag == "link":
            self.links.append(a)
        elif tag == "title":
            self._t = True
        elif tag == "script" and a.get("type") == "application/ld+json":
            self._l = True
            self.ld.append("")

    def handle_endtag(self, tag):
        self._t = self._t and tag != "title"
        self._l = self._l and tag != "script"

    def handle_data(self, d):
        if self._t:
            self.title += d
        if self._l:
            self.ld[-1] += d


def png_size(p):
    b = p.read_bytes()[:24]
    return struct.unpack(">II", b[16:24])


def main():
    page, root, base = Path(sys.argv[1]), Path(sys.argv[2]), sys.argv[3].rstrip("/")
    p = P()
    p.feed(page.read_text())
    bad = []
    need = ["description", "viewport", "robots", "theme-color", "color-scheme", "og:type", "og:site_name", "og:title",
            "og:description", "og:url", "og:image", "og:image:width", "og:image:height", "og:image:alt", "og:image:type",
            "og:locale", "twitter:card", "twitter:title", "twitter:description", "twitter:image", "twitter:image:alt"]
    for k in need:
        if k not in p.meta or not all(v.strip() for v in p.meta[k]):
            bad.append("missing meta " + k)
    if not p.title.strip():
        bad.append("missing title")
    rels = {l.get("rel") for l in p.links}
    for r in ("canonical", "icon", "apple-touch-icon", "manifest"):
        if r not in rels:
            bad.append("missing link " + r)
    if p.meta.get("twitter:card") != ["summary_large_image"]:
        bad.append("twitter:card is not summary_large_image")
    for t in (p.meta.get("description", []) + p.meta.get("og:title", []) + [p.title]):
        if "–" in t or "—" in t:
            bad.append("dash in text: " + t[:30])
    if not p.ld:
        bad.append("no JSON-LD")
    for j in p.ld:
        try:
            json.loads(j)
        except ValueError as e:
            bad.append("bad JSON-LD: %s" % e)
    urls = set(p.meta.get("og:image", []) + p.meta.get("twitter:image", []) + p.meta.get("og:url", []))
    urls |= {l["href"] for l in p.links if l.get("href")}
    for l in p.links:
        if l.get("rel") == "manifest":
            m = json.loads((root / l["href"].lstrip("/")).read_text())
            for k in ("name", "short_name", "theme_color", "background_color", "icons", "screenshots"):
                if not m.get(k):
                    bad.append("manifest lacks " + k)
            for it in m.get("icons", []) + m.get("screenshots", []):
                urls.add(it["src"])
    for u in sorted(urls):
        if u.startswith("http") and not u.startswith(base):
            continue
        rel = u[len(base):] if u.startswith(base) else u
        rel = rel.split("?")[0].lstrip("/")
        if rel and not (root / rel).is_file():
            bad.append("not in build output: " + u)
    img = p.meta["og:image"][0]
    f = root / img[len(base):].lstrip("/")
    if f.is_file():
        w, h = png_size(f)
        if (w, h) != (int(p.meta["og:image:width"][0]), int(p.meta["og:image:height"][0])):
            bad.append("og:image size %dx%d does not match tags" % (w, h))
        if f.stat().st_size > 300_000:
            bad.append("og:image over 300KB")
    for b in bad:
        print("FAIL", b)
    print("ok" if not bad else "%d problems" % len(bad))
    sys.exit(1 if bad else 0)


main()
