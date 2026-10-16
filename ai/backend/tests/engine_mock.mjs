import http from "node:http";
const sessions = new Map();
let n = 0;
const EM = String.fromCodePoint(0x2014);
const EN = String.fromCodePoint(0x2013);
const BRIDGE = process.env.BRIDGE_MCP ?? "http://127.0.0.1:18091/bridge/mcp";
const BRIDGE_SECRET = process.env.BRIDGE_SECRET ?? "";
const answerFor = (s) => (s.tool !== undefined ? `Tool said (${s.agent}): ${s.tool}` : `Echo: ${s.prompt.slice(0, 40)} ${EM} done. Range 1${EN}5.`);
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
http.createServer((req, res) => {
  let body = "";
  req.on("data", (d) => (body += d));
  req.on("end", () => {
    const url = new URL(req.url, "http://x");
    const send = (code, obj) => { res.writeHead(code, { "Content-Type": "application/json" }); res.end(JSON.stringify(obj)); };
    let m;
    if (req.method === "POST" && url.pathname === "/session") { const id = `ses_${++n}`; sessions.set(id, { prompt: "", start: 0 }); return send(200, { id }); }
    if (req.method === "GET" && url.pathname === "/session/status") {
      const busy = {};
      for (const [id, s] of sessions) if (s.waiting || (s.start && Date.now() - s.start < s.span)) busy[id] = { type: "busy" };
      return send(200, busy);
    }
    if ((m = /^\/session\/([^/]+)\/prompt_async$/.exec(url.pathname))) { const s = sessions.get(m[1]); const b = JSON.parse(body); s.prompt = b.parts[0].text; s.agent = b.agent; s.start = Date.now(); s.tool = undefined; s.span = 1000 * Number(/slow: (\d+)/.exec(s.prompt)?.[1] ?? 0) + 1200; if (s.prompt.includes("<desktop_session>")) { s.waiting = true; callDesktop(s).then(() => (s.waiting = false)); } res.writeHead(204); return res.end(); }
    if ((m = /^\/session\/([^/]+)\/message$/.exec(url.pathname))) {
      const s = sessions.get(m[1]);
      if (!s) return send(404, {});
      const full = answerFor(s);
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
