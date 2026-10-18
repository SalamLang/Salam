const API = (process.env.API ?? "http://127.0.0.1:18090").replace(/\/$/, "");
const V1 = (process.env.V1 ?? "http://127.0.0.1:18091").replace(/\/$/, "");
const ADMIN = process.env.ADMIN_EMAIL ?? "admin@test.ir";
const START = Number(process.env.START_SECONDS ?? 120);
const DASHES = new RegExp(`[${String.fromCodePoint(0x2013, 0x2014)}]`);

function sse(raw) {
  const events = raw
    .split("\n\n")
    .map((b) => b.trim())
    .filter((b) => b.startsWith("data: "))
    .map((b) => b.slice(6));
  const chunks = [];
  let broken = 0;
  for (const e of events) {
    if (e === "[DONE]") continue;
    try {
      chunks.push(JSON.parse(e));
    } catch {
      broken += 1;
    }
  }
  return {
    events,
    chunks,
    broken,
    ended: events.at(-1) === "[DONE]",
    finish: chunks.flatMap((c) => c.choices?.map((ch) => ch.finish_reason) ?? []).filter(Boolean),
    text: chunks.map((c) => c.choices?.[0]?.delta?.content ?? "").join(""),
    error: chunks.find((c) => c.error)?.error,
  };
}

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
check("/models lists salam only", models.object === "list" && models.data[0].id === "salam" && models.data.length === 1, models);
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
check("system instructions reach the engine as marked caller context", pj.choices[0].message.content.includes("<caller_instructions>"), pj.choices[0].message.content);

const st = await chat(key, { model: "salam", stream: true, stream_options: { include_usage: true }, messages: [{ role: "user", content: [{ type: "text", text: "Stream please" }] }] });
const raw = await st.text();
const events = raw.split("\n\n").filter((b) => b.startsWith("data: ")).map((b) => b.slice(6));
const chunks = events.filter((e) => e !== "[DONE]").map((e) => JSON.parse(e));
const streamed = chunks.map((c) => c.choices?.[0]?.delta?.content ?? "").join("");
check("streaming uses text/event-stream", st.headers.get("content-type").startsWith("text/event-stream"));
check("the first chunk carries the assistant role", chunks[0].choices[0].delta.role === "assistant");
check("the stream ends with stop, usage and [DONE]", chunks.some((c) => c.choices?.[0]?.finish_reason === "stop") && chunks.some((c) => c.usage && c.choices.length === 0) && events.at(-1) === "[DONE]", events.slice(-3));
check("streamed text is the whole answer without dashes", streamed.startsWith("Echo: Stream please") && streamed.includes("done") && !DASHES.test(streamed), streamed);

console.log("long answers");
const slowStream = await chat(key, { model: "salam", stream: true, messages: [{ role: "user", content: "slow: 12 stream" }] });
const slowRaw = await slowStream.text();
const slowText = slowRaw.split("\n\n").filter((b) => b.startsWith("data: {")).map((b) => JSON.parse(b.slice(6)).choices?.[0]?.delta?.content ?? "").join("");
check("a stream that is silent for long sends keepalives", slowRaw.includes(": ping") && slowText.startsWith("Echo: slow: 12 stream") && slowRaw.trimEnd().endsWith("data: [DONE]"), slowRaw.slice(-300));
const slowPlain = await chat(key, { model: "salam", messages: [{ role: "user", content: "slow: 12 plain" }] });
const slowBody = await slowPlain.text();
const slowJson = JSON.parse(slowBody);
check("a long non-streaming answer keeps the connection alive and stays valid JSON", slowPlain.status === 200 && slowBody.startsWith("\n") && slowJson.choices[0].message.content.startsWith("Echo: slow: 12 plain"), slowBody.slice(0, 200));

console.log("compiler check");
const badCode = await (await chat(key, { model: "salam", messages: [{ role: "user", content: "bad code please" }] })).json();
const fixedText = badCode.choices?.[0]?.message?.content ?? "";
check("code that does not compile is sent back and fixed", fixedText.startsWith("Fixed (salam, saw errors)") && fixedText.includes('println "hi"') && !fixedText.includes("{"), fixedText);
const snippet = await (await chat(key, { model: "salam", messages: [{ role: "user", content: "snippet code please" }] })).json();
check("a fragment with only unknown names is not sent back", (snippet.choices?.[0]?.message?.content ?? "").startsWith("Use:"), snippet);

console.log("engine never starts");
const t0 = Date.now();
const never = await chat(key, { model: "salam", messages: [{ role: "user", content: "never start" }] });
const neverBody = await never.text();
check("a turn the engine never starts fails soon after the start limit", Date.now() - t0 < (START + 25) * 1000 && neverBody.includes("engine_failed"), `${Date.now() - t0} ms ${neverBody.slice(-200)}`);

console.log("agent clients");
const agentSession = await account("agent@test.ir", "Agent");
const agentKey = (await api("POST", "/keys", agentSession, { name: "agent-bot" })).json.secret;
const cut = await chat(agentKey, { model: "salam", stream: true, messages: [{ role: "user", content: "fail midway please" }] });
const cutSse = sse(await cut.text());
check("an engine failure half way still closes the stream", cut.status === 200 && cutSse.broken === 0 && cutSse.ended && cutSse.finish.length === 1, cutSse.events.slice(-3));
check("the half answer arrives and is marked as cut off", cutSse.text.startsWith("Echo: fail midway") && cutSse.finish[0] === "length", `${cutSse.finish} ${cutSse.text.slice(0, 80)}`);
const deadStream = await chat(agentKey, { model: "salam", stream: true, messages: [{ role: "user", content: "never start" }] });
const deadSse = sse(await deadStream.text());
check("a turn the engine never starts closes the stream with an error and a finish reason", deadStream.status === 200 && deadSse.ended && deadSse.error?.code === "engine_failed" && deadSse.finish.length === 1, deadSse.events.slice(-3));
const tQuiet = Date.now();
const quiet = await chat(agentKey, { model: "salam", stream: true, messages: [{ role: "user", content: "goes quiet after starting" }] });
const quietSse = sse(await quiet.text());
check("an engine that goes quiet after starting does not hold the stream open", Date.now() - tQuiet < (START + 25) * 1000 && quietSse.ended && quietSse.finish[0] === "length" && quietSse.text.includes("half an answer"), `${Date.now() - tQuiet} ms ${quietSse.finish} ${quietSse.text.slice(0, 60)}`);
const noCheck = await chat(agentKey, { model: "salam", stream: true, messages: [{ role: "user", content: "bad code please" }] });
const noCheckSse = sse(await noCheck.text());
check("a streamed answer is never rewritten under the client, so no text is taken back", noCheckSse.ended && noCheckSse.finish[0] === "stop" && noCheckSse.text.startsWith("Here:") && !noCheckSse.text.includes("Fixed ("), noCheckSse.text.slice(0, 80));
const versioned = await fetch(`${V1}/v1/chat/completions`, { method: "POST", headers: { "Content-Type": "application/json", Authorization: `Bearer ${agentKey}` }, body: JSON.stringify({ model: "salam", messages: [{ role: "user", content: "with a version prefix" }] }) });
const versionedJson = await versioned.json();
check("a base URL that already ends in /v1 works", versioned.status === 200 && versionedJson.choices?.[0]?.message?.content?.startsWith("Echo:"), versionedJson);
const versionedModels = await fetch(`${V1}/v1/models`);
check("/v1/models works too", versionedModels.status === 200 && (await versionedModels.json()).data[0].id === "salam");

console.log("long conversations");
const turns = [];
for (let i = 0; i < 50; i += 1) turns.push({ role: i % 2 ? "assistant" : "user", content: `step ${i}: ${"x".repeat(3000)}` });
const long = await chat(agentKey, { model: "salam", messages: [{ role: "system", content: `agent rules ${"y".repeat(40000)}` }, ...turns, { role: "user", content: "what now?" }] });
const longJson = await long.json();
check("a conversation and a system prompt past 64 KB are answered", long.status === 200 && longJson.choices?.[0]?.message?.content?.startsWith("Echo:"), longJson.error ?? longJson.choices?.[0]?.message?.content?.slice(0, 80));
const longLog = (await api("GET", "/keys/requests", agentSession)).json.requests[0];
check("the whole system prompt reaches the engine", longLog.prompt_chars > 180000 && longLog.prompt.includes("y".repeat(20000)), longLog.prompt_chars);
const huge = [];
for (let i = 0; i < 40; i += 1) huge.push({ role: i % 2 ? "assistant" : "user", content: `old ${i}: ${"z".repeat(30000)}` });
const trimmed = await chat(agentKey, { model: "salam", messages: [...huge, { role: "user", content: "the newest question" }] });
check("a conversation past the limit drops its oldest turns instead of failing", trimmed.status === 200, await trimmed.text());
const trimmedLog = (await api("GET", "/keys/requests", agentSession)).json.requests[0];
check("the newest turns survive the trim and the client is told", trimmedLog.prompt.includes("Earlier messages were left out") && trimmedLog.prompt_chars <= 1048576, trimmedLog.prompt_chars);

console.log("usage");
const um = await chat(key, { model: "salam", messages: [{ role: "user", content: "usage please" }] });
const uj = await um.json();
check("usage reports the engine's real tokens", uj.usage.prompt_tokens === 410 && uj.usage.completion_tokens === 25 && uj.usage.total_tokens === 435, uj.usage);
check("usage details carry cached and reasoning tokens", uj.usage.prompt_tokens_details?.cached_tokens === 300 && uj.usage.completion_tokens_details?.reasoning_tokens === 5, uj.usage);

console.log("logging");
const mine = await api("GET", "/keys/requests", session);
const last = mine.json.requests.find((r) => r.prompt.includes("Stream please"));
const first = mine.json.requests.find((r) => r.project === "website-bot");
check("requests are logged with status, prompt and response", last.status === "done" && last.stream === true && last.prompt.includes("Stream please") && last.response.includes("done"), last);
check("project header and end user are recorded", first?.end_user === "end-user-7" && first?.key?.name === "test-bot", first);
const adm = await api("GET", "/admin/requests?project=website-bot", session);
check("admin sees requests with the user", adm.json.requests[0]?.user?.email === ADMIN, adm.json.requests[0]);
const sum = await api("GET", "/admin/summary", session);
check("admin summary counts API requests and projects", sum.json.api_requests >= 2 && sum.json.top_projects.some((p) => p.name === "website-bot") && typeof sum.json.daily.at(-1).api === "number", sum.json);
const stored = (await api("GET", "/keys/requests", session)).json.requests.find((r) => r.prompt.includes("usage please"));
check("each request stores input, output, cache, cost and tools", stored?.usage?.input === 100 && stored.usage.output === 20 && stored.usage.cache_read === 300 && stored.usage.cache_write === 10 && stored.usage.reasoning === 5 && stored.usage.tools === 1 && Math.abs(stored.usage.cost - 0.0012) < 1e-9, stored?.usage);
const chatRes = await api("POST", "/chats", session);
const sent = await api("POST", `/chats/${chatRes.json.chat.id}/messages`, session, { text: "chat usage" });
let polled;
for (let i = 0; i < 40; i += 1) {
  polled = await api("GET", `/chats/${chatRes.json.chat.id}/messages/${sent.json.assistant.id}`, session);
  if (polled.json.status !== "pending") break;
  await new Promise((r) => setTimeout(r, 300));
}
check("a chat answer completes", polled.json.status === "done", polled.json);
const usum = (await api("GET", "/admin/summary", session)).json;
const today = usum.usage?.find((p) => p.key === "today");
check("admin usage has today, week, month, last month and all", ["today", "week", "month", "last_month", "all"].every((k) => usum.usage?.some((p) => p.key === k)), usum.usage?.map((p) => p.key));
check("today's chat usage counts the chat answer's tokens", today?.chat?.count >= 1 && today.chat.input >= 100 && today.chat.cache_read >= 300, today?.chat);
check("today's API usage sums the API requests", today?.api?.input >= 100 && today.api.output >= 20, today?.api);
const lastMonth = usum.usage.find((p) => p.key === "last_month");
check("last month ends where this month starts", lastMonth.to === usum.usage.find((p) => p.key === "month").from && lastMonth.from < lastMonth.to);
check("daily rows carry token totals", typeof usum.daily.at(-1).tokens_in === "number" && usum.daily.at(-1).tokens_in > 0, usum.daily.at(-1));
const urow = (await api("GET", "/admin/users?q=admin", session)).json.users[0];
check("users show tokens, last login and login count", urow.tokens > 0 && urow.last_login > 0 && urow.logins >= 1, urow);
const other = await account("someone-else@test.ir", "Other");
const theirs = await api("GET", "/keys/requests", other);
check("another user does not see these requests", theirs.json.requests.length === 0);

console.log("limits and revocation");
let limited = 0;
const tries = Number(process.env.HOURLY ?? 8) + 1;
for (let i = 0; i < tries; i += 1) {
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
