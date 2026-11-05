import http from "node:http";

const PORT = Number(process.env.PORT ?? 18097);
const TOKEN = process.env.TELEGRAM_TOKEN ?? "123456:TEST";
let queue = [];
let calls = [];
let nextUpdate = 100;
let nextMessage = 500;
const waiters = new Set();
const files = new Map();
let downloads = [];

const wake = () => {
  for (const w of [...waiters]) w();
};

http
  .createServer((req, res) => {
    let body = "";
    req.on("data", (d) => (body += d));
    req.on("end", () => {
      const url = new URL(req.url, "http://x");
      const send = (code, obj) => {
        res.writeHead(code, { "Content-Type": "application/json" });
        res.end(JSON.stringify(obj));
      };
      if (url.pathname === "/_inject" && req.method === "POST") {
        queue.push({ update_id: nextUpdate++, ...JSON.parse(body) });
        wake();
        return send(200, { ok: true });
      }
      if (url.pathname === "/_file" && req.method === "POST") {
        const f = JSON.parse(body);
        files.set(f.file_id, { ...f, bytes: Buffer.from(f.b64 ?? "", "base64") });
        return send(200, { ok: true });
      }
      if (url.pathname === "/_downloads") return send(200, downloads);
      if (url.pathname === "/_log") return send(200, calls);
      const dl = /^\/file\/bot([^/]+)\/(.+)$/.exec(url.pathname);
      if (dl) {
        const entry = [...files.values()].find((f) => f.path === dl[2]);
        downloads.push({ path: dl[2], tokenOk: dl[1] === TOKEN });
        if (dl[1] !== TOKEN || !entry) {
          res.writeHead(404, { "Content-Type": "application/json" });
          return res.end(JSON.stringify({ ok: false, error_code: 404, description: "Not Found" }));
        }
        if (entry.status && entry.status !== 200) {
          res.writeHead(entry.status, { "Content-Type": "text/plain", "Content-Length": 4 });
          return res.end("fail");
        }
        res.writeHead(200, { "Content-Type": "application/octet-stream", "Content-Length": entry.bytes.length });
        return res.end(entry.bytes);
      }
      if (url.pathname === "/_reset") {
        calls = [];
        downloads = [];
        return send(200, { ok: true });
      }
      const m = /^\/bot([^/]+)\/(\w+)$/.exec(url.pathname);
      if (!m || m[1] !== TOKEN) return send(401, { ok: false, error_code: 401, description: "Unauthorized" });
      const method = m[2];
      const args = body ? JSON.parse(body) : {};
      calls.push({ method, args });
      if (method === "getUpdates") {
        const offset = Number(args.offset ?? 0);
        queue = queue.filter((u) => u.update_id >= offset);
        const reply = () => send(200, { ok: true, result: queue.splice(0, queue.length) });
        if (queue.length) return reply();
        const timer = setTimeout(() => {
          waiters.delete(done);
          reply();
        }, 1500);
        const done = () => {
          clearTimeout(timer);
          waiters.delete(done);
          if (!res.writableEnded) reply();
        };
        waiters.add(done);
        return;
      }
      if (method === "getFile") {
        const entry = files.get(args.file_id);
        if (!entry) return send(400, { ok: false, error_code: 400, description: "Bad Request: wrong file_id" });
        const result = { file_id: entry.file_id, file_unique_id: "u", file_path: entry.path };
        result.file_size = entry.declared ?? entry.bytes.length;
        return send(200, { ok: true, result });
      }
      if (method === "getMe") return send(200, { ok: true, result: { id: 1, is_bot: true, username: "MockBot" } });
      if (method === "sendMessage") return send(200, { ok: true, result: { message_id: nextMessage++, chat: { id: args.chat_id }, text: args.text } });
      if (method === "editMessageText") return send(200, { ok: true, result: { message_id: args.message_id, text: args.text } });
      return send(200, { ok: true, result: true });
    });
  })
  .listen(PORT, "127.0.0.1", () => console.log(`telegram mock on ${PORT}`));
