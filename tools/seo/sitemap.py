#!/usr/bin/env python3
"""Write sitemap.xml for the static ai and editor sites with today as lastmod."""
import datetime
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
SITES = {
    "ai/frontend/public": ["https://ai.salamlang.ir/", "https://ai.salamlang.ir/plans"],
    "tools/mcp/site": ["https://mcp.salamlang.ir/"],
    "editor": ["https://editor.salamlang.ir/"],
}
today = datetime.date.today().isoformat()
for d, urls in SITES.items():
    rows = "".join("  <url><loc>%s</loc><lastmod>%s</lastmod></url>\n" % (u, today) for u in urls)
    (ROOT / d / "sitemap.xml").write_text(
        '<?xml version="1.0" encoding="UTF-8"?>\n<urlset xmlns="http://www.sitemaps.org/schemas/sitemap/0.9">\n' + rows + "</urlset>\n")
