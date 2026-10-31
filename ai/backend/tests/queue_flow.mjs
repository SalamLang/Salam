const API = (process.env.API ?? "http://127.0.0.1:18090").replace(/\/$/, "");

let passed = 0;
const failures = [];
function check(name, cond, detail) {
  if (cond) {
    passed += 1;
    console.log(`ok   ${name}`);
  } else {
    failures.push(name);
    console.log(`FAIL ${name}`, detail === undefined ? "" : JSON.stringify(detail).slice(0, 400));
  }
}

async function api(method, path, token, body) {
  const res = await fetch(`${API}${path}`, {
    method,
    headers: { "Content-Type": "application/json", ...(token ? { Authorization: `Bearer ${token}` } : {}) },
    body: body === undefined ? undefined : JSON.stringify(body),
  });
  let json = null;
  try {
    json = await res.json();
  } catch {}
  return { res, json };
}

const sleep = (ms) => new Promise((r) => setTimeout(r, ms));

async function until(what, fn, seconds = 60) {
  const end = Date.now() + seconds * 1000;
  while (Date.now() < end) {
    const v = await fn();
    if (v) return v;
    await sleep(500);
  }
  console.log(`timed out waiting for ${what}`);
  return null;
}

const email = `queue-${Date.now()}@test.ir`;
const reg = await api("POST", "/auth/register", undefined, { name: "Queue", email, password: "password-123" });
const token = reg.json.token;
const made = await api("POST", "/chats", token);
const chat = made.json.chat.id;
const messages = async () => (await api("GET", `/chats/${chat}`, token)).json;

console.log("queue behind a running answer");
const first = await api("POST", `/chats/${chat}/messages`, token, { text: "slow: 3 first question" });
check("the first message starts an answer", first.res.status === 201 && first.json.assistant.status === "pending", first.json);

const direct = await api("POST", `/chats/${chat}/messages`, token, { text: "direct while busy" });
check("a direct message while busy is still refused", direct.res.status === 409, direct.json);

const add = async (text, extra = {}) => api("POST", `/chats/${chat}/queue`, token, { text, ...extra });
const q1 = await add("second question");
const q2 = await add("third question");
const q3 = await add("fourth question");
check("queued messages are accepted", q1.res.status === 201 && q3.res.status === 201, q3.json);
check("the queue lists them in order", q3.json.queue.map((q) => q.text).join("|") === "second question|third question|fourth question", q3.json.queue);

const empty = await api("POST", `/chats/${chat}/queue`, token, { text: "   " });
check("an empty queued message is refused", empty.res.status === 400, empty.json);

const detail = await messages();
check("the chat carries its queue", detail.queue?.length === 3, detail.queue);

console.log("nobody is watching: the server keeps going on its own");
const finished = await until("all four answers", async () => {
  const d = await messages();
  const a = d.messages.filter((m) => m.role === "assistant");
  return a.length === 4 && a.every((m) => m.status === "done") && d.queue.length === 0 ? d : null;
}, 90);
check("every queued message ran without anyone polling", !!finished, finished);
if (finished) {
  const users = finished.messages.filter((m) => m.role === "user").map((m) => m.body);
  check("they ran one by one in the order they were queued", users.join("|") === "slow: 3 first question|second question|third question|fourth question", users);
  const order = finished.messages.map((m) => m.role[0]).join("");
  check("each answer follows its own question", order === "uauauaua", order);
}

console.log("run now");
const long = await api("POST", `/chats/${chat}/messages`, token, { text: "slow: 40 a very long job" });
check("a long answer starts", long.res.status === 201, long.json);
const a = (await add("message A")).json;
const b = (await add("message B")).json;
const idB = b.queue[1].id;
const now = await api("POST", `/chats/${chat}/queue/${idB}/now`, token);
check("run now answers right away", now.res.status === 201 || now.res.status === 200, now.json);
const after = await until("B running", async () => {
  const d = await messages();
  const pend = d.messages.find((m) => m.role === "assistant" && m.status === "pending");
  const bUser = d.messages.find((m) => m.role === "user" && m.body === "message B");
  return pend && bUser ? d : null;
}, 15);
check("the running answer was stopped and message B started at once", !!after, after?.messages?.slice(-4));
if (after) {
  const longAnswer = after.messages.find((m) => m.role === "user" && m.body.startsWith("slow: 40")) ? after.messages[after.messages.findIndex((m) => m.body.startsWith("slow: 40")) + 1] : null;
  check("the interrupted answer is kept as done", longAnswer?.status === "done", longAnswer);
  check("message A is still waiting its turn", after.queue.length === 1 && after.queue[0].text === "message A", after.queue);
}
const drained = await until("A to finish", async () => {
  const d = await messages();
  return d.queue.length === 0 && !d.messages.some((m) => m.status === "pending") && d.messages.some((m) => m.body === "message A") ? d : null;
}, 60);
const users2 = drained?.messages.filter((m) => m.role === "user").map((m) => m.body).slice(-3);
check("B ran before A", users2?.join("|") === "slow: 40 a very long job|message B|message A", users2);

console.log("remove and clear");
const hold = await api("POST", `/chats/${chat}/messages`, token, { text: "slow: 30 hold" });
const r1 = (await add("to remove")).json;
await add("to clear one");
await add("to clear two");
const removed = await api("DELETE", `/chats/${chat}/queue/${r1.queue[0].id}`, token);
check("one queued message can be removed", removed.res.status === 200 && removed.json.queue.length === 2, removed.json);
const cleared = await api("DELETE", `/chats/${chat}/queue`, token);
check("the whole queue can be cleared", cleared.res.status === 200 && cleared.json.queue.length === 0, cleared.json);
await api("POST", `/chats/${chat}/stop`, token);
void hold;

console.log("files in the queue");
const idle = await until("idle", async () => ((await messages()).messages.some((m) => m.status === "pending") ? null : true), 20);
check("the chat is idle again", !!idle);
await api("POST", `/chats/${chat}/messages`, token, { text: "slow: 3 warm up" });
const withFile = await add("look at this", { files: [{ name: "notes.txt", data: Buffer.from("queued file body").toString("base64") }] });
check("a queued message can carry a file", withFile.res.status === 201 && withFile.json.queue[0].files?.[0]?.name === "notes.txt", withFile.json);
const sawFile = await until("the file to arrive", async () => {
  const d = await messages();
  const m = d.messages.find((x) => x.role === "user" && x.body === "look at this");
  return m && m.files?.length === 1 && d.queue.length === 0 ? d : null;
}, 40);
check("the file went with the message when its turn came", !!sawFile, sawFile?.messages?.slice(-3));

console.log("limits and ownership");
const other = await api("POST", "/auth/register", undefined, { name: "Other", email: `other-${Date.now()}@test.ir`, password: "password-123" });
const peek = await api("GET", `/chats/${chat}/queue`, other.json.token);
check("another account cannot read the queue", peek.res.status === 404, peek.json);
const sneak = await api("POST", `/chats/${chat}/queue`, other.json.token, { text: "sneak" });
check("another account cannot add to it", sneak.res.status === 404, sneak.json);

console.log(`\n${passed} passed, ${failures.length} failed`);
if (failures.length > 0) {
  console.log(failures.join("\n"));
  process.exit(1);
}
