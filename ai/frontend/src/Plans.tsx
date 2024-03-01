import { useCallback, useEffect, useState } from "react";
import { api, ApiError, type Gateway, type Payment, type Plan, type PlanId, type Subscription } from "./api";
import { fmtDate, fmtNum } from "./format";
import { IconBadgeCheck, IconCheck, IconSpark } from "./icons";
import { t } from "./i18n";

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
    t(`گفت‌وگو: ${limit(p.chat_hourly, "پیام در ساعت")}`, `Chat: ${limit(p.chat_hourly, "messages per hour")}`),
    t(`کلید API: ${limit(p.api_hourly, "درخواست در ساعت")}`, `API key: ${limit(p.api_hourly, "requests per hour")}`),
    t(`درخواست هم‌زمان: ${limit(p.parallel, "درخواست")}`, `Parallel requests: ${limit(p.parallel, "at a time")}`),
    p.days === 0 ? t("بدون انقضا", "No expiry") : t(`${fmtNum(p.days)} روزه`, `${fmtNum(p.days)} days`),
  ];
}

export function Plans({ notify }: { notify: (msg: string) => void }) {
  const [plans, setPlans] = useState<Plan[] | null>(null);
  const [gateways, setGateways] = useState<Gateway[]>([]);
  const [current, setCurrent] = useState<Subscription | null>(null);
  const [payments, setPayments] = useState<Payment[]>([]);
  const [gateway, setGateway] = useState<Gateway>("zarinpal");
  const [busy, setBusy] = useState<PlanId | null>(null);
  const [error, setError] = useState("");
  const [result, setResult] = useState<{ tone: "ok" | "warn"; text: string } | null>(null);

  const load = useCallback(async () => {
    setError("");
    try {
      const [list, mine, paid] = await Promise.all([api.plans(), api.subscription(), api.payments()]);
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
        <p className="muted">{t("با یک اشتراک، سقف ساعتی گفت‌وگو و کلید API شما بالا می‌رود. پرداخت از راه درگاه‌های داخلی انجام می‌شود.", "A subscription raises your hourly limits for chat and API keys. Payment goes through Iranian payment gateways.")}</p>
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
              <IconSpark width={18} height={18} /> {current.plan.name}
            </strong>
            <span className="muted">
              {current.expires === 0 ? t("بدون تاریخ پایان", "No end date") : t(`تا ${fmtDate(current.expires)}`, `Until ${fmtDate(current.expires)}`)}
              {current.source === "admin" ? t(" (هدیه‌ی مدیر)", " (gift from an admin)") : ""}
            </span>
            <span className="muted">
              {t("در این ساعت:", "This hour:")} {fmtNum(current.chat_used)} {t("پیام", "messages")}
              {current.plan.chat_hourly > 0 ? t(` از ${fmtNum(current.plan.chat_hourly)}`, ` of ${fmtNum(current.plan.chat_hourly)}`) : ""} {t("و", "and")} {fmtNum(current.api_used)} {t("درخواست API", "API requests")}
              {current.plan.api_hourly > 0 ? t(` از ${fmtNum(current.plan.api_hourly)}`, ` of ${fmtNum(current.plan.api_hourly)}`) : ""}
            </span>
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
            const featured = p.id === "unlimited";
            const price = p.price === 0 ? t("رایگان", "Free") : p.price >= 1000000 && p.price % 1000000 === 0 ? `${fmtNum(p.price / 1000000)}` : fmtNum(p.price);
            const unit = p.price === 0 ? "" : p.price >= 1000000 && p.price % 1000000 === 0 ? t("میلیون تومان", "million toman") : t("تومان", "toman");
            const period = p.price === 0 ? t("برای همیشه", "forever") : p.days === 0 ? t("یک‌بار پرداخت، بدون انقضا", "one payment, no expiry") : t(`برای ${fmtNum(p.days)} روز`, `for ${fmtNum(p.days)} days`);
            return (
              <section key={p.id} className={`plan-card ${featured ? "featured" : ""} ${mine ? "mine" : ""}`}>
                {featured && <span className="plan-ribbon">{t("بیشترین امکانات", "Most capable")}</span>}
                <header className="plan-head">
                  <span className="plan-icon" aria-hidden="true">
                    <IconSpark width={20} height={20} />
                  </span>
                  <h3>{p.name}</h3>
                </header>
                <div className="plan-priceblock">
                  <span className="plan-price">{price}</span>
                  {unit && <span className="plan-unit">{unit}</span>}
                </div>
                <p className="plan-period">{period}</p>
                {p.note && <p className="plan-note">{p.note}</p>}
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
                    {busy === p.id ? t("در حال رفتن به درگاه...", "Going to the gateway...") : gateways.length === 0 ? t("درگاه پرداخت فعال نیست", "No payment gateway available") : t(`خرید با ${GATEWAY_NAMES[gateway]}`, `Buy with ${GATEWAY_NAMES[gateway]}`)}
                  </button>
                )}
              </section>
            );
          })}
        </div>
      )}

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
                    <td>{p.plan_name}</td>
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
