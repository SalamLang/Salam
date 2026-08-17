/*
 * Salam Programming Language (2024-2026)
 *
 * Protocol conformance tests for the salam-mcp server.
 *
 * Drives a real server process over stdio the way an MCP client does and
 * asserts on the wire format, so a regression in framing or in the dual-era
 * version handling fails the build rather than showing up as a mysteriously
 * broken client.
 *
 * Usage:
 *   node tools/mcp/tests/protocol_test.mjs <path-to-salam-mcp> [repo-root]
 */

import { spawn } from "node:child_process";
import { existsSync } from "node:fs";
import { join, resolve } from "node:path";

const serverPath = process.argv[2];
const repoRoot = resolve(process.argv[3] ?? process.cwd());
if (!serverPath) {
  console.error("usage: protocol_test.mjs <path-to-salam-mcp> [repo-root]");
  process.exit(2);
}

/*
 * Pin the compiler under test to the one in this checkout. Falling back to
 * "salam" on PATH silently tests whatever version happens to be installed:
 * a stale 0.2.7 on PATH parses `inspect --xml-out=` as a second input file,
 * which shows up as inexplicable tool failures rather than a version error.
 */
function repoCompiler() {
  for (const candidate of ["salam.exe", "salam"]) {
    const p = join(repoRoot, candidate);
    if (existsSync(p)) return p;
  }
  return process.env.SALAM_MCP_BIN ?? "salam";
}
const compiler = repoCompiler();

const MODERN = "2026-07-28";
const meta = { "io.modelcontextprotocol/protocolVersion": MODERN };

let passed = 0;
const failures = [];

function check(name, condition, detail) {
  if (condition) {
    passed += 1;
    console.log(`  ok   ${name}`);
  } else {
    failures.push(`${name}${detail ? `: ${detail}` : ""}`);
    console.log(`  FAIL ${name}${detail ? `: ${detail}` : ""}`);
  }
}

/**
 * Sends every message, then reads replies until the process exits. The server
 * shuts down on stdin EOF, so closing stdin is also the end-of-test signal.
 */
function converse(messages, env = {}) {
  return new Promise((resolvePromise, reject) => {
    const child = spawn(serverPath, [], {
      env: {
        ...process.env,
        SALAM_MCP_ROOT: repoRoot,
        SALAM_MCP_BIN: env.SALAM_MCP_BIN ?? compiler,
        ...env,
      },
      stdio: ["pipe", "pipe", "pipe"],
    });

    let out = "";
    let err = "";
    child.stdout.on("data", (d) => (out += d));
    child.stderr.on("data", (d) => (err += d));
    child.on("error", reject);
    child.on("close", () => resolvePromise({ raw: out, stderr: err }));

    for (const m of messages) child.stdin.write(`${JSON.stringify(m)}\n`);
    child.stdin.end();

    setTimeout(() => {
      child.kill();
      reject(new Error("server did not exit within 120s"));
    }, 120_000).unref();
  });
}

function parseLines(raw) {
  return raw
    .split("\n")
    .filter((l) => l.trim().length > 0)
    .map((l) => JSON.parse(l));
}

// --- transport ---------------------------------------------------------

async function testTransport() {
  console.log("transport");
  const { raw } = await converse([
    { jsonrpc: "2.0", id: 1, method: "ping", params: { _meta: meta } },
    { jsonrpc: "2.0", method: "notifications/initialized" },
    { jsonrpc: "2.0", id: 2, method: "ping", params: { _meta: meta } },
  ]);

  const lines = raw.split("\n").filter((l) => l.length > 0);
  check(
    "every stdout line is a complete JSON message",
    lines.every((l) => {
      try {
        JSON.parse(l);
        return true;
      } catch {
        return false;
      }
    }),
  );
  check(
    "notifications produce no reply",
    lines.length === 2,
    `got ${lines.length} lines`,
  );

  const msgs = parseLines(raw);
  check(
    "responses echo their request id",
    msgs[0].id === 1 && msgs[1].id === 2,
  );
  check(
    "responses carry jsonrpc 2.0",
    msgs.every((m) => m.jsonrpc === "2.0"),
  );
}

// --- modern era --------------------------------------------------------

async function testModern() {
  console.log("modern era (2026-07-28)");
  const { raw } = await converse([
    {
      jsonrpc: "2.0",
      id: 1,
      method: "server/discover",
      params: { _meta: meta },
    },
    { jsonrpc: "2.0", id: 2, method: "tools/list", params: { _meta: meta } },
    {
      jsonrpc: "2.0",
      id: 3,
      method: "resources/list",
      params: { _meta: meta },
    },
    {
      jsonrpc: "2.0",
      id: 4,
      method: "ping",
      params: {
        _meta: { "io.modelcontextprotocol/protocolVersion": "1900-01-01" },
      },
    },
    {
      jsonrpc: "2.0",
      id: 5,
      method: "no/such/method",
      params: { _meta: meta },
    },
  ]);
  const [discover, tools, resources, badVersion, unknown] = parseLines(raw);

  check(
    "server/discover advertises the modern version",
    discover.result.supportedVersions.includes(MODERN),
  );
  check(
    "server/discover reports serverInfo",
    typeof discover.result._meta?.["io.modelcontextprotocol/serverInfo"]
      ?.name === "string",
  );
  check(
    "server/discover declares tools + resources",
    !!discover.result.capabilities.tools &&
      !!discover.result.capabilities.resources,
  );
  check(
    "modern results are tagged complete",
    discover.result.resultType === "complete",
  );

  check(
    "tools/list returns the full catalog",
    tools.result.tools.length >= 14,
    `got ${tools.result.tools?.length}`,
  );
  check(
    "every tool has a valid object inputSchema",
    tools.result.tools.every((t) => t.inputSchema?.type === "object"),
  );
  check(
    "every tool has name, title and description",
    tools.result.tools.every((t) => t.name && t.title && t.description),
  );
  check(
    "tool names match the MCP charset",
    tools.result.tools.every((t) => /^[A-Za-z0-9_.-]{1,128}$/.test(t.name)),
  );
  check(
    "resources/list returns entries with uri + mimeType",
    resources.result.resources.length >= 3 &&
      resources.result.resources.every((r) => r.uri && r.mimeType),
  );

  check(
    "unsupported version returns -32022",
    badVersion.error?.code === -32022,
  );
  check(
    "version error lists supported versions",
    Array.isArray(badVersion.error?.data?.supported) &&
      badVersion.error.data.supported.includes(MODERN),
  );
  check("unknown method returns -32601", unknown.error?.code === -32601);
}

// --- legacy era --------------------------------------------------------

async function testLegacy() {
  console.log("legacy era (initialize handshake)");
  const { raw } = await converse([
    {
      jsonrpc: "2.0",
      id: 1,
      method: "initialize",
      params: {
        protocolVersion: "2025-06-18",
        capabilities: {},
        clientInfo: { name: "conformance", version: "1.0.0" },
      },
    },
    { jsonrpc: "2.0", method: "notifications/initialized" },
    { jsonrpc: "2.0", id: 2, method: "tools/list", params: {} },
  ]);
  const [init, tools] = parseLines(raw);

  check(
    "initialize echoes the requested protocol version",
    init.result.protocolVersion === "2025-06-18",
    init.result?.protocolVersion,
  );
  check(
    "initialize returns serverInfo",
    typeof init.result.serverInfo?.name === "string",
  );
  check("legacy results omit resultType", init.result.resultType === undefined);
  check(
    "legacy tools/list omits resultType",
    tools.result.resultType === undefined,
  );
  check(
    "legacy tools/list still returns tools",
    tools.result.tools.length >= 14,
  );
}

// --- tool behaviour ----------------------------------------------------

async function testTools() {
  console.log("tool behaviour");
  const call = (id, name, args) => ({
    jsonrpc: "2.0",
    id,
    method: "tools/call",
    params: { _meta: meta, name, arguments: args },
  });

  const { raw } = await converse([
    call(1, "salam_stdlib_packages", {}),
    call(2, "salam_stdlib_symbols", { package: "str", query: "TrimPrefix" }),
    call(3, "salam_check", { path: "tools/mcp/tests/fixtures/broken.salam" }),
    call(4, "salam_check", { path: "tools/mcp/tests/fixtures/ok.salam" }),
    call(5, "salam_check", { path: "../outside.salam" }),
    call(6, "no_such_tool", {}),
    call(7, "salam_keywords", {}),
  ]);
  const [pkgs, symbols, broken, ok, traversal, unknownTool, keywords] =
    parseLines(raw);

  check(
    "stdlib packages are discovered",
    pkgs.result.structuredContent.length > 20,
    `got ${pkgs.result.structuredContent?.length}`,
  );
  check(
    "stdlib packages exclude non-importable dirs",
    !pkgs.result.structuredContent.includes("excel_bak"),
  );
  check(
    "symbol lookup finds a known declaration",
    symbols.result.content[0].text.includes("TrimPrefix"),
  );

  check("a broken file reports isError", broken.result.isError === true);
  check(
    "a broken file yields structured diagnostics",
    Array.isArray(broken.result.structuredContent?.diagnostics) &&
      broken.result.structuredContent.diagnostics.length > 0,
  );
  check(
    "diagnostics carry line, column and code",
    broken.result.structuredContent.diagnostics.every(
      (d) =>
        typeof d.line === "number" && typeof d.column === "number" && d.code,
    ),
  );

  check(
    "a valid file reports success",
    ok.result.isError === false,
    ok.result.content[0].text,
  );
  check("path traversal is refused", traversal.result.isError === true);
  check("unknown tool is a protocol error", unknownTool.error?.code === -32601);
  check(
    "keyword table covers all three languages",
    keywords.result.content[0].text.includes("english") &&
      keywords.result.content[0].text.includes("persian"),
  );
}

// --- resources ---------------------------------------------------------

async function testResources() {
  console.log("resources");
  const { raw } = await converse([
    {
      jsonrpc: "2.0",
      id: 1,
      method: "resources/read",
      params: { _meta: meta, uri: "salam://stdlib/index.json" },
    },
    {
      jsonrpc: "2.0",
      id: 2,
      method: "resources/read",
      params: { _meta: meta, uri: "salam://does/not/exist" },
    },
  ]);
  const [index, missing] = parseLines(raw);

  check(
    "stdlib index is served as JSON",
    index.result.contents[0].mimeType === "application/json",
  );
  const parsed = JSON.parse(index.result.contents[0].text);
  check(
    "stdlib index maps packages to declarations",
    Object.keys(parsed.packages).length > 20 &&
      Array.isArray(parsed.packages.str),
  );
  check("missing resource returns -32602", missing.error?.code === -32602);
}

// --- runner ------------------------------------------------------------

const suites = [
  testTransport,
  testModern,
  testLegacy,
  testTools,
  testResources,
];
for (const suite of suites) {
  await suite();
}

console.log(`\n${passed} passed, ${failures.length} failed`);
if (failures.length > 0) {
  console.error("\nfailures:");
  for (const f of failures) console.error(`  - ${f}`);
  process.exit(1);
}
