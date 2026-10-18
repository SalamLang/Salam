import { useCallback, useEffect, useState } from "react";
import { api, ApiError, type Gateway, type Payment, type Plan, type PlanId, type Subscription } from "./api";
import { fmtDate, fmtNum } from "./format";
import { IconBadgeCheck, IconCheck, IconSpark } from "./icons";

const GATEWAY_NAMES: Record<Gateway, string> = {
  zarinpal: "زرین‌پال",
  zibal: "زیبال",
};

const PAYMENT_WORDS: Record<Payment["status"], string> = {
  pending: "در انتظار پرداخت",
  verifying: "در حال تایید",
  paid: "پرداخت شده",
  failed: "ناموفق",
  canceled: "لغو شده",
};

const RESULTS: Record<string, { tone: "ok" | "warn"; text: string }> = {
  ok: { tone: "ok", text: "پرداخت انجام شد و اشتراک شما فعال است." },
  canceled: { tone: "warn", text: "پرداخت لغو شد و چیزی از حساب شما کم نشده است." },
  failed: { tone: "warn", text: "پرداخت تایید نشد. اگر مبلغی کم شده تا ۷۲ ساعت برمی‌گردد." },
  pending: { tone: "warn", text: "پرداخت شما در حال بررسی است. چند لحظه بعد این صفحه را تازه کنید." },
};

function errText(e: unknown, fallback: string) {
  return e instanceof ApiError ? e.message : fallback;
}

function toman(n: number): string {
  if (n >= 1000000 && n % 1000000 === 0) return `${fmtNum(n / 1000000)} میلیون تومان`;
  return `${fmtNum(n)} تومان`;
}

function limit(n: number, unit: string): string {
  return n === 0 ? "نامحدود" : `${fmtNum(n)} ${unit}`;
}

function features(p: Plan): string[] {
  return [
    `گفت‌وگو: ${limit(p.chat_hourly, "پیام در ساعت")}`,
    `کلید API: ${limit(p.api_hourly, "درخواست در ساعت")}`,
    `درخواست هم‌زمان: ${limit(p.parallel, "درخواست")}`,
    p.days === 0 ? "بدون انقضا" : `${fmtNum(p.days)} روزه`,
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
      setError(errText(e, "بارگذاری پلن‌ها ناموفق بود."));
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
      notify(errText(e, "شروع پرداخت ناموفق بود."));
      setBusy(null);
    }
  };

  const active = current?.plan;

  return (
    <div className="page plans-page">
      <header className="page-head">
        <h2>اشتراک‌ها</h2>
        <p className="muted">با یک اشتراک، سقف ساعتی گفت‌وگو و کلید API شما بالا می‌رود. پرداخت از راه درگاه‌های داخلی انجام می‌شود.</p>
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
            تلاش دوباره
          </button>
        </div>
      )}

      {current && (
        <section className="card panel current-plan">
          <div className="panel-head">
            <h3>اشتراک فعلی شما</h3>
          </div>
          <div className="plan-now">
            <strong className="plan-now-name">
              <IconSpark width={18} height={18} /> {current.plan.name}
            </strong>
            <span className="muted">
              {current.expires === 0 ? "بدون تاریخ پایان" : `تا ${fmtDate(current.expires)}`}
              {current.source === "admin" ? " (هدیه‌ی مدیر)" : ""}
            </span>
            <span className="muted">
              در این ساعت: {fmtNum(current.chat_used)} پیام
              {current.plan.chat_hourly > 0 ? ` از ${fmtNum(current.plan.chat_hourly)}` : ""} و {fmtNum(current.api_used)} درخواست API
              {current.plan.api_hourly > 0 ? ` از ${fmtNum(current.plan.api_hourly)}` : ""}
            </span>
          </div>
        </section>
      )}

      {gateways.length > 1 && (
        <div className="tabs seg-tabs" role="group" aria-label="درگاه پرداخت">
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
          <span className="sr-only">در حال بارگذاری</span>
        </div>
      ) : (
        <div className="plan-grid">
          {plans?.map((p) => {
            const mine = active?.id === p.id;
            return (
              <section key={p.id} className={`card plan-card ${p.id === "unlimited" ? "featured" : ""} ${mine ? "mine" : ""}`}>
                <h3>{p.name}</h3>
                <p className="plan-price">{p.price === 0 ? "رایگان" : toman(p.price)}</p>
                <p className="muted plan-note">{p.note}</p>
                <ul className="plan-features">
                  {features(p).map((f) => (
                    <li key={f}>
                      <IconCheck width={15} height={15} /> {f}
                    </li>
                  ))}
                </ul>
                {mine ? (
                  <span className="badge ok plan-badge">اشتراک فعلی شما</span>
                ) : p.id === "free" ? (
                  <span className="badge plan-badge">برای همه فعال است</span>
                ) : (
                  <button type="button" className="primary" disabled={busy !== null || gateways.length === 0} onClick={() => void buy(p)}>
                    {busy === p.id ? "در حال رفتن به درگاه..." : gateways.length === 0 ? "درگاه پرداخت فعال نیست" : `خرید با ${GATEWAY_NAMES[gateway]}`}
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
            <h3>پرداخت‌های شما</h3>
          </div>
          <div className="table-wrap">
            <table className="data-table">
              <thead>
                <tr>
                  <th scope="col">تاریخ</th>
                  <th scope="col">پلن</th>
                  <th scope="col">مبلغ</th>
                  <th scope="col">درگاه</th>
                  <th scope="col">وضعیت</th>
                  <th scope="col">کد پیگیری</th>
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
