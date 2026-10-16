import http from "node:http";
const sessions = new Map();
let n = 0;
const EM = String.fromCodePoint(0x2014);
const EN = String.fromCodePoint(0x2013);
const answerFor = (prompt) => `Echo: ${prompt.slice(0, 40)} ${EM} done. Range 1${EN}5.`;
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
      for (const [id, s] of sessions) if (s.start && Date.now() - s.start < 1200) busy[id] = { type: "busy" };
      return send(200, busy);
    }
    if ((m = /^\/session\/([^/]+)\/prompt_async$/.exec(url.pathname))) { const s = sessions.get(m[1]); s.prompt = JSON.parse(body).parts[0].text; s.start = Date.now(); res.writeHead(204); return res.end(); }
    if ((m = /^\/session\/([^/]+)\/message$/.exec(url.pathname))) {
      const s = sessions.get(m[1]);
      if (!s) return send(404, {});
      const full = answerFor(s.prompt);
      const frac = Math.min(1, (Date.now() - s.start) / 1200);
      const done = frac >= 1;
      const text = full.slice(0, Math.max(1, Math.floor(full.length * frac)));
      return send(200, [
        { info: { role: "user" }, parts: [{ type: "text", text: s.prompt }] },
        { info: { role: "assistant", finish: "tool-calls", time: { created: s.start, completed: s.start + 200 }, cost: 0.0004, tokens: { input: 60, output: 8, reasoning: 2, cache: { read: 200, write: 10 } } }, parts: [{ type: "tool", tool: "salam_salam_check", state: { status: "completed" } }] },
        { info: { role: "assistant", finish: done ? "stop" : "", time: done ? { created: s.start + 200, completed: s.start + 1200 } : { created: s.start + 200 }, cost: 0.0008, tokens: { input: 40, output: 12, reasoning: 3, cache: { read: 100, write: 0 } } }, parts: [{ type: "text", text }] },
      ]);
    }
    if ((m = /^\/session\/([^/]+)\/abort$/.exec(url.pathname))) return send(200, true);
    if (req.method === "DELETE" && (m = /^\/session\/([^/]+)$/.exec(url.pathname))) { sessions.delete(m[1]); return send(200, true); }
    send(404, {});
  });
}).listen(Number(process.env.PORT ?? 18099), "127.0.0.1");
