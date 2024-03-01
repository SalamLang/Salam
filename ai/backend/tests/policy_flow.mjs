const API = (process.env.API ?? "http://127.0.0.1:18090").replace(/\/$/, "");
const ADMIN = process.env.ADMIN_EMAIL ?? "admin@test.ir";

let passed = 0;
const failures = [];
function check(name, cond, detail) {
  if (cond) {
    passed += 1;
    console.log(`  ok   ${name}`);
  } else {
    failures.push(name);
    console.log(`  FAIL ${name}${detail === undefined ? "" : `: ${typeof detail === "string" ? detail : JSON.stringify(detail).slice(0, 500)}`}`);
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

const sleep = (ms) => new Promise((r) => setTimeout(r, ms));
async function ask(token, text) {
  const chat = (await api("POST", "/chats", token, {})).json.chat.id;
  const sent = await api("POST", `/chats/${chat}/messages`, token, { text });
  const mid = sent.json?.assistant?.id;
  for (let i = 0; i < 150; i += 1) {
    const r = await api("GET", `/chats/${chat}/messages/${mid}`, token);
    if (r.json?.status && r.json.status !== "pending") return r.json.body;
    await sleep(300);
  }
  return "";
}

const admin = await account(ADMIN, "Admin");
const email = `policy-${Date.now()}@test.ir`;
const user = await account(email, "Policy User");
const me = (await api("GET", "/me", user)).json.user;

check("a user starts under the Salam-only policy", /agent=salam system=salam .*reminder=false/.test(await ask(user, "show policy")));

const denied = await api("PATCH", `/admin/users/${me.id}/policy`, user, { policy_free: true });
check("a normal user cannot lift the limit", denied.res.status === 403);
const bad = await api("PATCH", `/admin/users/${me.id}/policy`, admin, { policy_free: "maybe" });
check("a bad value is refused", bad.res.status === 400);
const lifted = await api("PATCH", `/admin/users/${me.id}/policy`, admin, { policy_free: true, policy_system: "Answer plainly." });
check("an admin lifts the limit for one user", lifted.res.status === 200 && lifted.json.user.policy_free === true && lifted.json.user.policy_system === "Answer plainly.", lifted.json);

const free = await ask(user, "show policy");
check("the user is answered by the plain profile", /agent=raw/.test(free), free);
check("without the Salam policy text", !/system=salam/.test(free) && /reminder=false/.test(free), free);

const other = await account(`policy-other-${Date.now()}@test.ir`, "Other User");
check("other users keep the limit", /agent=salam system=salam .*reminder=false/.test(await ask(other, "show policy")));

const listed = await api("GET", `/admin/users?q=${encodeURIComponent(email)}`, admin);
const row = listed.json?.users?.[0];
check("the admin list shows the lifted limit", row?.policy_free === true, row);
check("the admin list splits the dollar cost", row?.cost_in > 0 && row?.cost_out > 0 && Math.abs(row.cost_in + row.cost_out - 0.0024) < 0.0001, row);

const summary = await api("GET", "/admin/summary", admin);
const all = summary.json?.usage?.find((p) => p.key === "all");
check("the usage table splits cost into input and output", all?.chat?.cost > 0 && all.chat.cost_in > 0 && all.chat.cost_out > 0 && Math.abs(all.chat.cost_in + all.chat.cost_out - all.chat.cost) < 0.0001, all?.chat);

const restored = await api("PATCH", `/admin/users/${me.id}/policy`, admin, { policy_free: false });
check("the limit can be restored", restored.res.status === 200 && restored.json.user.policy_free === false);
check("and applies again", /agent=salam system=salam .*reminder=false/.test(await ask(user, "show policy")));

check("python in an unlabeled fence is sent back to be written in Salam", (await ask(user, "review then fix please")).startsWith("Rewritten in Salam"));
const spread = (await api("POST", "/chats", user, {})).json.chat.id;
const askMore = async (text) => {
  const sent = await api("POST", `/chats/${spread}/messages`, user, { text });
  for (let i = 0; i < 150; i += 1) {
    const r = await api("GET", `/chats/${spread}/messages/${sent.json.assistant.id}`, user);
    if (r.json?.status && r.json.status !== "pending") return r.json.body;
    await sleep(300);
  }
  return "";
};
const pieces = [await askMore("spread python please"), await askMore("spread python please"), await askMore("spread python please")];
check("small pieces of python stay alone at first", pieces[0].startsWith("One more part") && pieces[1].startsWith("One more part"), pieces);
check("but not once they add up across the conversation", pieces[2].startsWith("Rewritten in Salam"), pieces[2]);

const guestPost = (body) => fetch(`${API}/guest/messages`, { method: "POST", headers: { "Content-Type": "application/json", Origin: "https://www.salamlang.ir" }, body: JSON.stringify(body) });
const stuck = await guestPost({ text: "never start the answer" });
const stuckBody = await stuck.json();
check("a guest message the assistant never answers stays pending", stuck.status === 201 && typeof stuckBody.guest === "string", stuckBody);
const early = await guestPost({ guest: stuckBody.guest, text: "asking again" });
check("while it is running a second guest message is refused", early.status === 409 && (await early.json()).error === "busy");
await sleep(60000);
const later = await guestPost({ guest: stuckBody.guest, text: "guest hello again" });
const laterBody = await later.json();
check("an abandoned guest message no longer blocks the chat", later.status === 201 && laterBody.used === 2, laterBody);

const tg = await api("GET", "/admin/telegram", admin);
check("the admin sees Telegram sessions", tg.res.status === 200 && Array.isArray(tg.json.links) && Array.isArray(tg.json.guests) && typeof tg.json.online_links === "number", tg.json);
const tgDenied = await api("GET", "/admin/telegram", user);
check("only admins see Telegram sessions", tgDenied.res.status === 403);

console.log(`\n${passed} passed, ${failures.length} failed`);
if (failures.length) {
  console.log(failures.join("\n"));
  process.exit(1);
}
