import { spawnSync } from "node:child_process";

const API = (process.env.API ?? "http://127.0.0.1:18090").replace(/\/$/, "");
const PROVIDERS = (process.env.PROVIDERS ?? "http://127.0.0.1:18095").replace(/\/$/, "");
const ADMIN = process.env.ADMIN_EMAIL ?? "admin@test.ir";

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

const APP = (process.env.APP ?? "http://localhost:3000").replace(/\/$/, "");

async function begin(provider, next = "", poll) {
  const r = await api("POST", `/auth/provider/${provider}/start`, undefined, poll ? { next, poll } : { next });
  return { res: r.res, loc: r.json?.url ?? "", state: r.json?.state ?? "", error: r.json?.error ?? "" };
}

async function finish(provider, code, state, error = "", extra = {}) {
  const r = await api("POST", `/auth/provider/${provider}/complete`, undefined, { code, state, error, bound: true, ...extra });
  return { status: r.res.status, token: r.json?.token ?? "", next: r.json?.next ?? "", error: r.json?.error ?? "", json: r.json };
}

const hex = (n) => Array.from({ length: n }, () => Math.floor(Math.random() * 16).toString(16)).join("");

async function poll(secret) {
  const r = await api("POST", "/auth/provider-poll", undefined, { poll: secret });
  return r.json ?? {};
}

function sql(query) {
  const args = process.env.TEST_DB_ARGS;
  if (!args) return false;
  const run = spawnSync("mariadb", [...args.split(" "), "-e", query], { encoding: "utf8" });
  return run.status === 0;
}

async function signIn(provider, code, next = "") {
  const { state } = await begin(provider, next);
  return finish(provider, code, state);
}

async function main() {
  const tag = Date.now();
  const on = await api("GET", "/auth/providers");
  check("both providers are offered", on.json?.github === true && on.json?.figma === true, on.json);

  const g = await begin("github", "/telegram?code=abc123");
  check("github start returns the GitHub link with the sign in scopes", g.res.status === 200 && g.loc.startsWith(PROVIDERS) && /client_id=gh-client/.test(g.loc) && /scope=read%3Auser%20user%3Aemail|scope=read%3Auser\+user%3Aemail/.test(g.loc) && g.state.length === 48, g.loc);
  check("the return address is the app, not the API", g.loc.includes(encodeURIComponent(`${APP}/auth/callback/github`)), g.loc);
  const f = await begin("figma");
  check("figma start returns the Figma link", f.res.status === 200 && /client_id=fg-client/.test(f.loc) && /scope=current_user%3Aread/.test(f.loc) && f.loc.includes(encodeURIComponent(`${APP}/auth/callback/figma`)), f.loc);
  const unknown = await begin("gitlab");
  check("an unknown provider is refused", unknown.res.status === 400 && unknown.error === "unavailable");

  const stale = await finish("github", "x", "0".repeat(48));
  check("an unknown state says it was not found", stale.status === 400 && stale.error === "missing", stale.json);
  const refused = await begin("github");
  const denied = await finish("github", "", refused.state, "access_denied");
  check("a refusal at the provider is explained", denied.status === 400 && denied.error === "denied", denied.json);
  const reasoned = await begin("github");
  const why = await finish("github", "", reasoned.state, "access_denied", { error_description: "The user said no" });
  check("the provider's own explanation is passed on", why.error === "denied" && why.json?.detail === "The user said no", why.json);

  const email = `octo${tag}@dev.test`;
  const first = await signIn("github", `gh|${tag}|octo${tag}|${email}|1`, "/telegram?code=abc123");
  check("a new GitHub user gets a session", first.token.length >= 32 && first.next === "/telegram?code=abc123", first.json);
  const me = await api("GET", "/me", first.token);
  check("the account is made from the GitHub profile", me.json?.user?.email === email && me.json.user.name === `octo${tag}`, me.json);
  const again = await signIn("github", `gh|${tag}|octo${tag}|${email}|1`);
  const me2 = await api("GET", "/me", again.token);
  check("signing in again reaches the same account", me2.json?.user?.id === me.json?.user?.id && again.next === "", again.json);
  const guess = await api("POST", "/auth/login", undefined, { email, password: "password-123" });
  check("a provider account has no guessable password", guess.res.status === 401);

  const mine = `pw${tag}@dev.test`;
  const pw = await api("POST", "/auth/register", undefined, { name: "Password User", email: mine, password: "password-123" });
  const linked = await signIn("github", `gh|${tag + 1}|pwuser${tag}|${mine}|1`);
  const meLinked = await api("GET", "/me", linked.token);
  check("a verified GitHub email joins the existing account", linked.token !== "" && meLinked.json?.user?.id === (await api("GET", "/me", pw.json.token)).json?.user?.id, meLinked.json);
  const unverified = await signIn("github", `gh|${tag + 2}|sneaky|${mine}|0`);
  check("an unverified email never joins an account", unverified.error === "no_email" || unverified.error === "email_exists", unverified.json);

  await api("POST", "/auth/register", undefined, { name: "Admin", email: ADMIN, password: "password-123" });
  const admin = await signIn("github", `gh|${tag + 3}|root|${ADMIN}|1`);
  check("an admin account is never taken over by email", admin.error === "email_exists" && admin.token === "", admin.json);

  const clash = await signIn("figma", `fg|fig${tag}|designer${tag}|${mine}`);
  check("Figma does not join an account by email", clash.error === "email_exists", clash.json);
  const fresh = await signIn("figma", `fg|fig${tag}|designer${tag}|figma${tag}@dev.test`);
  const meFig = await api("GET", "/me", fresh.token);
  check("a new Figma user gets an account", meFig.json?.user?.email === `figma${tag}@dev.test` && meFig.json.user.name === `designer${tag}`, meFig.json);
  const figAgain = await signIn("figma", `fg|fig${tag}|designer${tag}|figma${tag}@dev.test`);
  check("and signs in to it again", (await api("GET", "/me", figAgain.token)).json?.user?.id === meFig.json?.user?.id);

  const none = await signIn("github", `gh|${tag + 4}|ghost${tag}||1`);
  check("no verified email means no account", none.error === "no_email", none.json);

  const evil = await signIn("github", `gh|${tag}|octo${tag}|${email}|1`, "//evil.example/x");
  check("an off-site next path is dropped", evil.token !== "" && evil.next === "", evil.json);

  const once = await begin("github");
  const code = `gh|${tag}|octo${tag}|${email}|1`;
  await finish("github", code, once.state);
  const replay = await finish("github", code, once.state);
  check("a state used twice says it was already used", replay.status === 400 && replay.error === "used", replay.json);

  const wrong = await begin("figma");
  const crossed = await finish("github", `gh|${tag}|octo${tag}|${email}|1`, wrong.state);
  check("a state made for one provider is useless for the other", crossed.error === "missing", crossed.json);

  const other = await begin("github");
  const unbound = await finish("github", code, other.state, "", { bound: false, iss: "https://github.com/login/oauth" });
  check("a web sign in finished in another browser is refused", unbound.error === "unbound" && unbound.token === "", unbound.json);
  const stillGood = await finish("github", code, other.state, "", { iss: "https://github.com/login/oauth" });
  check("and the state still works in the browser that began it", stillGood.token.length >= 32, stillGood.json);

  const secret = hex(48);
  const desk = await begin("github", "/chat/1", secret);
  const waiting = await poll(secret);
  check("the desktop app sees the sign in as pending", waiting.status === "pending" && !waiting.token, waiting);
  const handed = await finish("github", code, desk.state, "", { bound: false, iss: "https://github.com/login/oauth" });
  check("the system browser finishes without any session of its own", handed.status === 200 && handed.json?.purpose === "handoff" && handed.token === "", handed.json);
  const got = await poll(secret);
  check("the desktop app then receives the session", got.status === "done" && got.token.length >= 32 && got.next === "/chat/1", got);
  const gotMe = await api("GET", "/me", got.token);
  check("it is the account of the provider profile", gotMe.json?.user?.email === email, gotMe.json);
  const gone = await poll(secret);
  check("the session is handed over once", gone.status === "missing" && !gone.token, gone);
  const reload = await finish("github", code, desk.state, "", { bound: false });
  check("reloading the callback page says the state was used", reload.error === "used", reload.json);
  check("a wrong poll secret learns nothing", (await poll(hex(48))).status === "missing" && (await poll("short")).status === "missing");

  const secret2 = hex(48);
  const deskDenied = await begin("github", "", secret2);
  const refusedDesk = await finish("github", "", deskDenied.state, "access_denied", { bound: false, error_description: "No thanks" });
  check("a refusal in the system browser tells the browser page", refusedDesk.error === "denied" && refusedDesk.json?.detail === "No thanks" && refusedDesk.json?.handoff === true, refusedDesk.json);
  const refusedPoll = await poll(secret2);
  check("and the desktop app", refusedPoll.status === "failed" && refusedPoll.error === "denied", refusedPoll);

  const secret3 = hex(48);
  const deskNoEmail = await begin("github", "", secret3);
  const noEmail = await finish("github", `gh|${tag + 9}|ghost9${tag}||1`, deskNoEmail.state, "", { bound: false });
  const noEmailPoll = await poll(secret3);
  check("a provider account without an email fails in both places", noEmail.error === "no_email" && noEmailPoll.status === "failed" && noEmailPoll.error === "no_email", [noEmail.json, noEmailPoll]);

  const lapsing = await begin("github");
  if (sql(`UPDATE integration_states SET expires = 1 WHERE state = SHA2('${lapsing.state}', 256)`)) {
    const late = await finish("github", code, lapsing.state);
    check("a state past its time says it expired", late.error === "expired", late.json);
    const secret4 = hex(48);
    const deskLate = await begin("github", "", secret4);
    sql(`UPDATE integration_states SET expires = 1 WHERE poll = SHA2('${secret4}', 256)`);
    check("the desktop app is told it expired", (await poll(secret4)).status === "expired");
  } else {
    console.log("  skip expiry checks (set TEST_DB_ARGS)");
  }

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
