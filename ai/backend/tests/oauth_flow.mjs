import { createHash, randomBytes } from "node:crypto";

const API = (process.env.API ?? "http://127.0.0.1:18090").replace(/\/$/, "");
const MCP = (process.env.MCP ?? "http://127.0.0.1:18092/mcp").replace(/\/$/, "");
const ADMIN = process.env.ADMIN_EMAIL ?? "admin@test.ir";
const RESERVED = process.env.RESERVED_EMAIL ?? "reserved@test.ir";
const REDIRECT = "http://127.0.0.1:9999/callback";

let passed = 0;
const failures = [];
function check(name, cond, detail) {
  if (cond) {
    passed += 1;
    console.log(`  ok   ${name}`);
  } else {
    failures.push(name);
    console.log(`  FAIL ${name}${detail === undefined ? "" : `: ${typeof detail === "string" ? detail : JSON.stringify(detail)}`}`);
  }
}

const b64url = (buf) => buf.toString("base64").replace(/\+/g, "-").replace(/\//g, "_").replace(/=+$/, "");
const pkce = () => {
  const verifier = b64url(randomBytes(48));
  return { verifier, challenge: b64url(createHash("sha256").update(verifier).digest()) };
};
const form = (o) => new URLSearchParams(o).toString();
const rpc = (method, params = {}, id = 1) => ({ jsonrpc: "2.0", id, method, params });

async function mcp(token, body) {
  const headers = { "Content-Type": "application/json", Accept: "application/json, text/event-stream" };
  if (token) headers.Authorization = `Bearer ${token}`;
  const res = await fetch(MCP, { method: "POST", headers, body: JSON.stringify(body) });
  let json = null;
  try {
    json = await res.json();
  } catch {}
  return { res, json };
}

async function api(method, path, { token, body, formBody } = {}) {
  const headers = {};
  if (token) headers.Authorization = `Bearer ${token}`;
  let payload;
  if (formBody) {
    headers["Content-Type"] = "application/x-www-form-urlencoded";
    payload = form(formBody);
  } else if (body !== undefined) {
    headers["Content-Type"] = "application/json";
    payload = JSON.stringify(body);
  }
  const res = await fetch(API + path, { method, headers, body: payload, redirect: "manual" });
  let json = null;
  try {
    json = await res.clone().json();
  } catch {}
  return { res, json };
}

async function account(email, name) {
  const r = await api("POST", "/auth/register", { body: { name, email, password: "password-123" } });
  if (r.res.status === 409) return (await api("POST", "/auth/login", { body: { email, password: "password-123" } })).json.token;
  return r.json.token;
}

async function authorize(clientId, extra = {}) {
  const p = pkce();
  const state = b64url(randomBytes(12));
  const q = new URLSearchParams({
    response_type: "code",
    client_id: clientId,
    redirect_uri: REDIRECT,
    code_challenge: p.challenge,
    code_challenge_method: "S256",
    state,
    scope: "mcp",
    resource: MCP,
    ...extra,
  });
  const r = await fetch(`${API}/oauth/authorize?${q}`, { redirect: "manual" });
  const loc = r.headers.get("location") ?? "";
  return { status: r.status, loc, request: new URL(loc, "http://x").searchParams.get("request"), state, ...p };
}

console.log("discovery");
const anon = await mcp(null, rpc("tools/list"));
const challenge = anon.res.headers.get("www-authenticate") ?? "";
check("an anonymous MCP call gets 401", anon.res.status === 401);
check("the challenge names resource_metadata and scope", /resource_metadata="[^"]+"/.test(challenge) && /scope="mcp"/.test(challenge), challenge);
const prmUrl = /resource_metadata="([^"]+)"/.exec(challenge)?.[1];
const prm = await (await fetch(prmUrl)).json();
check("protected resource metadata names this resource", prm.resource === MCP, prm);
check("protected resource metadata lists the issuer", prm.authorization_servers?.[0] === API, prm);
const asMeta = await (await fetch(`${API}/.well-known/oauth-authorization-server`)).json();
check("issuer in AS metadata matches", asMeta.issuer === API, asMeta.issuer);
check("AS requires S256 PKCE", JSON.stringify(asMeta.code_challenge_methods_supported) === '["S256"]');
check("AS advertises CIMD and iss", asMeta.client_id_metadata_document_supported === true && asMeta.authorization_response_iss_parameter_supported === true);

console.log("registration");
const badReg = await api("POST", "/oauth/register", { body: { client_name: "Evil", redirect_uris: ["http://evil.example/cb"] } });
check("http redirect to a remote host is refused", badReg.res.status === 400, badReg.json);
const reg = await api("POST", "/oauth/register", { body: { client_name: "Test Client", redirect_uris: [REDIRECT], token_endpoint_auth_method: "none", grant_types: ["authorization_code", "refresh_token"] } });
check("dynamic registration returns a public client", reg.res.status === 201 && reg.json.token_endpoint_auth_method === "none", reg.json);
const clientId = reg.json.client_id;

console.log("authorization");
const noPkce = await fetch(`${API}/oauth/authorize?${new URLSearchParams({ response_type: "code", client_id: clientId, redirect_uri: REDIRECT, state: "s1" })}`, { redirect: "manual" });
const noPkceLoc = new URL(noPkce.headers.get("location") ?? "", "http://x");
check("authorize without PKCE redirects back with invalid_request and iss", noPkceLoc.searchParams.get("error") === "invalid_request" && noPkceLoc.searchParams.get("iss") === API && noPkceLoc.searchParams.get("state") === "s1", noPkceLoc.href);
const wrongRedirect = await fetch(`${API}/oauth/authorize?${new URLSearchParams({ response_type: "code", client_id: clientId, redirect_uri: "https://evil.example/cb", code_challenge: "x".repeat(43), code_challenge_method: "S256" })}`, { redirect: "manual" });
const wrongTarget = new URL(wrongRedirect.headers.get("location") ?? "/", API);
check("an unregistered redirect_uri never redirects there", wrongTarget.hostname !== "evil.example", wrongTarget.href);
const otherPort = await authorize(clientId, { redirect_uri: "http://127.0.0.1:41234/callback" });
check("a loopback redirect on another port is accepted", otherPort.status === 302 && !!otherPort.request, otherPort.loc);
const badTarget = await authorize(clientId, { resource: "https://elsewhere.example/mcp" });
check("an unknown resource is refused with invalid_target", new URL(badTarget.loc).searchParams.get("error") === "invalid_target", badTarget.loc);

const flow = await authorize(clientId);
check("authorize sends the browser to the consent page", flow.status === 302 && flow.loc.includes("/connect?request=") && !!flow.request, flow.loc);
const pending = await api("GET", `/oauth/requests/${flow.request}`);
check("the consent page can read the request", pending.json?.request?.client?.name === "Test Client" && pending.json.request.client.kind === "dcr", pending.json);
const noUser = await api("POST", `/oauth/requests/${flow.request}/approve`);
check("approving without signing in is 401", noUser.res.status === 401);

const reserved = await api("POST", "/auth/register", { body: { name: "Not Admin", email: RESERVED, password: "password-123" } });
check("an email listed in AI_ADMIN_EMAILS cannot be self-registered", reserved.res.status === 403, reserved.json);
const adminToken = await account(ADMIN, "Admin");
const me = await api("GET", "/me", { token: adminToken });
check("/me marks the admin", me.json?.user?.admin === true, me.json);
const approved = await api("POST", `/oauth/requests/${flow.request}/approve`, { token: adminToken });
const back = new URL(approved.json?.redirect ?? "http://x/");
check("approve returns the redirect with code, state and iss", back.origin + back.pathname === REDIRECT && back.searchParams.get("state") === flow.state && back.searchParams.get("iss") === API && !!back.searchParams.get("code"), approved.json);
const again = await api("POST", `/oauth/requests/${flow.request}/approve`, { token: adminToken });
check("a request can be answered only once", again.res.status === 410);
const code = back.searchParams.get("code");

console.log("token");
const wrongVerifier = await api("POST", "/oauth/token", { formBody: { grant_type: "authorization_code", code, client_id: clientId, redirect_uri: REDIRECT, code_verifier: pkce().verifier, resource: MCP } });
check("a wrong PKCE verifier is refused", wrongVerifier.json?.error === "invalid_grant", wrongVerifier.json);
const flow2 = await authorize(clientId);
const approved2 = await api("POST", `/oauth/requests/${flow2.request}/approve`, { token: adminToken });
const code2 = new URL(approved2.json.redirect).searchParams.get("code");
const tok = await api("POST", "/oauth/token", { formBody: { grant_type: "authorization_code", code: code2, client_id: clientId, redirect_uri: REDIRECT, code_verifier: flow2.verifier, resource: MCP } });
check("code exchange issues access and refresh tokens", tok.res.status === 200 && tok.json.token_type === "Bearer" && !!tok.json.access_token && !!tok.json.refresh_token, tok.json);
check("token responses are not cacheable", (tok.res.headers.get("cache-control") ?? "").includes("no-store"));
const flowR = await authorize(clientId);
const codeR = new URL((await api("POST", `/oauth/requests/${flowR.request}/approve`, { token: adminToken })).json.redirect).searchParams.get("code");
const noRedirect = await api("POST", "/oauth/token", { formBody: { grant_type: "authorization_code", code: codeR, client_id: clientId, code_verifier: flowR.verifier, resource: MCP } });
check("a token request must repeat redirect_uri when authorize had one", noRedirect.json?.error === "invalid_grant", noRedirect.json);
const replay = await api("POST", "/oauth/token", { formBody: { grant_type: "authorization_code", code: code2, client_id: clientId, redirect_uri: REDIRECT, code_verifier: flow2.verifier } });
check("a replayed code is refused", replay.json?.error === "invalid_grant", replay.json);

console.log("mcp with a token");
const flow3 = await authorize(clientId);
const code3 = new URL((await api("POST", `/oauth/requests/${flow3.request}/approve`, { token: adminToken })).json.redirect).searchParams.get("code");
const t3 = (await api("POST", "/oauth/token", { formBody: { grant_type: "authorization_code", code: code3, client_id: clientId, redirect_uri: REDIRECT, code_verifier: flow3.verifier, resource: MCP } })).json;
const listed = await mcp(t3.access_token, rpc("tools/list"));
check("a valid token reaches the tools", listed.res.status === 200 && Array.isArray(listed.json?.result?.tools), listed.json);
const called = await mcp(t3.access_token, rpc("tools/call", { name: "salam_check", arguments: { source: 'func main:\n    println "salam"\nend\n' } }, 2));
check("a tool call works with the token", called.json?.result?.isError === false, called.json);
const failedCall = await mcp(t3.access_token, rpc("tools/call", { name: "salam_check", arguments: { source: "func main:\n    println nope\nend\n" } }, 3));
check("a failing tool call still answers", failedCall.json?.result?.isError === true);
const bogus = await mcp("sa_this-is-not-a-real-token-at-all", rpc("tools/list"));
check("an unknown token gets 401 invalid_token", bogus.res.status === 401 && (bogus.res.headers.get("www-authenticate") ?? "").includes('error="invalid_token"'));

const plainHttp = await fetch(MCP, { method: "POST", headers: { "Content-Type": "application/json", "X-Forwarded-Proto": "http" }, body: JSON.stringify(rpc("tools/list")) });
check("MCP over plain HTTP is refused before any token is sent", plainHttp.status === 400 && !plainHttp.headers.get("www-authenticate"));

console.log("refresh");
const r1 = await api("POST", "/oauth/token", { formBody: { grant_type: "refresh_token", refresh_token: t3.refresh_token, client_id: clientId } });
check("refresh rotates both tokens", r1.res.status === 200 && r1.json.refresh_token !== t3.refresh_token, r1.json);
const r1use = await mcp(r1.json.access_token, rpc("tools/list"));
check("the refreshed access token works", r1use.res.status === 200);

console.log("activity");
await new Promise((r) => setTimeout(r, 1500));
const apps = await api("GET", "/apps", { token: adminToken });
const app = apps.json?.apps?.find((a) => a.client.name === "Test Client");
check("connected apps lists the client with its calls", !!app && app.calls >= 2, apps.json);
const evs = await api("GET", `/apps/${app?.id}/events`, { token: adminToken });
const ev = evs.json?.events?.[0];
check("events record tool, outcome and arguments", ev?.tool === "salam_check" && ev.ok === false && ev.args.includes("nope"), evs.json);
check("the earlier successful call is recorded too", evs.json?.events?.some((e) => e.ok === true && e.tool === "salam_check"));

console.log("admin");
const sum = await api("GET", "/admin/summary", { token: adminToken });
check("admin summary counts calls and lists 14 days", sum.json?.mcp_calls >= 2 && sum.json.daily?.length === 14 && sum.json.top_tools?.[0]?.tool === "salam_check", sum.json);
const users = await api("GET", "/admin/users?q=admin", { token: adminToken });
check("admin users shows MCP calls per user", users.json?.users?.[0]?.mcp_calls >= 2, users.json);
const aev = await api("GET", "/admin/events?tool=salam_check", { token: adminToken });
check("admin events include the user", aev.json?.events?.[0]?.user?.email === ADMIN, aev.json?.events?.[0]);
const aapps = await api("GET", "/admin/apps", { token: adminToken });
check("admin apps lists the connection", aapps.json?.apps?.some((a) => a.client.name === "Test Client"));
const plainToken = await account("someone@test.ir", "Someone");
const forbidden = await api("GET", "/admin/summary", { token: plainToken });
check("a normal user cannot open admin", forbidden.res.status === 403);
const notMine = await api("GET", `/apps/${app?.id}/events`, { token: plainToken });
check("a user cannot read another user's app events", notMine.res.status === 404);

console.log("revocation");
const reuse = await api("POST", "/oauth/token", { formBody: { grant_type: "refresh_token", refresh_token: t3.refresh_token, client_id: clientId } });
check("reusing a rotated refresh token is refused", reuse.json?.error === "invalid_grant", reuse.json);
const afterReuse = await api("POST", "/oauth/token", { formBody: { grant_type: "refresh_token", refresh_token: r1.json.refresh_token, client_id: clientId } });
check("refresh-token reuse revokes the whole connection", afterReuse.json?.error === "invalid_grant", afterReuse.json);
const flow4 = await authorize(clientId);
const code4 = new URL((await api("POST", `/oauth/requests/${flow4.request}/approve`, { token: plainToken })).json.redirect).searchParams.get("code");
const t4 = (await api("POST", "/oauth/token", { formBody: { grant_type: "authorization_code", code: code4, client_id: clientId, redirect_uri: REDIRECT, code_verifier: flow4.verifier, resource: MCP } })).json;
const mine = (await api("GET", "/apps", { token: plainToken })).json.apps[0];
const del = await api("DELETE", `/apps/${mine.id}`, { token: plainToken });
check("a user can disconnect an app", del.json?.ok === true);
const dead = await mcp(t4.access_token, rpc("tools/list"));
check("a disconnected app's token stops working", dead.res.status === 401, dead.res.status);
const revokeEp = await api("POST", "/oauth/revoke", { formBody: { token: "whatever" } });
check("the revocation endpoint answers 200", revokeEp.res.status === 200);

console.log("deny");
const flow5 = await authorize(clientId);
const denied = await api("POST", `/oauth/requests/${flow5.request}/deny`);
const deniedUrl = new URL(denied.json.redirect);
check("deny returns access_denied with state and iss", deniedUrl.searchParams.get("error") === "access_denied" && deniedUrl.searchParams.get("state") === flow5.state && deniedUrl.searchParams.get("iss") === API);

console.log(`\n${passed} passed, ${failures.length} failed`);
if (failures.length) process.exit(1);
