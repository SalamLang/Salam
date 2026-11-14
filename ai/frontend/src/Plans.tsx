import { useCallback, useEffect, useRef, useState } from "react";
import { api, ApiError, type Gateway, type Payment, type Plan, type PlanId, type PlanTier, type Subscription } from "./api";
import { fmtDate, fmtNum } from "./format";
import { IconBadgeCheck, IconCheck, IconChevronDown, IconSpark } from "./icons";
import { getCampaignFields } from "./campaign";
import { agent, isDesktop } from "./desktop";
import { isFa, t } from "./i18n";
import { Select } from "./Select";

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
    case "trial":
      return t("آزمایشی نامحدود", "Unlimited trial");
    default:
      return fallback;
  }
}

function planNote(id: PlanId, fallback: string): string {
  const known = REGISTRY[id];
  if (known && (known.noteFa || known.noteEn)) return isFa ? known.noteFa : known.noteEn;
  switch (id) {
    case "free":
      return t("برای شروع و آشنایی با سلام", "A simple start with Salam");
    case "plus":
      return t("برای کار و استفاده‌ی منظم هر روز", "For steady, everyday work");
    case "pro":
      return t("برای کار جدی و حرفه‌ای روزانه", "For serious daily, professional work");
    case "unlimited":
      return t("برای تیم‌ها و کار بی‌وقفه", "For teams and nonstop work");
    default:
      return fallback;
  }
}

function errText(e: unknown, fallback: string) {
  return e instanceof ApiError ? e.message : fallback;
}

function toman(n: number): string {
  if (n >= 1000000 && n % 1000000 === 0) return t(`${fmtNum(n / 1000000)} میلیون تومان`, `${fmtNum(n / 1000000)}M toman`);
  return t(`${fmtNum(n)} تومان`, `${fmtNum(n)} toman`);
}

function tierOf(p: Plan, parallel: number): PlanTier | undefined {
  return p.tiers?.find((x) => x.parallel === parallel) ?? p.tiers?.[0];
}

function effectivePlan(p: Plan, tier?: PlanTier): Plan {
  if (!tier) return p;
  return { ...p, ...tier };
}

function features(p: Plan): string[] {
  switch (p.id) {
    case "free":
      return [
        t("شروع سریع و رایگان", "Quick, free start"),
        t("مناسب برای آشنایی و کارهای سبک", "Great for trying things out"),
        t("دستیار ویژه‌ی کدنویسی سلام", "Focused on Salam coding"),
      ];
    case "trial":
      return [
        t("چند ساعت تجربه‌ی کامل امکانات", "A few hours of the full experience"),
        t("بدون محدودیت زبان برنامه‌نویسی", "Use any programming language"),
        t("راهی ساده برای امتحان قبل از انتخاب", "An easy way to try before choosing"),
      ];
    case "plus":
      return [
        t("برای کارهای روزمره و پروژه‌های شخصی", "For everyday tasks and personal projects"),
        t("پاسخ‌گویی روان‌تر برای استفاده‌ی منظم", "Smoother for regular use"),
        t("بدون محدودیت زبان و موضوع", "No language or topic limits"),
      ];
    case "pro":
      return [
        t("برای کار حرفه‌ای و پروژه‌های جدی", "For professional, serious projects"),
        t("ظرفیت بیشتر برای استفاده‌ی سنگین", "More room for heavy use"),
        t("انتخاب مطمئن برای کار روزانه", "A solid choice for daily work"),
      ];
    case "unlimited":
      return [
        t("آزادی عمل برای کارهای بزرگ و مداوم", "Freedom for big, ongoing work"),
        t("قابل تنظیم برای یک نفر تا تیم", "Scales from solo to team"),
        t("بهترین گزینه برای استفاده‌ی بی‌وقفه", "Best for nonstop use"),
      ];
    default:
      return [t("دسترسی به امکانات دستیار سلام", "Access to Salam assistant features")];
  }
}

export function Plans({ notify }: { notify: (msg: string, ok?: boolean) => void }) {
  const [plans, setPlans] = useState<Plan[] | null>(null);
  const [gateways, setGateways] = useState<Gateway[]>([]);
  const [current, setCurrent] = useState<Subscription | null>(null);
  const [payments, setPayments] = useState<Payment[]>([]);
  const [gateway, setGateway] = useState<Gateway>("zibal");
  const [busy, setBusy] = useState<string | null>(null);
  const [error, setError] = useState("");
  const [result, setResult] = useState<{ tone: "ok" | "warn"; text: string } | null>(null);
  const [code, setCode] = useState("");
  const [codeOpen, setCodeOpen] = useState(false);
  const [codeStatus, setCodeStatus] = useState<"idle" | "checking" | "valid" | "invalid">("idle");
  const [codeError, setCodeError] = useState("");
  const [discounts, setDiscounts] = useState<Record<string, { charge: number; discount: number } | "invalid" | undefined>>({});
  const [selectedTier, setSelectedTier] = useState<Record<string, number>>({});
  const [openDetails, setOpenDetails] = useState<Record<string, boolean>>({});
  const codeReq = useRef(0);

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

  const active = current?.plan;

  useEffect(() => {
    const trimmed = code.trim();
    if (!trimmed || !plans) {
      setCodeStatus("idle");
      setCodeError("");
      setDiscounts({});
      return;
    }
    const buyable = plans.filter((p) => p.price > 0 && active?.id !== p.id);
    if (buyable.length === 0) {
      setCodeStatus("idle");
      setCodeError("");
      setDiscounts({});
      return;
    }
    const req = ++codeReq.current;
    setCodeStatus("checking");
    setCodeError("");
    const timer = window.setTimeout(() => {
      void Promise.all(
        buyable.map((p) => {
          const par = p.tiers && p.tiers.length > 0 ? (selectedTier[p.id] ?? p.tiers[0].parallel) : undefined;
          const key = par !== undefined ? `${p.id}:${par}` : p.id;
          return api
            .previewDiscount(p.id, trimmed, par)
            .then((r) => ({ key, ok: true as const, charge: r.charge, discount: r.discount }))
            .catch((e) => ({ key, ok: false as const, message: errText(e, t("کد تخفیف نامعتبر است.", "Invalid discount code.")) }));
        }),
      ).then((results) => {
        if (codeReq.current !== req) return;
        const next: Record<string, { charge: number; discount: number } | "invalid" | undefined> = {};
        let firstError = "";
        let anyValid = false;
        for (const r of results) {
          if (r.ok) {
            next[r.key] = { charge: r.charge, discount: r.discount };
            anyValid = true;
          } else {
            next[r.key] = "invalid";
            if (!firstError) firstError = r.message;
          }
        }
        setDiscounts(next);
        setCodeStatus(anyValid ? "valid" : "invalid");
        setCodeError(anyValid ? "" : firstError);
      });
    }, 500);
    return () => window.clearTimeout(timer);
  }, [code, plans, active, selectedTier]);

  const buy = async (plan: Plan, parallel?: number) => {
    if (gateways.length === 0) return;
    setBusy(parallel !== undefined ? `${plan.id}:${parallel}` : plan.id);
    try {
      const trimmed = code.trim();
      const r = await api.checkout(plan.id, gateway, trimmed || undefined, getCampaignFields(), parallel);
      if (isDesktop) {
        await agent.payUrl(r.url);
        setBusy(null);
      } else {
        location.href = r.url;
      }
    } catch (e) {
      notify(errText(e, t("شروع پرداخت ناموفق بود.", "Could not start the payment.")));
      setBusy(null);
    }
  };

  return (
    <div className="page plans-page">
      <header className="page-head plans-hero">
        <h2>{t("پلنی که با کار شما هماهنگ است", "A plan that fits your work")}</h2>
        <p className="muted">{t("ساده انتخاب کنید و با خیال راحت شروع کنید. جزئیات فنی در پس‌زمینه مدیریت می‌شود.", "Pick simply and start with confidence. The technical details are handled for you.")}</p>
        <div className="plans-campaign" aria-label={t("تخفیف ویژه", "Special offer")}>
          <span className="plans-campaign-badge">{t("٪۳۵ تخفیف ویژه", "35% special offer")}</span>
          <span className="muted">{t("برای مدت محدود روی همه‌ی پلن‌ها", "Limited-time on all plans")}</span>
        </div>
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
        <section className="card panel current-plan compact-current">
          <div className="panel-head">
            <h3>{t("اشتراک فعلی شما", "Your current subscription")}</h3>
          </div>
          <div className="plan-now">
            <strong className="plan-now-name">
              <IconSpark width={18} height={18} /> {planName(current.plan.id, current.plan.name)}
            </strong>
            <span className="muted">
              {current.expires === 0 ? t("بدون تاریخ پایان", "No end date") : t(`تا ${fmtDate(current.expires)}`, `Until ${fmtDate(current.expires)}`)}
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

      <div className="discount-code">
        {codeOpen ? (
          <div className="discount-code-row">
            <label htmlFor="discount-code-input">{t("کد تخفیف", "Discount code")}</label>
            <input
              id="discount-code-input"
              type="text"
              dir="ltr"
              value={code}
              placeholder={t("کد خود را وارد کنید", "Enter your code")}
              onChange={(e) => setCode(e.target.value)}
            />
            {codeStatus === "checking" && <span className="spinner" />}
            {codeStatus === "valid" && (
              <span className="discount-code-ok">
                <IconBadgeCheck width={14} height={14} /> {t("اعمال شد", "Applied")}
              </span>
            )}
          </div>
        ) : (
          <button type="button" className="link" onClick={() => setCodeOpen(true)}>
            {t("کد تخفیف دارید؟", "Have a discount code?")}
          </button>
        )}
        {codeStatus === "invalid" && codeError && (
          <p className="form-error" role="alert">
            {codeError}
          </p>
        )}
      </div>

      {plans === null && !error ? (
        <div className="list-loading" role="status">
          <span className="spinner" />
          <span className="sr-only">{t("در حال بارگذاری", "Loading")}</span>
        </div>
      ) : (
        <div className="plan-grid plan-grid-compact">
          {plans?.map((p) => {
            const tier = p.tiers && p.tiers.length > 0 ? tierOf(p, selectedTier[p.id] ?? p.tiers[0].parallel) : undefined;
            const ep = effectivePlan(p, tier);
            const mine = active?.id === p.id;
            const featured = p.id === "unlimited" || p.id === "pro";
            const base = tier ? tier.price : p.price;
            const campaignOff = base > 0 ? Math.floor(base * 35 / 100) : 0;
            const campaignSale = Math.max(0, base - campaignOff);
            const discountKey = tier ? `${p.id}:${tier.parallel}` : p.id;
            const discount = discounts[discountKey];
            const applied = discount && discount !== "invalid" ? discount : null;
            const finalCharge = applied ? applied.charge : campaignSale;
            const showUsd = !isFa && base > 0;
            const display = base === 0 ? t("رایگان", "Free") : showUsd ? `$${fmtNum(ep.usd ?? 0)}` : toman(finalCharge);
            const originalDisplay = base === 0 ? "" : showUsd ? `$${fmtNum(Math.round((ep.usd ?? 0) / 0.65))}` : toman(base);
            const period = p.price === 0 ? t("همیشه در دسترس", "Always available") : p.hours > 0 ? t("دسترسی کوتاه‌مدت", "Short-term access") : p.days === 0 ? t("بدون تاریخ انقضا", "No expiry") : t("دسترسی ماهانه", "Monthly access");
            const busyKey = tier ? `${p.id}:${tier.parallel}` : p.id;
            const detailsOpen = !!openDetails[p.id];
            return (
              <section key={p.id} className={`plan-card ${featured ? "featured" : ""} ${mine ? "mine" : ""}`}>
                {(featured || base > 0) && <span className="plan-ribbon">{base > 0 ? t("٪۳۵ تخفیف", "35% off") : t("پیشنهادی", "Popular")}</span>}
                <header className="plan-head">
                  <span className="plan-icon" aria-hidden="true">
                    <IconSpark width={20} height={20} />
                  </span>
                  <h3>{planName(p.id, p.name)}</h3>
                </header>
                <p className="plan-note">{planNote(p.id, p.note)}</p>

                {p.tiers && p.tiers.length > 0 && (
                  <div className="plan-tier-picker plan-tier-picker-prominent">
                    <label>{t("ظرفیت هم‌زمان شما", "Your simultaneous capacity")}</label>
                    <Select
                      value={String(tier?.parallel ?? p.tiers[0].parallel)}
                      ariaLabel={t("انتخاب ظرفیت هم‌زمان", "Choose simultaneous capacity")}
                      options={p.tiers.map((opt) => ({ value: String(opt.parallel), label: t(`${fmtNum(opt.parallel)} کار هم‌زمان`, `${fmtNum(opt.parallel)} at once`) }))}
                      onChange={(v) => setSelectedTier((s) => ({ ...s, [p.id]: Number(v) }))}
                    />
                    <span className="muted plan-tier-hint">{t("هر زمان خواستید می‌توانید بیشترش کنید.", "You can scale it up anytime.")}</span>
                  </div>
                )}

                <div className="plan-priceblock plan-priceblock-attractive">
                  {base > 0 ? (
                    <>
                      <span className="plan-discount-pill">{t("٪۳۵ تخفیف", "35% off")}</span>
                      <s className="plan-price-old">{originalDisplay}</s>
                      <span className="plan-price plan-price-sale">{display}</span>
                      {applied && applied.discount > 0 && <span className="plan-off">{t("کد تخفیف هم اعمال شد", "Extra code applied")}</span>}
                    </>
                  ) : (
                    <span className="plan-price">{display}</span>
                  )}
                </div>
                <p className="plan-period">{period}</p>

                <ul className="plan-features plan-features-compact">
                  {features(ep).map((f) => (
                    <li key={f}>
                      <span className="plan-tick" aria-hidden="true">
                        <IconCheck width={13} height={13} />
                      </span>
                      {f}
                    </li>
                  ))}
                </ul>

                <button type="button" className="plan-more" onClick={() => setOpenDetails((s) => ({ ...s, [p.id]: !s[p.id] }))} aria-expanded={detailsOpen}>
                  {t(detailsOpen ? "نمایش کمتر" : "جزئیات بیشتر", detailsOpen ? "Show less" : "More details")}
                  <IconChevronDown width={14} height={14} className={detailsOpen ? "rot" : ""} />
                </button>
                {detailsOpen && (
                  <div className="plan-details muted">
                    <p>{p.id === "free" ? t("برای آشنایی و استفاده‌ی سبک مناسب است.", "Good for getting started and light use.") : p.id === "trial" ? t("یک شروع سریع برای تجربه‌ی کامل.", "A quick start to experience it fully.") : t("برای استفاده‌ی راحت و منعطف در طول دوره‌ی اشتراک.", "Comfortable, flexible use during your subscription.")}</p>
                    <p>{t("سیاست استفاده‌ی منصفانه برای جلوگیری از سوءاستفاده اعمال می‌شود.", "Fair-use safeguards help keep things reliable for everyone.")}</p>
                  </div>
                )}

                {mine ? (
                  <span className="plan-state ok">
                    <IconBadgeCheck width={16} height={16} /> {t("اشتراک فعلی شما", "Your current plan")}
                  </span>
                ) : p.id === "free" ? (
                  <span className="plan-state">{t("برای همه فعال است", "Active for everyone")}</span>
                ) : (
                  <button type="button" className={`button wide plan-buy ${featured ? "primary" : "ghost brand"}`} disabled={busy !== null || gateways.length === 0} onClick={() => void buy(p, tier?.parallel)}>
                    {busy === busyKey ? <span className="spinner" /> : null}
                    {busy === busyKey ? t("در حال انتقال...", "Redirecting...") : gateways.length === 0 ? t("درگاه پرداخت فعال نیست", "No payment gateway available") : t("انتخاب و پرداخت", "Choose & pay")}
                  </button>
                )}
              </section>
            );
          })}
        </div>
      )}

      <section className="card panel plans-note">
        <div className="panel-head">
          <h3>{t("نکته‌ای کوتاه", "A quick note")}</h3>
        </div>
        <p className="muted">{t("همه‌ی پلن‌ها با هدف استفاده‌ی راحت طراحی شده‌اند و برای جلوگیری از سوءاستفاده، محدودیت‌های منصفانه‌ای در پس‌زمینه دارند. اگر به ظرفیت بیشتری نیاز دارید، پلن بالاتر یا گزینه‌های نامحدود بهترین انتخاب‌اند.", "Every plan is designed for comfortable use, with fair safeguards behind the scenes to prevent abuse. If you need more room, a higher plan or the unlimited options are the best fit.")}</p>
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
