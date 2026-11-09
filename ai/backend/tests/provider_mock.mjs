import http from "node:http";
import { createHash } from "node:crypto";

const PORT = Number(process.env.PORT ?? 18095);
const GH_TOKEN = "ghp_validtoken0123456789abcdef";
const FG_TOKEN = "figd_validtoken0123456789abcdef";
const seen = [];
const mcp = { access: new Set(), refresh: new Set(), seq: 0, refreshes: 0, registered: [], calls: [], sessions: 0, grants: [], challenge: "" };
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
      seen.push({ method: req.method, path: url.pathname + url.search, auth: req.headers.authorization ?? req.headers["x-figma-token"] ?? "", body });
      const bearer = (req.headers.authorization ?? "").replace("Bearer ", "");
      const fields = (token, prefix) => (token.startsWith(prefix) ? decodeURIComponent(token.slice(prefix.length)).split("|") : null);
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
      if (url.pathname === "/v1/oauth/mcp/register") {
        const sent = JSON.parse(body || "{}");
        mcp.registered.push(sent);
        return send(201, { client_id: "mcp-client-1", client_secret: "mcp-secret-1", redirect_uris: sent.redirect_uris });
      }
      if (url.pathname === "/_mcp") return send(200, { registered: mcp.registered.length, refreshes: mcp.refreshes, calls: mcp.calls, sessions: mcp.sessions, grants: mcp.grants });
      if (url.pathname === "/_mcp_expect") {
        mcp.challenge = url.searchParams.get("challenge") ?? "";
        return send(200, {});
      }
      if (url.pathname === "/_mcp_revoke") {
        mcp.access.clear();
        return send(200, {});
      }
      if (url.pathname === "/_mcp_expire") {
        mcp.access.clear();
        return send(200, {});
      }
      if (url.pathname === "/mcp") {
        const rpc = JSON.parse(body || "{}");
        if (!mcp.access.has(bearer)) {
          res.writeHead(401, { "Content-Type": "text/plain", "WWW-Authenticate": 'Bearer resource_metadata="x"' });
          return res.end("Unauthorized");
        }
        mcp.calls.push(rpc.method);
        if (rpc.method === "initialize") {
          mcp.sessions += 1;
          res.writeHead(200, { "Content-Type": "application/json", "Mcp-Session-Id": `sess-${mcp.sessions}` });
          return res.end(JSON.stringify({ jsonrpc: "2.0", id: rpc.id, result: { protocolVersion: "2025-06-18", capabilities: { tools: {} }, serverInfo: { name: "figma-mock", version: "1" } } }));
        }
        if (!/^sess-\d+$/.test(req.headers["mcp-session-id"] ?? "")) {
          res.writeHead(400, { "Content-Type": "application/json" });
          return res.end(JSON.stringify({ jsonrpc: "2.0", id: rpc.id ?? null, error: { code: -32000, message: "Missing session" } }));
        }
        if (rpc.method === "notifications/initialized") {
          res.writeHead(202);
          return res.end();
        }
        if (rpc.method === "tools/list") {
          res.writeHead(200, { "Content-Type": "text/event-stream" });
          const tools = [
            { name: "get_design_context", description: "Design context for a node.", inputSchema: { type: "object", properties: { fileKey: { type: "string" }, nodeId: { type: "string" } } } },
            { name: "get_screenshot", description: "Screenshot of a node.", inputSchema: { type: "object", properties: { nodeId: { type: "string" } } } },
          ];
          return res.end(`event: message\ndata: ${JSON.stringify({ jsonrpc: "2.0", id: rpc.id, result: { tools } })}\n\n`);
        }
        if (rpc.method === "tools/call") {
          const name = rpc.params?.name;
          const a = rpc.params?.arguments ?? {};
          res.writeHead(200, { "Content-Type": "application/json" });
          if (name === "get_screenshot") return res.end(JSON.stringify({ jsonrpc: "2.0", id: rpc.id, result: { content: [{ type: "image", data: "SU1BR0VEQVRB", mimeType: "image/png" }] } }));
          if (name === "get_design_context") return res.end(JSON.stringify({ jsonrpc: "2.0", id: rpc.id, result: { content: [{ type: "text", text: `design ${a.fileKey}/${a.nodeId}: Hero button` }] } }));
          return res.end(JSON.stringify({ jsonrpc: "2.0", id: rpc.id, result: { isError: true, content: [{ type: "text", text: `unknown tool ${name}` }] } }));
        }
        res.writeHead(200, { "Content-Type": "application/json" });
        return res.end(JSON.stringify({ jsonrpc: "2.0", id: rpc.id, error: { code: -32601, message: "Method not found" } }));
      }
      if (url.pathname === "/v1/oauth/token" && req.headers.authorization === `Basic ${Buffer.from("mcp-client-1:mcp-secret-1").toString("base64")}`) {
        const form = new URLSearchParams(body);
        mcp.grants.push(form.get("grant_type"));
        const issueMcp = (secs) => {
          mcp.seq += 1;
          mcp.access.add(`fmcp_access${mcp.seq}0123456789abcdefghij`);
          mcp.refresh.add(`fmcp_refresh${mcp.seq}0123456789abcdefghij`);
          return { access_token: `fmcp_access${mcp.seq}0123456789abcdefghij`, refresh_token: `fmcp_refresh${mcp.seq}0123456789abcdefghij`, expires_in: secs, token_type: "Bearer" };
        };
        if (form.get("grant_type") === "refresh_token") {
          if (!mcp.refresh.has(form.get("refresh_token"))) return send(400, { error: "invalid_grant", error_description: "The refresh token is not valid." });
          mcp.refresh.delete(form.get("refresh_token"));
          mcp.refreshes += 1;
          return send(200, issueMcp(7776000));
        }
        const verifier = form.get("code_verifier") ?? "";
        const challenge = createHash("sha256").update(verifier).digest("base64url");
        if (!verifier || challenge !== mcp.challenge) return send(400, { error: "invalid_grant", error_description: "PKCE verification failed." });
        if (form.get("code") === "denied") return send(400, { error: "invalid_grant", error_description: "The code is incorrect or expired." });
        return send(200, issueMcp(form.get("code") === "mcpsoon" ? 1 : 7776000));
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
