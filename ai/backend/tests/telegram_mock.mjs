import http from "node:http";

const PORT = Number(process.env.PORT ?? 18097);
const TOKEN = process.env.TELEGRAM_TOKEN ?? "123456:TEST";
let queue = [];
let calls = [];
let nextUpdate = 100;
let nextMessage = 500;
const waiters = new Set();

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
      if (url.pathname === "/_log") return send(200, calls);
      if (url.pathname === "/_reset") {
        calls = [];
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
      if (method === "getMe") return send(200, { ok: true, result: { id: 1, is_bot: true, username: "MockBot" } });
      if (method === "sendMessage") return send(200, { ok: true, result: { message_id: nextMessage++, chat: { id: args.chat_id }, text: args.text } });
      if (method === "editMessageText") return send(200, { ok: true, result: { message_id: args.message_id, text: args.text } });
      return send(200, { ok: true, result: true });
    });
  })
  .listen(PORT, "127.0.0.1", () => console.log(`telegram mock on ${PORT}`));
