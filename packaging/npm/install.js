#!/usr/bin/env node

const fs = require("node:fs");
const os = require("node:os");
const path = require("node:path");
const zlib = require("node:zlib");
const { execFileSync } = require("node:child_process");
const { pipeline } = require("node:stream/promises");

const VERSION = require("./package.json").version;
const BASE = `https://github.com/SalamLang/Salam/releases/download/v${VERSION}`;
const VENDOR = path.join(__dirname, "vendor");

function target() {
  const platform = os.platform();
  const arch = os.arch();

  if (platform === "darwin") {
    if (arch !== "arm64") {
      throw new Error("salam publishes macOS builds for Apple Silicon only");
    }
    return {
      slug: "macos-arm64",
      dir: "salam-macos-arm64",
      exe: "salam",
      ext: "tar.gz",
    };
  }

  if (platform === "linux") {
    const map = { x64: "x86_64", arm64: "aarch64", arm: "armhf", ia32: "i686" };
    const rel = map[arch];
    if (!rel) throw new Error(`salam has no Linux build for ${arch}`);
    return {
      slug: `linux-${rel}`,
      dir: `salam-linux-${rel}`,
      exe: "salam",
      ext: "tar.gz",
    };
  }

  if (platform === "win32") {
    if (arch === "x64") {
      return {
        slug: "windows-x86_64",
        dir: "salam-windows",
        exe: "salam.exe",
        ext: "zip",
      };
    }
    if (arch === "ia32") {
      return {
        slug: "windows-i686",
        dir: "salam-windows-i686",
        exe: "salam.exe",
        ext: "zip",
      };
    }
    throw new Error(`salam has no Windows build for ${arch}`);
  }

  throw new Error(`salam has no build for ${platform}`);
}

async function download(url, dest) {
  const res = await fetch(url, { redirect: "follow" });
  if (!res.ok) throw new Error(`${url} -> HTTP ${res.status}`);
  await pipeline(res.body, fs.createWriteStream(dest));
}

async function verify(archive, url) {
  const res = await fetch(`${url}.sha256`, { redirect: "follow" });
  if (!res.ok) {
    console.warn(
      `salam: no published checksum for ${path.basename(archive)}; skipping verification`,
    );
    return;
  }
  const expected = (await res.text()).trim().split(/\s+/)[0].toLowerCase();
  const actual = require("node:crypto")
    .createHash("sha256")
    .update(fs.readFileSync(archive))
    .digest("hex");
  if (expected !== actual) {
    throw new Error(
      `checksum mismatch for ${path.basename(archive)}: expected ${expected}, got ${actual}`,
    );
  }
}

function extract(archive, into, ext) {
  fs.mkdirSync(into, { recursive: true });
  if (ext === "zip") {
    // tar handles zip on Windows 10+ (bsdtar); PowerShell is the fallback.
    try {
      execFileSync("tar", ["-xf", archive, "-C", into], { stdio: "inherit" });
    } catch {
      execFileSync(
        "powershell",
        [
          "-NoProfile",
          "-NonInteractive",
          "-Command",
          `Expand-Archive -LiteralPath '${archive}' -DestinationPath '${into}' -Force`,
        ],
        { stdio: "inherit" },
      );
    }
  } else {
    execFileSync("tar", ["-xzf", archive, "-C", into], { stdio: "inherit" });
  }
}

async function main() {
  const t = target();
  const name = `salam-${VERSION}-${t.slug}.${t.ext}`;
  const url = `${BASE}/${name}`;

  if (fs.existsSync(path.join(VENDOR, t.dir, t.exe))) {
    return;
  }

  const tmp = fs.mkdtempSync(path.join(os.tmpdir(), "salam-"));
  const archive = path.join(tmp, name);

  try {
    console.log(`salam ${VERSION}: downloading ${name}`);
    await download(url, archive);
    await verify(archive, url);
    fs.rmSync(VENDOR, { recursive: true, force: true });
    extract(archive, VENDOR, t.ext);

    const exe = path.join(VENDOR, t.dir, t.exe);
    if (!fs.existsSync(exe))
      throw new Error(`${name} did not contain ${t.dir}/${t.exe}`);
    if (os.platform() !== "win32") fs.chmodSync(exe, 0o755);
    console.log(`salam ${VERSION}: installed`);
  } finally {
    fs.rmSync(tmp, { recursive: true, force: true });
  }
}

main().catch((err) => {
  console.error(`salam: install failed: ${err.message}`);
  process.exit(1);
});
