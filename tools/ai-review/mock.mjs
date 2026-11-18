import net from "node:net";

const mode = process.env.MODE || "good";
const port = Number(process.env.PORT || 18120);
const posted = [];
const hits = { chat: 0 };

const pr = {
  number: 1,
  title: "Mock change",
  body: "Mock body",
  state: "open",
  draft: false,
  user: { login: "mock" },
  base: { ref: "main", sha: "a".repeat(40) },
  head: { ref: "topic", sha: "b".repeat(40) },
  changed_files: 1,
  additions: 1,
  deletions: 0,
  commits: 1,
  labels: [],
};
const files = [{ filename: "a.txt", status: "added", additions: 1, deletions: 0, patch: "@@ -0,0 +1 @@\n+hello" }];
const commits = [{ commit: { message: "mock", author: { name: "mock" } } }];

const review = JSON.stringify({ summary: "Adds a file.", decision: "merge", comment: "## Review\nLooks fine." });
const good = JSON.stringify({ choices: [{ index: 0, message: { role: "assistant", content: review } }] });

function http(status, type, body) {
  const data = Buffer.from(body);
  return `HTTP/1.1 ${status}\r\nContent-Type: ${type}\r\nContent-Length: ${data.length}\r\nConnection: close\r\n\r\n${body}`;
}

function openStream() {
  return "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nConnection: close\r\n\r\n";
}

function chat(sock) {
  hits.chat++;
  const attempt = hits.chat;
  const flaky = mode === "flaky" && attempt === 1;
  if (mode === "good" || (mode === "flaky" && !flaky)) {
    return sock.end(http("200 OK", "application/json", good));
  }
  if (mode === "padding") {
    sock.write(openStream() + "\n\n\n");
    return setTimeout(() => sock.end("\n\n" + good), 50);
  }
  if (mode === "html502" || flaky) {
    return sock.end(http("502 Bad Gateway", "text/html", "<html><body><h1>502 Bad Gateway</h1>nginx</body></html>"));
  }
  if (mode === "empty") {
    return sock.end(http("200 OK", "application/json", ""));
  }
  if (mode === "blank") {
    sock.write(openStream() + "\n\n\n");
    return setTimeout(() => sock.end(), 50);
  }
  if (mode === "truncated") {
    return sock.end(openStream() + "\n\n" + good.slice(0, 40));
  }
  if (mode === "errorobject") {
    return sock.end(http("200 OK", "application/json", JSON.stringify({ error: { message: "Salam AI Assistant could not answer.", type: "server_error", code: "engine_failed" } })));
  }
  if (mode === "text") {
    return sock.end(http("200 OK", "text/plain", "upstream connect error"));
  }
  if (mode === "unauthorized") {
    return sock.end(http("401 Unauthorized", "application/json", JSON.stringify({ error: { message: "bad key", code: "invalid_api_key" } })));
  }
  return sock.end(http("500 Internal Server Error", "text/plain", "unknown mode"));
}

function route(sock, method, path) {
  if (path === "/chat/completions") return chat(sock);
  if (path === "/stats") return sock.end(http("200 OK", "application/json", JSON.stringify({ chat: hits.chat, posted: posted.length })));
  if (path === "/repos/o/r/pulls/1") return sock.end(http("200 OK", "application/json", JSON.stringify(pr)));
  if (path.startsWith("/repos/o/r/pulls/1/files")) return sock.end(http("200 OK", "application/json", JSON.stringify(files)));
  if (path.startsWith("/repos/o/r/pulls/1/commits")) return sock.end(http("200 OK", "application/json", JSON.stringify(commits)));
  if (path.startsWith("/repos/o/r/issues/1/comments") && method === "GET") return sock.end(http("200 OK", "application/json", "[]"));
  if (path.startsWith("/repos/o/r/issues/1/comments") && method === "POST") {
    posted.push(1);
    return sock.end(http("201 Created", "application/json", JSON.stringify({ id: 7 })));
  }
  return sock.end(http("404 Not Found", "text/plain", "no route " + path));
}

net
  .createServer((sock) => {
    let buf = Buffer.alloc(0);
    sock.on("error", () => {});
    sock.on("data", (d) => {
      buf = Buffer.concat([buf, d]);
      const text = buf.toString("latin1");
      const end = text.indexOf("\r\n\r\n");
      if (end < 0) return;
      const head = text.slice(0, end);
      const len = Number((/content-length:\s*(\d+)/i.exec(head) || [0, 0])[1]);
      if (buf.length < end + 4 + len) return;
      sock.removeAllListeners("data");
      const [method, path] = head.split("\r\n")[0].split(" ");
      route(sock, method, path.split("?")[0]);
    });
  })
  .listen(port, "127.0.0.1", () => console.log("listening " + port));
