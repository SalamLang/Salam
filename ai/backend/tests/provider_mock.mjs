import http from "node:http";

const PORT = Number(process.env.PORT ?? 18095);
const GH_TOKEN = "ghp_validtoken0123456789abcdef";
const FG_TOKEN = "figd_validtoken0123456789abcdef";
const seen = [];

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
      if (url.pathname === "/login/oauth/access_token") {
        const code = JSON.parse(body || "{}").code ?? "";
        return send(200, code === "abc" ? { access_token: GH_TOKEN, token_type: "bearer" } : { access_token: `gho_${encodeURIComponent(code)}`, token_type: "bearer" });
      }
      if (url.pathname === "/v1/oauth/token") {
        const code = new URLSearchParams(body).get("code") ?? "";
        return send(200, code === "abc" ? { access_token: FG_TOKEN.replace("figd_", "figo"), refresh_token: "refresh-token-abc", expires_in: 7776000 } : { access_token: `figo_${encodeURIComponent(code)}`, refresh_token: "r", expires_in: 7776000 });
      }
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
