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
import { existsSync, mkdtempSync, rmSync } from "node:fs";
import { tmpdir } from "node:os";
import { join, resolve } from "node:path";
import { createServer } from "node:net";
import { createServer as createHttpServer } from "node:http";

const serverPath = process.argv[2] && resolve(process.argv[2]);
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
function converse(messages, env = {}, cwd = undefined) {
  return new Promise((resolvePromise, reject) => {
    const child = spawn(serverPath, [], {
      env: {
        ...process.env,
        SALAM_MCP_ROOT: repoRoot,
        SALAM_MCP_BIN: env.SALAM_MCP_BIN ?? compiler,
        ...env,
      },
      stdio: ["pipe", "pipe", "pipe"],
      cwd,
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
    call(3, "salam_stdlib_symbols", { package: "encoding.json", query: "Marshal" }),
    call(4, "salam_check", { path: "tools/mcp/tests/fixtures/broken.salam" }),
    call(5, "salam_check", { path: "tools/mcp/tests/fixtures/ok.salam" }),
    call(6, "salam_check", { path: "../outside.salam" }),
    call(7, "no_such_tool", {}),
    call(8, "salam_keywords", {}),
  ]);
  const [pkgs, symbols, nested, broken, ok, traversal, unknownTool, keywords] =
    parseLines(raw);

  const pkgNames = (pkgs.result.structuredContent?.packages ?? []).map((p) => p.name);
  check(
    "stdlib packages are discovered",
    pkgNames.length > 20,
    `got ${pkgNames.length}`,
  );
  check(
    "stdlib packages exclude non-importable dirs",
    !pkgNames.includes("excel_bak"),
  );
  // std/encoding/json is imported as `encoding.json`, so that is the name a
  // model needs back. Listing only the top level hides it behind `encoding`,
  // whose own directory holds nothing but the base64/hex helpers.
  check(
    "stdlib packages include nested ones by dotted name",
    pkgNames.includes("encoding.json") && pkgNames.includes("net.http"),
  );
  check(
    "symbol lookup finds a known declaration",
    symbols.result.content[0].text.includes("TrimPrefix"),
  );
  check(
    "symbol lookup resolves a dotted package name",
    nested.result.isError === false &&
      nested.result.content[0].text.includes("Marshal"),
    nested.result.content?.[0]?.text,
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
  check("unknown tool is invalid params", unknownTool.error?.code === -32602);
  check(
    "keyword table covers all three languages",
    keywords.result.content[0].text.includes("english") &&
      keywords.result.content[0].text.includes("persian"),
  );
}

// --- workspace ---------------------------------------------------------

async function testWorkspace() {
  console.log("workspace (server started outside the checkout)");
  const call = (id, name, args) => ({
    jsonrpc: "2.0",
    id,
    method: "tools/call",
    params: { _meta: meta, name, arguments: args },
  });
  const elsewhere = mkdtempSync(join(tmpdir(), "salam-mcp-cwd-"));
  const okAbs = join(repoRoot, "tools/mcp/tests/fixtures/ok.salam");
  let raw;
  try {
    ({ raw } = await converse(
      [
        call(1, "salam_check", { path: "tools/mcp/tests/fixtures/ok.salam" }),
        call(2, "salam_check", { path: okAbs }),
        call(3, "salam_read_source", { path: "/etc/hostname" }),
        call(4, "salam_build", { path: "tools/mcp/tests/fixtures/broken.salam" }),
        call(5, "salam_find_examples", { query: "println", limit: 3 }),
        call(6, "salam_build", { path: "tools/mcp/tests/fixtures/ok.salam", backend: "bogus" }),
        call(7, "salam_inspect", { path: "tools/mcp/tests/fixtures/ok.salam", emit: "bogus" }),
      ],
      {},
      elsewhere,
    ));
  } finally {
    rmSync(elsewhere, { recursive: true, force: true });
  }
  const [rel, abs, outside, build, examples, backend, emit] = parseLines(raw);
  check(
    "relative paths resolve against SALAM_MCP_ROOT, not the cwd",
    rel.result.isError === false,
    rel.result.content?.[0]?.text,
  );
  check(
    "an absolute path inside the workspace is accepted",
    abs.result.isError === false,
    abs.result.content?.[0]?.text,
  );
  check("an absolute path outside the workspace is refused", outside.result.isError === true);
  check(
    "a failed build still yields structured diagnostics",
    build.result.isError === true &&
      build.result.structuredContent?.diagnostics?.length > 0 &&
      build.result.structuredContent.diagnostics.every((d) => d.line > 0 && d.code),
    JSON.stringify(build.result.structuredContent),
  );
  const found = examples.result.structuredContent?.examples ?? [];
  check(
    "example paths are workspace-relative",
    found.length > 0 && found.every((e) => e.file.startsWith("tests/")),
    JSON.stringify(found[0]),
  );
  check("an unknown backend is refused", backend.result.isError === true);
  check("an unknown emit stage is refused", emit.result.isError === true);
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
  // The bug this guards: _decls_of used to walk the whole subtree, so every
  // declaration in std/encoding/json landed under `encoding` and the name a
  // caller would actually import was nowhere in the file.
  check(
    "nested packages are keyed by their import name",
    Array.isArray(parsed.packages["encoding.json"]) &&
      parsed.packages["encoding.json"].some((d) => d.includes("Marshal")),
  );
  check(
    "a parent package carries only its own declarations",
    !parsed.packages.encoding.some((d) =>
      parsed.packages["encoding.json"].includes(d),
    ),
  );
  check("missing resource returns -32602", missing.error?.code === -32602);
}

async function testKeywords() {
  console.log("keywords");
  const [reply] = parseLines(
    (
      await converse([
        {
          jsonrpc: "2.0",
          id: 1,
          method: "tools/call",
          params: { _meta: meta, name: "salam_keywords", arguments: {} },
        },
      ])
    ).raw,
  );
  const rows = reply.result.structuredContent.keywords;
  const persian = (en) => rows.find((r) => r.english === en)?.persian;
  check("func pairs with its Persian spelling", persian("func") === "تابع");
  check("ret pairs with its Persian spelling", persian("ret") === "برگشت");
  check("if pairs with its Persian spelling", persian("if") === "اگر");
}

function freePort() {
  return new Promise((resolvePromise, reject) => {
    const probe = createServer();
    probe.on("error", reject);
    probe.listen(0, "127.0.0.1", () => {
      const { port } = probe.address();
      probe.close(() => resolvePromise(port));
    });
  });
}

async function testHttp() {
  console.log("http transport");
  const port = await freePort();
  const child = spawn(serverPath, ["--http"], {
    env: {
      ...process.env,
      SALAM_MCP_ROOT: repoRoot,
      SALAM_MCP_BIN: compiler,
      SALAM_MCP_HOST: "127.0.0.1",
      SALAM_MCP_PORT: String(port),
    },
    stdio: ["ignore", "ignore", "pipe"],
  });
  const base = `http://127.0.0.1:${port}`;
  try {
    let up = false;
    for (let i = 0; i < 100 && !up; i += 1) {
      up = await fetch(`${base}/health`).then(
        (r) => r.ok,
        () => false,
      );
      if (!up) await new Promise((r) => setTimeout(r, 100));
    }
    check("server answers /health", up);
    if (!up) return;

    const post = (body, headers = {}) =>
      fetch(`${base}/mcp`, {
        method: "POST",
        headers: {
          "Content-Type": "application/json",
          Accept: "application/json, text/event-stream",
          ...headers,
        },
        body: typeof body === "string" ? body : JSON.stringify(body),
      });
    const call = async (name, args) =>
      (
        await (
          await post({
            jsonrpc: "2.0",
            id: 1,
            method: "tools/call",
            params: { _meta: meta, name, arguments: args },
          })
        ).json()
      ).result;

    const init = await post({
      jsonrpc: "2.0",
      id: 1,
      method: "initialize",
      params: {
        protocolVersion: "2025-06-18",
        capabilities: {},
        clientInfo: { name: "test", version: "1" },
      },
    });
    const initBody = await init.json();
    check("initialize over POST returns 200 JSON", init.status === 200);
    check(
      "initialize over POST negotiates the version",
      initBody.result?.protocolVersion === "2025-06-18",
    );
    check(
      "legacy requests over HTTP omit resultType",
      initBody.result?.resultType === undefined,
    );

    const note = await post({
      jsonrpc: "2.0",
      method: "notifications/initialized",
    });
    check("a notification gets 202", note.status === 202);

    const list = await (
      await post({
        jsonrpc: "2.0",
        id: 2,
        method: "tools/list",
        params: { _meta: meta },
      })
    ).json();
    const names = list.result.tools.map((t) => t.name);
    check("modern requests over HTTP carry resultType", list.result.resultType === "complete");
    check(
      "modern list results are cacheable",
      typeof list.result.ttlMs === "number" &&
        ["public", "private"].includes(list.result.cacheScope),
    );
    check(
      "modern results name the server in _meta",
      list.result._meta?.["io.modelcontextprotocol/serverInfo"]?.name === "salam-mcp",
    );
    check(
      "legacy results carry no cache fields",
      initBody.result?.ttlMs === undefined,
    );
    check(
      "hosted mode offers no tool that runs code",
      !names.includes("salam_run") &&
        !names.includes("salam_exec") &&
        !names.includes("salam_build"),
    );
    const checkTool = list.result.tools.find((t) => t.name === "salam_check");
    check(
      "hosted compiler tools take source",
      checkTool?.inputSchema.properties?.source !== undefined &&
        checkTool?.inputSchema.properties?.path === undefined,
    );

    const bad = await call("salam_check", {
      source: "func main:\n    println missing_name\nend\n",
    });
    const diag = bad.structuredContent?.diagnostics?.[0];
    check("a source check reports diagnostics", bad.isError === true && diag?.line === 2);
    check("diagnostics name the staged file, not a temp path", diag?.file === "main.salam");

    const good = await call("salam_check", {
      source: 'func main:\n    println "salam"\nend\n',
    });
    check("a clean source passes", good.isError === false);

    const empty = await call("salam_check", { source: "  " });
    check("an empty source is refused", empty.isError === true);

    const run = await (
      await post({
        jsonrpc: "2.0",
        id: 3,
        method: "tools/call",
        params: { _meta: meta, name: "salam_run", arguments: { source: "x" } },
      })
    ).json();
    check("salam_run is refused in hosted mode", run.error?.code === -32602);

    const together = await Promise.all(
      Array.from({ length: 8 }, (_, i) =>
        call("salam_check", { source: `func main:\n    println ${i}\nend\n` }),
      ),
    );
    check(
      "concurrent checks all succeed",
      together.every((r) => r.isError === false),
    );

    check("GET /mcp is 405", (await fetch(`${base}/mcp`)).status === 405);
    check(
      "DELETE /mcp is 405",
      (await fetch(`${base}/mcp`, { method: "DELETE" })).status === 405,
    );
    check(
      "an unknown MCP-Protocol-Version header is 400",
      (
        await post(
          { jsonrpc: "2.0", id: 4, method: "ping" },
          { "MCP-Protocol-Version": "1900-01-01" },
        )
      ).status === 400,
    );
    const mismatch = await post(
      { jsonrpc: "2.0", id: 5, method: "tools/list", params: { _meta: meta } },
      { "Mcp-Method": "tools/call" },
    );
    check(
      "a mismatched Mcp-Method header is 400 with -32020",
      mismatch.status === 400 && (await mismatch.json()).error?.code === -32020,
    );
    const matched = await post(
      {
        jsonrpc: "2.0",
        id: 6,
        method: "tools/call",
        params: { _meta: meta, name: "salam_version", arguments: {} },
      },
      { "Mcp-Method": "tools/call", "Mcp-Name": "salam_version" },
    );
    check("matching Mcp-Method and Mcp-Name headers pass", matched.status === 200);
    check(
      "a non-JSON body is 415",
      (await post("{}", { "Content-Type": "text/plain" })).status === 415,
    );
    const pre = await fetch(`${base}/mcp`, { method: "OPTIONS" });
    check(
      "CORS preflight allows MCP headers",
      pre.status === 204 &&
        (pre.headers.get("access-control-allow-headers") ?? "").includes("Mcp-Method"),
    );
    const home = await fetch(`${base}/`);
    check(
      "the setup page is served",
      home.status === 200 && (await home.text()).includes("mcp.salamlang.ir/mcp"),
    );
  } finally {
    child.kill();
  }
}

// --- agent-facing behaviour --------------------------------------------

function textOf(reply) {
  return reply?.result?.content?.[0]?.text ?? "";
}

async function testAgentTools() {
  console.log("agent-facing tools");
  const call = (id, name, args) => ({
    jsonrpc: "2.0",
    id,
    method: "tools/call",
    params: { _meta: meta, name, arguments: args },
  });
  const foreign = [
    "import sort",
    "",
    "func main:",
    "    v := Vector {} as Vector<int>",
    "    v.push(2)",
    "    sort.SortBy(v, func (a: int, b: int): bool: ret a < b end)",
    "    for x in v:",
    "        println x",
    "    end",
    "    ret 0",
    "end",
    "",
  ].join("\n");
  const unfinished = "func main:\n    v := Vector {} as Vector<int>\n    println v.len(\n    each x in v:\n        println x\n    end\n    ret 0\nend\n";
  const { raw } = await converse(
    [
      { jsonrpc: "2.0", id: 1, method: "tools/list", params: { _meta: meta } },
      call(2, "salam_check", { source: foreign }),
      call(3, "salam_check", { source: unfinished }),
      call(4, "salam_docs", { query: "lambda" }),
      call(5, "salam_docs", { query: "E085" }),
      call(6, "salam_docs", {}),
      call(7, "salam_stdlib_search", { query: "read file" }),
      call(8, "salam_stdlib_symbols", { package: "json" }),
      call(9, "salam_stdlib_symbols", { package: "jsn" }),
      call(10, "salam_find_examples", { query: "sort.SortBy(", limit: 2, context: 1 }),
      call(11, "salam_format", { source: "func main:\n  x:=1\n  println x\n  ret 0\nend\n" }),
      call(12, "salam_translate", { source: "func main:\n    println 1\n    ret 0\nend\n", to: "fa" }),
      call(13, "salam_run", { source: 'func main:\n    name := input ""\n    println "hi ", name\n    ret 0\nend\n', stdin: "Sara\n" }),
      call(14, "salam_run", {
        files: {
          "main.salam": 'include util "util.salam"\nfunc main:\n    println util.Twice(21)\n    ret 0\nend\n',
          "util.salam": "package util\npub func Twice(x: int): int: ret x * 2 end\n",
        },
      }),
      call(15, "salam_check", { source: "func main:\n    ret 0\nend\n", files: { "../evil.salam": "x" } }),
      call(16, "salam_check", { path: "tools/mcp/tests/fixtures/ok.salam", source: "func main:\n    ret 0\nend\n" }),
      call(17, "salam_exec", { source: "func main:\n    mut i := 0\n    until i >= 0:\n        i += 1\n        if i > 1000: i = 0 end\n    end\n    ret 0\nend\n" }),
      call(18, "salam_check", { path: "tools/mcp/tests/fixtures/missing.salam" }),
      { jsonrpc: "2.0", id: 19, method: "prompts/list", params: { _meta: meta } },
      { jsonrpc: "2.0", id: 20, method: "prompts/get", params: { _meta: meta, name: "salam-write", arguments: { task: "print the first ten squares" } } },
      { jsonrpc: "2.0", id: 21, method: "prompts/get", params: { _meta: meta, name: "salam-port", arguments: {} } },
      { jsonrpc: "2.0", id: 22, method: "resources/templates/list", params: { _meta: meta } },
      { jsonrpc: "2.0", id: 23, method: "resources/read", params: { _meta: meta, uri: "salam://stdlib/encoding.json" } },
      { jsonrpc: "2.0", id: 24, method: "resources/read", params: { _meta: meta, uri: "salam://docs/error%20handling" } },
      { jsonrpc: "2.0", id: 25, method: "resources/read", params: { _meta: meta, uri: "salam://guide/skill.md" } },
      { jsonrpc: "2.0", id: 26, method: "server/discover", params: { _meta: meta } },
    ],
    { SALAM_MCP_RUN_TIMEOUT: "3" },
  );
  const r = Object.fromEntries(parseLines(raw).map((m) => [m.id, m]));

  const tools = r[1].result.tools;
  const names = tools.map((t) => t.name);
  for (const n of ["salam_docs", "salam_stdlib_search", "salam_translate"]) {
    check(`${n} is offered`, names.includes(n));
  }
  check(
    "every tool carries annotations with readOnlyHint",
    tools.every((t) => typeof t.annotations?.readOnlyHint === "boolean"),
  );
  check(
    "only the tools that execute code are not read-only",
    tools.filter((t) => !t.annotations.readOnlyHint).map((t) => t.name).sort().join() === "salam_exec,salam_run",
  );
  check(
    "local compiler tools accept path or source",
    tools.find((t) => t.name === "salam_check").inputSchema.properties.source !== undefined &&
      tools.find((t) => t.name === "salam_check").inputSchema.properties.path !== undefined,
  );

  const foreignText = textOf(r[2]);
  check("a check reports a one-line summary first", /^salam check main\.salam: \d+ errors/.test(foreignText), foreignText.split("\n")[0]);
  check(
    "a func-style lambda is explained",
    foreignText.includes("A lambda is written `(x: int): ret x * 2 end`") || foreignText.includes("a lambda is written without 'func'"),
  );
  check(
    "a for loop is explained with each/repeat",
    (foreignText.includes("Salam has no `for` loop") || foreignText.includes("Salam has no 'for' loop")) && foreignText.includes("each"),
  );
  check("diagnostics show the offending source line", foreignText.includes("| ") && foreignText.includes("for x in v:"));
  check("the same hint is shown once", foreignText.split("hint: Salam has no `for` loop").length <= 2);
  const diags = r[2].result.structuredContent.diagnostics;
  const cascade = diags.length >= 3 && diags[0].level === "error" && !diags[0].code;
  check("a parse-error cascade is called out", !cascade || foreignText.includes("knock-on effects"));
  const d0 = diags.find((d) => d.hint || d.help);
  check("structured diagnostics carry source and guidance", typeof d0?.source === "string" && typeof (d0?.hint ?? d0?.help) === "string");
  check("no diagnostic is the bare 'build aborted' line", diags.every((d) => !String(d.message).startsWith("build aborted")));

  check(
    "a keyword after an unfinished line points at the line above",
    textOf(r[3]).includes("the line above is unfinished") || textOf(r[3]).includes("the line above it is not finished"),
    textOf(r[3]),
  );

  check("salam_docs answers a topic from the reference", textOf(r[4]).includes("Functions") && textOf(r[4]).includes("SKILL.md"));
  check("salam_docs explains an error code with a failing program", textOf(r[5]).includes("// EXPECT: E085"));
  check("salam_docs with no query lists topics", textOf(r[6]).includes("Control flow"));

  const search = r[7].result.structuredContent.results;
  check(
    "stdlib search puts a whole-file reader first for 'read file'",
    ["ReadFile", "ReadAll", "ReadText"].includes(search[0]?.name) && search.slice(0, 5).some((d) => d.name === "ReadFile"),
    JSON.stringify(search[0]),
  );
  check("stdlib search gives the file and line", search.every((d) => d.file.startsWith("std/") && d.line > 0));

  check("a bare package leaf resolves to its dotted name", r[8].result.structuredContent?.package === "encoding.json");
  const jsonDecls = r[8].result.structuredContent.declarations;
  check("struct fields come with the struct", jsonDecls.some((d) => d.kind === "struct" && d.name === "JSONOptions" && d.fields?.includes("sort_keys")));
  check("methods are attached to their receiver", jsonDecls.some((d) => d.kind === "method" && d.receiver === "JSONEncoder" && d.name === "Encode"));
  check("one-line function bodies do not leak into signatures", jsonDecls.every((d) => !d.signature.includes(" ret ")));
  check("an unknown package suggests the near one", r[9].result.isError === true && textOf(r[9]).includes("encoding.json"));

  const ex = r[10].result.structuredContent.examples;
  check("examples come with context lines", ex.length > 0 && ex[0].snippet.split("\n").length === 3 && ex[0].snippet.includes("> "));

  check("format returns the canonical text", r[11].result.structuredContent?.changed === true && r[11].result.structuredContent.formatted.includes("    x := 1"));
  check("translate returns Persian source", r[12].result.structuredContent?.translated?.includes("تابع"));
  check("run feeds stdin to the program", r[13].result.structuredContent?.stdout?.includes("hi  Sara"), JSON.stringify(r[13].result.structuredContent));
  check("a multi-file program runs from files", r[14].result.structuredContent?.stdout?.trim() === "42", textOf(r[14]));
  check("a file name escaping the stage is refused", r[15].result.isError === true && textOf(r[15]).includes("Refused file name"));
  check("path and source together are refused", r[16].result.isError === true);
  check("an endless loop is stopped and reported as a timeout", r[17].result.structuredContent?.timedOut === true, textOf(r[17]));
  check("a missing path says so and suggests source", r[18].result.isError === true && textOf(r[18]).includes("No such file"));

  const prompts = r[19].result.prompts.map((p) => p.name).sort().join();
  check("prompts are offered", prompts === "salam-fix,salam-port,salam-write", prompts);
  const msg = r[20].result.messages?.[0]?.content?.text ?? "";
  check("a prompt carries the task and the rules", msg.includes("first ten squares") && msg.includes("`until cond:` is WHILE"));
  check("a prompt without its required argument is invalid params", r[21].error?.code === -32602);
  check("resource templates are listed", r[22].result.resourceTemplates?.some((t) => t.uriTemplate === "salam://stdlib/{package}"));
  check("a stdlib template resource is served", r[23].result.contents?.[0]?.text?.includes("import encoding.json"));
  check("a docs template resource decodes its topic", (r[24].result.contents?.[0]?.text ?? "").startsWith("From "));
  check("the full reference is served", (r[25].result.contents?.[0]?.text ?? "").includes("# Writing Salam"));
  const instructions = r[26].result.instructions ?? "";
  check("instructions carry the language crib", instructions.includes("There is no `for` and no `while`") && instructions.includes("salam_docs"));
  check(
    "every structuredContent is an object",
    parseLines(raw).every((m) => m.result?.structuredContent === undefined || (typeof m.result.structuredContent === "object" && !Array.isArray(m.result.structuredContent))),
  );
}

async function testHostedInputs() {
  console.log("hosted inputs");
  const port = await freePort();
  const child = spawn(serverPath, ["--http"], {
    env: {
      ...process.env,
      SALAM_MCP_ROOT: repoRoot,
      SALAM_MCP_BIN: compiler,
      SALAM_MCP_HOST: "127.0.0.1",
      SALAM_MCP_PORT: String(port),
    },
    stdio: ["ignore", "ignore", "pipe"],
  });
  const base = `http://127.0.0.1:${port}`;
  try {
    let up = false;
    for (let i = 0; i < 100 && !up; i += 1) {
      up = await fetch(`${base}/health`).then(
        (res) => res.ok,
        () => false,
      );
      if (!up) await new Promise((res) => setTimeout(res, 100));
    }
    if (!up) {
      check("hosted server starts", false);
      return;
    }
    const rpc = async (id, method, params) => {
      const res = await fetch(`${base}/mcp`, {
        method: "POST",
        headers: { "Content-Type": "application/json", Accept: "application/json, text/event-stream" },
        body: JSON.stringify({ jsonrpc: "2.0", id, method, params: { _meta: meta, ...params } }),
      });
      return res.json();
    };
    const list = await rpc(1, "tools/list", {});
    const check_schema = list.result.tools.find((t) => t.name === "salam_check").inputSchema.properties;
    check("hosted tools take source and files but not path", check_schema.source && check_schema.files && !check_schema.path);
    const byPath = await rpc(2, "tools/call", { name: "salam_check", arguments: { path: "main.salam" } });
    check("a path on the hosted server explains why it cannot work", textOf(byPath).includes("cannot read files from your machine"));
    const multi = await rpc(3, "tools/call", {
      name: "salam_check",
      arguments: {
        files: {
          "main.salam": 'include util "util.salam"\nfunc main:\n    println util.Twice(2)\n    ret 0\nend\n',
          "util.salam": "package util\npub func Twice(x: int): int: ret x * 2 end\n",
        },
      },
    });
    check("a hosted multi-file check passes", multi.result?.isError === false, textOf(multi));
    const docs = await rpc(4, "tools/call", { name: "salam_docs", arguments: { query: "loops" } });
    check("the hosted server answers docs", textOf(docs).includes("until"));
  } finally {
    child.kill();
  }
}

async function testSignIn() {
  console.log("sign-in");
  const authPort = await freePort();
  const port = await freePort();
  const resource = `http://127.0.0.1:${port}/mcp`;
  const auth = createHttpServer((req, res) => {
    let body = "";
    req.on("data", (d) => (body += d));
    req.on("end", () => {
      if (req.url === "/internal/mcp/events") {
        res.writeHead(204).end();
        return;
      }
      const token = (new URLSearchParams(body).get("token") ?? "").replace(/-token-for-tests$/, "");
      if (token === "down") {
        res.writeHead(500).end();
        return;
      }
      const exp = Math.floor(Date.now() / 1000) + 600;
      const grants = {
        good: { active: true, aud: resource, scope: "mcp", exp, sub: "u1", client_name: "test" },
        noscope: { active: true, aud: resource, scope: "profile", exp, sub: "u1", client_name: "test" },
      };
      res.writeHead(200, { "Content-Type": "application/json" });
      res.end(JSON.stringify(grants[token] ?? { active: false }));
    });
  });
  await new Promise((r) => auth.listen(authPort, "127.0.0.1", r));
  const child = spawn(serverPath, ["--http"], {
    env: {
      ...process.env,
      SALAM_MCP_ROOT: repoRoot,
      SALAM_MCP_BIN: compiler,
      SALAM_MCP_HOST: "127.0.0.1",
      SALAM_MCP_PORT: String(port),
      SALAM_MCP_AUTH_URL: `http://127.0.0.1:${authPort}`,
      SALAM_MCP_AUTH_SECRET: "test-secret",
      SALAM_MCP_RESOURCES: resource,
    },
    stdio: ["ignore", "ignore", "pipe"],
  });
  const base = `http://127.0.0.1:${port}`;
  try {
    let up = false;
    for (let i = 0; i < 100 && !up; i += 1) {
      up = await fetch(`${base}/health`).then(
        (res) => res.ok,
        () => false,
      );
      if (!up) await new Promise((res) => setTimeout(res, 100));
    }
    if (!up) {
      check("signed-in server starts", false);
      return;
    }
    const ping = (token) =>
      fetch(`${base}/mcp`, {
        method: "POST",
        headers: {
          "Content-Type": "application/json",
          Accept: "application/json, text/event-stream",
          ...(token ? { Authorization: `Bearer ${token}-token-for-tests` } : {}),
        },
        body: JSON.stringify({ jsonrpc: "2.0", id: 1, method: "ping", params: { _meta: meta } }),
      });
    const good = await ping("good");
    check("a valid token is accepted", good.status === 200, String(good.status));
    const none = await ping("");
    check("no token is 401 with a challenge", none.status === 401 && (none.headers.get("www-authenticate") ?? "").includes("resource_metadata"));
    const noscope = await ping("noscope");
    check(
      "a token without the mcp scope is 403 insufficient_scope",
      noscope.status === 403 && (noscope.headers.get("www-authenticate") ?? "").includes("insufficient_scope"),
      `${noscope.status} ${noscope.headers.get("www-authenticate")}`,
    );
    const down = await ping("down");
    check(
      "an unreachable sign-in service is 503 with Retry-After, not 401",
      down.status === 503 && down.headers.get("retry-after") === "5",
      `${down.status} ${down.headers.get("www-authenticate")}`,
    );
    const inactive = await ping("revoked");
    check(
      "an inactive token is 401 invalid_token",
      inactive.status === 401 && (inactive.headers.get("www-authenticate") ?? "").includes("invalid_token"),
      String(inactive.status),
    );
  } finally {
    child.kill();
    auth.close();
  }
}

// --- runner ------------------------------------------------------------

const suites = [
  testTransport,
  testModern,
  testLegacy,
  testTools,
  testWorkspace,
  testResources,
  testKeywords,
  testAgentTools,
  testHostedInputs,
  testSignIn,
  testHttp,
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
