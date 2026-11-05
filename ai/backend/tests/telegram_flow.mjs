const API = (process.env.API ?? "http://127.0.0.1:18090").replace(/\/$/, "");
const TG = (process.env.TG ?? "http://127.0.0.1:18097").replace(/\/$/, "");
const GUEST_LIMIT = Number(process.env.GUEST_LIMIT ?? 2);
const SPEECH = (process.env.SPEECH ?? "http://127.0.0.1:18088").replace(/\/$/, "");

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


async function voiceSection() {
  console.log("voice messages");
  const mode = async (m) => (await fetch(`${SPEECH}/mode?mode=${m}`)).json();
  const file = (file_id, path, bytes, extra = {}) => fetch(`${TG}/_file`, { method: "POST", body: JSON.stringify({ file_id, path, b64: bytes.toString("base64"), ...extra }) });
  const ogg = Buffer.concat([Buffer.from("OggS"), Buffer.alloc(300, 7)]);
  const mp3 = Buffer.concat([Buffer.from("ID3"), Buffer.alloc(500, 9)]);
  const wav = Buffer.concat([Buffer.from("RIFF\0\0\0\0WAVEfmt "), Buffer.alloc(200, 3)]);
  const mp4 = Buffer.concat([Buffer.alloc(4), Buffer.from("ftypisom"), Buffer.alloc(200, 5)]);
  let mid = 9000;
  const media = (id, field, obj, extra = {}, chat = { id, type: "private" }) => {
    mid += 1;
    return inject({ message: { message_id: mid, from: user(id), chat, [field]: obj, ...extra } });
  };
  const sent = async (chat, from, pattern) =>
    until(async () => (await log()).slice(from).find((c) => ["sendMessage", "editMessageText"].includes(c.method) && String(c.args.chat_id) === String(chat) && pattern.test(c.args.text)), 20000);
  const mark = async () => (await log()).length;
  const downloads = async () => (await fetch(`${TG}/_downloads`)).json();
  const served = async () => (await mode("ok")).served;

  await mode("ok");
  await file("v1", "voice/file_1.oga", ogg);
  await file("a1", "music/file_2.mp3", mp3);
  await file("w1", "documents/file_3.wav", wav);
  await file("n1", "video_notes/file_4.mp4", mp4);
  await file("pdf", "documents/file_5.pdf", Buffer.from("%PDF-1.4 hello"));
  await file("junk", "voice/file_6.oga", Buffer.from("this is plainly not audio at all"));
  await file("evil", "../../etc/passwd", ogg);
  await file("lie", "voice/file_7.oga", Buffer.alloc(400000, 1), { declared: 900 });
  await file("huge", "voice/file_8.oga", ogg, { declared: 400000 });
  await file("broken", "voice/file_9.oga", ogg, { status: 500 });

  const linkUser = async (id, name) => {
    const reg = await api("POST", "/auth/register", null, { name, email: `voice-${id}-${Date.now()}@test.ir`, password: "password123" });
    const token = reg.json?.token;
    const code = await api("POST", "/telegram/code", token);
    const start = /start=link_([0-9a-f]{32})$/.exec(code.json?.url ?? "")?.[1];
    await say(id, `/start link_${start}`);
    await lastText(id, /Choose your language/);
    await press(id, "l:en");
    await until(async () => sentTo(await log(), id).find((c) => c.args.reply_markup?.inline_keyboard?.some((r) => r[0]?.callback_data === `y:${start}`)));
    await press(id, `y:${start}`);
    check(`${name} linked`, !!(await lastText(id, /Connected\. Your Salam account/)));
    return token;
  };
  const token = await linkUser(7007, "Voice User");

  let from = await mark();
  await media(7007, "voice", { file_id: "v1", file_size: ogg.length, duration: 3 });
  const heard = await sent(7007, from, /^I heard: "hello "world", this is a test"$/);
  check("a voice note is transcribed and shown back", !!heard, textsTo(await log(), 7007).slice(-3));
  check("the transcript is a reply to the voice message", heard?.args.reply_parameters?.message_id === mid, heard?.args);
  check("the transcript is then answered like a typed message", !!(await sent(7007, from, /Echo: hello/)));
  const first = (await mode("ok")).last;
  check("the speech service got the exact audio in the bot language", first?.size === ogg.length && first.language === "en" && first.authorized, first);
  check("a voice note was fetched from the file endpoint", (await downloads()).some((d) => d.path === "voice/file_1.oga" && d.tokenOk));
  const typingSeen = (await log()).slice(from).some((c) => c.method === "sendChatAction" && c.args.action === "typing");
  check("typing is shown while the voice message is worked on", typingSeen);

  await until(async () => (await api("GET", "/chats", token)).json?.chats?.length === 1);
  const chats = (await api("GET", "/chats", token)).json?.chats ?? [];
  check("the transcript is the chat title and history entry", chats[0]?.title.startsWith("hello"), chats);

  from = await mark();
  await media(7007, "audio", { file_id: "a1", file_size: mp3.length, duration: 20, title: "Song", performer: "Band", mime_type: "audio/mpeg", file_name: "song.mp3" });
  check("an mp3 audio file is accepted", !!(await sent(7007, from, /^I heard: /)));
  await lastText(7007, /Echo: hello/);

  from = await mark();
  await media(7007, "document", { file_id: "w1", file_size: wav.length, mime_type: "audio/x-wav", file_name: "clip.wav" });
  check("an audio document is accepted", !!(await sent(7007, from, /^I heard: /)));
  from = await mark();
  await media(7007, "video_note", { file_id: "n1", file_size: mp4.length, duration: 5, length: 240 });
  check("a video note is accepted", !!(await sent(7007, from, /^I heard: /)));
  await lastText(7007, /Echo: hello/);

  from = await mark();
  await media(7007, "document", { file_id: "pdf", file_size: 14, mime_type: "application/pdf", file_name: "a.pdf" });
  check("other documents still get the text only message", !!(await sent(7007, from, /Only text and voice messages/)));

  await mode("empty");
  from = await mark();
  await media(7007, "voice", { file_id: "v1", file_size: ogg.length, duration: 3 });
  check("silence gets a clear message", !!(await sent(7007, from, /Nothing could be recognised\. Try again closer/)));
  from = await mark();
  await media(7007, "audio", { file_id: "a1", file_size: mp3.length, duration: 20, mime_type: "audio/mpeg" });
  check("music with no speech says lyrics may be missed", !!(await sent(7007, from, /Only speech is converted into text, so song lyrics may be missed/)));
  check("an empty transcript is not sent to the assistant", !(await log()).slice(from).some((c) => c.method === "sendMessage" && /Echo/.test(c.args.text ?? "")));

  await mode("ok");
  from = await mark();
  await media(7007, "voice", { file_id: "v1", file_size: ogg.length, duration: 3 }, { caption: "/fa" });
  await sent(7007, from, /^I heard/);
  check("a caption with a language code picks that language", (await mode("ok")).last.language === "fa");

  await mode("too_long");
  from = await mark();
  await media(7007, "voice", { file_id: "v1", file_size: ogg.length, duration: 3 });
  check("the service saying too long is explained", !!(await sent(7007, from, /longer than 120 seconds/)));
  await mode("ok");

  const sttBefore = await served();
  from = await mark();
  const dlBefore = (await downloads()).length;
  await media(7007, "voice", { file_id: "v1", file_size: ogg.length, duration: 600 });
  check("a voice note over the length limit is refused", !!(await sent(7007, from, /longer than 120 seconds/)));
  await media(7007, "voice", { file_id: "huge", file_size: 400000, duration: 3 });
  check("a file over the size limit is refused", !!(await sent(7007, from, /larger than 292 KB/)));
  await media(7007, "voice", { file_id: "huge", duration: 3 });
  check("a size only known from getFile is refused too", (await until(async () => (await log()).slice(from).filter((c) => /larger than/.test(c.args.text ?? "")).length >= 2)) === true);
  await sleep(300);
  check("refused files are not downloaded and not converted", (await downloads()).length === dlBefore && (await served()) === sttBefore);

  from = await mark();
  await media(7007, "voice", { file_id: "nope", duration: 3 });
  check("a file Telegram cannot give out is explained", !!(await sent(7007, from, /Fetching the file from Telegram failed/)));
  from = await mark();
  await media(7007, "voice", { file_id: "broken", duration: 3 });
  check("a failing download is explained", !!(await sent(7007, from, /Fetching the file from Telegram failed/)));
  from = await mark();
  await media(7007, "voice", { file_id: "evil", duration: 3 });
  await sent(7007, from, /Fetching the file from Telegram failed/);
  check("a path with dots is never requested", !(await downloads()).some((d) => d.path.includes("..")));
  from = await mark();
  await media(7007, "voice", { file_id: "lie", duration: 3 });
  check("a file larger than Telegram declared is cut off", !!(await sent(7007, from, /Fetching the file from Telegram failed/)));
  from = await mark();
  await media(7007, "voice", { file_id: "junk", duration: 3 });
  check("bytes that are not audio are explained", !!(await sent(7007, from, /could not be read/)));
  check("none of those reached the speech service", (await served()) === sttBefore);

  const token2 = await linkUser(9009, "Second Voice");
  check("the second account is separate", token2 !== token);
  await mode("busy");
  from = await mark();
  await media(9009, "voice", { file_id: "v1", file_size: ogg.length, duration: 3 });
  check("a busy service is explained", !!(await sent(9009, from, /busy right now/)));
  await mode("broken");
  from = await mark();
  await media(9009, "voice", { file_id: "v1", file_size: ogg.length, duration: 3 });
  check("a broken service is explained", !!(await sent(9009, from, /not available right now\. Please send your message as text/)));

  await mode("down");
  await sleep(1500);
  from = await mark();
  await media(9009, "voice", { file_id: "v1", file_size: ogg.length, duration: 3 });
  check("with the speech service down the user is told to send text", !!(await sent(9009, from, /Voice messages are not available right now\. Please send your message as text/)));
  check("nothing is fetched while voice is off", !(await log()).slice(from).some((c) => c.method === "getFile"));
  await say(9009, "typed text still works");
  check("typed messages keep working while voice is off", !!(await lastText(9009, /Echo: typed text still works/)));
  await mode("ok");
  await sleep(1500);

  from = await mark();
  await media(9009, "voice", { file_id: "v1", file_size: ogg.length, duration: 3 }, {}, { id: -4004, type: "group" });
  await sleep(1200);
  check("voice in a group is ignored like text", !(await log()).slice(from).some((c) => String(c.args.chat_id) === "-4004"));

  for (let n = 0; n < 7; n += 1) {
    from = await mark();
    await media(9009, "voice", { file_id: "v1", file_size: ogg.length, duration: 3 });
    await sent(9009, from, /^I heard/);
    await sleep(500);
  }
  from = await mark();
  await media(9009, "voice", { file_id: "v1", file_size: ogg.length, duration: 3 });
  check("the hourly voice allowance of a linked user ends", !!(await sent(9009, from, /used this hour's voice messages/)));

  await press(8008, "l:fa");
  from = await mark();
  await media(8008, "voice", { file_id: "v1", file_size: ogg.length, duration: 3 });
  check("a guest in Persian is answered in Persian", !!(await sent(8008, from, /^شنیدم: «/)));
  check("the language sent to the service follows the bot language", (await mode("ok")).last.language === "fa");
  await lastText(8008, /Echo:/);
  await sleep(500);
  for (const n of [1, 2]) {
    from = await mark();
    await media(8008, "voice", { file_id: "v1", file_size: ogg.length, duration: 3 });
    await sent(8008, from, /^شنیدم/);
    await sleep(800);
  }
  from = await mark();
  await media(8008, "voice", { file_id: "v1", file_size: ogg.length, duration: 3 });
  const guestLimit = await sent(8008, from, /سهم پیام صوتی مهمان/);
  check("guests get a smaller voice allowance with a sign in button", !!guestLimit && !!guestLimit.args.reply_markup?.inline_keyboard?.[0]?.[0]?.url, guestLimit?.args);
}

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
  await lastText(1001, /fresh chat/);
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

  await voiceSection();

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
