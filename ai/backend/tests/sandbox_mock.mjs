import http from "node:http";
import fs from "node:fs";
import os from "node:os";
import path from "node:path";
import { spawn } from "node:child_process";

const PORT = Number(process.env.PORT ?? 18096);
const SECRET = process.env.SANDBOX_SECRET ?? "sandboxsecret123456";
const ROOT = fs.mkdtempSync(path.join(os.tmpdir(), "sbx-mock-"));
const boxes = new Set();
const calls = [];

http
  .createServer((req, res) => {
    let body = "";
    req.on("data", (d) => (body += d));
    req.on("end", () => {
      const send = (code, obj) => {
        res.writeHead(code, { "Content-Type": "application/json" });
        res.end(JSON.stringify(obj));
      };
      if (req.url === "/_calls") return send(200, calls);
      if (req.headers.authorization !== `Bearer ${SECRET}`) return send(401, { error: "unauthorized", message: "the sandbox secret is wrong" });
      const data = body ? JSON.parse(body) : {};
      if (req.url === "/v1/destroy") {
        boxes.delete(data.session);
        fs.rmSync(path.join(ROOT, data.session), { recursive: true, force: true });
        return send(200, { ok: true });
      }
      if (req.url !== "/v1/exec") return send(404, { error: "not_found" });
      calls.push({ session: data.session, cmd: data.cmd, env: Object.keys(data.env ?? {}) });
      const work = path.join(ROOT, data.session, "work");
      fs.mkdirSync(work, { recursive: true });
      boxes.add(data.session);
      const cwd = data.cwd.startsWith("/work") ? path.join(work, data.cwd.slice(5)) : work;
      fs.mkdirSync(cwd, { recursive: true });
      // codeql[js/command-line-injection] a test double that runs the command it is given on purpose, bound to 127.0.0.1 and used only by the flow tests
      const child = spawn("bash", ["-c", data.cmd], { cwd, env: { ...process.env, ...(data.env ?? {}), HOME: path.join(ROOT, data.session) } });
      let out = "";
      let timedOut = false;
      child.stdout.on("data", (d) => (out += d));
      child.stderr.on("data", (d) => (out += d));
      const timer = setTimeout(() => {
        timedOut = true;
        child.kill("SIGKILL");
      }, (data.timeout ?? 120) * 1000);
      if (data.stdin) child.stdin.write(data.stdin);
      child.stdin.end();
      child.on("close", (code) => {
        clearTimeout(timer);
        send(200, { exit: timedOut ? 124 : (code ?? -1), output: out, timed_out: timedOut });
      });
    });
  })
  .listen(PORT, "127.0.0.1", () => console.log(`sandbox mock on ${PORT}`));
