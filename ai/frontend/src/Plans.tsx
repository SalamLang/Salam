import { useCallback, useEffect, useState } from "react";
import { api, ApiError, type Gateway, type Payment, type Plan, type PlanId, type Subscription } from "./api";
import { fmtDate, fmtNum } from "./format";
import { IconBadgeCheck, IconCheck, IconSpark } from "./icons";
import { isFa, t } from "./i18n";

const GATEWAY_NAMES: Record<Gateway, string> = {
  zarinpal: t("زرین‌پال", "Zarinpal"),
  zibal: t("زیبال", "Zibal"),
};

const PAYMENT_WORDS: Record<Payment["status"], string> = {
  pending: t("در انتظار پرداخت", "Awaiting payment"),
  verifying: t("در حال تایید", "Verifying"),
  paid: t("پرداخت شده", "Paid"),
  failed: t("ناموفق", "Failed"),
  canceled: t("لغو شده", "Canceled"),
};

const RESULTS: Record<string, { tone: "ok" | "warn"; text: string }> = {
  ok: { tone: "ok", text: t("پرداخت انجام شد و اشتراک شما فعال است.", "Payment done. Your subscription is active.") },
  canceled: { tone: "warn", text: t("پرداخت لغو شد و چیزی از حساب شما کم نشده است.", "Payment canceled. Nothing was charged to your account.") },
  failed: { tone: "warn", text: t("پرداخت تایید نشد. اگر مبلغی کم شده تا ۷۲ ساعت برمی‌گردد.", "Payment was not verified. If you were charged, the amount is refunded within 72 hours.") },
  pending: { tone: "warn", text: t("پرداخت شما در حال بررسی است. چند لحظه بعد این صفحه را تازه کنید.", "Your payment is being checked. Refresh this page in a moment.") },
};

const REGISTRY: Record<string, { fa: string; en: string; noteFa: string; noteEn: string }> = {};

export function registerPlans(list: Plan[]) {
  for (const p of list) REGISTRY[p.id] = { fa: p.name, en: p.name_en || p.name, noteFa: p.note, noteEn: p.note_en || p.note };
}

export function planName(id: PlanId, fallback: string): string {
  const known = REGISTRY[id];
  if (known) return isFa ? known.fa : known.en;
  switch (id) {
    case "free":
      return t("رایگان", "Free");
    case "plus":
      return t("کارمندی", "Employee");
    case "pro":
      return t("حرفه‌ای", "Pro");
    case "unlimited":
      return t("نامحدود", "Unlimited");
    default:
      return fallback;
  }
}

function planNote(id: PlanId, fallback: string): string {
  const known = REGISTRY[id];
  if (known && (known.noteFa || known.noteEn)) return isFa ? known.noteFa : known.noteEn;
  switch (id) {
    case "free":
      return t("برای آشنایی با دستیار سلام و کارهای کوچک", "To get to know the Salam assistant and for small tasks");
    case "plus":
      return t("یک روز کاری سبک، تا ۷ ساعت، یا ۳ تا ۴ ساعت کار سنگین", "A light workday of up to 7 hours, or 3 to 4 hours of heavy work");
    case "pro":
      return t("هر روز دست‌کم ۸ ساعت کار کامل، با ایجنت‌هایی مثل Cline", "At least 8 full hours of work every day, with agents like Cline");
    case "unlimited":
      return t("بدون سقف روزانه برای تیم‌ها و کار شبانه‌روزی", "No daily caps, for teams and round-the-clock work");
    default:
      return fallback;
  }
}

function errText(e: unknown, fallback: string) {
  return e instanceof ApiError ? e.message : fallback;
}

function toman(n: number): string {
  if (n >= 1000000 && n % 1000000 === 0) return t(`${fmtNum(n / 1000000)} میلیون تومان`, `${fmtNum(n / 1000000)} million toman`);
  return t(`${fmtNum(n)} تومان`, `${fmtNum(n)} toman`);
}

function limit(n: number, unit: string): string {
  return n === 0 ? t("نامحدود", "Unlimited") : `${fmtNum(n)} ${unit}`;
}

function features(p: Plan): string[] {
  return [
    p.price === 0 ? t("فقط کد زبان سلام می‌نویسد", "Writes Salam code only") : t("هر زبان برنامه‌نویسی و هر کاری", "Any programming language and any task"),
    t(`گفت‌وگو: ${limit(p.chat_daily, "پیام در روز")}`, `Chat: ${limit(p.chat_daily, "messages per day")}`),
    ...(p.chat_weekly > 0 ? [t(`گفت‌وگو در هفته: ${fmtNum(p.chat_weekly)} پیام`, `Chat per week: ${fmtNum(p.chat_weekly)} messages`)] : []),
    ...(p.chat_5h > 0 ? [t(`گفت‌وگو در ۵ ساعت: ${fmtNum(p.chat_5h)} پیام`, `Chat per 5 hours: ${fmtNum(p.chat_5h)} messages`)] : []),
    t(`کلید API: ${limit(p.api_daily, "درخواست در روز")}`, `API key: ${limit(p.api_daily, "requests per day")}`),
    ...(p.api_weekly > 0 ? [t(`کلید API در هفته: ${fmtNum(p.api_weekly)} درخواست`, `API key per week: ${fmtNum(p.api_weekly)} requests`)] : []),
    ...(p.api_5h > 0 ? [t(`کلید API در ۵ ساعت: ${fmtNum(p.api_5h)} درخواست`, `API key per 5 hours: ${fmtNum(p.api_5h)} requests`)] : []),
    t(`درخواست هم‌زمان: ${limit(p.parallel, "درخواست")}`, `Parallel requests: ${limit(p.parallel, "at a time")}`),
    p.days === 0 ? t("بدون انقضا", "No expiry") : t(`${fmtNum(p.days)} روزه`, `${fmtNum(p.days)} days`),
  ];
}

export function Plans({ notify }: { notify: (msg: string, ok?: boolean) => void }) {
  const [plans, setPlans] = useState<Plan[] | null>(null);
  const [gateways, setGateways] = useState<Gateway[]>([]);
  const [current, setCurrent] = useState<Subscription | null>(null);
  const [payments, setPayments] = useState<Payment[]>([]);
  const [gateway, setGateway] = useState<Gateway>("zibal");
  const [busy, setBusy] = useState<PlanId | null>(null);
  const [error, setError] = useState("");
  const [result, setResult] = useState<{ tone: "ok" | "warn"; text: string } | null>(null);

  const load = useCallback(async () => {
    setError("");
    try {
      const [list, mine, paid] = await Promise.all([api.plans(), api.subscription(), api.payments()]);
      registerPlans(list.plans);
      setPlans(list.plans);
      setGateways(list.gateways);
      if (list.gateways.length > 0 && !list.gateways.includes(gateway)) setGateway(list.gateways[0]);
      setCurrent(mine.subscription);
      setPayments(paid.payments);
    } catch (e) {
      setError(errText(e, t("بارگذاری پلن‌ها ناموفق بود.", "Could not load plans.")));
    }
  }, [gateway]);

  useEffect(() => {
    const done = new URLSearchParams(location.search).get("payment");
    if (done && RESULTS[done]) {
      setResult(RESULTS[done]);
      history.replaceState(null, "", "/plans");
    }
    void load();
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, []);

  useEffect(() => {
    const back = (e: PageTransitionEvent) => {
      if (!e.persisted) return;
      setBusy(null);
      void load();
    };
    window.addEventListener("pageshow", back);
    return () => window.removeEventListener("pageshow", back);
  }, [load]);

  const buy = async (plan: Plan) => {
    if (gateways.length === 0) return;
    setBusy(plan.id);
    try {
      const r = await api.checkout(plan.id, gateway);
      location.href = r.url;
    } catch (e) {
      notify(errText(e, t("شروع پرداخت ناموفق بود.", "Could not start the payment.")));
      setBusy(null);
    }
  };

  const active = current?.plan;

  return (
    <div className="page plans-page">
      <header className="page-head">
        <h2>{t("اشتراک‌ها", "Subscriptions")}</h2>
        <p className="muted">{t("با یک اشتراک، سقف روزانه‌ی گفت‌وگو و کلید API شما بالا می‌رود و هر روز ساعت ۰۰:۰۰ به وقت تهران دوباره شارژ می‌شود. پرداخت از راه درگاه‌های داخلی انجام می‌شود.", "A subscription raises your daily limits for chat and API keys, and they reset every day at midnight Tehran time. Payment goes through Iranian payment gateways and is charged in toman.")}</p>
      </header>

      {result && (
        <div className={`plan-result ${result.tone}`} role="status">
          {result.tone === "ok" ? <IconBadgeCheck width={16} height={16} /> : null} {result.text}
        </div>
      )}

      {error && (
        <div className="form-error" role="alert">
          {error}{" "}
          <button type="button" className="link" onClick={() => void load()}>
            {t("تلاش دوباره", "Try again")}
          </button>
        </div>
      )}

      {current && (
        <section className="card panel current-plan">
          <div className="panel-head">
            <h3>{t("اشتراک فعلی شما", "Your current subscription")}</h3>
          </div>
          <div className="plan-now">
            <strong className="plan-now-name">
              <IconSpark width={18} height={18} /> {planName(current.plan.id, current.plan.name)}
            </strong>
            <span className="muted">
              {current.expires === 0 ? t("بدون تاریخ پایان", "No end date") : t(`تا ${fmtDate(current.expires)}`, `Until ${fmtDate(current.expires)}`)}
              {current.source === "admin" ? t(" (هدیه‌ی مدیر)", " (gift from an admin)") : ""}
            </span>
            <span className="muted">
              {t("امروز:", "Today:")} {fmtNum(current.chat_used)} {t("پیام", "messages")}
              {current.plan.chat_daily > 0 ? t(` از ${fmtNum(current.plan.chat_daily)}`, ` of ${fmtNum(current.plan.chat_daily)}`) : ""} {t("و", "and")} {fmtNum(current.api_used)} {t("درخواست API", "API requests")}
              {current.plan.api_daily > 0 ? t(` از ${fmtNum(current.plan.api_daily)}`, ` of ${fmtNum(current.plan.api_daily)}`) : ""}
            </span>
            {(current.plan.chat_weekly > 0 || current.plan.api_weekly > 0) && (
              <span className="muted">
                {t("این هفته:", "This week:")} {fmtNum(current.chat_used_week)} {t("پیام", "messages")}
                {current.plan.chat_weekly > 0 ? t(` از ${fmtNum(current.plan.chat_weekly)}`, ` of ${fmtNum(current.plan.chat_weekly)}`) : ""} {t("و", "and")} {fmtNum(current.api_used_week)} {t("درخواست API", "API requests")}
                {current.plan.api_weekly > 0 ? t(` از ${fmtNum(current.plan.api_weekly)}`, ` of ${fmtNum(current.plan.api_weekly)}`) : ""}
              </span>
            )}
          </div>
        </section>
      )}

      {gateways.length > 1 && (
        <div className="tabs seg-tabs" role="group" aria-label={t("درگاه پرداخت", "Payment gateway")}>
          {gateways.map((g) => (
            <button key={g} type="button" className={g === gateway ? "active" : ""} aria-pressed={g === gateway} onClick={() => setGateway(g)}>
              {GATEWAY_NAMES[g]}
            </button>
          ))}
        </div>
      )}

      {plans === null && !error ? (
        <div className="list-loading" role="status">
          <span className="spinner" />
          <span className="sr-only">{t("در حال بارگذاری", "Loading")}</span>
        </div>
      ) : (
        <div className="plan-grid">
          {plans?.map((p) => {
            const mine = active?.id === p.id;
            const featured = p.price > 0 && p.chat_daily === 0 && p.api_daily === 0 && plans.find((q) => q.price > 0 && q.chat_daily === 0 && q.api_daily === 0)?.id === p.id;
            const price = p.price === 0 ? t("رایگان", "Free") : isFa ? (p.price % 1000000 === 0 ? fmtNum(p.price / 1000000) : fmtNum(p.price)) : `$${fmtNum(p.usd)}`;
            const unit = p.price === 0 ? "" : isFa ? (p.price % 1000000 === 0 ? "میلیون تومان" : "تومان") : t("", "per month");
            const period = p.price === 0 ? t("برای همیشه", "forever") : p.days === 0 ? t("یک‌بار پرداخت، بدون انقضا", "one payment, no expiry") : t(`برای ${fmtNum(p.days)} روز (حدود ${fmtNum(p.usd)} دلار)`, `${fmtNum(p.days)} days, charged in toman at checkout`);
            return (
              <section key={p.id} className={`plan-card ${featured ? "featured" : ""} ${mine ? "mine" : ""}`}>
                {featured && <span className="plan-ribbon">{t("بیشترین امکانات", "Most capable")}</span>}
                <header className="plan-head">
                  <span className="plan-icon" aria-hidden="true">
                    <IconSpark width={20} height={20} />
                  </span>
                  <h3>{planName(p.id, p.name)}</h3>
                </header>
                <div className="plan-priceblock">
                  <span className="plan-price">{price}</span>
                  {unit && <span className="plan-unit">{unit}</span>}
                </div>
                <p className="plan-period">{period}</p>
                {p.note && <p className="plan-note">{planNote(p.id, p.note)}</p>}
                <ul className="plan-features">
                  {features(p).map((f) => (
                    <li key={f}>
                      <span className="plan-tick" aria-hidden="true">
                        <IconCheck width={13} height={13} />
                      </span>
                      {f}
                    </li>
                  ))}
                </ul>
                {mine ? (
                  <span className="plan-state ok">
                    <IconBadgeCheck width={16} height={16} /> {t("اشتراک فعلی شما", "Your current plan")}
                  </span>
                ) : p.id === "free" ? (
                  <span className="plan-state">{t("برای همه فعال است", "Active for everyone")}</span>
                ) : (
                  <button type="button" className={`button wide plan-buy ${featured ? "primary" : "ghost brand"}`} disabled={busy !== null || gateways.length === 0} onClick={() => void buy(p)}>
                    {busy === p.id ? <span className="spinner" /> : null}
                    {busy === p.id ? t("در حال انتقال...", "Redirecting...") : gateways.length === 0 ? t("درگاه پرداخت فعال نیست", "No payment gateway available") : t(`خرید با ${GATEWAY_NAMES[gateway]}`, `Buy with ${GATEWAY_NAMES[gateway]}`)}
                  </button>
                )}
              </section>
            );
          })}
        </div>
      )}

      <section className="card panel">
        <div className="panel-head">
          <h3>{t("محدودیت‌ها، صادقانه", "Limits, honestly")}</h3>
        </div>
        <ul className="plan-limits">
          <li>{t("پلن رایگان فقط برای زبان سلام است: دستیار در آن فقط کد سلام می‌نویسد. پلن‌های پولی محدودیت زبان ندارند و هر زبان برنامه‌نویسی و هر کار دیگری را انجام می‌دهند.", "The free plan is for the Salam language only: the assistant writes only Salam code. Paid plans have no language limit and handle any programming language and any other task.")}</li>
          <li>{t("پلن‌های نامحدود سقف پیام و درخواست در روز و هفته ندارند. تعداد درخواست هم‌زمان هر پلن را روی کارت آن می‌بینید.", "Unlimited plans have no per-day or per-week message or request cap. Each plan's number of parallel requests is on its card.")}</li>
          <li>{t("هر نوبت در پلن‌های نامحدود تا ۳۰ دقیقه می‌تواند ادامه پیدا کند. پلن‌های دیگر: گفت‌وگو ۱۰ دقیقه و ایجنت رومیزی ۱ ساعت برای هر نوبت.", "On Unlimited plans a single turn can run up to 30 minutes. Other plans: 10 minutes per chat turn and 1 hour per desktop agent turn.")}</li>
          <li>{t("اگر نوبتی به سقف زمانی برسد، متن تا آن لحظه می‌ماند و با نوشتن «ادامه بده» از همان‌جا ادامه پیدا می‌کند.", "If a turn reaches its time limit, the text so far is kept and you can write \"continue\" to pick up from there.")}</li>
          <li>{t("پیام‌هایی که مهمان‌ها از راه یک پیوند همکاری می‌فرستند هم از پیام‌ها و سهمیه‌ی پلن صاحب گفت‌وگو کم می‌شود. با رسیدن به سقف روزانه، پیوند خودکار متوقف می‌شود.", "Messages guests send through a collaborate link also count against the chat owner's plan and limits. When the daily limit is reached the link pauses itself.")}</li>
          <li>{t("هر ابزار روی رایانه شما تا ۱۰ دقیقه برای پاسخ مهلت دارد و سرور برای دادن حافظه گاهی دوباره راه‌اندازی می‌شود. بستن برنامه یا قطع اینترنت رایانه شما کار را متوقف می‌کند، اما صفحه پس از وصل شدن دوباره به نوبت در حال اجرا برمی‌گردد.", "Each tool on your computer gets up to 10 minutes to answer, and the server sometimes restarts to free memory. Closing the app or losing your computer's internet pauses the work, but the page reattaches to the running turn when it reconnects.")}</li>
        </ul>
      </section>

      {payments.length > 0 && (
        <section className="card panel">
          <div className="panel-head">
            <h3>{t("پرداخت‌های شما", "Your payments")}</h3>
          </div>
          <div className="table-wrap">
            <table className="data-table">
              <thead>
                <tr>
                  <th scope="col">{t("تاریخ", "Date")}</th>
                  <th scope="col">{t("پلن", "Plan")}</th>
                  <th scope="col">{t("مبلغ", "Amount")}</th>
                  <th scope="col">{t("درگاه", "Gateway")}</th>
                  <th scope="col">{t("وضعیت", "Status")}</th>
                  <th scope="col">{t("کد پیگیری", "Reference")}</th>
                </tr>
              </thead>
              <tbody>
                {payments.map((p) => (
                  <tr key={p.id}>
                    <td>{fmtDate(p.created)}</td>
                    <td>{planName(p.plan, p.plan_name)}</td>
                    <td>{toman(p.amount)}</td>
                    <td>{GATEWAY_NAMES[p.gateway] ?? p.gateway}</td>
                    <td>
                      <span className={`badge ${p.status === "paid" ? "ok" : p.status === "pending" || p.status === "verifying" ? "pending" : "danger"}`}>{PAYMENT_WORDS[p.status]}</span>
                    </td>
                    <td dir="ltr">{p.ref || "-"}</td>
                  </tr>
                ))}
              </tbody>
            </table>
          </div>
        </section>
      )}
    </div>
  );
}
