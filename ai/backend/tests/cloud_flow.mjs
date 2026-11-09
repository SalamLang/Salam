const API = (process.env.API ?? "http://127.0.0.1:18090").replace(/\/$/, "");
const TG = (process.env.TG ?? "http://127.0.0.1:18097").replace(/\/$/, "");
const PROVIDERS = (process.env.PROVIDERS ?? "http://127.0.0.1:18095").replace(/\/$/, "");
const SANDBOX = (process.env.SANDBOX ?? "http://127.0.0.1:18096").replace(/\/$/, "");
const GH_TOKEN = "ghp_validtoken0123456789abcdef";
const FG_TOKEN = "figd_validtoken0123456789abcdef";

let passed = 0;
const failures = [];
function check(name, cond, detail) {
  if (cond) {
    passed += 1;
    console.log(`  ok   ${name}`);
  } else {
    failures.push(name);
    console.log(`  FAIL ${name}${detail === undefined ? "" : `: ${typeof detail === "string" ? detail : JSON.stringify(detail).slice(0, 600)}`}`);
  }
}

async function api(method, path, token, body, extra = {}) {
  const headers = {};
  if (token) headers.Authorization = `Bearer ${token}`;
  if (body !== undefined) headers["Content-Type"] = "application/json";
  const res = await fetch(API + path, { method, headers, body: body === undefined ? undefined : JSON.stringify(body), ...extra });
  let json = null;
  try {
    json = await res.json();
  } catch {}
  return { res, json };
}

const sleep = (ms) => new Promise((r) => setTimeout(r, ms));
async function until(fn, ms = 30000) {
  const end = Date.now() + ms;
  while (Date.now() < end) {
    const v = await fn();
    if (v) return v;
    await sleep(250);
  }
  return null;
}
const tglog = async () => (await fetch(`${TG}/_log`)).json();
const inject = (update) => fetch(`${TG}/_inject`, { method: "POST", body: JSON.stringify(update) });
const user = (id) => ({ id, is_bot: false, first_name: `Dev${id}`, username: `d${id}`, language_code: "en" });
const press = (id, data) => inject({ callback_query: { id: `cb${Math.random()}`, from: user(id), data, message: { message_id: 1, chat: { id, type: "private" } } } });
const say = (id, text, message_id = 1) => inject({ message: { message_id, from: user(id), chat: { id, type: "private" }, text } });
const textsTo = (calls, chat) => calls.filter((c) => ["sendMessage", "editMessageText"].includes(c.method) && String(c.args.chat_id) === String(chat)).map((c) => c.args.text);

async function ask(token, chat, text) {
  const sent = await api("POST", `/chats/${chat}/messages`, token, { text });
  if (sent.res.status !== 201) return { error: sent.json };
  const mid = sent.json.assistant.id;
  const done = await until(async () => {
    const r = await api("GET", `/chats/${chat}/messages/${mid}`, token);
    return r.json?.status && r.json.status !== "pending" ? r.json : null;
  }, 60000);
  return done ?? { error: "timeout" };
}

async function main() {
  const email = `cloud-${Date.now()}@test.ir`;
  const tgid = 7000000 + (Date.now() % 1000000);
  const reg = await api("POST", "/auth/register", null, { name: "Cloud Dev", email, password: "password123" });
  const token = reg.json.token;

  const overview = await api("GET", "/integrations", token);
  check("sandbox and token storage are on", overview.json?.sandbox === true && overview.json?.storage === true, overview.json);
  check("nothing is connected at first", overview.json?.integrations?.every((i) => !i.connected));

  const bad = await api("POST", "/integrations/github/token", token, { token: "ghp_wrongtoken0123456789abcdefgh" });
  check("a rejected GitHub token is refused", bad.res.status === 400, bad.json);
  const short = await api("POST", "/integrations/github/token", token, { token: "short" });
  check("a malformed token is refused", short.res.status === 400);
  const gh = await api("POST", "/integrations/github/token", token, { token: GH_TOKEN });
  check("GitHub token accepted", gh.res.status === 200 && gh.json.connected && gh.json.login === "octo-dev", gh.json);
  const fg = await api("POST", "/integrations/figma/token", token, { token: FG_TOKEN });
  check("Figma token accepted", fg.res.status === 200 && fg.json.login === "designer", fg.json);
  check("the token is not returned anywhere", !JSON.stringify(gh.json).includes(GH_TOKEN) && !JSON.stringify((await api("GET", "/integrations", token)).json).includes(GH_TOKEN));

  const created = await api("POST", "/chats", token, { cloud: true });
  check("a cloud chat is created", created.res.status === 201 && created.json.chat.cloud === true, created.json);
  const chat = created.json.chat.id;

  let r = await ask(token, chat, 'tool: shell {"command": "echo hello > a.txt && cat a.txt && pwd"}');
  check("shell runs in the sandbox", /Tool said \(salam-agent\): ok hello\n/.test(r.body ?? ""), r);
  r = await ask(token, chat, 'tool: write_file {"path": "src/main.txt", "content": "line one\\nline two\\n"}');
  check("write_file creates folders", /wrote src\/main.txt/.test(r.body ?? ""), r);
  r = await ask(token, chat, 'tool: read_file {"path": "src/main.txt"}');
  check("read_file numbers lines", /1\tline one\n2\tline two/.test(r.body ?? ""), r);
  r = await ask(token, chat, 'tool: edit_file {"path": "src/main.txt", "old_text": "line two", "new_text": "line 2"}');
  check("edit_file replaces text", /edited src\/main.txt \(1 replacement\)/.test(r.body ?? ""), r);
  r = await ask(token, chat, 'tool: edit_file {"path": "src/main.txt", "old_text": "missing", "new_text": "x"}');
  check("edit_file reports a missing text", /was not found/.test(r.body ?? ""), r);
  r = await ask(token, chat, 'tool: list_dir {"path": "."}');
  check("list_dir lists the folder", /a\.txt/.test(r.body ?? "") && /src\//.test(r.body ?? ""), r);
  if (process.env.REAL_SANDBOX) {
    r = await ask(token, chat, 'tool: glob {"pattern": "*.txt", "path": "."}');
    check("glob finds files", /a\.txt/.test(r.body ?? "") && /src\/main\.txt/.test(r.body ?? ""), r);
    r = await ask(token, chat, 'tool: grep {"pattern": "line", "path": "src", "ignore_case": true}');
    check("grep finds text", /main\.txt:1:line one/.test(r.body ?? ""), r);
    r = await ask(token, chat, 'tool: grep {"pattern": "nothing-here-xyz"}');
    check("grep says when nothing matches", /no matches/.test(r.body ?? ""), r);
  }
  r = await ask(token, chat, 'tool: shell {"command": "sleep 3", "timeout": 1}');
  check("a slow command is stopped", /timed out/.test(r.body ?? ""), r);

  r = await ask(token, chat, 'tool: shell {"command": "echo login=${GH_TOKEN:+set} ${FIGMA_TOKEN:+set}; git config --global --get credential.https://github.com.helper"}');
  check("the shell is already signed in to the connected accounts", /login=set set/.test(r.body ?? "") && /GH_TOKEN/.test(r.body ?? ""), r);
  r = await ask(token, chat, 'tool: shell {"command": "echo $GH_TOKEN $FIGMA_AUTH_HEADER"}');
  check("a token printed by a command is hidden in the result", /\*\*\*/.test(r.body ?? "") && !/ghp_validtoken|figd_validtoken/.test(r.body ?? ""), r);
  r = await ask(token, chat, 'tool: github_api {"path": "/user/repos"}');
  check("github_api reads private repositories", /octo-dev\/secret-app/.test(r.body ?? ""), r);
  r = await ask(token, chat, 'tool: github_api {"method": "DELETE", "path": "/repos/octo-dev/secret-app"}');
  check("repository deletion is blocked", /Deleting a repository is not allowed/.test(r.body ?? ""), r);
  r = await ask(token, chat, 'tool: github_api {"method": "DELETE", "path": "//repos//octo-dev/secret-app/"}');
  check("repository deletion cannot be hidden behind slashes", /Deleting a repository is not allowed/.test(r.body ?? ""), r);
  r = await ask(token, chat, 'tool: github_api {"method": "POST", "path": "/repos/octo-dev/secret-app/transfer", "body": "{}"}');
  check("repository transfer is blocked", /Transferring a repository is not allowed/.test(r.body ?? ""), r);
  r = await ask(token, chat, 'tool: github_api {"method": "DELETE", "path": "/orgs/acme"}');
  check("organization deletion is blocked", /Deleting an organization is not allowed/.test(r.body ?? ""), r);
  r = await ask(token, chat, 'tool: github_api {"path": "/user/keys"}');
  check("credential endpoints are blocked", /manages account credentials/.test(r.body ?? ""), r);

  const mreg = await api("POST", "/auth/register", null, { name: "Mcp Dev", email: `mcp-${Date.now()}@test.ir`, password: "password123" });
  const mtoken = mreg.json.token;
  await api("POST", "/integrations/github/token", mtoken, { token: GH_TOKEN });
  const mchat = (await api("POST", "/chats", mtoken, { cloud: true })).json.chat.id;
  r = await ask(mtoken, mchat, 'tool: github_mcp {"action": "tools"}');
  check("github_mcp is off until the user opts in", /has not turned on GitHub MCP/.test(r.body ?? ""), r);
  let lvl = await api("POST", "/integrations/github/mcp", mtoken, { level: "bogus" });
  check("an unknown mcp level is refused", lvl.res.status === 400, lvl.json);
  lvl = await api("POST", "/integrations/github/mcp", mtoken, { level: "read" });
  check("the user can allow read only", lvl.res.status === 200 && lvl.json.mcp === "read" && lvl.json.mcp_available === true, lvl.json);
  r = await ask(mtoken, mchat, 'tool: github_mcp {"action": "tools"}');
  check("only allowed GitHub tools are listed", /search_code/.test(r.body ?? "") && !/merge_pull_request|delete_file|create_repository|issue_write/.test(r.body ?? ""), r);
  r = await ask(mtoken, mchat, 'tool: github_mcp {"action": "call", "tool": "search_code", "arguments": {"query": "TODO"}}');
  check("github_mcp runs a read tool", /found in octo-dev\/secret-app/.test(r.body ?? ""), r);
  check("tokens in the result are hidden", !/ghp_validtoken|gho_abcdefghijkl/.test(r.body ?? "") && /\*\*\*/.test(r.body ?? ""), r);
  r = await ask(mtoken, mchat, 'tool: github_mcp {"action": "call", "tool": "get_file_contents", "arguments": {"path": "big"}}');
  check("a huge result is cut", (r.body ?? "").length < 40000 && /xxxx/.test(r.body ?? ""), (r.body ?? "").length);
  r = await ask(mtoken, mchat, 'tool: github_mcp {"action": "call", "tool": "merge_pull_request", "arguments": {}}');
  check("merging is never offered", /not available/.test(r.body ?? ""), r);
  r = await ask(mtoken, mchat, 'tool: github_mcp {"action": "call", "tool": "issue_write", "confirmed": true, "arguments": {"title": "Bug"}}');
  check("writes are refused at read level", /Writing through GitHub MCP is off/.test(r.body ?? ""), r);
  lvl = await api("POST", "/integrations/github/mcp", mtoken, { level: "write" });
  check("the user can allow writes", lvl.json?.mcp === "write", lvl.json);
  r = await ask(mtoken, mchat, 'tool: github_mcp {"action": "call", "tool": "issue_write", "arguments": {"title": "Bug"}}');
  check("a write needs confirmation first", /changes GitHub/.test(r.body ?? ""), r);
  r = await ask(mtoken, mchat, 'tool: github_mcp {"action": "call", "tool": "issue_write", "confirmed": true, "arguments": {"title": "Bug"}}');
  check("a confirmed write goes through", /issue created: Bug/.test(r.body ?? ""), r);
  r = await ask(mtoken, mchat, 'tool: github_mcp {"action": "call", "tool": "delete_file", "confirmed": true, "arguments": {}}');
  check("deleting stays blocked even with writes on", /not available/.test(r.body ?? ""), r);
  const mcpSeen = await (await fetch(`${PROVIDERS}/_mcp_seen`)).json();
  check("the MCP server got the user's bearer token", mcpSeen.some((c) => c.auth === `Bearer ${GH_TOKEN}`), mcpSeen.length);
  lvl = await api("POST", "/integrations/github/mcp", mtoken, { level: "off" });
  check("the user can turn it off again", lvl.json?.mcp === "", lvl.json);
  r = await ask(token, chat, 'tool: github_git {"args": ["clone", "https://evil.example/x.git"]}');
  check("git refuses foreign hosts", /not allowed/.test(r.body ?? ""), r);
  r = await ask(token, chat, 'tool: github_git {"args": ["status"]}');
  check("git refuses commands without a login", /Only clone, fetch, pull, push/.test(r.body ?? ""), r);
  r = await ask(token, chat, 'tool: figma_api {"path": "/v1/files/abc123"}');
  check("figma_api reads a file", /Landing page/.test(r.body ?? ""), r);
  r = await ask(token, chat, 'tool: figma_api {"method": "DELETE", "path": "/v1/files/abc123"}');
  check("figma_api refuses writes", /Only GET requests/.test(r.body ?? ""), r);
  r = await ask(token, chat, 'tool: figma_export {"file_key": "abc123", "ids": "1:2", "format": "png"}');
  check("figma_export saves images", /saved assets\/1-2\.png/.test(r.body ?? ""), r);
  r = await ask(token, chat, 'tool: shell {"command": "cat assets/1-2.png"}');
  check("the exported image is in the sandbox", /PNGDATA/.test(r.body ?? ""), r);
  r = await ask(token, chat, 'tool: browser {"action": "open", "url": "http://localhost:3000/"}');
  check("browser opens a page in the sandbox and the screenshot reaches the model", /Mock page/.test(r.body ?? "") && /images=1/.test(r.body ?? ""), r);
  r = await ask(token, chat, 'tool: docker {"args": ["ps"]}');
  check("desktop-only tools are refused", /only available in Salam Agent/.test(r.body ?? ""), r);

  const detail = await api("GET", `/chats/${chat}`, token);
  const calls = detail.json?.calls ?? [];
  const steps = (detail.json?.messages ?? []).flatMap((m) => (Array.isArray(m.activity) ? m.activity : []));
  const shellStep = steps.find((s) => s.kind === "tool" && s.tool === "shell");
  check("a sandbox shell step does not claim the user's computer", shellStep?.title === "اجرای فرمان" && shellStep?.title_en === "Running a command", shellStep);
  check("every tool call is recorded", calls.length >= 16, calls.length);
  const shotCall = calls.find((c) => c.tool === "browser");
  check("the browser call is marked as having a screenshot", shotCall?.image === true && !calls.some((c) => c.tool === "shell" && c.image), shotCall);
  const shotImage = await api("GET", `/chats/${chat}/calls/${shotCall?.call_id}/image`, token);
  check("the screenshot can be read back by its owner", shotImage.json?.mime === "image/jpeg" && shotImage.json.data === "SU1BR0VEQVRB", shotImage.json);
  const otherReg = await api("POST", "/auth/register", null, { name: "Other", email: `cloud-other-${Date.now()}@test.ir`, password: "password123" });
  const stranger = await api("GET", `/chats/${chat}/calls/${shotCall?.call_id}/image`, otherReg.json?.token);
  check("nobody else can read the screenshot", stranger.res.status === 404, stranger.res.status);
  check("a shell call keeps its command and output", calls.some((c) => c.tool === "shell" && c.kind === "exec" && c.state === "done" && /hello/.test(c.output)), calls[0]);
  check("file calls are read or write kinds", calls.some((c) => c.tool === "write_file" && c.kind === "write") && calls.some((c) => c.tool === "read_file" && c.kind === "read"));

  const plain = await api("POST", "/chats", token, {});
  const answered = await ask(token, plain.json.chat.id, "tool: shell {\"command\": \"echo x\"}");
  check("a normal chat has no sandbox", !/Tool said/.test(answered.body ?? ""), answered);

  const oauth = await api("POST", "/integrations/github/oauth", token);
  const state = oauth.json?.state;
  check("GitHub connect link is made", oauth.res.status === 200 && /client_id=gh-client/.test(oauth.json.url) && /auth%2Fcallback%2Fgithub/.test(oauth.json.url) && (process.env.AI_GITHUB_SCOPES ? /scope=/.test(oauth.json.url) : /scope=repo%20workflow%20admin%3Aorg/.test(oauth.json.url)) && !/delete_repo|delete%3Apackages/.test(oauth.json.url) && !!state, oauth.res.status);
  await api("DELETE", "/integrations/github", token);
  const back = await api("POST", "/auth/provider/github/complete", undefined, { code: "abc", state });
  check("the app completes the connection", back.res.status === 200 && back.json.purpose === "connect" && back.json.ok === true, back.json);
  const after = await api("GET", "/integrations", token);
  check("OAuth connected GitHub", after.json.integrations.find((i) => i.provider === "github").connected === true, after.json);
  const replay = await api("POST", "/auth/provider/github/complete", undefined, { code: "abc", state });
  check("a state works once", replay.res.status === 400 && replay.json.error === "expired");
  const mock = async (path) => (await fetch(`${PROVIDERS}${path}`)).json();
  const connectWith = async (code) => {
    const o = await api("POST", "/integrations/github/oauth", token);
    return api("POST", "/auth/provider/github/complete", undefined, { code, state: o.json.state });
  };
  const ghItem = async () => (await api("GET", "/integrations", token)).json.integrations.find((i) => i.provider === "github");
  const denied = await connectWith("denied");
  check("a failed exchange says why", denied.json.ok === false && /incorrect or expired/.test(denied.json.reason ?? ""), denied.json);
  await api("DELETE", "/integrations/github", token);
  const before = await mock("/_state");
  const expiring = await connectWith("exp");
  check("an expiring GitHub token connects", expiring.json.ok === true && (await ghItem()).connected === true && (await ghItem()).expired === false, expiring.json);
  await mock("/_expire");
  r = await ask(token, chat, 'tool: github_api {"path": "/user/repos"}');
  check("an expired access token is refreshed during a call", /octo-exp\/rotating/.test(r.body ?? ""), r);
  check("one refresh was used", (await mock("/_state")).refreshes === before.refreshes + 1);
  await mock("/_expire");
  r = await ask(token, chat, 'tool: github_api {"path": "/user/repos"}');
  check("the rotated refresh token was stored", /octo-exp\/rotating/.test(r.body ?? "") && (await mock("/_state")).refreshes === before.refreshes + 2, r);
  await mock("/_expire");
  await api("DELETE", "/integrations/github", token);
  check("the validity cache does not hide a disconnect", (await ghItem()).connected === false);
  await connectWith("expsoon");
  await sleep(1200);
  const soon = (await mock("/_state")).refreshes;
  r = await ask(token, chat, 'tool: github_api {"path": "/user/repos"}');
  check("a token past its expiry is refreshed before use", /octo-exp\/rotating/.test(r.body ?? "") && (await mock("/_state")).refreshes === soon + 1, r);
  await mock("/_revoke");
  r = await ask(token, chat, 'tool: github_api {"path": "/user/repos"}');
  check("a revoked refresh token reports the login as rejected", /rejected the saved login/.test(r.body ?? ""), r);
  check("a revoked login is marked expired", (await ghItem()).expired === true, await ghItem());
  await connectWith("exp");
  check("reconnecting replaces the dead login", (await ghItem()).expired === false);
  await api("POST", "/integrations/github/token", token, { token: GH_TOKEN });
  await api("DELETE", "/integrations/figma", token);
  r = await ask(token, chat, 'tool: figma_api {"path": "/v1/me"}');
  check("a disconnected service says so", /has not connected Figma/.test(r.body ?? ""), r);

  const tgBoot = await until(async () => (await tglog()).some((c) => c.method === "getMe"));
  check("telegram bot is up", !!tgBoot);
  await press(tgid, "l:en");
  await say(tgid, "/login");
  const loginMsg = await until(async () => (await tglog()).filter((c) => String(c.args.chat_id) === String(tgid) && c.args.reply_markup?.inline_keyboard && /Tap the button/.test(c.args.text ?? "")).at(-1));
  const code = /code=([0-9a-f]{32})/.exec(loginMsg?.args.reply_markup.inline_keyboard[0][0].url ?? "")?.[1];
  const linked = await api("POST", "/telegram/link", token, { code });
  check("telegram linked for the cloud test", linked.res.status === 200, linked.json);
  await say(tgid, '/github');
  check("/github explains how to connect", !!(await until(async () => textsTo(await tglog(), tgid).find((t) => /GitHub is connected: octo-dev/.test(t)))));
  await say(tgid, `/github ${GH_TOKEN}`, 4242);
  check("/github with a token connects it", !!(await until(async () => textsTo(await tglog(), tgid).find((t) => /GitHub connected: octo-dev/.test(t)))));
  check("the message with the token is deleted", (await tglog()).some((c) => c.method === "deleteMessage" && c.args.message_id === 4242));
  await say(tgid, '/github nottherealtoken0123456789abcdef');
  check("a bad token is explained", !!(await until(async () => textsTo(await tglog(), tgid).find((t) => /did not accept this token/.test(t)))));
  await say(tgid, "/github mcp");
  check("/github mcp shows the setting", !!(await until(async () => textsTo(await tglog(), tgid).find((t) => /GitHub MCP: off/.test(t)))));
  await say(tgid, "/github mcp read");
  check("/github mcp read turns it on", !!(await until(async () => textsTo(await tglog(), tgid).find((t) => /GitHub MCP is now read/.test(t)))));
  check("the site sees the Telegram choice", (await api("GET", "/integrations", token)).json.integrations.find((i) => i.provider === "github").mcp === "read");
  await say(tgid, "/github mcp bogus");
  check("/github mcp refuses unknown levels", !!(await until(async () => textsTo(await tglog(), tgid).find((t) => /Use one of: read, write or off/.test(t)))));
  await say(tgid, "/github mcp off");
  check("/github mcp off turns it off", (await until(async () => ((await api("GET", "/integrations", token)).json.integrations.find((i) => i.provider === "github").mcp === "" ? true : null))) === true);
  await say(tgid, 'tool: shell {"command": "echo from telegram"}');
  const tgAnswer = await until(async () => textsTo(await tglog(), tgid).find((t) => /Tool said \(salam-agent\): ok from telegram/.test(t)), 60000);
  check("telegram runs commands in the sandbox", !!tgAnswer);
  const listed = await api("GET", "/chats", token);
  check("the telegram cloud chat is in the history", listed.json.chats.some((c) => c.cloud === true && /tool: shell/.test(c.title)), listed.json.chats.map((c) => c.title));
  await say(tgid, "/sandbox");
  check("/sandbox explains itself", !!(await until(async () => textsTo(await tglog(), tgid).find((t) => /private Linux sandbox/.test(t)))));

  const mcpItem = async () => (await api("GET", "/integrations", token)).json.integrations.find((i) => i.provider === "figma-mcp");
  check("Figma MCP is listed and off by default", (await mcpItem())?.connected === false && (await mcpItem())?.oauth === true, await mcpItem());
  const pasted = await api("POST", "/integrations/figma-mcp/token", token, { token: "figd_validtoken0123456789abcdef" });
  check("a pasted token cannot stand in for Figma MCP", pasted.res.status === 400, pasted.json);
  r = await ask(token, chat, 'tool: figma_mcp {"action": "tools"}');
  check("figma_mcp says when it is not connected", /has not connected Figma MCP/.test(r.body ?? ""), r);
  const mcpStart = async () => {
    const o = await api("POST", "/integrations/figma-mcp/oauth", token);
    const link = new URL(o.json.url);
    await fetch(`${PROVIDERS}/_mcp_expect?challenge=${encodeURIComponent(link.searchParams.get("code_challenge") ?? "")}`);
    return { o, link };
  };
  const mcpConnect = async (code) => {
    const { o } = await mcpStart();
    return api("POST", "/auth/provider/figma-mcp/complete", undefined, { code, state: o.json.state });
  };
  const reg0 = (await mock("/_mcp")).registered;
  const refresh0 = (await mock("/_mcp")).refreshes;
  const first = await mcpStart();
  check("the Figma MCP link asks for the mcp scope with PKCE", first.link.pathname === "/oauth/mcp" && first.link.searchParams.get("client_id") === "mcp-client-1" && first.link.searchParams.get("scope") === "mcp:connect" && first.link.searchParams.get("code_challenge_method") === "S256" && /\/auth\/callback\/figma-mcp$/.test(first.link.searchParams.get("redirect_uri") ?? "") && !!first.link.searchParams.get("resource"), first.link.href);
  const joined = await api("POST", "/auth/provider/figma-mcp/complete", undefined, { code: "mcp", state: first.o.json.state });
  check("the app completes the Figma MCP connection", joined.res.status === 200 && joined.json.purpose === "connect" && joined.json.ok === true, joined.json);
  check("Figma MCP shows as connected without leaking a token", (await mcpItem())?.connected === true && (await mcpItem())?.scope === "mcp:connect" && !/fmcp_/.test(JSON.stringify((await api("GET", "/integrations", token)).json)));
  r = await ask(token, chat, 'tool: figma_mcp {"action": "tools"}');
  check("figma_mcp lists the Figma tools", /get_design_context: Design context/.test(r.body ?? "") && /get_screenshot/.test(r.body ?? ""), r);
  r = await ask(token, chat, 'tool: figma_mcp {"tool": "get_design_context", "arguments": {"fileKey": "abc123", "nodeId": "1:2"}}');
  check("figma_mcp calls a Figma tool", /design abc123\/1:2: Hero button/.test(r.body ?? ""), r);
  r = await ask(token, chat, 'tool: figma_mcp {"tool": "get_screenshot", "arguments": {"nodeId": "1:2"}}');
  check("a Figma screenshot reaches the model as an image", /images=1/.test(r.body ?? ""), r);
  r = await ask(token, chat, 'tool: figma_mcp {"tool": "nope", "arguments": {}}');
  check("a Figma tool error is reported", /unknown tool nope/.test(r.body ?? ""), r);
  r = await ask(token, chat, 'tool: figma_mcp {"tool": "get_design_context", "arguments": "not json"}');
  check("figma_mcp refuses arguments that are not an object", /arguments a JSON object/.test(r.body ?? ""), r);
  await api("DELETE", "/integrations/figma-mcp", token);
  const second = await mcpConnect("denied");
  check("a failed Figma MCP exchange says why", second.json.ok === false && /incorrect or expired/.test(second.json.reason ?? ""), second.json);
  const wrong = await api("POST", "/integrations/figma-mcp/oauth", token);
  const badChallenge = await api("POST", "/auth/provider/figma-mcp/complete", undefined, { code: "mcp", state: wrong.json.state });
  check("the code is useless without the matching PKCE verifier", badChallenge.json.ok === false && /PKCE/.test(badChallenge.json.reason ?? ""), badChallenge.json);
  check("the Figma MCP app is registered at most once", (await mock("/_mcp")).registered - reg0 <= 1, await mock("/_mcp"));
  await mcpConnect("mcpsoon");
  await sleep(1200);
  r = await ask(token, chat, 'tool: figma_mcp {"tool": "get_design_context", "arguments": {"fileKey": "k", "nodeId": "3:4"}}');
  check("an expired Figma MCP token is refreshed before use", /design k\/3:4/.test(r.body ?? "") && (await mock("/_mcp")).refreshes - refresh0 === 1, r);
  await mock("/_mcp_revoke");
  r = await ask(token, chat, 'tool: figma_mcp {"action": "tools"}');
  check("a revoked Figma MCP login is reported", /rejected the saved login/.test(r.body ?? ""), r);
  check("a revoked Figma MCP login is marked expired", (await mcpItem())?.expired === true, await mcpItem());
  await api("DELETE", "/integrations/figma-mcp", token);
  check("disconnecting Figma MCP turns it off", (await mcpItem())?.connected === false);
  const login = await api("POST", "/auth/provider/figma-mcp/start", undefined, {});
  check("Figma MCP cannot be used to sign in", login.res.status >= 400, login.json);

  const deleted = await api("DELETE", `/chats/${chat}`, token);
  check("deleting a chat works", deleted.res.status === 200);

  console.log(`\n${passed} passed, ${failures.length} failed`);
  if (failures.length) {
    console.log(failures.join("\n"));
    process.exit(1);
  }
}

main().catch((e) => {
  console.error(e);
  process.exit(1);
});
