import http from "node:http";
const sessions = new Map();
let n = 0;
const EM = String.fromCodePoint(0x2014);
const EN = String.fromCodePoint(0x2013);
const BRIDGE = process.env.BRIDGE_MCP ?? "http://127.0.0.1:18091/bridge/mcp";
const BRIDGE_SECRET = process.env.BRIDGE_SECRET ?? "";
const FENCE = "`".repeat(3);
const code = (body) => `${FENCE}salam\n${body}\n${FENCE}`;
const answerFor = (s) => {
  if (s.tool !== undefined) return `Tool said (${s.agent}): ${s.tool}`;
  if (s.prompt.includes("<compiler_check>")) return `Fixed (${s.agent}, ${s.prompt.includes("unexpected") ? "saw errors" : "no errors"}):\n${code('func main:\n    println "hi"\nend')}`;
  if (s.prompt.includes("bad code")) return `Here:\n${code('func main() {\n    println("hi");\n}')}`;
  if (s.prompt.includes("snippet code")) return `Use:\n${code("println name")}`;
  return `Echo: ${s.prompt.slice(0, 40)} ${EM} done. Range 1${EN}5.`;
};
async function callDesktop(s) {
  const session = /session: ([0-9a-f]{32})/.exec(s.prompt)?.[1] ?? "";
  const command = /run: (.*)$/m.exec(s.prompt)?.[1] ?? "echo hi";
  const call = async (name, args) => {
    const res = await fetch(BRIDGE, {
      method: "POST",
      headers: { "Content-Type": "application/json", Authorization: `Bearer ${BRIDGE_SECRET}` },
      body: JSON.stringify({ jsonrpc: "2.0", id: 1, method: "tools/call", params: { name, arguments: args } }),
    });
    return (await res.json()).result;
  };
  try {
    let r = await call("shell", { session, command });
    s.waits = 0;
    for (let id = /call_id "([0-9a-f]+)"/.exec(r.content[0].text)?.[1]; id && !r.isError; id = /call_id "([0-9a-f]+)"/.exec(r.content[0].text)?.[1]) {
      s.waits += 1;
      r = await call("wait", { session, call_id: id });
    }
    s.tool = `${r.isError ? "error" : "ok"} ${r.content[0].text}${s.waits ? ` after ${s.waits} waits` : ""}`;
  } catch (e) {
    s.tool = `error ${e}`;
  }
  s.start = Date.now();
}
const live = new Set();
const emit = (obj) => {
  for (const res of live) {
    try {
      res.write(`data: ${JSON.stringify(obj)}\n\n`);
    } catch {}
  }
};
function streamParts(id, s) {
  if (!process.env.MOCK_LIVE) return;
  if (s.prompt.includes("goes quiet") || s.prompt.includes("fail midway")) return;
  const full = answerFor(s);
  const pid = `prt_${id}_${++n}`;
  const mid = `msg_${id}_${n}`;
  emit({ type: "message.part.updated", properties: { part: { id: pid, sessionID: id, messageID: mid, type: "text", text: "" } } });
  let at = 0;
  const step = () => {
    if (s.dead || !live.size) return;
    const piece = full.slice(at, at + 12);
    at += 12;
    if (piece) emit({ type: "message.part.delta", properties: { sessionID: id, partID: pid, messageID: mid, field: "text", delta: piece } });
    if (at < full.length) setTimeout(step, 30);
    else emit({ type: "session.idle", properties: { sessionID: id } });
  };
  setTimeout(step, 80);
}

http.createServer((req, res) => {
  let body = "";
  req.on("data", (d) => (body += d));
  req.on("end", () => {
    const url = new URL(req.url, "http://x");
    const send = (code, obj) => { res.writeHead(code, { "Content-Type": "application/json" }); res.end(JSON.stringify(obj)); };
    let m;
    if (req.method === "GET" && url.pathname === "/event") {
      res.writeHead(200, { "Content-Type": "text/event-stream", "Cache-Control": "no-cache", Connection: "keep-alive" });
      res.write(": ready\n\n");
      live.add(res);
      req.on("close", () => live.delete(res));
      return;
    }
    if (req.method === "GET" && url.pathname === "/agent") return send(200, (process.env.MOCK_AGENTS ?? "salam,salam-agent").split(",").map((name) => ({ name })));
    if (req.method === "POST" && url.pathname === "/session") { const id = `ses_${++n}`; sessions.set(id, { prompt: "", start: 0 }); return send(200, { id }); }
    if (req.method === "GET" && url.pathname === "/session/status") {
      const busy = {};
      for (const [id, s] of sessions) if (!s.dead && (s.waiting || (s.start && Date.now() - s.start < s.span))) busy[id] = { type: "busy" };
      return send(200, busy);
    }
    if ((m = /^\/session\/([^/]+)\/prompt_async$/.exec(url.pathname))) { const s = sessions.get(m[1]); const b = JSON.parse(body); s.dead = false; s.prompt = b.parts[0].text; s.agent = b.agent; s.start = Date.now(); s.tool = undefined; s.span = 1000 * Number(/slow: (\d+)/.exec(s.prompt)?.[1] ?? 0) + 1200; if (s.prompt.includes("never start") || !(process.env.MOCK_AGENTS ?? "salam,salam-agent").split(",").includes(s.agent)) { s.dead = true; res.writeHead(204); return res.end(); } if (s.prompt.includes("<desktop_session>")) { s.waiting = true; callDesktop(s).then(() => (s.waiting = false)); } else { streamParts(m[1], s); } res.writeHead(204); return res.end(); }
    if ((m = /^\/session\/([^/]+)\/message$/.exec(url.pathname))) {
      const s = sessions.get(m[1]);
      if (!s) return send(404, {});
      if (s.dead) return send(200, [{ info: { role: "user", agent: s.agent }, parts: [{ type: "text", text: s.prompt }] }]);
      const full = answerFor(s);
      if (s.prompt.includes("goes quiet")) {
        return send(200, [
          { info: { role: "user", agent: s.agent }, parts: [{ type: "text", text: s.prompt }] },
          { info: { role: "assistant", finish: "tool-calls", time: { created: s.start, completed: s.start + 100 }, cost: 0, tokens: { input: 10, output: 2, reasoning: 0, cache: { read: 0, write: 0 } } }, parts: [{ type: "text", text: "half an answer and then nothing" }] },
        ]);
      }
      if (s.prompt.includes("fail midway")) {
        const broke = Date.now() - s.start > 1500;
        return send(200, [
          { info: { role: "user", agent: s.agent }, parts: [{ type: "text", text: s.prompt }] },
          { info: { role: "assistant", error: broke ? { name: "ProviderError", data: { message: "the provider stopped" } } : null, finish: "", time: { created: s.start }, cost: 0, tokens: { input: 10, output: 2, reasoning: 0, cache: { read: 0, write: 0 } } }, parts: [{ type: "text", text: full.slice(0, Math.floor(full.length / 2)) }] },
        ]);
      }
      const frac = s.waiting ? 0 : Math.max(0, Math.min(1, (Date.now() - s.start - s.span + 1200) / 1200));
      const done = frac >= 1;
      const text = s.waiting || frac === 0 ? "" : full.slice(0, Math.max(1, Math.floor(full.length * frac)));
      return send(200, [
        { info: { role: "user", agent: s.agent }, parts: [{ type: "text", text: s.prompt }] },
        { info: { role: "assistant", finish: "tool-calls", time: { created: s.start, completed: s.start + 200 }, cost: 0.0004, tokens: { input: 60, output: 8, reasoning: 2, cache: { read: 200, write: 10 } } }, parts: [{ type: "tool", tool: "salam_salam_check", state: { status: "completed" } }] },
        { info: { role: "assistant", finish: done ? "stop" : "", time: done ? { created: s.start + 200, completed: s.start + s.span } : { created: s.start + 200 }, cost: 0.0008, tokens: { input: 40, output: 12, reasoning: 3, cache: { read: 100, write: 0 } } }, parts: [{ type: "text", text }] },
      ]);
    }
    if ((m = /^\/session\/([^/]+)\/abort$/.exec(url.pathname))) return send(200, true);
    if (req.method === "DELETE" && (m = /^\/session\/([^/]+)$/.exec(url.pathname))) { sessions.delete(m[1]); return send(200, true); }
    send(404, {});
  });
}).listen(Number(process.env.PORT ?? 18099), "127.0.0.1");
