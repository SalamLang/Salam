#!/usr/bin/env python3
"""Mirror the public Salam release assets into a static tree served at
https://cdn.salamlang.ir/ so users who cannot reach github.com can download.

Layout (relative to --root), a copy of GitHub's own paths:

  releases/<tag>/<asset>                same bytes as GitHub serves at
                                        releases/download/<tag>/<asset>
  releases/latest                       newest stable compiler tag, one line
  releases/tags.txt                     every complete stable compiler tag, newest first
  releases/index.json                   tags, assets with size and sha256, published date
  std/<sha>.tar.gz (+ .sha256)          the std/ folder of one commit
  std/main.commit, std/<tag>.commit     commit id a ref points at
  install.sh, install.ps1               the installers on main
  index.html, releases/index.html       a plain page listing every mirrored release

Rules: public data only; every file is downloaded to <root>/.tmp, its size and
sha256 are checked (GitHub's digest and the published .sha256 beside it), and
only then renamed into place. Nothing in releases/ is ever deleted. Only the
std/ snapshots are bounded (the newest few are kept).

Usage: cdn-sync.py [--root DIR] [--max-tags N] [--tag TAG ...] [--dry-run]
                   [--no-std] [--no-installers] [--quiet]
Set GITHUB_TOKEN (or put it in /etc/salam-cdn-sync.env) to lift the API rate limit.
"""

import argparse
import fcntl
import hashlib
import html
import io
import json
import os
import re
import sys
import tarfile
import time
import urllib.error
import urllib.request

REPO = "SalamLang/Salam"
API = "https://api.github.com/repos/" + REPO
UA = "salam-cdn-sync/1.0"
MIRROR = "https://cdn.salamlang.ir"
FAMILIES = [
    ("compiler", re.compile(r"^v\d+\.\d+\.\d+$")),
    ("agent", re.compile(r"^agent-v\d+\.\d+\.\d+$")),
    ("android", re.compile(r"^android-v\d+\.\d+\.\d+$")),
]
SAFE_NAME = re.compile(r"^[A-Za-z0-9][A-Za-z0-9._+~-]*$")
STD_KEEP = 8

QUIET = False


def log(msg):
    if not QUIET:
        print(time.strftime("%H:%M:%S"), msg, flush=True)


def request(url, accept=None, token=None, timeout=60):
    headers = {"User-Agent": UA}
    if accept:
        headers["Accept"] = accept
    if token:
        headers["Authorization"] = "Bearer " + token
    return urllib.request.Request(url, headers=headers)


def get_bytes(url, accept=None, token=None, tries=4):
    last = None
    for n in range(tries):
        try:
            with urllib.request.urlopen(request(url, accept, token), timeout=60) as r:
                return r.read()
        except urllib.error.HTTPError as e:
            last = e
            if e.code in (403, 429):
                time.sleep(5 * (n + 1))
            elif 400 <= e.code < 500:
                raise
        except (urllib.error.URLError, OSError) as e:
            last = e
        time.sleep(2 * (n + 1))
    raise last


def atomic_write(path, data, mode=0o644):
    tmp = os.path.join(os.path.dirname(path), "." + os.path.basename(path) + ".part-%d" % os.getpid())
    with open(tmp, "wb") as f:
        f.write(data if isinstance(data, bytes) else data.encode())
    os.chmod(tmp, mode)
    os.replace(tmp, path)


def read_text(path):
    try:
        with open(path, "r") as f:
            return f.read()
    except OSError:
        return None


def write_if_changed(path, text):
    if read_text(path) == text:
        return False
    os.makedirs(os.path.dirname(path), exist_ok=True)
    atomic_write(path, text)
    return True


def download(url, dest, size, want_sha, tmpdir):
    """Download url to dest through tmpdir; verify size and sha256; return sha256."""
    part = os.path.join(tmpdir, os.path.basename(dest) + ".part")
    last = None
    for attempt in range(5):
        have = os.path.getsize(part) if os.path.exists(part) else 0
        headers = {"User-Agent": UA}
        if have > 0 and have < size:
            headers["Range"] = "bytes=%d-" % have
        else:
            have = 0
        try:
            req = urllib.request.Request(url, headers=headers)
            with urllib.request.urlopen(req, timeout=60) as r:
                if have > 0 and r.status != 206:
                    have = 0
                with open(part, "ab" if have > 0 else "wb") as f:
                    while True:
                        chunk = r.read(1 << 20)
                        if not chunk:
                            break
                        f.write(chunk)
            got = os.path.getsize(part)
            if got != size:
                last = "size %d, expected %d" % (got, size)
                if got > size:
                    os.remove(part)
                time.sleep(2 * (attempt + 1))
                continue
            h = hashlib.sha256()
            with open(part, "rb") as f:
                for block in iter(lambda: f.read(1 << 20), b""):
                    h.update(block)
            digest = h.hexdigest()
            if want_sha and digest != want_sha:
                os.remove(part)
                last = "sha256 %s, expected %s" % (digest, want_sha)
                time.sleep(2 * (attempt + 1))
                continue
            os.chmod(part, 0o644)
            os.replace(part, dest)
            return digest
        except (urllib.error.URLError, OSError) as e:
            last = str(e)
            time.sleep(3 * (attempt + 1))
    raise RuntimeError("could not fetch %s: %s" % (url, last))


def list_releases(token):
    out = []
    for page in range(1, 6):
        body = get_bytes(API + "/releases?per_page=100&page=%d" % page,
                         "application/vnd.github+json", token)
        batch = json.loads(body)
        out.extend(batch)
        if len(batch) < 100:
            break
    return out


def first_token(text):
    for line in text.splitlines():
        parts = line.split()
        if parts:
            return parts[0].lower()
    return ""


def sync_release(rel, root, tmpdir, dry):
    tag = rel["tag_name"]
    ddir = os.path.join(root, "releases", tag)
    assets = rel["assets"]
    names = {a["name"] for a in assets}
    ordered = sorted(assets, key=lambda a: (not (a["name"].endswith(".sha256") or a["name"] == "SHA256SUMS"), a["size"]))
    entries = []
    complete = True
    for a in ordered:
        name = a["name"]
        if not SAFE_NAME.match(name) or a.get("state") != "uploaded":
            log("skip %s/%s (name or state)" % (tag, name))
            complete = False
            continue
        dest = os.path.join(ddir, name)
        want = ""
        dg = a.get("digest") or ""
        if dg.startswith("sha256:"):
            want = dg[7:].lower()
        sidecar = None
        if name + ".sha256" in names:
            sidecar = first_token(read_text(os.path.join(ddir, name + ".sha256")) or "")
            if len(sidecar) != 64:
                sidecar = None
        if os.path.exists(dest) and os.path.getsize(dest) == a["size"]:
            sha = want or sidecar or ""
            if not sha:
                h = hashlib.sha256()
                with open(dest, "rb") as f:
                    for block in iter(lambda: f.read(1 << 20), b""):
                        h.update(block)
                sha = h.hexdigest()
            entries.append({"name": name, "size": a["size"], "sha256": sha,
                            "url": "%s/releases/%s/%s" % (MIRROR, tag, name)})
            continue
        if dry:
            log("would download %s/%s (%d bytes)" % (tag, name, a["size"]))
            complete = False
            continue
        os.makedirs(ddir, exist_ok=True)
        os.chmod(ddir, 0o755)
        log("download %s/%s (%d bytes)" % (tag, name, a["size"]))
        digest = download(a["browser_download_url"], dest, a["size"], want, tmpdir)
        if sidecar and sidecar != digest:
            os.remove(dest)
            raise RuntimeError("%s/%s does not match its published .sha256" % (tag, name))
        entries.append({"name": name, "size": a["size"], "sha256": digest,
                        "url": "%s/releases/%s/%s" % (MIRROR, tag, name)})
    entries.sort(key=lambda e: e["name"])
    return complete, entries


def sync_std(root, token, dry, tags):
    sdir = os.path.join(root, "std")
    refs = [("main", "main")]
    if tags:
        refs.append((tags[0], tags[0]))
    for ref, label in refs:
        try:
            sha = get_bytes(API + "/commits/" + ref,
                            "application/vnd.github.sha", token).decode().strip()
        except Exception as e:
            log("std: cannot resolve %s: %s" % (ref, e))
            continue
        if not re.match(r"^[0-9a-f]{40}$", sha):
            log("std: %s did not resolve to a commit" % ref)
            continue
        tgz = os.path.join(sdir, sha + ".tar.gz")
        if not os.path.exists(tgz):
            if dry:
                log("would build std snapshot %s for %s" % (sha[:12], ref))
                continue
            log("std: building snapshot %s for %s" % (sha[:12], ref))
            raw = get_bytes("https://codeload.github.com/%s/tar.gz/%s" % (REPO, sha))
            top = "Salam-" + sha
            out = io.BytesIO()
            with tarfile.open(fileobj=io.BytesIO(raw), mode="r:gz") as src, \
                    tarfile.open(fileobj=out, mode="w") as dst:
                for m in src:
                    parts = m.name.split("/", 2)
                    if len(parts) < 2 or parts[1] != "std":
                        continue
                    if not (m.isfile() or m.isdir()):
                        continue
                    if ".." in m.name.split("/"):
                        continue
                    m.name = top + "/" + "/".join(parts[1:])
                    m.uid = m.gid = 0
                    m.uname = m.gname = ""
                    m.mtime = 0
                    m.mode = 0o755 if m.isdir() else 0o644
                    dst.addfile(m, src.extractfile(m) if m.isfile() else None)
            import gzip
            gz = io.BytesIO()
            with gzip.GzipFile(fileobj=gz, mode="wb", mtime=0, compresslevel=9) as g:
                g.write(out.getvalue())
            data = gz.getvalue()
            if len(data) < 100000:
                log("std: snapshot %s is suspiciously small, skipped" % sha[:12])
                continue
            os.makedirs(sdir, exist_ok=True)
            os.chmod(sdir, 0o755)
            atomic_write(tgz + ".sha256", "%s  %s\n" % (hashlib.sha256(data).hexdigest(), sha + ".tar.gz"))
            atomic_write(tgz, data)
        if not dry:
            write_if_changed(os.path.join(sdir, label + ".commit"), sha + "\n")
    if dry or not os.path.isdir(sdir):
        return
    keep = set()
    for f in os.listdir(sdir):
        if f.endswith(".commit"):
            keep.add((read_text(os.path.join(sdir, f)) or "").strip())
    snaps = sorted((f for f in os.listdir(sdir) if f.endswith(".tar.gz")),
                   key=lambda f: os.path.getmtime(os.path.join(sdir, f)), reverse=True)
    for f in snaps[STD_KEEP:]:
        if f[:-7] not in keep:
            os.remove(os.path.join(sdir, f))
            sidecar = os.path.join(sdir, f + ".sha256")
            if os.path.exists(sidecar):
                os.remove(sidecar)


def sync_installers(root, dry):
    for name in ("install.sh", "install.ps1"):
        url = "https://raw.githubusercontent.com/%s/refs/heads/main/%s" % (REPO, name)
        try:
            data = get_bytes(url)
        except Exception as e:
            log("installer %s: %s" % (name, e))
            continue
        if len(data) < 2048 or b"Salam" not in data or data.lstrip().startswith(b"<"):
            log("installer %s: unexpected content, kept the old copy" % name)
            continue
        dest = os.path.join(root, name)
        old = None
        try:
            with open(dest, "rb") as f:
                old = f.read()
        except OSError:
            pass
        if old != data and not dry:
            atomic_write(dest, data)
            log("installer %s updated" % name)


def human(n):
    n = float(n)
    for u in ("B", "KB", "MB", "GB"):
        if n < 1024 or u == "GB":
            return ("%d %s" % (n, u)) if u == "B" else ("%.1f %s" % (n, u))
        n /= 1024


def render_page(ordered):
    """One static page for people who open the mirror in a browser. Direct
    links, sizes and the sha256 of every file; no scripts, no external assets."""
    e = html.escape
    out = ['<!doctype html><html lang="en"><head><meta charset="utf-8">',
           '<meta name="viewport" content="width=device-width,initial-scale=1">',
           '<title>Salam releases</title>',
           '<style>body{font:15px/1.5 system-ui,sans-serif;max-width:60rem;margin:2rem auto;padding:0 1rem}'
           'table{border-collapse:collapse;width:100%;margin:.5rem 0 1.5rem}'
           'td,th{padding:.25rem .5rem;border-bottom:1px solid #8884;text-align:left}'
           'td.n,th.n{text-align:right;white-space:nowrap}code{font-size:12px;word-break:break-all}'
           '.fa{direction:rtl}</style></head><body>',
           '<h1>Salam releases</h1>',
           '<p>These files are copies of <a href="https://github.com/SalamLang/Salam/releases">'
           'the GitHub releases</a>, served by the Salam project. Check the sha256 of what you download.</p>',
           '<p class="fa" lang="fa">این فایل\u200cها نسخه\u200cی کپی انتشارهای گیت\u200cهاب هستند. '
           'پیش از اجرا، مقدار sha256 را بررسی کنید.</p>',
           '<p>Install: <code>curl -fsSL https://cdn.salamlang.ir/install.sh | sh</code></p>']
    for rel in ordered:
        tag = rel["tag"]
        out.append("<h2>%s <small>%s</small></h2><table><tr><th>File</th><th class=n>Size</th><th>sha256</th></tr>"
                   % (e(tag), e(rel["published_at"][:10])))
        for a in sorted(rel["assets"], key=lambda x: x["name"]):
            if a["name"].endswith(".sha256"):
                continue
            out.append('<tr><td><a href="/releases/%s/%s">%s</a></td><td class=n>%s</td><td><code>%s</code></td></tr>'
                       % (e(tag), e(a["name"]), e(a["name"]), human(a["size"]), e(a.get("sha256", ""))))
        out.append("</table>")
    out.append("</body></html>\n")
    return "\n".join(out)


def main():
    global QUIET
    ap = argparse.ArgumentParser()
    ap.add_argument("--root", default="/hosts/cdn.salamlang.ir")
    ap.add_argument("--max-tags", type=int, default=10,
                    help="newest tags per family to mirror (0 means all)")
    ap.add_argument("--tag", action="append", default=[], help="mirror only these tags")
    ap.add_argument("--dry-run", action="store_true")
    ap.add_argument("--no-std", action="store_true")
    ap.add_argument("--no-installers", action="store_true")
    ap.add_argument("--quiet", action="store_true")
    args = ap.parse_args()
    QUIET = args.quiet

    token = os.environ.get("GITHUB_TOKEN") or os.environ.get("GH_TOKEN") or None
    root = os.path.abspath(args.root)
    os.makedirs(os.path.join(root, ".tmp"), exist_ok=True)
    os.makedirs(os.path.join(root, "releases"), exist_ok=True)
    os.chmod(root, 0o755)
    lock = open(os.path.join(root, ".sync.lock"), "w")
    try:
        fcntl.flock(lock, fcntl.LOCK_EX | fcntl.LOCK_NB)
    except OSError:
        log("another sync is running")
        return 0
    tmpdir = os.path.join(root, ".tmp")

    releases = [r for r in list_releases(token) if not r["draft"] and not r["prerelease"]]
    releases.sort(key=lambda r: r["published_at"], reverse=True)
    wanted = []
    for _, rx in FAMILIES:
        fam = [r for r in releases if rx.match(r["tag_name"])]
        if args.max_tags > 0:
            fam = fam[:args.max_tags]
        wanted.extend(fam)
    if args.tag:
        wanted = [r for r in releases if r["tag_name"] in args.tag]

    index_path = os.path.join(root, "releases", "index.json")
    try:
        index = {e["tag"]: e for e in json.loads(read_text(index_path) or "{}").get("releases", [])}
    except (ValueError, AttributeError, KeyError):
        index = {}
    for e in index.values():
        for x in e.get("assets", []):
            x.setdefault("url", "%s/releases/%s/%s" % (MIRROR, e["tag"], x["name"]))

    failed = 0
    for rel in wanted:
        tag = rel["tag_name"]
        try:
            complete, entries = sync_release(rel, root, tmpdir, args.dry_run)
        except Exception as e:
            log("FAILED %s: %s" % (tag, e))
            failed += 1
            continue
        if complete and entries and not args.dry_run:
            index[tag] = {"tag": tag, "name": rel.get("name") or tag,
                          "published_at": rel["published_at"], "assets": entries}

    if not args.dry_run:
        ordered = sorted(index.values(), key=lambda e: e["published_at"], reverse=True)
        doc = {"repo": REPO, "mirror": MIRROR + "/releases/",
               "releases": ordered}
        compilers = [e["tag"] for e in ordered if FAMILIES[0][1].match(e["tag"])]
        latest = compilers[0] if compilers else None
        old = json.loads(read_text(index_path) or "{}")
        if old.get("releases") != doc["releases"]:
            doc["generated"] = time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime())
            atomic_write(index_path, json.dumps(doc, indent=1, sort_keys=True) + "\n")
            log("index.json now lists %d releases" % len(ordered))
        if latest:
            if write_if_changed(os.path.join(root, "releases", "latest"), latest + "\n"):
                log("latest is now %s" % latest)
            write_if_changed(os.path.join(root, "releases", "tags.txt"), "\n".join(compilers) + "\n")
        page = render_page(ordered)
        write_if_changed(os.path.join(root, "releases", "index.html"), page)
        write_if_changed(os.path.join(root, "index.html"), page)

    if not args.no_std:
        compilers = [r["tag_name"] for r in releases if FAMILIES[0][1].match(r["tag_name"])]
        try:
            sync_std(root, token, args.dry_run, compilers)
        except Exception as e:
            log("std snapshot failed: %s" % e)
            failed += 1
    if not args.no_installers:
        sync_installers(root, args.dry_run)

    for f in os.listdir(tmpdir):
        p = os.path.join(tmpdir, f)
        if f.endswith(".part") and time.time() - os.path.getmtime(p) > 86400:
            os.remove(p)
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
