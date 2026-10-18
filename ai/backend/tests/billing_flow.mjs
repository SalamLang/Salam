const API = (process.env.API ?? "http://127.0.0.1:18090").replace(/\/$/, "");
const V1 = (process.env.V1 ?? "http://127.0.0.1:18091").replace(/\/$/, "");
const ADMIN = process.env.ADMIN_EMAIL ?? "admin@test.ir";
const HOURLY = Number(process.env.HOURLY ?? 12);
const DAY = 86400;
const RUN = Date.now().toString(36);
const BUYER = `buyer-${RUN}@test.ir`;
const HEAVY = `heavy-${RUN}@test.ir`;

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

const tokenOf = (url) => url.slice(url.lastIndexOf("/") + 1);
const back = (gateway, query) => fetch(`${API}/billing/callback/${gateway}?${query}`, { redirect: "manual" });

console.log("plans");
const plans = await api("GET", "/plans");
const ids = (plans.json?.plans ?? []).map((p) => p.id);
check("the four plans are listed", plans.res.status === 200 && ids.join(",") === "free,plus,pro,unlimited", ids);
const priced = Object.fromEntries((plans.json?.plans ?? []).map((p) => [p.id, p.price]));
check("the prices are 0, 1, 2 and 10 million toman", priced.free === 0 && priced.plus === 1000000 && priced.pro === 2000000 && priced.unlimited === 10000000, priced);
const unlimited = plans.json.plans.find((p) => p.id === "unlimited");
check("the unlimited plan has no limit and no expiry", unlimited.api_hourly === 0 && unlimited.chat_hourly === 0 && unlimited.parallel === 0 && unlimited.days === 0, unlimited);
check("both gateways are on", (plans.json.gateways ?? []).includes("zarinpal") && plans.json.gateways.includes("zibal"), plans.json.gateways);

console.log("a new account is free");
const admin = await account(ADMIN, "Admin");
const buyer = await account(BUYER, "Buyer");
const first = await api("GET", "/billing/subscription", buyer);
check("a new account is on the free plan", first.json?.subscription?.plan?.id === "free" && first.json.subscription.expires === 0, first.json);
check("the free plan reports its hourly room and what is used", typeof first.json.subscription.api_used === "number" && first.json.subscription.plan.api_hourly > 0, first.json.subscription);

console.log("checkout refuses what it should");
check("an unknown plan is refused", (await api("POST", "/billing/checkout", buyer, { plan: "gold", gateway: "zarinpal" })).res.status === 400);
check("the free plan cannot be bought", (await api("POST", "/billing/checkout", buyer, { plan: "free", gateway: "zarinpal" })).res.status === 400);
check("an unknown gateway is refused", (await api("POST", "/billing/checkout", buyer, { plan: "plus", gateway: "paypal" })).res.status === 400);
check("checkout needs a signed in account", (await api("POST", "/billing/checkout", undefined, { plan: "plus", gateway: "zarinpal" })).res.status === 401);

console.log("paying with zarinpal");
const start = await api("POST", "/billing/checkout", buyer, { plan: "plus", gateway: "zarinpal" });
check("checkout returns the gateway page and the amount", start.res.status === 200 && start.json.url.includes("/pg/StartPay/") && start.json.amount === 1000000, start.json);
const authority = tokenOf(start.json.url);
const pendingList = await api("GET", "/billing/payments", buyer);
check("the payment is written down as pending", pendingList.json.payments[0]?.status === "pending" && pendingList.json.payments[0].gateway === "zarinpal" && pendingList.json.payments[0].amount === 1000000, pendingList.json.payments[0]);
const done = await back("zarinpal", `Authority=${authority}&Status=OK`);
check("the gateway return sends the browser back to the app", done.status === 302 && (done.headers.get("location") ?? "").includes("payment=ok"), done.headers.get("location"));
const afterPay = await api("GET", "/billing/subscription", buyer);
const now = Math.floor(Date.now() / 1000);
check("the plan is active right away", afterPay.json.subscription.plan.id === "plus" && afterPay.json.subscription.source === "payment", afterPay.json.subscription);
check("it runs for thirty days", Math.abs(afterPay.json.subscription.expires - (now + 30 * DAY)) < 120, afterPay.json.subscription.expires - now);
const paidList = await api("GET", "/billing/payments", buyer);
check("the payment is marked paid with its reference and card", paidList.json.payments[0].status === "paid" && paidList.json.payments[0].ref === "987654321" && paidList.json.payments[0].card.endsWith("5995"), paidList.json.payments[0]);
const again = await back("zarinpal", `Authority=${authority}&Status=OK`);
const twice = await api("GET", "/billing/subscription", buyer);
check("coming back twice does not buy twice", again.status === 302 && (again.headers.get("location") ?? "").includes("payment=ok") && twice.json.subscription.expires === afterPay.json.subscription.expires, twice.json.subscription);

console.log("a payment the user walks away from");
const walk = await api("POST", "/billing/checkout", buyer, { plan: "pro", gateway: "zarinpal" });
const walkBack = await back("zarinpal", `Authority=${tokenOf(walk.json.url)}&Status=NOK`);
const stillPlus = await api("GET", "/billing/subscription", buyer);
check("a canceled payment leaves the plan alone", walkBack.status === 302 && (walkBack.headers.get("location") ?? "").includes("payment=canceled") && stillPlus.json.subscription.plan.id === "plus", walkBack.headers.get("location"));
const canceled = (await api("GET", "/billing/payments", buyer)).json.payments.find((p) => p.plan === "pro");
check("the canceled payment is written down as canceled", canceled?.status === "canceled", canceled);
const unknownBack = await back("zarinpal", "Authority=A0000000000000000000000000000000000&Status=OK");
check("a gateway return for an unknown payment fails safely", unknownBack.status === 302 && (unknownBack.headers.get("location") ?? "").includes("payment=failed"), unknownBack.headers.get("location"));

console.log("paying with zibal");
const zi = await api("POST", "/billing/checkout", buyer, { plan: "pro", gateway: "zibal" });
check("zibal checkout returns its start page", zi.res.status === 200 && zi.json.url.includes("/start/"), zi.json);
const ziBack = await back("zibal", `trackId=${tokenOf(zi.json.url)}&success=1&status=2`);
const afterZi = await api("GET", "/billing/subscription", buyer);
check("the zibal payment moves the account to the bigger plan", ziBack.status === 302 && afterZi.json.subscription.plan.id === "pro", afterZi.json.subscription);

console.log("a payment whose callback never arrives");
const SWEEP = Number(process.env.SWEEP_SECONDS ?? 0);
if (SWEEP > 0) {
  const lost = await api("POST", "/billing/checkout", buyer, { plan: "plus", gateway: "zarinpal" });
  check("a checkout that is never returned from starts out pending", lost.res.status === 200 && (await api("GET", "/billing/payments", buyer)).json.payments[0].status === "pending", lost.json);
  let settled = null;
  for (let i = 0; i < Math.ceil((SWEEP * 3 + 10) / 2); i += 1) {
    await new Promise((r) => setTimeout(r, 2000));
    settled = (await api("GET", "/billing/payments", buyer)).json.payments.find((p) => p.id === lost.json.payment_id);
    if (settled && settled.status !== "pending") break;
  }
  check("the sweep settles it with the gateway without the user coming back", settled?.status === "paid" && settled.ref === "987654321", settled);
  const recovered = await api("GET", "/billing/subscription", buyer);
  check("the plan the user paid for is activated by the sweep", recovered.json.subscription.plan.id === "plus", recovered.json.subscription);
}

console.log("the admin side");
const forbidden = await api("GET", "/admin/subscriptions", buyer);
check("a normal account cannot see the subscriptions", forbidden.res.status === 403);
const grant = await api("POST", "/admin/subscriptions", admin, { email: BUYER, plan: "unlimited", note: "تست مدیر" });
check("an admin can give a paid plan away", grant.res.status === 201 && grant.json.plan === "unlimited", grant.json);
const granted = await api("GET", "/billing/subscription", buyer);
check("the gift is unlimited and never expires", granted.json.subscription.plan.id === "unlimited" && granted.json.subscription.expires === 0 && granted.json.subscription.source === "admin", granted.json.subscription);
check("an unlimited account is not asked to buy again", (await api("POST", "/billing/checkout", buyer, { plan: "pro", gateway: "zarinpal" })).res.status === 409);
const active = await api("GET", "/admin/subscriptions", admin);
const mine = active.json.subscriptions.find((s) => s.user?.email === BUYER);
check("active subscriptions carry the user and the plan", mine?.plan === "unlimited" && mine.status === "active" && mine.note === "تست مدیر", mine);
check("every account appears once in the active list", active.json.subscriptions.filter((s) => s.user?.email === BUYER).length === 1, active.json.subscriptions.length);
const latest = await api("GET", "/admin/subscriptions?latest=1", admin);
check("the last subscription of each account can be listed", latest.json.subscriptions.some((s) => s.user?.email === BUYER), latest.json.subscriptions.length);
const payments = await api("GET", "/admin/payments", admin);
check("payments are listed with the account that paid", payments.res.status === 200 && payments.json.payments.some((p) => p.user?.email === BUYER && p.status === "paid"), payments.json.payments?.[0]);
check("the income adds up what was really paid", payments.json.income >= 3000000, payments.json.income);
const onlyPaid = await api("GET", "/admin/payments?status=canceled", admin);
check("payments can be filtered by state", onlyPaid.json.payments.every((p) => p.status === "canceled") && onlyPaid.json.payments.length > 0, onlyPaid.json.payments.length);
const drop = await api("DELETE", `/admin/subscriptions/${mine.id}`, admin);
const dropped = await api("GET", "/billing/subscription", buyer);
check("an admin can take a subscription back", drop.res.status === 200 && dropped.json.subscription.plan.id === "free", dropped.json.subscription);

console.log("the plan decides the limits");
const heavy = await account(HEAVY, "Heavy");
const key = (await api("POST", "/keys", heavy, { name: "heavy" })).json.secret;
const ask = () => fetch(`${V1}/chat/completions`, { method: "POST", headers: { "Content-Type": "application/json", Authorization: `Bearer ${key}` }, body: JSON.stringify({ model: "salam", messages: [{ role: "user", content: "hi" }] }) });
let limited = 0;
for (let i = 0; i < HOURLY + 2; i += 1) {
  const r = await ask();
  if (r.status === 429) limited += 1;
  await r.text();
}
check("the free plan stops at its hourly limit", limited > 0, limited);
await api("POST", "/admin/subscriptions", admin, { email: HEAVY, plan: "unlimited" });
const afterGift = await ask();
const giftBody = await afterGift.text();
check("the unlimited plan lifts the limit at once", afterGift.status === 200, `${afterGift.status} ${giftBody.slice(0, 120)}`);

console.log(`\n${passed} passed, ${failures.length} failed`);
if (failures.length) process.exit(1);
