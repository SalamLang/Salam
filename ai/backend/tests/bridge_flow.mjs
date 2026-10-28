const API = (process.env.API ?? "http://127.0.0.1:18090").replace(/\/$/, "");
const V1 = (process.env.V1 ?? "http://127.0.0.1:18091").replace(/\/$/, "");
const SECRET = process.env.BRIDGE_SECRET ?? "";
const HOLD = 1000 * Number(process.env.HOLD_SECONDS ?? 2);

let passed = 0;
const failures = [];
function check(name, cond, detail) {
  if (cond) {
    passed += 1;
    console.log(`  ok   ${name}`);
  } else {
    failures.push(name);
    console.log(`  FAIL ${name}${detail === undefined ? "" : `: ${typeof detail === "string" ? detail : JSON.stringify(detail).slice(0, 400)}`}`);
  }
}

async function api(method, path, token, body) {
  const headers = {};
  if (token) headers.Authorization = `Bearer ${token}`;
  if (body !== undefined) headers["Content-Type"] = "application/json";
  const res = await fetch(API + path, { method, headers, body: body === undefined ? undefined : JSON.stringify(body) });
  let json = null;
  try {
    json = await res.json();
  } catch {}
  return { res, json };
}

async function account(email, name) {
  const r = await api("POST", "/auth/register", undefined, { name, email, password: "password-123" });
  if (r.res.status === 409) return (await api("POST", "/auth/login", undefined, { email, password: "password-123" })).json.token;
  return r.json.token;
}

async function mcp(body, secret = SECRET) {
  const res = await fetch(`${V1}/bridge/mcp`, { method: "POST", headers: { "Content-Type": "application/json", Authorization: `Bearer ${secret}` }, body: JSON.stringify(body) });
  let json = null;
  try {
    json = await res.json();
  } catch {}
  return { res, json };
}

const poll = (secret) => fetch(`${V1}/bridge/poll`, { headers: { Authorization: `Bearer ${secret}` } });
const answer = (secret, body) => fetch(`${V1}/bridge/result`, { method: "POST", headers: { "Content-Type": "application/json", Authorization: `Bearer ${secret}` }, body: JSON.stringify(body) });

async function settle(token, chat, mid) {
  let polled;
  for (let i = 0; i < 100; i += 1) {
    polled = await api("GET", `/chats/${chat}/messages/${mid}`, token);
    if (polled.json.status !== "pending") break;
    await new Promise((r) => setTimeout(r, 300));
  }
  return polled.json;
}

console.log("devices");
const session = await account("desk@test.ir", "Desk");
const other = await account("desk-other@test.ir", "Other");
const made = await api("POST", "/bridge/devices", session, { name: "لپ‌تاپ", os: "linux" });
check("registering a device returns its secret once", made.res.status === 201 && made.json.secret.startsWith("sd-") && made.json.device.os === "linux", made.json);
const device = made.json.device;
const secret = made.json.secret;
const listed = await api("GET", "/bridge/devices", session);
check("the device is listed without its secret", listed.json.devices.some((d) => d.id === device.id) && !JSON.stringify(listed.json).includes(secret), listed.json);
check("another user does not see it", (await api("GET", "/bridge/devices", other)).json.devices.length === 0);
const badPoll = await poll("sd-wrong");
check("a wrong device secret cannot poll", badPoll.status === 401);

console.log("agent chats");
const theirs = await api("POST", "/chats", other, { device_id: device.id, workspace: "/tmp/ws" });
check("another user cannot open a chat on this device", theirs.res.status === 404, theirs.json);
const noWs = await api("POST", "/chats", session, { device_id: device.id });
check("an agent chat needs a workspace", noWs.res.status === 400, noWs.json);
const chat = await api("POST", "/chats", session, { device_id: device.id, workspace: "/tmp/ws" });
check("an agent chat is marked as one", chat.res.status === 201 && chat.json.chat.agent === true && chat.json.chat.workspace === "/tmp/ws", chat.json);
const plain = await api("POST", "/chats", session);
check("a plain chat is unchanged", plain.res.status === 201 && plain.json.chat.agent === undefined, plain.json);

console.log("offline device");
const offline = await api("POST", `/chats/${chat.json.chat.id}/messages`, session, { text: "run: echo offline" });
const offlineDone = await settle(session, chat.json.chat.id, offline.json.assistant.id);
check("a call to a device that never connected fails fast", offlineDone.status === "done" && offlineDone.body.includes("error") && offlineDone.body.includes("not connected"), offlineDone);

console.log("salam only");
const scaffold = await api("POST", `/chats/${chat.json.chat.id}/messages`, session, { text: "run: cd shop && composer create-project laravel/laravel ." });
const scaffoldDone = await settle(session, chat.json.chat.id, scaffold.json.assistant.id);
const project = { instructions_file: "AGENTS.md", instructions: "Use tabs.", memory_index: "# Salam Agent memory\n\n- [Build](memory/build.md) - how to build", memories: 1 };
const withProject = await api("POST", `/chats/${chat.json.chat.id}/messages`, session, { text: "run: echo context", project });
const withProjectDone = await settle(session, chat.json.chat.id, withProject.json.assistant.id);
check("the app's AGENTS.md and memory index reach the engine", withProjectDone.body.includes("[project]"), withProjectDone.body);
const sameProject = await api("POST", `/chats/${chat.json.chat.id}/messages`, session, { text: "run: echo again", project });
const sameProjectDone = await settle(session, chat.json.chat.id, sameProject.json.assistant.id);
check("an unchanged project context is not sent twice", sameProjectDone.status === "done" && !sameProjectDone.body.includes("[project]"), sameProjectDone.body);
const heredoc = (name) => `run: cat > ${name} <<'EOF'\n${Array.from({ length: 50 }, (_, i) => `echo ${i};`).join("\n")}\nEOF`;
const budget = [];
for (const name of ["a.php", "b.php", "c.php"]) {
  const r = await api("POST", `/chats/${chat.json.chat.id}/messages`, session, { text: heredoc(name) });
  budget.push((await settle(session, chat.json.chat.id, r.json.assistant.id)).body);
}
check("non-Salam code written through the shell adds up per chat", !budget[0].includes("Refused") && !budget[1].includes("Refused") && budget[2].includes("Refused by the Salam service: this conversation already wrote 104 lines"), budget);
check("starting a project in another language is refused before it reaches the device", scaffoldDone.body.includes("error Refused by the Salam service") && scaffoldDone.body.includes("composer create-project"), scaffoldDone);
const lines = (n) => Array.from({ length: n }, (_, i) => `print(${i})`);
const refusals = [];
for (const text of [
  `run: python3 - <<'PY'\n${lines(12).join("\n")}\nPY`,
  `tool: docker ${JSON.stringify({ args: ["run", "--rm", "python:3", "python", "-c", lines(12).join("\n")] })}`,
  `tool: write_file ${JSON.stringify({ path: "tool", content: `#!/usr/bin/env python3\n${lines(50).join("\n")}\n` })}`,
]) {
  const r = await api("POST", `/chats/${chat.json.chat.id}/messages`, session, { text });
  refusals.push((await settle(session, chat.json.chat.id, r.json.assistant.id)).body);
}
check("a program in another language run from the command line is refused", refusals[0].includes("Refused by the Salam service: this command runs a program in another language"), refusals[0]);
check("the same through the docker tool is refused", refusals[1].includes("Refused by the Salam service: this command runs a program in another language"), refusals[1]);
check("a foreign script hidden behind a plain filename is refused", refusals[2].includes("Refused by the Salam service: this would put 51 lines of code in another language into tool"), refusals[2]);

console.log("tool call round trip");
const first = poll(secret);
await new Promise((r) => setTimeout(r, 300));
const sent = await api("POST", `/chats/${chat.json.chat.id}/messages`, session, { text: "run: echo hello" });
const got = await first;
const call = got.status === 200 ? (await got.json()).call : null;
check("the device receives the call", call?.tool === "shell" && call.args.command === "echo hello" && call.workspace === "/tmp/ws" && call.chat === chat.json.chat.id, call);
const wrong = await answer("sd-" + "f".repeat(48), { call_id: call?.id, status: "ok", output: "x" });
check("another secret cannot answer the call", wrong.status === 401);
const ok = await answer(secret, { call_id: call?.id, status: "ok", output: "hello from the desktop" });
check("the device answers the call", ok.status === 200);
const done = await settle(session, chat.json.chat.id, sent.json.assistant.id);
check("the answer carries the tool output", done.status === "done" && done.body.includes("ok hello from the desktop"), done);
check("agent chats use the desktop engine agent", done.body.includes("(salam-agent)"), done.body);
const history = await api("GET", `/chats/${chat.json.chat.id}`, session);
check("the stored user message has no session block", history.json.messages.every((m) => !m.body.includes("desktop_session")), history.json.messages.map((m) => m.body));

console.log("refusal");
const second = poll(secret);
await new Promise((r) => setTimeout(r, 300));
const sent2 = await api("POST", `/chats/${chat.json.chat.id}/messages`, session, { text: "run: rm -rf /" });
const call2 = (await (await second).json()).call;
await answer(secret, { call_id: call2.id, status: "denied", output: "" });
const done2 = await settle(session, chat.json.chat.id, sent2.json.assistant.id);
check("a refused call reaches the model as an error", done2.body.includes("error The user refused this call."), done2.body);

console.log("long call");
const third = poll(secret);
await new Promise((r) => setTimeout(r, 300));
const sent3 = await api("POST", `/chats/${chat.json.chat.id}/messages`, session, { text: "run: make build" });
const call3 = (await (await third).json()).call;
const keepPolling = (async () => {
  for (let i = 0; i < 4; i += 1) await (await poll(secret)).text();
})();
await new Promise((r) => setTimeout(r, 2 * HOLD + 1500));
await answer(secret, { call_id: call3.id, status: "ok", output: "built after a long time" });
const done3 = await settle(session, chat.json.chat.id, sent3.json.assistant.id);
check("a call longer than the hold time is picked up with wait", done3.status === "done" && done3.body.includes("ok built after a long time after") && /after [2-9] waits/.test(done3.body), done3.body);
await keepPolling;
const lostWait = await mcp({ jsonrpc: "2.0", id: 9, method: "tools/call", params: { name: "wait", arguments: { session: "0".repeat(32), call_id: call3.id } } });
check("waiting on a finished or foreign call is a tool error", lostWait.json.result.isError === true, lostWait.json);

console.log("mcp endpoint");
const unauth = await mcp({ jsonrpc: "2.0", id: 1, method: "tools/list" }, "nope-nope-nope-nope");
check("the MCP endpoint needs the bridge secret", unauth.res.status === 401);
const init = await mcp({ jsonrpc: "2.0", id: "a", method: "initialize", params: { protocolVersion: "2025-06-18" } });
check("initialize answers with tools capability", init.json.id === "a" && init.json.result.capabilities.tools !== undefined && init.json.result.serverInfo.name === "salam-agent", init.json);
const tools = await mcp({ jsonrpc: "2.0", id: 2, method: "tools/list" });
const names = tools.json.result.tools.map((t) => t.name);
check("tools/list has every desktop tool", ["shell", "read_file", "write_file", "edit_file", "list_dir", "glob", "grep", "docker", "wsl", "run_tests", "wait"].every((n) => names.includes(n)), names);
check("every tool requires the session", tools.json.result.tools.every((t) => t.inputSchema.required.includes("session")));
const stray = await mcp({ jsonrpc: "2.0", id: 3, method: "tools/call", params: { name: "shell", arguments: { session: "0".repeat(32), command: "ls" } } });
check("an unknown session is a tool error", stray.json.result.isError === true, stray.json);
const note = await fetch(`${V1}/bridge/mcp`, { method: "POST", headers: { "Content-Type": "application/json", Authorization: `Bearer ${SECRET}` }, body: JSON.stringify({ jsonrpc: "2.0", method: "notifications/initialized" }) });
check("notifications get 202", note.status === 202);

console.log("revocation");
await api("DELETE", `/bridge/devices/${device.id}`, session);
const dead = await poll(secret);
check("a removed device cannot poll", dead.status === 401);

console.log(`\n${passed} passed, ${failures.length} failed`);
if (failures.length) process.exit(1);
