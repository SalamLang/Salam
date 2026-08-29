// The node/express equivalent of main.salam. Same routes, same response
// bodies, byte for byte, so the only thing that differs between the two
// columns is the server.
//
//   CLUSTER=4 PORT=8102 node server.js
//
// express is here because express is what "a node HTTP server" means to most
// people who would ask this question. A bare `http.createServer` is faster
// and would flatter node; it would also be measuring something nobody ships.
//
// node is single-threaded, so it gets `cluster` with one worker per core the
// same way nginx gets worker_processes and Salam gets threads. Measuring one
// node process against four nginx workers would be a comparison of core
// counts wearing a benchmark's clothes.

const cluster = require("node:cluster");
const fs = require("node:fs");
const path = require("node:path");
const express = require("express");

const PORT = Number(process.env.PORT) || 8102;
const CLUSTER = Math.max(1, Number(process.env.CLUSTER) || 1);
const ASSETS =
  process.env.HTTPBENCH_ASSETS || path.join(__dirname, "..", "..", "public");

// The forking primary supervises and never serves: it forks, replaces dead
// workers, and skips the listen at the bottom of this file. Everything in
// between is cheap enough to set up in a process that will not use it.
const SUPERVISOR = cluster.isPrimary && CLUSTER > 1;

if (SUPERVISOR) {
  for (let i = 0; i < CLUSTER; i++) cluster.fork();
  // A worker that dies mid-run would leave the rest of the routes measured
  // against a smaller server than the one the header claims. Replacing it
  // keeps the core budget honest; run.sh separately notices a server that
  // dies outright.
  cluster.on("exit", () => cluster.fork());
}

// Read once at boot, which is the whole point of /cached sitting next to
// /file. An unreadable asset leaves it empty rather than throwing, so the
// route answers instead of killing the worker.
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
  // A query parameter can arrive as an array (?n=1&n=2), so anything that
  // is not a plain string falls back rather than being cast.
  const raw = req.query[name];
  if (typeof raw !== "string" || raw === "") return fallback;
  const v = parseInt(raw, 10);
  if (Number.isNaN(v)) return fallback;
  return Math.max(lo, Math.min(hi, v));
}

function row(p, note) {
  return `<tr><td><a href="${p}">${p}</a></td><td>${note}</td></tr>\n`;
}

// Deliberately minimal, matching escape_json in main.salam: enough that an
// arbitrary request body cannot break out of the JSON string it goes into.
function escapeJson(s) {
  let out = "";
  for (const c of s) {
    if (c === '"') out += '\\"';
    else if (c === "\\") out += "\\\\";
    else if (c === "\n") out += "\\n";
    else if (c === "\r") out += "\\r";
    else if (c === "\t") out += "\\t";
    else out += c;
  }
  return out;
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

// Pays a full open/read/close per request, on purpose.
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

// The same bytes with the read hoisted out. /file minus /cached is the cost
// of the filesystem on this machine, per request.
app.get("/cached", (_req, res) => jsonOk(res, CACHED));

app.get("/users/:id", (req, res) => {
  const id = req.params.id;
  if (!id) {
    res.status(400).type("application/json").send('{"error":"missing id"}');
    return;
  }
  jsonOk(res, `{"id":"${id}","name":"user-${id}","active":true}`);
});

app.get("/search", (req, res) => {
  const raw = req.query.q;
  const q = typeof raw === "string" ? raw : "";
  const n = intQuery(req, "n", 5, 0, 100);
  let out = `{"query":"${q}","count":${n},"results":[`;
  for (let i = 0; i < n; i++) {
    if (i > 0) out += ",";
    out += `{"rank":${i + 1},"title":"${q} result ${i + 1}"}`;
  }
  jsonOk(res, `${out}]}`);
});

// Tunable CPU work, identical in shape to main.salam's loop.
app.get("/compute", (req, res) => {
  const n = intQuery(req, "n", 1000, 0, 5000000);
  let acc = 0;
  for (let i = 0; i < n; i++) acc += (i * i) % 7;
  jsonOk(res, `{"n":${n},"sum":${acc}}`);
});

app.get("/headers", (req, res) => {
  jsonOk(
    res,
    `{"host":"${req.headers.host || ""}",` +
      `"user_agent":"${req.headers["user-agent"] || ""}",` +
      `"accept":"${req.headers.accept || ""}",` +
      `"method":"${req.method}","path":"${req.path}"}`,
  );
});

// Raw body: express.text() with a wildcard type, because the generator sends
// text/plain and a JSON parser would reject anything that is not JSON and
// turn the route into a 400 factory.
app.post("/echo", express.text({ type: "*/*", limit: "10mb" }), (req, res) => {
  const body = typeof req.body === "string" ? req.body : "";
  jsonOk(
    res,
    `{"bytes":${Buffer.byteLength(body)},"echo":"${escapeJson(body)}"}`,
  );
});

// An HTML page assembled per request out of pieces, which is what a
// templating layer ultimately does. Byte-identical to main.salam's / route.
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
