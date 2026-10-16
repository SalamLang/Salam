const API = (process.env.API ?? "http://127.0.0.1:18090").replace(/\/$/, "");
const V1 = (process.env.V1 ?? "http://127.0.0.1:18091/v1").replace(/\/$/, "");
const ADMIN = process.env.ADMIN_EMAIL ?? "admin@test.ir";
const DASHES = new RegExp(`[${String.fromCodePoint(0x2013, 0x2014)}]`);

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

async function chat(key, body, headers = {}) {
  return fetch(`${V1}/chat/completions`, { method: "POST", headers: { "Content-Type": "application/json", Authorization: `Bearer ${key}`, ...headers }, body: JSON.stringify(body) });
}

console.log("keys");
const session = await account(ADMIN, "Admin");
const created = await api("POST", "/keys", session, { name: "test-bot" });
check("creating a key returns the secret once", created.res.status === 201 && created.json.secret.startsWith("sk-salam-") && created.json.key.prefix === created.json.secret.slice(0, 15), created.json);
const key = created.json.secret;
const listed = await api("GET", "/keys", session);
check("the key is listed without its secret", listed.json.keys.some((k) => k.name === "test-bot") && !JSON.stringify(listed.json).includes(key));
const empty = await api("POST", "/keys", session, { name: "" });
check("an empty key name is refused", empty.res.status === 400);

console.log("models and auth");
const models = await (await fetch(`${V1}/models`)).json();
check("/v1/models lists salam", models.object === "list" && models.data[0].id === "salam", models);
const noKey = await chat("sk-salam-wrong", { model: "salam", messages: [{ role: "user", content: "hi" }] });
const noKeyBody = await noKey.json();
check("a wrong key gets 401 invalid_api_key", noKey.status === 401 && noKeyBody.error.code === "invalid_api_key", noKeyBody);
const bad = await chat(key, { model: "salam" });
check("missing messages is a 400", bad.status === 400);

console.log("completions");
const plain = await chat(key, { model: "gpt-4o", messages: [{ role: "system", content: "Be brief." }, { role: "user", content: "What is Salam?" }], user: "end-user-7" }, { "X-Salam-Project": "website-bot" });
const pj = await plain.json();
check("a completion has the OpenAI shape", plain.status === 200 && pj.object === "chat.completion" && pj.choices[0].message.role === "assistant" && pj.choices[0].finish_reason === "stop" && typeof pj.usage.total_tokens === "number", pj);
check("the answer contains no em or en dash", !DASHES.test(pj.choices[0].message.content), pj.choices[0].message.content);
check("system instructions reach the engine", pj.choices[0].message.content.includes("Instructions from the calling"), pj.choices[0].message.content);

const st = await chat(key, { model: "salam", stream: true, stream_options: { include_usage: true }, messages: [{ role: "user", content: [{ type: "text", text: "Stream please" }] }] });
const raw = await st.text();
const events = raw.split("\n\n").filter((b) => b.startsWith("data: ")).map((b) => b.slice(6));
const chunks = events.filter((e) => e !== "[DONE]").map((e) => JSON.parse(e));
const streamed = chunks.map((c) => c.choices?.[0]?.delta?.content ?? "").join("");
check("streaming uses text/event-stream", st.headers.get("content-type").startsWith("text/event-stream"));
check("the first chunk carries the assistant role", chunks[0].choices[0].delta.role === "assistant");
check("the stream ends with stop, usage and [DONE]", chunks.some((c) => c.choices?.[0]?.finish_reason === "stop") && chunks.some((c) => c.usage && c.choices.length === 0) && events.at(-1) === "[DONE]", events.slice(-3));
check("streamed text is the whole answer without dashes", streamed.startsWith("Echo: Stream please") && streamed.includes("done") && !DASHES.test(streamed), streamed);

console.log("logging");
const mine = await api("GET", "/keys/requests", session);
const last = mine.json.requests[0];
const first = mine.json.requests.find((r) => r.project === "website-bot");
check("requests are logged with status, prompt and response", last.status === "done" && last.stream === true && last.prompt.includes("Stream please") && last.response.includes("done"), last);
check("project header and end user are recorded", first?.end_user === "end-user-7" && first?.key?.name === "test-bot", first);
const adm = await api("GET", "/admin/requests?project=website-bot", session);
check("admin sees requests with the user", adm.json.requests[0]?.user?.email === ADMIN, adm.json.requests[0]);
const sum = await api("GET", "/admin/summary", session);
check("admin summary counts API requests and projects", sum.json.api_requests >= 2 && sum.json.top_projects.some((p) => p.name === "website-bot") && typeof sum.json.daily.at(-1).api === "number", sum.json);
const other = await account("someone-else@test.ir", "Other");
const theirs = await api("GET", "/keys/requests", other);
check("another user does not see these requests", theirs.json.requests.length === 0);

console.log("limits and revocation");
let limited = 0;
for (let i = 0; i < 6; i += 1) {
  const r = await chat(key, { model: "salam", messages: [{ role: "user", content: `n${i}` }] });
  if (r.status === 429) limited += 1;
  await r.text();
}
check("the hourly limit answers 429", limited > 0, limited);
const k = listed.json.keys.find((x) => x.name === "test-bot");
await api("DELETE", `/keys/${k.id}`, session);
const dead = await chat(key, { model: "salam", messages: [{ role: "user", content: "hi" }] });
check("a revoked key stops working", dead.status === 401);

console.log(`\n${passed} passed, ${failures.length} failed`);
if (failures.length) process.exit(1);
