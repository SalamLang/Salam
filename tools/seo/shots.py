#!/usr/bin/env python3
"""Capture the product screenshots listed in the web manifests.

Usage: tools/seo/shots.py [site ...]
Takes a wide and a narrow shot of the live page of each site with a headless Chromium.
"""
import subprocess
import sys
import tempfile
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent))
ROOT = Path(__file__).resolve().parents[2]

SITES = {
    "ai": ("https://ai.salamlang.ir/?lang=fa", ROOT / "ai/frontend/public"),
    "editor": ("https://editor.salamlang.ir/", ROOT / "editor/assets"),
    "mcp": ("https://mcp.salamlang.ir/", ROOT / "tools/mcp/site"),
}
SIZES = {"wide": (1280, 720), "narrow": (390, 844)}


def chrome():
    src = (Path(__file__).parent / "covers.py").read_text().split("def render")[0]
    ns = {"__file__": str(Path(__file__).parent / "covers.py")}
    exec(compile(src, "covers", "exec"), ns)
    return ns["chrome"]()


def main():
    names = sys.argv[1:] or list(SITES)
    c = chrome()
    with tempfile.TemporaryDirectory() as t:
        for n in names:
            url, out = SITES[n]
            for kind, (w, h) in SIZES.items():
                raw = Path(t) / (n + kind + ".png")
                subprocess.run([c, "--headless=new", "--no-sandbox", "--disable-gpu", "--hide-scrollbars",
                                "--force-device-scale-factor=1", "--window-size=%d,%d" % (w, h),
                                "--virtual-time-budget=60000", "--screenshot=" + str(raw), url],
                               check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
                dst = out / ("screenshot-%s-%s.png" % (n, kind))
                subprocess.run(["convert", str(raw), "-strip", "-colors", "256", str(dst)], check=True)
                print(dst, dst.stat().st_size)


main()
