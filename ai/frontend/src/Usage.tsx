import { useEffect, useMemo, useState } from "react";
import { api, ApiError, type UsageBreakdown, type UsageDay, type UsagePeriod, type UsagePeriodKey, type UsageReport, type UsageTotals } from "./api";
import { fmtCompact, fmtDateTime, fmtDay, fmtMonth, fmtNum, tokenSplit } from "./format";
import { t } from "./i18n";
import { IconAlert } from "./icons";

type Mode = "all" | "chat" | "api";

const PERIODS: { id: UsagePeriodKey; label: string }[] = [
  { id: "today", label: t("امروز", "Today") },
  { id: "week", label: t("این هفته", "This week") },
  { id: "month", label: t("این ماه", "This month") },
  { id: "last_month", label: t("ماه گذشته", "Last month") },
  { id: "all", label: t("همه", "All time") },
];

const MODES: { id: Mode; label: string }[] = [
  { id: "all", label: t("همه", "All") },
  { id: "chat", label: t("گفت‌وگو", "Chat") },
  { id: "api", label: t("API", "API") },
];

function zero(): UsageTotals {
  return { count: 0, input: 0, output: 0, reasoning: 0, cache_read: 0, cache_write: 0, cost: 0, cost_in: 0, cost_out: 0, tools: 0 };
}

function add(a: UsageTotals, b: UsageTotals): UsageTotals {
  return {
    count: a.count + b.count,
    input: a.input + b.input,
    output: a.output + b.output,
    reasoning: a.reasoning + b.reasoning,
    cache_read: a.cache_read + b.cache_read,
    cache_write: a.cache_write + b.cache_write,
    cost: a.cost + b.cost,
    cost_in: a.cost_in + b.cost_in,
    cost_out: a.cost_out + b.cost_out,
    tools: (a.tools || 0) + (b.tools || 0),
  };
}

function pick(p: UsagePeriod, mode: Mode): UsageTotals {
  if (mode === "chat") return p.chat;
  if (mode === "api") return p.api;
  return add(p.chat, p.api);
}

function totalOf(tot: UsageTotals): { input: number; output: number; cache: number; total: number } {
  return tokenSplit(tot);
}

function Tile({ label, value, sub, display }: { label: string; value?: number; sub?: string; display?: string }) {
  return (
    <div className="card tile">
      <span className="tile-label">{label}</span>
      <strong className="tile-value" title={value !== undefined && Math.abs(value) >= 10000 ? fmtNum(value) : undefined}>
        {display ?? fmtCompact(value ?? 0)}
      </strong>
      {sub && <span className="tile-sub">{sub}</span>}
    </div>
  );
}

function Fact({ label, value, display }: { label: string; value?: number; display?: string }) {
  return (
    <div className="usage-fact">
      <span className="tile-label">{label}</span>
      <strong title={value !== undefined && Math.abs(value) >= 10000 ? fmtNum(value) : undefined}>{display ?? fmtNum(value ?? 0)}</strong>
    </div>
  );
}

function Segmented<T extends string>({ label, options, value, onChange }: { label: string; options: { id: T; label: string }[]; value: T; onChange: (v: T) => void }) {
  return (
    <div className="tabs seg-tabs" role="group" aria-label={label}>
      {options.map((o) => (
        <button key={o.id} type="button" className={o.id === value ? "active" : ""} aria-pressed={o.id === value} onClick={() => onChange(o.id)}>
          {o.label}
        </button>
      ))}
    </div>
  );
}

function Quota({ label, used, limit }: { label: string; used: number; limit: number }) {
  const unlimited = limit <= 0;
  const ratio = unlimited ? 0 : Math.min(1, used / limit);
  return (
    <div className="usage-quota">
      <div className="usage-quota-head">
        <span>{label}</span>
        <span className="muted">
          {unlimited ? t("بدون سقف", "No cap") : t(`${fmtNum(used)} از ${fmtNum(limit)}`, `${fmtNum(used)} of ${fmtNum(limit)}`)}
        </span>
      </div>
      {!unlimited && (
        <span className="meter" aria-hidden="true">
          <i style={{ width: `${Math.round(ratio * 100)}%` }} />
        </span>
      )}
    </div>
  );
}

function DailyChart({ days, mode }: { days: UsageDay[]; mode: Mode }) {
  const points = days.map((d) => {
    if (mode === "chat") return { in: d.chat_in, out: d.chat_out };
    if (mode === "api") return { in: d.api_in, out: d.api_out };
    return { in: d.chat_in + d.api_in, out: d.chat_out + d.api_out };
  });
  const max = Math.max(1, ...points.flatMap((p) => [p.in, p.out]));
  const W = 920;
  const H = 230;
  const PAD = 34;
  const BOTTOM = 26;
  const step = (W - PAD * 2) / Math.max(1, points.length);
  const bar = Math.max(2, Math.min(9, step * 0.28));
  const labelEvery = Math.max(1, Math.ceil(points.length / 7));
  const height = H - PAD - BOTTOM;
  return (
    <div className="usage-chart">
      <svg viewBox={`0 0 ${W} ${H}`} role="img" style={{ direction: "ltr" }} aria-label={t(`${fmtNum(days.length)} روز گذشته`, `The last ${fmtNum(days.length)} days`)}>
        {[0, 0.25, 0.5, 0.75, 1].map((r) => {
          const y = H - BOTTOM - height * r;
          return (
            <g key={r}>
              <line className="chart-grid" x1={PAD} x2={W - PAD} y1={y} y2={y} />
              <text className="chart-label" x={4} y={y + 4}>
                {fmtCompact(Math.round(max * r))}
              </text>
            </g>
          );
        })}
        {points.map((p, i) => {
          const x = PAD + i * step + step / 2;
          const ih = Math.max(0, Math.round((p.in / max) * height));
          const oh = Math.max(0, Math.round((p.out / max) * height));
          const d = days[i];
          const title = t(
            `${fmtDay(d.day)}: ${fmtNum(p.in)} ورودی، ${fmtNum(p.out)} خروجی`,
            `${fmtDay(d.day)}: ${fmtNum(p.in)} in, ${fmtNum(p.out)} out`,
          );
          return (
            <g key={d.day}>
              <rect x={x - bar - 1} y={H - BOTTOM - ih} width={bar} height={ih} rx={2} fill="var(--brand)">
                <title>{title}</title>
              </rect>
              <rect x={x + 1} y={H - BOTTOM - oh} width={bar} height={oh} rx={2} fill="var(--tk-f)">
                <title>{title}</title>
              </rect>
              {i % labelEvery === 0 && (
                <text className="chart-label" x={x} y={H - 6} textAnchor="middle">
                  {fmtDay(d.day)}
                </text>
              )}
            </g>
          );
        })}
        <line className="chart-axis" x1={PAD} x2={W - PAD} y1={H - BOTTOM} y2={H - BOTTOM} />
      </svg>
      <div className="usage-legend">
        <span>
          <i className="swatch in" /> {t("ورودی", "Input")}
        </span>
        <span>
          <i className="swatch out" /> {t("خروجی", "Output")}
        </span>
      </div>
    </div>
  );
}

function Breakdown({ title, rows, empty }: { title: string; rows: UsageBreakdown[]; empty: string }) {
  return (
    <section className="card panel usage-breakdown">
      <h3>{title}</h3>
      {rows.length === 0 ? (
        <p className="muted small">{empty}</p>
      ) : (
        <table className="data-table">
          <thead>
            <tr>
              <th scope="col">{t("نام", "Name")}</th>
              <th scope="col">{t("تعداد", "Count")}</th>
              <th scope="col">{t("ورودی", "Input")}</th>
              <th scope="col">{t("خروجی", "Output")}</th>
            </tr>
          </thead>
          <tbody>
            {rows.map((r) => (
              <tr key={`${r.kind || ""}-${r.name}`}>
                <td dir="auto">{r.name}</td>
                <td>{fmtNum(r.count)}</td>
                <td>{fmtCompact(r.input)}</td>
                <td>{fmtCompact(r.output)}</td>
              </tr>
            ))}
          </tbody>
        </table>
      )}
    </section>
  );
}

function sourceLabel(s: UsageBreakdown): string {
  if (s.kind === "api") return t("کلید API", "API keys");
  if (s.name === "agent") return t("دستیار دسکتاپ", "Desktop agent");
  if (s.name === "cloud") return t("محیط ابری", "Cloud sandbox");
  if (s.name === "telegram") return t("تلگرام", "Telegram");
  if (s.name === "web") return t("وب", "Web");
  return s.name;
}

export function Usage({ onNavigate }: { onNavigate: (path: string) => void }) {
  const [report, setReport] = useState<UsageReport | null>(null);
  const [error, setError] = useState("");
  const [period, setPeriod] = useState<UsagePeriodKey>("today");
  const [mode, setMode] = useState<Mode>("all");
  const [loading, setLoading] = useState(true);

  const load = () => {
    setLoading(true);
    setError("");
    api
      .usage()
      .then((r) => setReport(r))
      .catch((e) => setError(e instanceof ApiError ? e.message : t("بارگذاری مصرف ناموفق بود.", "Could not load usage.")))
      .finally(() => setLoading(false));
  };

  useEffect(() => {
    load();
  }, []);

  const active = useMemo(() => report?.periods.find((p) => p.key === period) ?? report?.periods[0], [report, period]);

  if (!report && loading) {
    return (
      <div className="page">
        <div className="loading-chat" role="status">
          <span className="spinner" />
          <span className="sr-only">{t("در حال بارگذاری", "Loading")}</span>
        </div>
      </div>
    );
  }

  if (!report) {
    return (
      <div className="page">
        <header className="page-head">
          <h2>{t("مصرف و توکن‌ها", "Usage and tokens")}</h2>
        </header>
        <div className="form-error" role="alert">
          <IconAlert width={16} height={16} /> {error || t("بارگذاری مصرف ناموفق بود.", "Could not load usage.")}{" "}
          <button type="button" className="link" onClick={load}>
            {t("تلاش دوباره", "Try again")}
          </button>
        </div>
      </div>
    );
  }

  const current = active ? pick(active, mode) : zero();
  const split = totalOf(current);
  const selectedLabel = PERIODS.find((p) => p.id === (active?.key ?? period))?.label ?? "";
  const upto = active ? Math.min(active.to, report.now) : report.now;
  const range = active && active.key !== "all" ? t(`${fmtDateTime(active.from)} تا ${fmtDateTime(upto)}`, `${fmtDateTime(active.from)} to ${fmtDateTime(upto)}`) : t("از آغاز تا امروز", "Since the beginning");

  const chatLimit = report.plan.chat_daily;
  const apiLimit = report.plan.api_daily;

  return (
    <div className="page usage-page">
      <header className="page-head">
        <h2>{t("مصرف و توکن‌ها", "Usage and tokens")}</h2>
        <p className="muted">
          {t("همه‌ی مصرف حساب شما در یک نگاه: امروز، این هفته، این ماه، هر روز و هر منبع. توکن‌های ورودی و خروجی، کش، استدلال و فراخوانی ابزار جدا آمده‌اند.", "Everything this account uses in one place: today, this week, this month, every day and every source. Input and output tokens, cache, reasoning and tool calls are shown separately.")}
        </p>
      </header>

      <section className="card panel usage-plan">
        <div className="panel-head">
          <h3>{t("پلن و سقف‌ها", "Plan and limits")}</h3>
          <button type="button" className="link" onClick={() => onNavigate("/plans")}>
            {t("مدیریت اشتراک", "Manage plan")}
          </button>
        </div>
        <div className="usage-plan-body">
          <div className="usage-plan-name">
            <span className="tile-label">{t("پلن فعلی", "Current plan")}</span>
            <strong>{report.plan.name}</strong>
            <span className="muted small">
              {report.plan.expires > 0 ? t(`تا ${fmtDateTime(report.plan.expires)}`, `Until ${fmtDateTime(report.plan.expires)}`) : t("پلن رایگان", "Free plan")}
            </span>
          </div>
          <Quota label={t("پیام گفت‌وگو امروز", "Chat messages today")} used={report.today.chat_used} limit={chatLimit} />
          <Quota label={t("درخواست API امروز", "API requests today")} used={report.today.api_used} limit={apiLimit} />
          {report.plan.chat_weekly > 0 && <Quota label={t("پیام گفت‌وگو این هفته", "Chat messages this week")} used={report.today.chat_used_week} limit={report.plan.chat_weekly} />}
          {report.plan.api_weekly > 0 && <Quota label={t("درخواست API این هفته", "API requests this week")} used={report.today.api_used_week} limit={report.plan.api_weekly} />}
          {report.plan.chat_5h > 0 && <Quota label={t("پیام گفت‌وگو در ۵ ساعت اخیر", "Chat messages, last 5 hours")} used={report.today.chat_used_5h} limit={report.plan.chat_5h} />}
          {report.plan.api_5h > 0 && <Quota label={t("درخواست API در ۵ ساعت اخیر", "API requests, last 5 hours")} used={report.today.api_used_5h} limit={report.plan.api_5h} />}
          <div className="usage-quota">
            <div className="usage-quota-head">
              <span>{t("درخواست هم‌زمان", "Parallel requests")}</span>
              <span className="muted">{report.plan.parallel > 0 ? fmtNum(report.plan.parallel) : t("بدون سقف", "No cap")}</span>
            </div>
          </div>
        </div>
      </section>

      <div className="tabs seg-tabs usage-periods" role="group" aria-label={t("دوره", "Period")}>
        {PERIODS.map((p) => (
          <button key={p.id} type="button" className={p.id === (active?.key ?? period) ? "active" : ""} aria-pressed={p.id === (active?.key ?? period)} onClick={() => setPeriod(p.id)}>
            {p.label}
          </button>
        ))}
      </div>

      <div className="usage-toolbar">
        <span className="muted small">
          {selectedLabel}: {range}
        </span>
        <Segmented label={t("منبع", "Source")} options={MODES} value={mode} onChange={setMode} />
      </div>

      <div className="tiles">
        <Tile label={t("جمع توکن‌ها", "Total tokens")} value={split.total} sub={t("ورودی و خروجی", "Input and output")} />
        <Tile label={t("توکن ورودی", "Input tokens")} value={split.input} sub={t(`شامل ${fmtCompact(split.cache)} توکن کش`, `${fmtCompact(split.cache)} cached`)} />
        <Tile label={t("توکن خروجی", "Output tokens")} value={split.output} sub={t("شامل توکن‌های استدلال", "Reasoning included")} />
        <Tile label={mode === "api" ? t("درخواست API", "API requests") : mode === "chat" ? t("پاسخ گفت‌وگو", "Chat replies") : t("پاسخ و درخواست", "Replies and requests")} value={current.count} />
        {split.cache > 0 && <Tile label={t("توکن کش", "Cached tokens")} value={split.cache} sub={t("خوانده و نوشته", "Read and written")} />}
        {(current.tools || 0) > 0 && <Tile label={t("فراخوانی ابزار", "Tool calls")} value={current.tools || 0} />}
      </div>

      <section className="card panel usage-facts">
        <div className="panel-head">
          <h3>{t(`جزئیات ${selectedLabel}`, `${selectedLabel} details`)}</h3>
          <span className="muted small">{t("توکن‌های خام، بدون گرد کردن", "Raw token counts, not rounded")}</span>
        </div>
        <div className="usage-facts-grid">
          <Fact label={t("ورودی تازه", "Fresh input")} value={current.input} />
          <Fact label={t("خوانده از کش", "Read from cache")} value={current.cache_read} />
          <Fact label={t("نوشته در کش", "Written to cache")} value={current.cache_write} />
          <Fact label={t("خروجی", "Output")} value={current.output} />
          <Fact label={t("توکن استدلال", "Reasoning tokens")} value={current.reasoning} />
          <Fact label={t("جمع کل", "Grand total")} value={split.total} />
          <Fact label={t("فراخوانی ابزار", "Tool calls")} value={current.tools || 0} />
        </div>
      </section>

      <section className="card panel">
        <div className="panel-head">
          <h3>{t("همه‌ی دوره‌ها", "Every period")}</h3>
          <span className="muted small">{t("توکن‌ها بر اساس تاریخ تهران", "Tokens by Tehran date")}</span>
        </div>
        <div className="table-wrap">
          <table className="data-table">
            <thead>
              <tr>
                <th scope="col">{t("دوره", "Period")}</th>
                <th scope="col">{t("پاسخ", "Replies")}</th>
                <th scope="col">{t("درخواست API", "API requests")}</th>
                <th scope="col">{t("ورودی", "Input")}</th>
                <th scope="col">{t("خروجی", "Output")}</th>
                <th scope="col">{t("کش", "Cache")}</th>
                <th scope="col">{t("جمع", "Total")}</th>
              </tr>
            </thead>
            <tbody>
              {report.periods.map((p) => {
                const all = pick(p, "all");
                const sp = totalOf(all);
                return (
                  <tr key={p.key} className={p.key === active?.key ? "selected" : ""}>
                    <td>
                      <span className="strong">{PERIODS.find((x) => x.id === p.key)?.label ?? p.key}</span>
                      {(p.key === "month" || p.key === "last_month") && <span className="cell-sub">{fmtMonth(p.from)}</span>}
                    </td>
                    <td>{fmtNum(p.chat.count)}</td>
                    <td>{fmtNum(p.api.count)}</td>
                    <td title={fmtNum(sp.input)}>{fmtCompact(sp.input)}</td>
                    <td title={fmtNum(sp.output)}>{fmtCompact(sp.output)}</td>
                    <td title={fmtNum(sp.cache)}>{fmtCompact(sp.cache)}</td>
                    <td className="strong" title={fmtNum(sp.total)}>
                      {fmtCompact(sp.total)}
                    </td>
                  </tr>
                );
              })}
            </tbody>
          </table>
        </div>
      </section>

      <section className="card panel">
        <div className="panel-head">
          <h3>{t("مصرف روزانه‌ی ۳۰ روز گذشته", "Daily usage in the last 30 days")}</h3>
          <Segmented label={t("منبع نمودار", "Chart source")} options={MODES} value={mode} onChange={setMode} />
        </div>
        <DailyChart days={report.daily} mode={mode} />
        <details className="chart-table">
          <summary>{t("جدول روزانه", "Daily table")}</summary>
          <div className="table-wrap">
            <table className="data-table">
              <thead>
                <tr>
                  <th scope="col">{t("روز", "Day")}</th>
                  <th scope="col">{t("پاسخ", "Replies")}</th>
                  <th scope="col">{t("ورودی گفت‌وگو", "Chat input")}</th>
                  <th scope="col">{t("خروجی گفت‌وگو", "Chat output")}</th>
                  <th scope="col">{t("درخواست API", "API requests")}</th>
                  <th scope="col">{t("ورودی API", "API input")}</th>
                  <th scope="col">{t("خروجی API", "API output")}</th>
                </tr>
              </thead>
              <tbody>
                {report.daily.map((d) => (
                  <tr key={d.day}>
                    <td>{fmtDay(d.day)}</td>
                    <td>{fmtNum(d.chat_count)}</td>
                    <td title={fmtNum(d.chat_in)}>{fmtCompact(d.chat_in)}</td>
                    <td title={fmtNum(d.chat_out)}>{fmtCompact(d.chat_out)}</td>
                    <td>{fmtNum(d.api_count)}</td>
                    <td title={fmtNum(d.api_in)}>{fmtCompact(d.api_in)}</td>
                    <td title={fmtNum(d.api_out)}>{fmtCompact(d.api_out)}</td>
                  </tr>
                ))}
              </tbody>
            </table>
          </div>
        </details>
      </section>

      <div className="split three usage-breakdowns">
        <Breakdown title={t("بر اساس منبع", "By source")} rows={report.sources.map((s) => ({ ...s, name: sourceLabel(s) }))} empty={t("هنوز مصرفی ثبت نشده است.", "No usage recorded yet.")} />
        <Breakdown title={t("بر اساس کلید API", "By API key")} rows={report.keys} empty={t("با کلید API در این بازه درخواستی فرستاده نشده است.", "No API requests were sent in this range.")} />
        <Breakdown title={t("بر اساس پروژه", "By project")} rows={report.projects} empty={t("درخواست API با نام پروژه‌ای در این بازه ثبت نشده است.", "No API request named a project in this range.")} />
      </div>

      <p className="muted small usage-note">
        {t("ورودی شامل توکن‌های کش است و خروجی شامل توکن‌های استدلال. عددها از سرویس پاسخ‌دهی می‌آیند و برای پاسخ‌های قدیمی ممکن است تقریبی باشند. مصرف فقط از حساب خود شماست و هیچ داده‌ی شخص دیگری در این صفحه نیست.", "Input includes cached tokens, and output includes reasoning tokens. The numbers come from the answering service and may be approximate for older replies. This page shows only your own account, never another person's data.")}
      </p>
    </div>
  );
}
