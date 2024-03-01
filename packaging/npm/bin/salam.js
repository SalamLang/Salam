#!/usr/bin/env node

const fs = require("node:fs");
const os = require("node:os");
const path = require("node:path");
const { spawnSync } = require("node:child_process");

const VENDOR = path.join(__dirname, "..", "vendor");

function locate() {
  if (!fs.existsSync(VENDOR)) return null;
  for (const entry of fs.readdirSync(VENDOR)) {
    for (const exe of ["salam", "salam.exe"]) {
      const candidate = path.join(VENDOR, entry, exe);
      if (fs.existsSync(candidate)) return candidate;
    }
  }
  return null;
}

const bin = locate();
if (!bin) {
  console.error(
    "salam: the compiler is not unpacked; re-run `npm install salamlang`",
  );
  process.exit(1);
}

// The compiler derives std/ from its own location, which is reliable here
// because bin is an absolute path, but pin it anyway.
const env = { ...process.env };
if (!env.SALAM_STD) {
  const std = path.join(path.dirname(bin), "std");
  if (fs.existsSync(std)) env.SALAM_STD = std;
}

const result = spawnSync(bin, process.argv.slice(2), { stdio: "inherit", env });
if (result.error) {
  console.error(`salam: ${result.error.message}`);
  process.exit(1);
}
process.exit(result.status === null ? 1 : result.status);
