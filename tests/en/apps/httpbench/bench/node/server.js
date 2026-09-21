const cluster = require("node:cluster");
const fs = require("node:fs");
const path = require("node:path");
const express = require("express");

const PORT = Number(process.env.PORT) || 8102;
const CLUSTER = Math.max(1, Number(process.env.CLUSTER) || 1);
const ASSETS =
  process.env.HTTPBENCH_ASSETS || path.join(__dirname, "..", "..", "public");

const SUPERVISOR = cluster.isPrimary && CLUSTER > 1;

if (SUPERVISOR) {
  for (let i = 0; i < CLUSTER; i++) cluster.fork();
  cluster.on("exit", () => cluster.fork());
}

let CACHED = "";
try {
  CACHED = fs.readFileSync(path.join(ASSETS, "data.json"), "utf8");
} catch {
  CACHED = "";
}

const app = express();
app.disable("x-powered-by");
app.disable("etag");

function text(res, body) {
  res.status(200).type("text/plain; charset=utf-8").send(body);
}

function jsonOk(res, body) {
  res.status(200).type("application/json").send(body);
}

function intQuery(req, name, fallback, lo, hi) {
  const raw = req.query[name];
  if (typeof raw !== "string" || raw === "") return fallback;
  const v = parseInt(raw, 10);
  if (Number.isNaN(v)) return fallback;
  return Math.max(lo, Math.min(hi, v));
}

function row(p, note) {
  return `<tr><td><a href="${p}">${p}</a></td><td>${note}</td></tr>\n`;
}

app.get("/plaintext", (_req, res) => text(res, "Hello, World!"));

app.get("/health", (_req, res) =>
  jsonOk(res, '{"status":"ok","service":"httpbench"}'),
);

app.get("/json", (_req, res) =>
  jsonOk(
    res,
    '{"message":"Hello, World!","server":"node","routes":13,"ok":true}',
  ),
);

app.get("/file", (_req, res) => {
  fs.readFile(path.join(ASSETS, "data.json"), "utf8", (err, body) => {
    if (err || !body) {
      res
        .status(500)
        .type("application/json")
        .send('{"error":"asset missing"}');
      return;
    }
    jsonOk(res, body);
  });
});

app.get("/cached", (_req, res) => jsonOk(res, CACHED));

app.get("/users/:id", (req, res) => {
  const id = req.params.id;
  if (!id) {
    res.status(400).type("application/json").send('{"error":"missing id"}');
    return;
  }
  jsonOk(res, JSON.stringify({ id: id, name: `user-${id}`, active: true }));
});

app.get("/search", (req, res) => {
  const raw = req.query.q;
  const q = typeof raw === "string" ? raw : "";
  const n = intQuery(req, "n", 5, 0, 100);
  const results = [];
  for (let i = 0; i < n; i++) {
    results.push({ rank: i + 1, title: `${q} result ${i + 1}` });
  }
  jsonOk(res, JSON.stringify({ query: q, count: n, results: results }));
});

app.get("/compute", (req, res) => {
  const n = intQuery(req, "n", 1000, 0, 5000000);
  let acc = 0;
  for (let i = 0; i < n; i++) acc += (i * i) % 7;
  jsonOk(res, `{"n":${n},"sum":${acc}}`);
});

app.get("/headers", (req, res) => {
  jsonOk(
    res,
    JSON.stringify({
      host: req.headers.host || "",
      user_agent: req.headers["user-agent"] || "",
      accept: req.headers.accept || "",
      method: req.method,
      path: req.path,
    }),
  );
});

app.post("/echo", express.text({ type: "*/*", limit: "10mb" }), (req, res) => {
  const body = typeof req.body === "string" ? req.body : "";
  jsonOk(res, JSON.stringify({ bytes: Buffer.byteLength(body), echo: body }));
});

app.get("/", (_req, res) => {
  let b = '<!doctype html>\n<html lang="en">\n<head>\n';
  b += '<meta charset="utf-8">\n';
  b += '<meta name="viewport" content="width=device-width, initial-scale=1">\n';
  b += "<title>Salam httpbench</title>\n";
  b += '<link rel="stylesheet" href="/static/style.css">\n';
  b += "</head>\n<body>\n<main>\n";
  b += "<h1>Salam httpbench</h1>\n";
  b += "<p>A server built to be measured. Each route isolates one cost.</p>\n";
  b += "<table>\n<tr><th>Route</th><th>What it costs</th></tr>\n";
  b += row("/plaintext", "the floor: accept, parse, route, write");
  b += row("/json", "small-object serialization");
  b += row("/", "this page, assembled per request");
  b += row("/file", "a disk read on every request");
  b += row("/cached", "the same bytes, read once at boot");
  b += row("/static/style.css", "the built-in static file path");
  b += row("/users/:id", "one router-extracted path parameter");
  b += row("/search?q=salam&n=5", "query-string parsing");
  b += row("/compute?n=1000", "tunable CPU work");
  b += row("/headers", "request headers walked and echoed");
  b += row("/echo (POST)", "request body read back out");
  b += row("/health", "a load balancer's poll");
  b += "</table>\n</main>\n</body>\n</html>\n";
  res.status(200).type("text/html; charset=utf-8").send(b);
});

app.use(
  "/static",
  express.static(ASSETS, { etag: false, lastModified: false }),
);

app.use((_req, res) =>
  res.status(404).type("text/plain; charset=utf-8").send("404 Not Found"),
);

if (!SUPERVISOR) {
  app.listen(PORT, "127.0.0.1", () => {
    if (!cluster.isPrimary) return;
    console.log(`httpbench-node on http://127.0.0.1:${PORT}`);
  });
}
