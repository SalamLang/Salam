import crypto from "node:crypto";
import http from "node:http";

const PORT = Number(process.env.PORT ?? 18089);
const CONTROL = Number(process.env.CONTROL_PORT ?? 18088);
const SECRET = process.env.SPEECH_SECRET ?? "";
const TEXT = { fa: 'سلام دنیا، این یک "آزمایش" است', en: 'hello "world", this is a test' };

let mode = "ok";
let last = null;
let served = 0;
let inflight = 0;
const open = new Set();

function reply(res, status, body) {
  const data = Buffer.from(JSON.stringify(body));
  res.writeHead(status, { "Content-Type": "application/json", "Content-Length": data.length, Connection: "close" });
  res.end(data);
}

const service = http.createServer((req, res) => {
  const url = new URL(req.url, "http://mock");
  if (req.method === "GET" && url.pathname === "/health") {
    reply(res, 200, { ok: true, languages: ["fa", "en"], max_seconds: 120, max_bytes: 300000, model: "SecretModelName" });
    return;
  }
  const chunks = [];
  req.on("data", (c) => chunks.push(c));
  req.on("end", async () => {
    const body = Buffer.concat(chunks);
    last = {
      path: url.pathname,
      language: url.searchParams.get("language"),
      size: body.length,
      sha: crypto.createHash("sha256").update(body).digest("hex"),
      type: req.headers["content-type"],
      authorized: !SECRET || req.headers.authorization === `Bearer ${SECRET}`,
    };
    served += 1;
    inflight += 1;
    try {
      if (!last.authorized) reply(res, 401, { error: "unauthorized" });
      else if (mode === "busy") reply(res, 503, { error: "busy", detail: "SecretModelName is full" });
      else if (mode === "too_long") reply(res, 413, { error: "too_long" });
      else if (mode === "bad_audio") reply(res, 400, { error: "bad_audio", detail: "SecretModelName could not decode" });
      else if (mode === "broken") reply(res, 500, { error: "failed", detail: "/models/SecretModelName missing" });
      else if (mode === "hang") return;
      else if (mode === "slow") {
        await new Promise((r) => setTimeout(r, 800));
        reply(res, 200, { text: TEXT[last.language], language: last.language, seconds: 1.5 });
      } else reply(res, 200, { text: TEXT[last.language], language: last.language, seconds: mode === "long" ? 900.2 : 2.4, model: "SecretModelName" });
    } finally {
      inflight -= 1;
    }
  });
});
service.on("connection", (socket) => {
  open.add(socket);
  socket.on("close", () => open.delete(socket));
});

function setMode(next) {
  const wasDown = mode === "down";
  mode = next;
  if (next === "down" && !wasDown) {
    service.close();
    for (const socket of open) socket.destroy();
  } else if (next !== "down" && wasDown) {
    service.listen(PORT, "127.0.0.1");
  }
}

http
  .createServer((req, res) => {
    const url = new URL(req.url, "http://mock");
    if (url.pathname === "/mode") setMode(url.searchParams.get("mode"));
    reply(res, 200, { mode, last, served, inflight });
  })
  .listen(CONTROL, "127.0.0.1");
service.listen(PORT, "127.0.0.1", () => console.log(`speech mock on ${PORT}, control on ${CONTROL}`));
