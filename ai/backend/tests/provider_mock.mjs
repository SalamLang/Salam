import http from "node:http";

const PORT = Number(process.env.PORT ?? 18095);
const GH_TOKEN = "ghp_validtoken0123456789abcdef";
const FG_TOKEN = "figd_validtoken0123456789abcdef";
const seen = [];
const mcpSeen = [];
const exp = { access: new Set(), refresh: new Set(), seq: 0, refreshes: 0 };
const issue = (secs) => {
  exp.seq += 1;
  exp.access.add(`ghu_exp${exp.seq}0123456789abcdefghij`);
  exp.refresh.add(`ghr_exp${exp.seq}0123456789abcdefghij`);
  return { access_token: `ghu_exp${exp.seq}0123456789abcdefghij`, refresh_token: `ghr_exp${exp.seq}0123456789abcdefghij`, expires_in: secs, refresh_token_expires_in: 15811200, token_type: "bearer" };
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
      if (url.pathname === "/_seen") return send(200, seen);
      if (url.pathname === "/mcp/" || url.pathname === "/mcp") {
        const auth = req.headers.authorization ?? "";
        mcpSeen.push({ method: req.method, auth, body });
        if (auth !== `Bearer ${GH_TOKEN}`) return send(auth.startsWith("Bearer ghu_") ? 403 : 401, { message: "bad token" });
        let rpc = {};
        try {
          rpc = JSON.parse(body || "{}");
        } catch {}
        if (rpc.method === "initialize") {
          res.writeHead(200, { "Content-Type": "application/json", "Mcp-Session-Id": "sess-1" });
          return res.end(JSON.stringify({ jsonrpc: "2.0", id: rpc.id, result: { protocolVersion: "2025-06-18", capabilities: {}, serverInfo: { name: "mock" } } }));
        }
        if (rpc.method === "notifications/initialized") {
          res.writeHead(202);
          return res.end();
        }
        if (req.headers["mcp-session-id"] !== "sess-1") return send(400, { error: "no session" });
        const reply = (result) => {
          res.writeHead(200, { "Content-Type": "text/event-stream" });
          res.end(`event: message\ndata: ${JSON.stringify({ jsonrpc: "2.0", id: rpc.id, result })}\n\n`);
        };
        if (rpc.method === "tools/list") {
          const names = ["search_code", "get_file_contents", "issue_write", "merge_pull_request", "delete_file", "create_repository"];
          return reply({ tools: names.map((name) => ({ name, description: `mock ${name}`, inputSchema: { type: "object" } })) });
        }
        if (rpc.method === "tools/call") {
          const name = rpc.params.name;
          if (name === "get_file_contents") return reply({ content: [{ type: "text", text: "x".repeat(60000) }] });
          if (name === "search_code") return reply({ content: [{ type: "text", text: `found in octo-dev/secret-app; leaked ${GH_TOKEN} and gho_abcdefghijklmnopqrstuvwxyz0123` }] });
          if (name === "issue_write") return reply({ content: [{ type: "text", text: `issue created: ${rpc.params.arguments.title}` }] });
          return reply({ content: [{ type: "text", text: `ran ${name}` }], isError: true });
        }
        return send(404, { error: "unknown method" });
      }
      seen.push({ method: req.method, path: url.pathname + url.search, auth: req.headers.authorization ?? req.headers["x-figma-token"] ?? "", body });
      const bearer = (req.headers.authorization ?? "").replace("Bearer ", "");
      const fields = (token, prefix) => (token.startsWith(prefix) ? decodeURIComponent(token.slice(prefix.length)).split("|") : null);
      if (url.pathname === "/_mcp_seen") return send(200, mcpSeen);
      if (url.pathname === "/_state") return send(200, { refreshes: exp.refreshes, access: exp.access.size, refresh: exp.refresh.size });
      if (url.pathname === "/_expire") {
        exp.access.clear();
        return send(200, {});
      }
      if (url.pathname === "/_revoke") {
        exp.access.clear();
        exp.refresh.clear();
        return send(200, {});
      }
      if (url.pathname === "/login/oauth/access_token") {
        const sent = JSON.parse(body || "{}");
        if (sent.grant_type === "refresh_token") {
          if (!exp.refresh.has(sent.refresh_token)) return send(200, { error: "bad_refresh_token", error_description: "The refresh token passed is incorrect or expired." });
          exp.refresh.delete(sent.refresh_token);
          exp.refreshes += 1;
          return send(200, issue(28800));
        }
        const code = sent.code ?? "";
        if (code === "exp") return send(200, issue(28800));
        if (code === "expsoon") return send(200, issue(1));
        if (code === "denied") return send(200, { error: "bad_verification_code", error_description: "The code passed is incorrect or expired." });
        return send(200, code === "abc" ? { access_token: GH_TOKEN, token_type: "bearer" } : { access_token: `gho_${encodeURIComponent(code)}`, token_type: "bearer" });
      }
      if (url.pathname === "/v1/oauth/token") {
        const code = new URLSearchParams(body).get("code") ?? "";
        return send(200, code === "abc" ? { access_token: FG_TOKEN.replace("figd_", "figo"), refresh_token: "refresh-token-abc", expires_in: 7776000 } : { access_token: `figo_${encodeURIComponent(code)}`, refresh_token: "r", expires_in: 7776000 });
      }
      if (url.pathname === "/user" && bearer.startsWith("ghu_")) return exp.access.has(bearer) ? send(200, { login: "octo-exp", id: 9 }) : send(401, { message: "Bad credentials" });
      if (url.pathname === "/user/repos" && bearer.startsWith("ghu_")) return exp.access.has(bearer) ? send(200, [{ full_name: "octo-exp/rotating", private: true }]) : send(401, { message: "Bad credentials" });
      if (url.pathname === "/user") {
        if (bearer === GH_TOKEN) return send(200, { login: "octo-dev", id: 7 });
        const f = fields(bearer, "gho_");
        return f ? send(200, { id: Number(f[1]), login: f[2], name: f[2] }) : send(401, { message: "Bad credentials" });
      }
      if (url.pathname === "/user/emails") {
        const f = fields(bearer, "gho_");
        if (!f || !f[3]) return send(200, []);
        return send(200, [{ email: f[3], primary: true, verified: f[4] !== "0" }]);
      }
      if (url.pathname === "/user/repos") return bearer === GH_TOKEN ? send(200, [{ full_name: "octo-dev/secret-app", private: true }]) : send(401, { message: "Bad credentials" });
      if (url.pathname === "/v1/me") {
        const token = req.headers["x-figma-token"] ?? bearer;
        const f = fields(token, "figo_");
        if (f) return send(200, { id: f[1], handle: f[2], email: f[3] });
        return token === FG_TOKEN || token.startsWith("figo") ? send(200, { handle: "designer", email: "d@test.ir", id: "fig-1" }) : send(403, { err: "Invalid token" });
      }
      if (url.pathname.startsWith("/v1/files/")) return send(200, { name: "Landing page", document: { children: [{ id: "1:2", name: "Hero" }] } });
      if (url.pathname.startsWith("/v1/images/")) return send(200, { err: null, images: { "1:2": `http://127.0.0.1:${PORT}/img/hero.png` } });
      if (url.pathname === "/img/hero.png") {
        res.writeHead(200, { "Content-Type": "image/png" });
        return res.end("PNGDATA");
      }
      send(404, { message: "Not Found" });
    });
  })
  .listen(PORT, "127.0.0.1", () => console.log(`provider mock on ${PORT}`));
