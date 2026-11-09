#!/usr/bin/env python3
"""Render the 1200x630 link-preview covers for ai, editor and mcp.

Usage: tools/seo/covers.py [site ...]
Needs a Chromium (CHROME env var or a Playwright download) and ImageMagick.
"""
import os
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
FONT = ROOT / "editor/assets/fonts/Vazirmatn-Variable.woff2"
LOGO = (ROOT / "ai/frontend/public/logo.svg").read_text()

SITES = {
    "ai": {
        "out": ROOT / "ai/frontend/public",
        "badge": "دستیار هوشمند",
        "title": "سلام",
        "line1": "دستیار هوشمند زبان سلام",
        "line2": "AI assistant for the Salam language",
        "domain": "ai.salamlang.ir",
    },
    "editor": {
        "out": ROOT / "editor/assets",
        "badge": "ویرایشگر آنلاین",
        "title": "سلام",
        "line1": "ویرایشگر و زمین بازی آنلاین سلام",
        "line2": "Online editor and playground",
        "domain": "editor.salamlang.ir",
    },
    "mcp": {
        "out": ROOT / "tools/mcp/site",
        "badge": "سرور MCP",
        "title": "سلام",
        "line1": "سرور MCP برای ابزارهای هوش مصنوعی",
        "line2": "MCP server for AI tools",
        "domain": "mcp.salamlang.ir",
    },
}

PAGE = """<!doctype html><html lang="fa" dir="rtl"><head><meta charset="utf-8"><style>
@font-face{font-family:V;src:url("file://%(font)s");font-weight:100 900}
*{margin:0;box-sizing:border-box}
html,body{width:1200px;height:630px;overflow:hidden;background:#111214;font-family:V,sans-serif}
body{position:relative;background:radial-gradient(ellipse 700px 420px at 100%% 0%%,#4d2410 0%%,#111214 70%%)}
.logo{position:absolute;left:96px;top:155px;width:300px;height:300px}
.logo svg{width:100%%;height:100%%;fill:#ff5c00}
.t{position:absolute;right:90px;top:0;height:630px;width:760px;display:flex;flex-direction:column;justify-content:center;align-items:flex-start;direction:rtl}
.badge{background:#3a2112;color:#ff7a2a;font-weight:600;font-size:28px;padding:14px 28px;border-radius:40px;margin-bottom:28px}
h1{color:#ff5c00;font-weight:800;font-size:128px;line-height:1.15}
.l1{color:#f5f5f5;font-weight:700;font-size:40px;line-height:1.5;white-space:nowrap;margin-top:14px}
.l2{color:#b8b4ae;font-size:30px;direction:ltr;margin-top:6px;font-weight:400;align-self:flex-start}
.d{color:#a09c96;font-size:32px;direction:ltr;margin-top:42px;font-weight:500;align-self:flex-start}
</style></head><body>
<div class="logo">%(logo)s</div>
<div class="t"><div class="badge">%(badge)s</div><h1>%(title)s</h1>
<div class="l1">%(line1)s</div><div class="l2">%(line2)s</div>
<div class="d">%(domain)s</div></div>
</body></html>"""


def chrome():
    c = os.environ.get("CHROME")
    if c:
        return c
    for n in ("chromium", "chromium-browser", "google-chrome", "chrome"):
        if shutil.which(n):
            return shutil.which(n)
    hits = sorted(Path.home().glob(".cache/ms-playwright/chromium-*/chrome-linux*/chrome"))
    if hits:
        return str(hits[-1])
    sys.exit("no chromium found, set CHROME")


def render(name, s, tmp):
    html = tmp / (name + ".html")
    body = LOGO.split("?>", 1)[-1]
    html.write_text(PAGE % {**s, "font": FONT, "logo": body})
    out = s["out"]
    out.mkdir(parents=True, exist_ok=True)
    big = out / ("og-" + name + ".png")
    raw = tmp / (name + ".png")
    subprocess.run([chrome(), "--headless=new", "--no-sandbox", "--disable-gpu", "--hide-scrollbars",
                    "--force-device-scale-factor=1", "--window-size=1200,630",
                    "--virtual-time-budget=3000", "--screenshot=" + str(raw), "file://" + str(html)],
                   check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(["convert", str(raw), "-strip", "-colors", "256", "-define", "png:compression-level=9", str(big)], check=True)
    small = out / ("og-" + name + "-small.png")
    subprocess.run(["convert", str(raw), "-resize", "600x315", "-strip", "-colors", "256", str(small)], check=True)
    print(big, big.stat().st_size, small.stat().st_size)


def main():
    names = sys.argv[1:] or list(SITES)
    with tempfile.TemporaryDirectory() as t:
        for n in names:
            render(n, SITES[n], Path(t))


main()
