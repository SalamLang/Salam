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

const hop = (path) => fetch(API + path, { redirect: "manual" });
const where = (res) => res.headers.get("location") ?? "";

async function begin(provider, next = "") {
  const res = await hop(`/auth/provider/${provider}/start${next ? `?next=${encodeURIComponent(next)}` : ""}`);
  const loc = where(res);
  return { res, loc, state: /state=([0-9a-f]+)/.exec(loc)?.[1] ?? "" };
}

async function signIn(provider, code, next = "") {
  const { state } = await begin(provider, next);
  const back = await hop(`/auth/provider/${provider}/callback?code=${encodeURIComponent(code)}&state=${state}`);
  const loc = where(back);
  return { loc, token: /#token=([A-Za-z0-9_-]+)/.exec(loc)?.[1] ?? "", error: /login_error=([a-z_]+)/.exec(loc)?.[1] ?? "" };
}

async function main() {
  const tag = Date.now();
  const on = await api("GET", "/auth/providers");
  check("both providers are offered", on.json?.github === true && on.json?.figma === true, on.json);

  const g = await begin("github", "/telegram?code=abc123");
  check("github start redirects to GitHub with the sign in scopes", g.res.status === 302 && g.loc.startsWith(PROVIDERS) && /client_id=gh-client/.test(g.loc) && /scope=read%3Auser%20user%3Aemail|scope=read%3Auser\+user%3Aemail/.test(g.loc) && g.state.length === 48, g.loc);
  check("the redirect target is this API's callback", g.loc.includes(encodeURIComponent(`${API}/auth/provider/github/callback`)), g.loc);
  const f = await begin("figma");
  check("figma start redirects to Figma", f.res.status === 302 && /client_id=fg-client/.test(f.loc) && /scope=current_user%3Aread/.test(f.loc), f.loc);

  const stale = await hop(`/auth/provider/github/callback?code=x&state=${"0".repeat(48)}`);
  check("an unknown state is refused", /login_error=expired/.test(where(stale)), where(stale));
  const refused = await begin("github");
  const denied = await hop(`/auth/provider/github/callback?error=access_denied&state=${refused.state}`);
  check("a refusal at the provider is explained", /login_error=denied/.test(where(denied)), where(denied));

  const email = `octo${tag}@dev.test`;
  const first = await signIn("github", `gh|${tag}|octo${tag}|${email}|1`, "/telegram?code=abc123");
  check("a new GitHub user gets a session", first.token.length >= 32 && first.loc.includes("/auth/done?next=/telegram%3Fcode%3Dabc123#token="), first.loc);
  const me = await api("GET", "/me", first.token);
  check("the account is made from the GitHub profile", me.json?.user?.email === email && me.json.user.name === `octo${tag}`, me.json);
  const again = await signIn("github", `gh|${tag}|octo${tag}|${email}|1`);
  const me2 = await api("GET", "/me", again.token);
  check("signing in again reaches the same account", me2.json?.user?.id === me.json?.user?.id && !again.loc.includes("next="), again.loc);
  const guess = await api("POST", "/auth/login", undefined, { email, password: "password-123" });
  check("a provider account has no guessable password", guess.res.status === 401);

  const mine = `pw${tag}@dev.test`;
  const pw = await api("POST", "/auth/register", undefined, { name: "Password User", email: mine, password: "password-123" });
  const linked = await signIn("github", `gh|${tag + 1}|pwuser${tag}|${mine}|1`);
  const meLinked = await api("GET", "/me", linked.token);
  check("a verified GitHub email joins the existing account", linked.token !== "" && meLinked.json?.user?.id === (await api("GET", "/me", pw.json.token)).json?.user?.id, meLinked.json);
  const unverified = await signIn("github", `gh|${tag + 2}|sneaky|${mine}|0`);
  check("an unverified email never joins an account", unverified.error === "no_email" || unverified.error === "email_exists", unverified.loc);

  await api("POST", "/auth/register", undefined, { name: "Admin", email: ADMIN, password: "password-123" });
  const admin = await signIn("github", `gh|${tag + 3}|root|${ADMIN}|1`);
  check("an admin account is never taken over by email", admin.error === "email_exists" && admin.token === "", admin.loc);

  const clash = await signIn("figma", `fg|fig${tag}|designer${tag}|${mine}`);
  check("Figma does not join an account by email", clash.error === "email_exists", clash.loc);
  const fresh = await signIn("figma", `fg|fig${tag}|designer${tag}|figma${tag}@dev.test`);
  const meFig = await api("GET", "/me", fresh.token);
  check("a new Figma user gets an account", meFig.json?.user?.email === `figma${tag}@dev.test` && meFig.json.user.name === `designer${tag}`, meFig.json);
  const figAgain = await signIn("figma", `fg|fig${tag}|designer${tag}|figma${tag}@dev.test`);
  check("and signs in to it again", (await api("GET", "/me", figAgain.token)).json?.user?.id === meFig.json?.user?.id);

  const none = await signIn("github", `gh|${tag + 4}|ghost${tag}||1`);
  check("no verified email means no account", none.error === "no_email", none.loc);

  const evil = await signIn("github", `gh|${tag}|octo${tag}|${email}|1`, "//evil.example/x");
  check("an off-site next path is dropped", evil.token !== "" && !evil.loc.includes("evil"), evil.loc);

  const once = await begin("github");
  const code = encodeURIComponent(`gh|${tag}|octo${tag}|${email}|1`);
  await hop(`/auth/provider/github/callback?code=${code}&state=${once.state}`);
  const replay = await hop(`/auth/provider/github/callback?code=${code}&state=${once.state}`);
  check("a state works once", /login_error=expired/.test(where(replay)), where(replay));

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
