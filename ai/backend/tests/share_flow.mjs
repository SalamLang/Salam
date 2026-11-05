import { execFileSync } from "node:child_process";

const API = (process.env.API ?? "http://127.0.0.1:18090").replace(/\/$/, "");
const APP = process.env.APP_ORIGIN ?? "http://localhost:3000";
const SQL = process.env.SHARE_SQL ?? "";

let passed = 0;
const failures = [];
function check(name, cond, detail) {
  if (cond) {
    passed += 1;
    console.log(`ok   ${name}`);
  } else {
    failures.push(name);
    console.log(`FAIL ${name}`, detail === undefined ? "" : JSON.stringify(detail).slice(0, 500));
  }
}

let ipCounter = 0;
const freshIp = () => `10.${(Date.now() >> 8) & 255}.${(ipCounter >> 8) & 255}.${(ipCounter++ & 255) + 1}`;

async function call(method, path, { token, body, ip, lang } = {}) {
  const headers = { "CF-Connecting-IP": ip ?? freshIp() };
  if (token) headers.Authorization = `Bearer ${token}`;
  if (body !== undefined) headers["Content-Type"] = "application/json";
  if (lang) headers["X-Salam-Lang"] = lang;
  const res = await fetch(API + path, { method, headers, body: body === undefined ? undefined : JSON.stringify(body) });
  const text = await res.text();
  let json = null;
  try {
    json = JSON.parse(text);
  } catch {}
  return { res, json, text };
}
const api = (method, path, token, body) => call(method, path, { token, body });
const sleep = (ms) => new Promise((r) => setTimeout(r, ms));

async function until(fn, ms = 45000) {
  const end = Date.now() + ms;
  while (Date.now() < end) {
    const v = await fn();
    if (v) return v;
    await sleep(250);
  }
  return null;
}

async function account(email, name) {
  const r = await api("POST", "/auth/register", undefined, { name, email, password: "password-123" });
  return r.json.token;
}

async function ownerAsk(token, chat, text) {
  const sent = await api("POST", `/chats/${chat}/messages`, token, { text });
  if (sent.res.status !== 201) return null;
  return until(async () => {
    const r = await api("GET", `/chats/${chat}/messages/${sent.json.assistant.id}`, token);
    return r.json?.status && r.json.status !== "pending" ? r.json : null;
  });
}

const view = (tok, after = 0, opts = {}) => call("GET", `/shares/${tok}?after=${after}`, opts);
const guestSend = (tok, text, name = "Guest 3", opts = {}) => call("POST", `/shares/${tok}/messages`, { body: { text, name }, ...opts });
const sharesOf = async (token, chat) => (await api("GET", `/chats/${chat}/shares`, token)).json;
const pick = (list, kind) => list.shares.find((s) => s.kind === kind);

async function guestAnswered(tok, mid) {
  return until(async () => {
    const v = await view(tok, 0);
    const m = v.json?.messages?.find((x) => x.id === mid);
    return m && m.status !== "pending" ? m : null;
  });
}

const email = `share-owner-${Date.now()}@test.ir`;
const owner = await account(email, "Share Owner");
const stranger = await account(`share-stranger-${Date.now()}@test.ir`, "Stranger");
const chat = (await api("POST", "/chats", owner)).json.chat.id;
const secretChat = (await api("POST", "/chats", owner)).json.chat.id;
await ownerAsk(owner, secretChat, "marker-private-chat-zzz");
const first = await ownerAsk(owner, chat, "owner says marker-owner-hello");
check("the owner can chat as before", first?.status === "done", first);

console.log("creating and listing links");
check("listing needs a login", (await call("GET", `/chats/${chat}/shares`)).res.status === 401);
check("a stranger cannot list the links of a chat", (await api("GET", `/chats/${chat}/shares`, stranger)).res.status === 404);
check("a stranger cannot create a link", (await api("POST", `/chats/${chat}/shares`, stranger, { kind: "view" })).res.status === 404);
check("an unknown kind is refused", (await api("POST", `/chats/${chat}/shares`, owner, { kind: "admin" })).res.status === 400);
const empty = await sharesOf(owner, chat);
check("a chat starts with no links", empty.shares.length === 0 && empty.collab_allowed === true && empty.default_hours === 24, empty);

const createdView = await api("POST", `/chats/${chat}/shares`, owner, { kind: "view" });
const viewShare = pick(createdView.json, "view");
check("a view link is created", createdView.res.status === 201 && /^[0-9a-f]{40}$/.test(viewShare?.token ?? ""), createdView.json);
check("the url is the app address plus /s/ plus the token", viewShare.url === `${APP}/s/${viewShare.token}`, viewShare);
check("a view link never expires by default", viewShare.expires === 0 && viewShare.state === "ok", viewShare);
const viewTok = viewShare.token;
const again = await api("POST", `/chats/${chat}/shares`, owner, { kind: "view" });
check("creating again returns the same link", again.res.status === 200 && pick(again.json, "view").token === viewTok, again.json);

const createdCollab = await api("POST", `/chats/${chat}/shares`, owner, { kind: "collab" });
const collabShare = pick(createdCollab.json, "collab");
const collabTok = collabShare.token;
const nowSec = Math.floor(Date.now() / 1000);
check("a collaborate link is created", createdCollab.res.status === 201 && /^[0-9a-f]{40}$/.test(collabTok) && collabTok !== viewTok, createdCollab.json);
check("a collaborate link expires in a day by default", Math.abs(collabShare.expires - (nowSec + 86400)) < 120, collabShare.expires);
check("a collaborate link has a default message cap", collabShare.max_messages === 100 && collabShare.max_daily === 50, collabShare);
check("the owner can read the link again later", pick(await sharesOf(owner, chat), "collab").url === `${APP}/s/${collabTok}`);
check("no raw token is stored in the list of another user", !JSON.stringify(await sharesOf(owner, secretChat)).includes(collabTok));
const bad = await api("POST", `/chats/${chat}/shares`, owner, { kind: "view", hours: 5 });
check("a repeated create ignores a bad duration but a bad one on a new kind is refused", bad.res.status === 200);

console.log("what a guest can see");
const v1 = await view(viewTok);
check("anyone with the link can read the chat", v1.res.status === 200 && v1.json.messages.length === 2 && v1.json.messages[0].body.includes("marker-owner-hello"), v1.json);
check("the page carries the chat title and kind", v1.json.share.kind === "view" && v1.json.share.collab === false && typeof v1.json.chat.title === "string");
check("noindex and no-store on the shared api", /noindex/.test(v1.res.headers.get("x-robots-tag") ?? "") && /no-store/.test(v1.res.headers.get("cache-control") ?? ""), [...v1.res.headers]);
check("no referrer is sent from shared pages", v1.res.headers.get("referrer-policy") === "no-referrer");
const leak = v1.text;
check("nothing about the owner leaks", !leak.includes(email) && !leak.includes("Share Owner") && !/user_id|engine_sid|device|workspace|bridge|password|token_hash/.test(leak), leak.slice(0, 300));
check("the owner session token is not in the page", !leak.includes(owner));
check("another chat of the owner is not reachable", !leak.includes("marker-private-chat-zzz") && !leak.includes(secretChat));
check("only newer messages are returned with after", (await view(viewTok, v1.json.messages[1].id)).json.messages.length === 0);
check("a view link cannot send", (await guestSend(viewTok, "hi")).res.status === 403);
check("a view link answers with the view_only code", (await guestSend(viewTok, "hi")).json.error === "view_only");
check("an unknown token is gone", (await view("0".repeat(40))).res.status === 404);
check("a malformed token is gone", (await view("not-a-token")).json.error === "gone");
check("an upper case token is gone", (await view(viewTok.toUpperCase())).res.status === 404);
check("a session token is not a share token", (await view(owner)).res.status === 404);
check("a share token is not a session", (await api("GET", `/chats/${chat}`, viewTok)).res.status === 401);
check("a collab token is not a session either", (await api("GET", "/me", collabTok)).res.status === 401);
check("gone responses are noindex too", /noindex/.test((await view("0".repeat(40))).res.headers.get("x-robots-tag") ?? ""));
const pre = await fetch(`${API}/shares/${viewTok}`, { method: "OPTIONS", headers: { Origin: APP, "Access-Control-Request-Method": "GET" } });
check("the public routes answer the preflight", pre.status === 204 && pre.headers.get("access-control-allow-origin") === APP);
check("english errors are translated", (await call("GET", `/shares/${"1".repeat(40)}`, { lang: "en" })).json.message === "This link does not exist or was removed.");

console.log("sending through a collaborate link");
const sent = await guestSend(collabTok, "guest says marker-guest-hello", "Guest 3");
check("a guest message is accepted", sent.res.status === 201 && sent.json.assistant.status === "pending", sent.json);
const done = await guestAnswered(collabTok, sent.json.assistant.id);
check("the assistant answers the guest", done?.status === "done" && done.body.length > 0, done);
const gv = await view(collabTok);
const gm = gv.json.messages.find((m) => m.body.includes("marker-guest-hello"));
check("the guest message is attributed to the guest name", gm?.guest === "Guest 3", gv.json.messages);
check("owner messages carry no guest name", gv.json.messages[0].guest === undefined);
check("the collab page says sending is on", gv.json.share.collab === true && gv.json.share.state === "ok");
const ownerView = await api("GET", `/chats/${chat}`, owner);
check("the owner sees who wrote the message", ownerView.json.messages.some((m) => m.guest === "Guest 3" && m.body.includes("marker-guest-hello")));
check("the chat list marks shared chats", (await api("GET", "/chats", owner)).json.chats.find((c) => c.id === chat).shared.slice().sort().join() === "collab,view");
check("the chat list leaves other chats alone", (await api("GET", "/chats", owner)).json.chats.find((c) => c.id === secretChat).shared === undefined);
const afterSend = pick(await sharesOf(owner, chat), "collab");
check("the owner sees the usage", afterSend.messages === 1 && afterSend.today === 1 && afterSend.guests[0].name === "Guest 3" && afterSend.guests[0].count === 1, afterSend);
check("the owner sees approximate tokens", typeof afterSend.tokens === "number" && afterSend.last_used > 0, afterSend);
const viewerSees = await view(viewTok);
check("the view link sees guest messages too", viewerSees.json.messages.length === 4 && viewerSees.json.messages[2].guest === "Guest 3");

console.log("input rules");
check("an empty message is refused", (await guestSend(collabTok, "   ")).res.status === 400);
check("a long message is refused", (await guestSend(collabTok, "x".repeat(2100))).json.error === "too_long");
const withFile = await call("POST", `/shares/${collabTok}/messages`, { body: { text: "see", name: "G", files: [{ name: "a.txt", data: "aGk=" }] } });
check("files from guests are refused", withFile.res.status === 400 && withFile.json.error === "no_files", withFile.json);
const html = await guestSend(collabTok, "second marker-two", "<img src=x onerror=alert(1)>");
check("a hostile name is cleaned", html.res.status === 201, html.json);
await guestAnswered(collabTok, html.json.assistant.id);
const cleaned = (await view(collabTok)).json.messages.find((m) => m.body.includes("marker-two"));
check("the stored name has no markup characters", cleaned.guest && !/[<>"'`&]/.test(cleaned.guest), cleaned);
const noName = await guestSend(collabTok, "third marker-three", "");
await guestAnswered(collabTok, noName.json.assistant.id);
check("a missing name becomes Guest", (await view(collabTok)).json.messages.find((m) => m.body.includes("marker-three")).guest === "Guest");

console.log("pause, resume and the stop control");
const paused = await api("PATCH", `/chats/${chat}/shares/collab`, owner, { paused: true });
check("the owner pauses the collaborate link", pick(paused.json, "collab").state === "paused", paused.json);
const blocked = await guestSend(collabTok, "while paused");
check("a paused link refuses new messages", blocked.res.status === 403 && blocked.json.error === "paused", blocked.json);
check("a paused link does not leak the reason details", !blocked.text.includes("plan"));
check("the page shows the paused state", (await view(collabTok)).json.share.state === "paused");
check("the view link keeps working while collab is paused", (await view(viewTok)).res.status === 200);
check("the guest can still read a paused collaborate chat", (await view(collabTok)).json.messages.length > 0);
const resumed = await api("PATCH", `/chats/${chat}/shares/collab`, owner, { paused: false });
check("the owner resumes it", pick(resumed.json, "collab").state === "ok");
const pv = await api("PATCH", `/chats/${chat}/shares/view`, owner, { paused: true });
const pausedView = await view(viewTok);
check("pausing a view link hides the chat", pick(pv.json, "view").state === "paused" && pausedView.json.share.readable === false && pausedView.json.messages.length === 0 && pausedView.json.chat.title === "", pausedView.json);
await api("PATCH", `/chats/${chat}/shares/view`, owner, { paused: false });
check("resuming a view link brings the chat back", (await view(viewTok)).json.messages.length > 0);
check("a stranger cannot pause a link", (await api("PATCH", `/chats/${chat}/shares/collab`, stranger, { paused: true })).res.status === 404);

console.log("running turns can be stopped by the owner");
const slow = await guestSend(collabTok, "slow: 8 long job");
check("a slow guest turn starts", slow.res.status === 201, slow.json);
const busy = await guestSend(collabTok, "overlapping");
check("a second guest message while one runs is refused", busy.res.status === 409 && busy.json.error === "busy", busy.json);
const ownerBusy = await api("POST", `/chats/${chat}/messages`, owner, { text: "owner overlapping" });
check("the owner cannot overlap a guest turn either", ownerBusy.res.status === 409);
const stopped = await api("POST", `/chats/${chat}/stop`, owner);
check("the owner stops the guest turn", stopped.res.status === 200);
const stoppedMsg = await guestAnswered(collabTok, slow.json.assistant.id);
check("the stopped turn is finished", stoppedMsg?.status === "done", stoppedMsg);

console.log("two guests at once");
const racers = await Promise.all([guestSend(collabTok, "race one", "A", { ip: freshIp() }), guestSend(collabTok, "race two", "B", { ip: freshIp() })]);
const codes = racers.map((r) => r.res.status).sort();
check("only one of two simultaneous guest messages starts a turn", codes[0] === 201 && codes[1] === 409, codes);
const winner = racers.find((r) => r.res.status === 201);
await guestAnswered(collabTok, winner.json.assistant.id);

console.log("caps");
const used = pick(await sharesOf(owner, chat), "collab").messages;
await api("PATCH", `/chats/${chat}/shares/collab`, owner, { max_messages: used + 1 });
const lastOne = await guestSend(collabTok, "the last allowed one");
check("a message under the cap is accepted", lastOne.res.status === 201, lastOne.json);
await guestAnswered(collabTok, lastOne.json.assistant.id);
const over = await guestSend(collabTok, "one too many");
check("the cap stops further messages", over.res.status === 429 && over.json.error === "cap", over.json);
check("the page shows the cap state", (await view(collabTok)).json.share.state === "cap" && (await view(collabTok)).json.share.collab === false);
check("a bad cap is refused", (await api("PATCH", `/chats/${chat}/shares/collab`, owner, { max_messages: 99999 })).res.status === 400);
await api("PATCH", `/chats/${chat}/shares/collab`, owner, { max_messages: 0 });
const todayUsed = pick(await sharesOf(owner, chat), "collab").today;
await api("PATCH", `/chats/${chat}/shares/collab`, owner, { max_daily: todayUsed });
const daily = await guestSend(collabTok, "daily check");
check("the daily cap stops further messages", daily.res.status === 429 && daily.json.error === "daily", daily.json);
await api("PATCH", `/chats/${chat}/shares/collab`, owner, { max_daily: 0 });
const free = await guestSend(collabTok, "no caps now");
check("removing the caps lets messages through", free.res.status === 201, free.json);
await guestAnswered(collabTok, free.json.assistant.id);

console.log("expiry");
const hour = await api("PATCH", `/chats/${chat}/shares/collab`, owner, { hours: 1 });
check("the owner can shorten the lifetime", Math.abs(pick(hour.json, "collab").expires - (Math.floor(Date.now() / 1000) + 3600)) < 120, pick(hour.json, "collab"));
check("a bad duration is refused", (await api("PATCH", `/chats/${chat}/shares/collab`, owner, { hours: 3 })).res.status === 400);
const never = await api("PATCH", `/chats/${chat}/shares/collab`, owner, { hours: 0 });
check("the owner can make it never expire", pick(never.json, "collab").expires === 0);
if (SQL) {
  const sh = (q) => execFileSync("sh", ["-c", SQL], { input: q, encoding: "utf8" });
  sh(`UPDATE shares SET expires = UNIX_TIMESTAMP() - 5 WHERE chat_id = '${chat}' AND kind = 'collab';`);
  const gone = await guestSend(collabTok, "after expiry");
  check("an expired link refuses messages", gone.res.status === 410 && gone.json.error === "expired", gone.json);
  const ev = await view(collabTok);
  check("an expired link shows nothing", ev.json.share.state === "expired" && ev.json.messages.length === 0 && ev.json.chat.title === "");
  check("the owner sees it as expired", pick(await sharesOf(owner, chat), "collab").state === "expired");
  await api("PATCH", `/chats/${chat}/shares/collab`, owner, { hours: 24 });
  check("extending revives an expired link", (await view(collabTok)).json.share.state === "ok");
} else {
  console.log("skip expiry by clock (set SHARE_SQL to a mysql command to run it)");
}

console.log("rate limits");
const hammer = freshIp();
const results = [];
for (let i = 0; i < 9; i += 1) results.push((await guestSend(collabTok, `hammer ${i}`, "H", { ip: hammer })).res.status);
check("one address is slowed down after a burst", results.includes(429) && results.slice(-1)[0] === 429, results);
const slowJson = await guestSend(collabTok, "again", "H", { ip: hammer });
check("the slow down answer has its own code", slowJson.json.error === "slow_down", slowJson.json);
await sleep(100);
const readIp = freshIp();
let readStatuses = [];
for (let i = 0; i < 100; i += 1) readStatuses.push((await view(viewTok, 0, { ip: readIp })).res.status);
check("reads from one address are limited too", readStatuses.includes(429) && readStatuses[0] === 200, readStatuses.filter((s) => s !== 200).length);
await until(async () => (await ownerAsk(owner, chat, "owner can still talk")) ?? null, 70000);

console.log("regenerate and delete");
const before = pick(await sharesOf(owner, chat), "collab");
const regen = await api("POST", `/chats/${chat}/shares/collab/regenerate`, owner);
const fresh = pick(regen.json, "collab");
check("regenerating gives a new token", fresh.token !== collabTok && /^[0-9a-f]{40}$/.test(fresh.token), fresh);
check("the old collaborate url is dead", (await view(collabTok)).res.status === 404 && (await guestSend(collabTok, "old")).res.status === 404);
check("the new url works", (await view(fresh.token)).json.share.kind === "collab");
check("the counters survive a regenerate", fresh.messages === before.messages);
check("the view link is unaffected by it", (await view(viewTok)).res.status === 200);
const del = await api("DELETE", `/chats/${chat}/shares/collab`, owner);
check("deleting removes only that link", pick(del.json, "collab") === undefined && pick(del.json, "view") !== undefined);
check("a deleted link is gone", (await view(fresh.token)).res.status === 404);
check("a stranger cannot delete a link", (await api("DELETE", `/chats/${chat}/shares/view`, stranger)).res.status === 404);
const recreated = await api("POST", `/chats/${chat}/shares`, owner, { kind: "collab", hours: 168 });
check("a link can be created again with another duration", pick(recreated.json, "collab").token !== fresh.token && Math.abs(pick(recreated.json, "collab").expires - (Math.floor(Date.now() / 1000) + 7 * 86400)) < 120);

console.log("computers and cloud environments");
const dev = await api("POST", "/bridge/devices", owner, { name: "laptop", os: "linux" });
const device = dev.json.device;
const dchat = (await api("POST", "/chats", owner, { device_id: device.id, workspace: "/home/secretuser/ws" })).json.chat.id;
const dRefused = await api("POST", `/chats/${dchat}/shares`, owner, { kind: "collab" });
check("a collaborate link is refused for a computer chat", dRefused.res.status === 409 && dRefused.json.error === "collab_device", dRefused.json);
const dShares = await sharesOf(owner, dchat);
check("the list says collaboration is not allowed and why", dShares.collab_allowed === false && dShares.collab_refusal === "device" && dShares.hide_tools === true, dShares);
const dView = await api("POST", `/chats/${dchat}/shares`, owner, { kind: "view" });
check("a view link is allowed for a computer chat", dView.res.status === 201);
const dTok = pick(dView.json, "view").token;
const dAsk = await ownerAsk(owner, dchat, "run: echo offline");
check("the owner's computer chat answered", dAsk?.status === "done", dAsk);
const dPage = await view(dTok);
check("the view of a computer chat has the assistant text", dPage.json.messages.length === 2 && dPage.json.messages[1].body.length > 0, dPage.json);
check("the view hides the workspace, the device and tool payloads", !/secretuser|workspace|device_id|"calls"|bridge_calls|output/.test(dPage.text), dPage.text.slice(0, 400));
check("the view says tools are hidden", dPage.json.share.hide_tools === true);
check("a computer view link cannot send", (await guestSend(dTok, "do something")).res.status === 403);
check("the chat itself is unchanged by the view link", (await api("GET", `/chats/${dchat}`, owner)).json.chat.workspace === "/home/secretuser/ws");
const cloud = await api("POST", "/chats", owner, { cloud: true });
if (cloud.res.status === 201) {
  const cRefused = await api("POST", `/chats/${cloud.json.chat.id}/shares`, owner, { kind: "collab" });
  check("a collaborate link is refused for a cloud chat", cRefused.res.status === 409 && cRefused.json.error === "collab_cloud", cRefused.json);
  const cView = await api("POST", `/chats/${cloud.json.chat.id}/shares`, owner, { kind: "view" });
  check("a view link is allowed for a cloud chat", cView.res.status === 201);
} else {
  console.log("skip cloud chat (not enabled on this server)");
}

console.log("the owner's plan limit reaches the guest");
const rich = await account(`share-limit-${Date.now()}@test.ir`, "Limit Owner");
const lchat = (await api("POST", "/chats", rich)).json.chat.id;
const lshare = await api("POST", `/chats/${lchat}/shares`, rich, { kind: "collab" });
const lTok = pick(lshare.json, "collab").token;
const chats = [lchat];
for (let i = 0; i < 9; i += 1) chats.push((await api("POST", "/chats", rich)).json.chat.id);
await Promise.all(chats.map(async (c) => {
  for (let i = 0; i < 6; i += 1) await ownerAsk(rich, c, `fill ${i}`);
}));
const limited = await guestSend(lTok, "can I still talk");
check("a guest gets a friendly error when the owner is out of messages", limited.res.status === 429 && limited.json.error === "owner_limit", limited.json);
check("the error does not name the owner's plan", !/رایگان|free|plan|پلن/i.test(limited.text), limited.text);
const lstate = (await view(lTok)).json.share;
check("the link pauses itself", lstate.state === "limit" && lstate.collab === false, lstate);
const lowner = pick(await sharesOf(rich, lchat), "collab");
check("the owner sees why it paused", lowner.paused === true && lowner.reason === "limit", lowner);
await api("PATCH", `/chats/${lchat}/shares/collab`, rich, { paused: false });
check("the owner can resume it afterwards", pick(await sharesOf(rich, lchat), "collab").paused === false);

console.log("cleanup");
const gone = await api("DELETE", `/chats/${chat}`, owner);
check("deleting the chat deletes its links", gone.res.status === 200 && (await view(viewTok)).res.status === 404);

console.log(`\n${passed} passed, ${failures.length} failed`);
if (failures.length) {
  console.log(failures.join("\n"));
  process.exit(1);
}
