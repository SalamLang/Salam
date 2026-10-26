const API = (process.env.API ?? "http://127.0.0.1:18090").replace(/\/$/, "");
const TG = (process.env.TG ?? "http://127.0.0.1:18097").replace(/\/$/, "");
const GUEST_LIMIT = Number(process.env.GUEST_LIMIT ?? 2);

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

const log = async () => (await fetch(`${TG}/_log`)).json();
const sleep = (ms) => new Promise((r) => setTimeout(r, ms));
async function until(fn, ms = 20000) {
  const end = Date.now() + ms;
  while (Date.now() < end) {
    const v = await fn();
    if (v) return v;
    await sleep(200);
  }
  return null;
}
const inject = (update) => fetch(`${TG}/_inject`, { method: "POST", body: JSON.stringify(update) });
const user = (id, extra = {}) => ({ id, is_bot: false, first_name: `User${id}`, username: `u${id}`, language_code: "en", ...extra });
const say = (id, text) => inject({ message: { message_id: 1, from: user(id), chat: { id, type: "private" }, text } });
const press = (id, data) => inject({ callback_query: { id: `cb${Math.random()}`, from: user(id), data, message: { message_id: 1, chat: { id, type: "private" } } } });

const sentTo = (calls, chat) => calls.filter((c) => ["sendMessage", "editMessageText"].includes(c.method) && String(c.args.chat_id) === String(chat));
const textsTo = (calls, chat) => sentTo(calls, chat).map((c) => c.args.text);
const lastText = async (chat, pattern) => {
  const found = await until(async () => {
    const t = textsTo(await log(), chat);
    return t.reverse().find((x) => pattern.test(x));
  });
  return found;
};

async function main() {
  const boot = await until(async () => (await log()).some((c) => c.method === "getMe"));
  check("bot starts and asks getMe", !!boot);
  const calls0 = await log();
  check("webhook cleared", calls0.some((c) => c.method === "deleteWebhook"));
  check("commands registered", calls0.some((c) => c.method === "setMyCommands"));

  await say(1001, "/start");
  check("a new user is asked for a language first", !!(await lastText(1001, /Choose your language/)));
  const menu = sentTo(await log(), 1001).find((c) => /Choose your language/.test(c.args.text));
  check("both languages are offered", menu?.args.reply_markup?.inline_keyboard?.[0]?.length === 2, menu?.args.reply_markup);
  await say(1001, "hello before choosing");
  await sleep(600);
  check("nothing is answered before a language is chosen", !textsTo(await log(), 1001).some((t) => /Echo:|Tool said/.test(t)));
  await press(1001, "l:en");
  const welcome = await lastText(1001, /Salam AI assistant/);
  check("the chosen language gets its own command menu", (await log()).some((c) => c.method === "setMyCommands" && c.args.scope?.chat_id === 1001));
  check("guest welcome", !!welcome);
  const welcomeCall = sentTo(await log(), 1001).find((c) => /Salam AI assistant/.test(c.args.text));
  const loginUrl = welcomeCall?.args.reply_markup?.inline_keyboard?.[0]?.[0]?.url ?? "";
  check("welcome has a login button", /\/telegram\?code=[0-9a-f]{32}$/.test(loginUrl), loginUrl);

  await say(1001, "hello bot");
  const answer = await lastText(1001, /Echo:/);
  check("guest gets an answer", !!answer, textsTo(await log(), 1001));

  await say(1001, "second one");
  await lastText(1001, /Guest messages/);
  await sleep(500);
  await say(1001, "third one");
  const limited = await lastText(1001, /message limit/);
  check("guest limit stops the chat", !!limited);

  const limitCall = sentTo(await log(), 1001).find((c) => /message limit/.test(c.args.text));
  const url = limitCall?.args.reply_markup?.inline_keyboard?.[0]?.[0]?.url ?? loginUrl;
  const code = /code=([0-9a-f]{32})/.exec(url)?.[1] ?? "";
  check("limit message carries a login link", code.length === 32, url);

  const info = await api("GET", `/telegram/link/${code}`);
  check("link info shows the telegram account", info.res.status === 200 && info.json.name === "User1001" && info.json.username === "u1001", info.json);
  const missing = await api("GET", `/telegram/link/${"0".repeat(32)}`);
  check("unknown link is 404", missing.res.status === 404);

  const email = `tg-${Date.now()}@test.ir`;
  const reg = await api("POST", "/auth/register", null, { name: "Tele User", email, password: "password123" });
  const token = reg.json?.token;
  check("account created", !!token);
  const noauth = await api("POST", "/telegram/link", null, { code });
  check("linking needs sign in", noauth.res.status === 401);
  const linked = await api("POST", "/telegram/link", token, { code });
  check("link confirmed", linked.res.status === 200 && linked.json.ok === true, linked.json);
  const again = await api("POST", "/telegram/link", token, { code });
  check("code works once", again.res.status === 404);
  check("telegram told about the link", !!(await lastText(1001, /Connected\. Your Salam account: Tele User/)));

  await say(1001, "remember my history");
  const afterLink = await lastText(1001, /Echo:|Tool said/);
  check("linked user gets an answer", !!afterLink);
  const chats = await until(async () => {
    const r = await api("GET", "/chats", token);
    return r.json?.chats?.length && r.json.chats[0].title === "remember my history" ? r.json.chats : null;
  });
  check("telegram chat is in the account history", chats?.length === 1 && chats[0].title === "remember my history", chats);
  const detail = await until(async () => {
    const r = await api("GET", `/chats/${chats?.[0]?.id}`, token);
    return r.json?.messages?.length === 2 && r.json.messages[1].status === "done" ? r.json : null;
  });
  check("history holds question and answer", !!detail && /Echo:|Tool said/.test(detail.messages[1].body), detail);

  await say(1001, "/chats");
  const listed = await until(async () => sentTo(await log(), 1001).find((c) => c.args.reply_markup?.inline_keyboard?.some((r) => r[0]?.callback_data === `c:${chats[0].id}`)));
  check("/chats lists the chat with a switch button", !!listed);
  await say(1001, "/new");
  await lastText(1001, /fresh chat/);
  await say(1001, "a brand new topic");
  await lastText(1001, /Echo:|Tool said/);
  const two = await until(async () => {
    const r = await api("GET", "/chats", token);
    return r.json?.chats?.length === 2 ? r.json.chats : null;
  });
  check("/new starts a second chat", !!two);
  await say(1001, "/new");
  await say(1001, "این یک پیام فارسی بسیار بلند برای آزمایش عنوان گفت‌وگو در فهرست است");
  await until(async () => (await api("GET", "/chats", token)).json?.chats?.some((c) => c.title.startsWith("این یک پیام")));
  await sleep(1500);
  const before = (await log()).length;
  await say(1001, "/chats");
  const persian = await until(async () => (await log()).slice(before).find((c) => c.args.reply_markup?.inline_keyboard?.some((r) => /این یک پیام/.test(r[0]?.text ?? ""))));
  check("a long Persian title is cut on a character boundary", !!persian && !JSON.stringify(persian.args.reply_markup).includes("\ufffd"), persian?.args.reply_markup);
  await press(1001, `c:${chats[0].id}`);
  check("switching reopens an old chat", !!(await lastText(1001, /Chat opened: remember my history/)));

  const code2 = await api("POST", "/telegram/code", token);
  const start = /start=link_([0-9a-f]{32})$/.exec(code2.json?.url ?? "")?.[1];
  check("site makes a Telegram link", code2.res.status === 201 && !!start, code2.json);
  await say(2002, `/start link_${start}`);
  check("a link from the site still asks the language", !!(await lastText(2002, /Choose your language/)));
  await press(2002, "l:en");
  const ask = await until(async () => sentTo(await log(), 2002).find((c) => c.args.reply_markup?.inline_keyboard?.some((r) => r[0]?.callback_data === `y:${start}`)));
  check("bot asks before linking from the site", !!ask && /Tele User/.test(ask.args.text), ask?.args.text);
  await press(2002, `y:${start}`);
  check("second Telegram connected", !!(await lastText(2002, /Connected\. Your Salam account/)));
  const status = await api("GET", "/telegram", token);
  check("account lists both Telegram accounts", status.json?.links?.length === 2 && status.json.bot === "MockBot", status.json);
  const removed = await api("DELETE", "/telegram/2002", token);
  check("site disconnects a Telegram", removed.res.status === 200);
  check("bot says it was disconnected", !!(await lastText(2002, /جدا شد/)));
  const status2 = await api("GET", "/telegram", token);
  check("link list shrinks", status2.json?.links?.length === 1);

  await say(1001, "/logout");
  check("logout answers", !!(await lastText(1001, /disconnected from your account/)));
  const status3 = await api("GET", "/telegram", token);
  check("logout removes the link", status3.json?.links?.length === 0);

  await press(3003, "l:en");
  await say(3003, "/stop");
  check("stop with nothing running", !!(await lastText(3003, /No answer is being written/)));
  await say(3003, "/lang");
  check("language toggles", !!(await lastText(3003, /زبان فارسی شد/)));
  await press(5005, "l:en");
  await say(5005, "slow: 8 explain");
  check("a slow answer shows a live status", !!(await until(async () => textsTo(await log(), 5005).find((t) => /^Thinking\.\.\. \(5s\)/.test(t)), 30000)), textsTo(await log(), 5005));
  check("the final answer replaces the status", !!(await lastText(5005, /Echo: slow: 8/)));

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
