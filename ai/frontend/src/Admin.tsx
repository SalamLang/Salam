import { Fragment, useCallback, useEffect, useLayoutEffect, useRef, useState, type FormEvent } from "react";
import { api, ApiError, PAGE_SIZE, type AdminApp, type AdminTelegram, type AdminKey, type AdminPlatforms, type AdminSubscription, type AdminGuest, type AdminSession, type AdminSummary, type GuestMessage, type AdminUpload, type AdminUser, type ApiRequest, type AppEvent, type Chat, type EventFilters, type Payment, type PlanId, type RequestFilters, type UsagePeriod, type UsagePeriodKey, type UsageTotals } from "./api";
import { EventList } from "./Events";
import { fmtCompact, fmtCost, fmtDate, fmtDateTime, fmtDay, fmtMonth, fmtNum, fmtRelative, tokenSplit } from "./format";
import { IconBadgeCheck, IconChevronDown, IconClose, IconSearch, IconShield } from "./icons";
import { fmtSize } from "./Attach";
import { Markdown } from "./Markdown";
import { RequestList } from "./Requests";
import { confirmDialog, Subject } from "./Confirm";
import { t } from "./i18n";
import { Checkbox } from "./Checkbox";

type Picked = { id: number; name: string } | null;
type UsageMode = "all" | "chat" | "api";
type ChartMode = "calls" | "tokens";

const USAGE_MODES: { id: UsageMode; label: string }[] = [
  { id: "all", label: t("همه", "All") },
  { id: "chat", label: t("گفت‌وگو", "Chat") },
  { id: "api", label: "API" },
];

const CHART_MODES: { id: ChartMode; label: string }[] = [
  { id: "calls", label: t("فراخوانی‌ها", "Calls") },
  { id: "tokens", label: t("توکن‌ها", "Tokens") },
];

const PERIOD_LABELS: Record<UsagePeriodKey, string> = {
  today: t("امروز", "Today"),
  week: t("این هفته", "This week"),
  month: t("این ماه", "This month"),
  last_month: t("ماه قبل", "Last month"),
  all: t("کل", "All time"),
};

function errText(e: unknown, fallback: string): string {
  return e instanceof ApiError ? e.message : fallback;
}

function exactTitle(n: number): string | undefined {
  return Math.abs(n) >= 10000 ? fmtNum(n) : undefined;
}

function Tile({ label, value, sub, tone, compact, display }: { label: string; value: number; sub?: string; tone?: "danger"; compact?: boolean; display?: string }) {
  return (
    <div className={`card tile ${tone || ""}`}>
      <span className="tile-label">{label}</span>
      <strong className="tile-value" title={compact ? exactTitle(value) : undefined}>
        {display ?? (compact ? fmtCompact(value) : fmtNum(value))}
      </strong>
      {sub && <span className="tile-sub">{sub}</span>}
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

function pickUsage(p: UsagePeriod, mode: UsageMode): UsageTotals {
  if (mode === "chat") return p.chat;
  if (mode === "api") return p.api;
  return {
    count: p.chat.count + p.api.count,
    input: p.chat.input + p.api.input,
    output: p.chat.output + p.api.output,
    reasoning: p.chat.reasoning + p.api.reasoning,
    cache_read: p.chat.cache_read + p.api.cache_read,
    cache_write: p.chat.cache_write + p.api.cache_write,
    cost: p.chat.cost + p.api.cost,
    cost_in: p.chat.cost_in + p.api.cost_in,
    cost_out: p.chat.cost_out + p.api.cost_out,
  };
}

function periodSub(p: UsagePeriod): string {
  if (p.key === "month" || p.key === "last_month") return fmtMonth(p.from);
  return "";
}

function CompactCell({ n }: { n: number }) {
  return <td title={exactTitle(n)}>{fmtCompact(n)}</td>;
}

function UsageTiles({ usage }: { usage: UsagePeriod[] }) {
  const find = (key: UsagePeriodKey) => usage.find((p) => p.key === key);
  const today = find("today");
  const month = find("month");
  const all = find("all");
  const tile = (label: string, p: UsagePeriod | undefined) => {
    if (!p) return null;
    const tk = tokenSplit(pickUsage(p, "all"));
    return <Tile key={p.key} label={label} value={tk.total} compact sub={t(`${fmtCompact(tk.input)} ورودی، ${fmtCompact(tk.output)} خروجی`, `${fmtCompact(tk.input)} in, ${fmtCompact(tk.output)} out`)} />;
  };
  const monthTotals = month ? pickUsage(month, "all") : null;
  const monthCost = monthTotals ? monthTotals.cost : 0;
  return (
    <div className="tiles">
      {tile(t("توکن‌های امروز", "Tokens today"), today)}
      {tile(month ? t(`توکن‌های ${fmtMonth(month.from)}`, `Tokens in ${fmtMonth(month.from)}`) : t("توکن‌های این ماه", "Tokens this month"), month)}
      {tile(t("توکن‌های کل", "Tokens all time"), all)}
      {month && monthCost > 0 && <Tile label={t("هزینه‌ی این ماه", "Cost this month")} value={monthCost} display={fmtCost(monthCost)} sub={monthTotals ? t(`${fmtCost(monthTotals.cost_in)} ورودی، ${fmtCost(monthTotals.cost_out)} خروجی`, `${fmtCost(monthTotals.cost_in)} in, ${fmtCost(monthTotals.cost_out)} out`) : fmtMonth(month.from)} />}
    </div>
  );
}

function UsagePanel({ usage }: { usage: UsagePeriod[] }) {
  const [mode, setMode] = useState<UsageMode>("all");
  const rows = usage.map((p) => ({ p, t: pickUsage(p, mode) }));
  const showCost = rows.some((r) => r.t.cost > 0);
  const countLabel = mode === "chat" ? t("پاسخ", "Replies") : mode === "api" ? t("درخواست", "Requests") : t("پاسخ و درخواست", "Replies and requests");
  return (
    <section className="card panel">
      <div className="panel-head">
        <h3>{t("مصرف توکن", "Token usage")}</h3>
        <Segmented label={t("منبع مصرف", "Usage source")} options={USAGE_MODES} value={mode} onChange={setMode} />
      </div>
      <div className="table-wrap">
        <table className="data-table">
          <thead>
            <tr>
              <th scope="col">{t("دوره", "Period")}</th>
              <th scope="col">{countLabel}</th>
              <th scope="col">{t("ورودی", "Input")}</th>
              <th scope="col">{t("خروجی", "Output")}</th>
              <th scope="col">{t("کش", "Cache")}</th>
              <th scope="col">{t("جمع", "Total")}</th>
              {showCost && <th scope="col">{t("هزینه‌ی ورودی", "Input cost")}</th>}
              {showCost && <th scope="col">{t("هزینه‌ی خروجی", "Output cost")}</th>}
              {showCost && <th scope="col">{t("هزینه‌ی کل", "Total cost")}</th>}
            </tr>
          </thead>
          <tbody>
            {rows.map(({ p, t: tot }) => {
              const sp = tokenSplit(tot);
              const sub = periodSub(p);
              return (
                <tr key={p.key}>
                  <td title={p.key === "all" ? undefined : t(`${fmtDateTime(p.from)} تا ${fmtDateTime(p.to)}`, `${fmtDateTime(p.from)} to ${fmtDateTime(p.to)}`)}>
                    <span className="strong">{PERIOD_LABELS[p.key] || p.key}</span>
                    {sub && <span className="cell-sub">{sub}</span>}
                  </td>
                  <td>{fmtNum(tot.count)}</td>
                  <CompactCell n={sp.input} />
                  <CompactCell n={sp.output} />
                  <CompactCell n={sp.cache} />
                  <td className="strong" title={exactTitle(sp.total)}>
                    {fmtCompact(sp.total)}
                  </td>
                  {showCost && <td dir="ltr">{tot.cost_in > 0 ? fmtCost(tot.cost_in) : <span className="muted">-</span>}</td>}
                  {showCost && <td dir="ltr">{tot.cost_out > 0 ? fmtCost(tot.cost_out) : <span className="muted">-</span>}</td>}
                  {showCost && <td dir="ltr" className="strong">{tot.cost > 0 ? fmtCost(tot.cost) : <span className="muted">-</span>}</td>}
                </tr>
              );
            })}
          </tbody>
        </table>
      </div>
      <p className="muted small usage-note">
        {t("ورودی شامل توکن‌های کش هم هست و ستون کش همان بخش را جدا نشان می‌دهد. خروجی شامل توکن‌های استدلال است. هزینه‌ها به دلار است؛ سهم ورودی (با کش) و خروجی از قیمت مدل محاسبه می‌شود و برای ردیف‌های قدیمی تخمینی است. «همه» جمع گفت‌وگوهای سایت و درخواست‌های API است.", "Input includes cached tokens, and the cache column shows that part on its own. Output includes reasoning tokens. Costs are in US dollars; the input (cache included) and output shares come from the model's prices and are estimates for older rows. 'All' is site chats plus API requests.")}
      </p>
    </section>
  );
}

function DailyPanel({ daily }: { daily: AdminSummary["daily"] }) {
  const [mode, setMode] = useState<ChartMode>("calls");
  return (
    <section className="card panel">
      <div className="panel-head">
        <h3>{mode === "tokens" ? t("مصرف توکن در ۱۴ روز گذشته", "Token usage in the last 14 days") : t("فراخوانی‌های MCP و درخواست‌های API در ۱۴ روز گذشته", "MCP calls and API requests in the last 14 days")}</h3>
        <Segmented label={t("داده‌ی نمودار", "Chart data")} options={CHART_MODES} value={mode} onChange={setMode} />
      </div>
      <DailyChart daily={daily} mode={mode} />
    </section>
  );
}

function DailyChart({ daily, mode }: { daily: AdminSummary["daily"]; mode: ChartMode }) {
  const wrap = useRef<HTMLDivElement>(null);
  const [width, setWidth] = useState(640);
  const [hover, setHover] = useState<number | null>(null);

  useLayoutEffect(() => {
    const el = wrap.current;
    if (!el) return;
    const measure = () => setWidth(Math.max(240, Math.floor(el.clientWidth)));
    measure();
    const ro = new ResizeObserver(measure);
    ro.observe(el);
    return () => ro.disconnect();
  }, []);

  const height = 200;
  const top = 12;
  const bottom = 28;
  const axis = 44;
  const plotW = width - axis;
  const plotH = height - top - bottom;
  const tokens = mode === "tokens";
  const va = (d: AdminSummary["daily"][number]) => (tokens ? d.tokens_in || 0 : d.calls);
  const vb = (d: AdminSummary["daily"][number]) => (tokens ? d.tokens_out || 0 : d.api);
  const ca = tokens ? "in" : "mcp";
  const cb = tokens ? "out" : "api";
  const fmtAxis = tokens ? fmtCompact : fmtNum;
  const max = Math.max(1, ...daily.map((d) => Math.max(va(d), vb(d))));
  const nice = (() => {
    const p = 10 ** Math.floor(Math.log10(max));
    for (const m of [1, 2, 2.5, 5, 10]) if (m * p >= max) return m * p;
    return max;
  })();
  const n = Math.max(1, daily.length);
  const slot = plotW / n;
  const gap = 2;
  const groupW = Math.max(8, Math.min(44, slot - 6));
  const barW = (groupW - gap) / 2;
  const gx = (i: number) => plotW - (i + 1) * slot + (slot - groupW) / 2;
  const y = (v: number) => top + plotH - (v / nice) * plotH;
  const ticks = [0, nice / 2, nice];
  const labelEvery = slot < 34 ? 2 : 1;
  const totalA = daily.reduce((s, d) => s + va(d), 0);
  const totalB = daily.reduce((s, d) => s + vb(d), 0);

  const bar = (bx: number, by: number, bw: number, bh: number) => {
    if (bh <= 0) return "";
    const r = Math.min(3, bw / 2, bh);
    return `M${bx},${by + bh}V${by + r}Q${bx},${by} ${bx + r},${by}H${bx + bw - r}Q${bx + bw},${by} ${bx + bw},${by + r}V${by + bh}Z`;
  };

  const hovered = hover !== null ? daily[hover] : null;

  return (
    <div className="chart" ref={wrap}>
      <svg
        width={width}
        height={height}
        role="img"
        aria-label={
          tokens
            ? t(`نمودار ${fmtNum(daily.length)} روز گذشته: روی هم ${fmtNum(totalA)} توکن ورودی و ${fmtNum(totalB)} توکن خروجی`, `Chart of the last ${fmtNum(daily.length)} days: ${fmtNum(totalA)} input tokens and ${fmtNum(totalB)} output tokens in total`)
            : t(`نمودار ${fmtNum(daily.length)} روز گذشته: روی هم ${fmtNum(totalA)} فراخوانی MCP و ${fmtNum(totalB)} درخواست API`, `Chart of the last ${fmtNum(daily.length)} days: ${fmtNum(totalA)} MCP calls and ${fmtNum(totalB)} API requests in total`)
        }
        onMouseLeave={() => setHover(null)}
      >
        {ticks.map((t) => (
          <g key={t}>
            <line className="chart-grid" x1={0} x2={plotW} y1={y(t)} y2={y(t)} />
            <text className="chart-label" x={plotW + 8} y={y(t)} dominantBaseline="middle" textAnchor="start">
              {fmtAxis(t)}
            </text>
          </g>
        ))}
        {daily.map((d, i) => {
          const x0 = gx(i);
          const mcpX = x0 + barW + gap;
          const label = tokens
            ? t(`${fmtDay(d.day)}: ${fmtNum(va(d))} توکن ورودی، ${fmtNum(vb(d))} توکن خروجی`, `${fmtDay(d.day)}: ${fmtNum(va(d))} input tokens, ${fmtNum(vb(d))} output tokens`)
            : t(`${fmtDay(d.day)}: ${fmtNum(d.calls)} فراخوانی MCP، ${fmtNum(d.api)} درخواست API، ${fmtNum(d.users)} کاربر`, `${fmtDay(d.day)}: ${fmtNum(d.calls)} MCP calls, ${fmtNum(d.api)} API requests, ${fmtNum(d.users)} users`);
          return (
            <g key={d.day}>
              <path className={`chart-bar ${ca} ${hover === i ? "active" : ""}`} d={bar(mcpX, y(va(d)), barW, top + plotH - y(va(d)))} />
              <path className={`chart-bar ${cb} ${hover === i ? "active" : ""}`} d={bar(x0, y(vb(d)), barW, top + plotH - y(vb(d)))} />
              <rect
                className="chart-hit"
                x={plotW - (i + 1) * slot}
                y={top}
                width={slot}
                height={plotH}
                tabIndex={0}
                role="img"
                aria-label={label}
                onMouseEnter={() => setHover(i)}
                onFocus={() => setHover(i)}
                onBlur={() => setHover(null)}
              />
              {(daily.length - 1 - i) % labelEvery === 0 && (
                <text className="chart-label" x={x0 + groupW / 2} y={height - 8} textAnchor="middle">
                  {fmtDay(d.day)}
                </text>
              )}
            </g>
          );
        })}
        <line className="chart-axis" x1={0} x2={plotW} y1={top + plotH} y2={top + plotH} />
      </svg>
      {hovered && hover !== null && (
        <div
          className="chart-tip"
          style={{ left: Math.min(width - 90, Math.max(90, gx(hover) + groupW / 2)), top: Math.max(0, y(Math.max(va(hovered), vb(hovered))) - 8) }}
          aria-hidden="true"
        >
          <strong>{fmtDay(hovered.day)}</strong>
          {tokens ? (
            <>
              <span>
                <i className={`swatch ${ca}`} />
                {fmtNum(va(hovered))} {t("توکن ورودی", "input tokens")}
              </span>
              <span>
                <i className={`swatch ${cb}`} />
                {fmtNum(vb(hovered))} {t("توکن خروجی", "output tokens")}
              </span>
            </>
          ) : (
            <>
              <span>
                <i className="swatch mcp" />
                {fmtNum(hovered.calls)} {t("فراخوانی MCP", "MCP calls")}
              </span>
              <span>
                <i className="swatch api" />
                {fmtNum(hovered.api)} {t("درخواست API", "API requests")}
              </span>
              <span>{fmtNum(hovered.users)} {t("کاربر", "users")}</span>
            </>
          )}
        </div>
      )}
      <ul className="chart-legend">
        <li>
          <span className={`swatch ${ca}`} aria-hidden="true" />
          {tokens ? t("توکن‌های ورودی", "Input tokens") : t("فراخوانی‌های MCP", "MCP calls")}
        </li>
        <li>
          <span className={`swatch ${cb}`} aria-hidden="true" />
          {tokens ? t("توکن‌های خروجی", "Output tokens") : t("درخواست‌های API", "API requests")}
        </li>
      </ul>
      <details className="chart-table">
        <summary>{t("نمایش جدول داده‌ها", "Show data table")}</summary>
        <div className="table-wrap">
          <table className="data-table">
            <thead>
              {tokens ? (
                <tr>
                  <th scope="col">{t("روز", "Day")}</th>
                  <th scope="col">{t("توکن ورودی", "Input tokens")}</th>
                  <th scope="col">{t("توکن خروجی", "Output tokens")}</th>
                  <th scope="col">{t("جمع", "Total")}</th>
                </tr>
              ) : (
                <tr>
                  <th scope="col">{t("روز", "Day")}</th>
                  <th scope="col">{t("فراخوانی MCP", "MCP calls")}</th>
                  <th scope="col">{t("درخواست API", "API requests")}</th>
                  <th scope="col">{t("کاربر", "User")}</th>
                </tr>
              )}
            </thead>
            <tbody>
              {daily.map((d) =>
                tokens ? (
                  <tr key={d.day}>
                    <td>{fmtDay(d.day)}</td>
                    <td>{fmtNum(va(d))}</td>
                    <td>{fmtNum(vb(d))}</td>
                    <td>{fmtNum(va(d) + vb(d))}</td>
                  </tr>
                ) : (
                  <tr key={d.day}>
                    <td>{fmtDay(d.day)}</td>
                    <td>{fmtNum(d.calls)}</td>
                    <td>{fmtNum(d.api)}</td>
                    <td>{fmtNum(d.users)}</td>
                  </tr>
                ),
              )}
            </tbody>
          </table>
        </div>
      </details>
    </div>
  );
}

function TopList({ title, items, ltr }: { title: string; items: { name: string; count: number }[]; ltr?: boolean }) {
  const max = Math.max(1, ...items.map((i) => i.count));
  return (
    <section className="card panel">
      <h3>{title}</h3>
      {items.length === 0 ? (
        <p className="empty-list">{t("هنوز داده‌ای نیست.", "No data yet.")}</p>
      ) : (
        <ol className="top-list">
          {items.map((it) => (
            <li key={it.name}>
              <div className="top-row">
                {ltr ? (
                  <code dir="ltr">{it.name}</code>
                ) : (
                  <span>
                    <bdi>{it.name}</bdi>
                  </span>
                )}
                <span className="top-count">{fmtNum(it.count)}</span>
              </div>
              <span className="meter" aria-hidden="true">
                <i style={{ width: `${(it.count / max) * 100}%` }} />
              </span>
            </li>
          ))}
        </ol>
      )}
    </section>
  );
}

function Users({ picked, onPick, notify }: { picked: Picked; onPick: (p: Picked) => void; notify: (msg: string) => void }) {
  const [saving, setSaving] = useState<number | null>(null);
  const [q, setQ] = useState("");
  const [query, setQuery] = useState("");
  const [offset, setOffset] = useState(0);
  const [data, setData] = useState<{ users: AdminUser[]; total: number } | null>(null);
  const [loading, setLoading] = useState(false);
  const [error, setError] = useState("");

  useEffect(() => {
    const t = window.setTimeout(() => {
      setQuery(q.trim());
      setOffset(0);
    }, 350);
    return () => window.clearTimeout(t);
  }, [q]);

  useEffect(() => {
    let live = true;
    setLoading(true);
    setError("");
    api
      .adminUsers(query, offset)
      .then((r) => live && setData(r))
      .catch((e) => live && setError(errText(e, t("بارگذاری کاربران ناموفق بود.", "Could not load users."))))
      .finally(() => live && setLoading(false));
    return () => {
      live = false;
    };
  }, [query, offset]);

  const togglePolicy = async (u: AdminUser) => {
    const lift = !u.policy_free;
    const ok = await confirmDialog({
      title: lift ? t("برداشتن محدودیت فقط-سلام", "Lift the Salam-only limit") : t("برگرداندن محدودیت فقط-سلام", "Restore the Salam-only limit"),
      subject: u.name,
      message: lift
        ? t("دستیار برای این کاربر (سایت، تلگرام، ایجنت و کلیدهای API او) به زبان‌ها و فناوری‌های دیگر هم کد می‌نویسد و بررسی‌های سلام‌محور اعمال نمی‌شود. بقیه‌ی کاربران همچنان محدود می‌مانند.", "The assistant will write code in other languages and technologies for this user (site, Telegram, agent and their API keys) and the Salam-only checks are skipped. Everyone else stays limited.")
        : t("محدودیت فقط-سلام دوباره برای این کاربر فعال می‌شود.", "The Salam-only limit applies to this user again."),
      confirmLabel: lift ? t("برداشتن", "Lift") : t("فعال کردن", "Restore"),
      icon: "alert",
    });
    if (!ok) return;
    setSaving(u.id);
    try {
      const r = await api.adminUserPolicy(u.id, { policy_free: lift });
      setData((cur) => (cur ? { ...cur, users: cur.users.map((x) => (x.id === u.id ? { ...x, policy_free: r.user.policy_free } : x)) } : cur));
      notify(lift ? t("محدودیت برای این کاربر برداشته شد.", "The limit was lifted for this user.") : t("محدودیت دوباره فعال شد.", "The limit is on again."));
    } catch (e) {
      notify(errText(e, t("ذخیره ناموفق بود.", "Could not save.")));
    } finally {
      setSaving(null);
    }
  };

  const total = data?.total ?? 0;
  const from = total === 0 ? 0 : offset + 1;
  const to = Math.min(total, offset + (data?.users.length ?? 0));

  return (
    <section className="card panel">
      <div className="panel-head">
        <h3>{t("کاربران", "Users")}</h3>
        <label className="search">
          <IconSearch width={16} height={16} />
          <span className="sr-only">{t("جست‌وجوی کاربر", "Search users")}</span>
          <input type="search" value={q} onChange={(e) => setQ(e.target.value)} placeholder={t("جست‌وجوی نام یا ایمیل", "Search name or email")} />
        </label>
      </div>
      {error && (
        <div className="form-error" role="alert">
          {error}
        </div>
      )}
      <div className={`table-wrap scroll-y ${loading ? "busy" : ""}`}>
        <table className="data-table">
          <thead>
            <tr>
              <th scope="col">{t("نام", "Name")}</th>
              <th scope="col">{t("ایمیل", "Email")}</th>
              <th scope="col">{t("عضویت", "Joined")}</th>
              <th scope="col">{t("گفت‌وگو", "Chats")}</th>
              <th scope="col">{t("پیام", "Messages")}</th>
              <th scope="col">{t("تصویر", "Images")}</th>
              <th scope="col">{t("نشست", "Sessions")}</th>
              <th scope="col">{t("نسخه‌ی ایجنت", "Agent version")}</th>
              <th scope="col">{t("برنامه", "Apps")}</th>
              <th scope="col">{t("فراخوانی MCP", "MCP calls")}</th>
              <th scope="col">API</th>
              <th scope="col">{t("توکن ورودی", "Input tokens")}</th>
              <th scope="col">{t("توکن خروجی", "Output tokens")}</th>
              <th scope="col">{t("هزینه‌ی ورودی ($)", "Input cost ($)")}</th>
              <th scope="col">{t("هزینه‌ی خروجی ($)", "Output cost ($)")}</th>
              <th scope="col">{t("هزینه‌ی کل ($)", "Total cost ($)")}</th>
              <th scope="col">{t("تلگرام", "Telegram")}</th>
              <th scope="col">{t("محدودیت فقط سلام", "Salam-only limit")}</th>
              <th scope="col">{t("آخرین ورود", "Last login")}</th>
              <th scope="col">{t("آخرین چت", "Last chat")}</th>
              <th scope="col">{t("آخرین ایجنت", "Last agent")}</th>
              <th scope="col">{t("آخرین API", "Last API")}</th>
            </tr>
          </thead>
          <tbody>
            {data?.users.map((u) => (
              <tr key={u.id} className={picked?.id === u.id ? "selected" : ""}>
                <td>
                  <button type="button" className="link" onClick={() => onPick(picked?.id === u.id ? null : { id: u.id, name: u.name })} aria-pressed={picked?.id === u.id} title={t("نمایش فعالیت، درخواست‌ها و برنامه‌های این کاربر", "Show this user's activity, requests and apps")}>
                    <bdi>{u.name}</bdi>
                  </button>
                </td>
                <td dir="ltr" className="cell-ltr">
                  {u.email}
                </td>
                <td title={fmtDateTime(u.created)}>{fmtDate(u.created)}</td>
                <td>{fmtNum(u.chats)}</td>
                <td>{fmtNum(u.messages)}</td>
                <td title={u.files ? t(`${fmtNum(u.files)} فایل روی هم`, `${fmtNum(u.files)} files in total`) : undefined}>{fmtNum(u.images ?? 0)}</td>
                <td>{fmtNum(u.sessions ?? 0)}</td>
                <td dir="ltr" className="cell-ltr">{u.agent_version || "-"}</td>
                <td>{fmtNum(u.apps)}</td>
                <td>{fmtNum(u.mcp_calls)}</td>
                <td>{fmtNum(u.api_requests)}</td>
                <td title={exactTitle(u.tokens_in || 0)}>{fmtCompact(u.tokens_in || 0)}</td>
                <td title={exactTitle(u.tokens_out || 0)}>{fmtCompact(u.tokens_out || 0)}</td>
                <td dir="ltr">{(u.cost_in ?? 0) > 0 ? fmtCost(u.cost_in ?? 0) : <span className="muted">-</span>}</td>
                <td dir="ltr">{(u.cost_out ?? 0) > 0 ? fmtCost(u.cost_out ?? 0) : <span className="muted">-</span>}</td>
                <td dir="ltr" className="strong">{(u.cost_in ?? 0) + (u.cost_out ?? 0) > 0 ? fmtCost((u.cost_in ?? 0) + (u.cost_out ?? 0)) : <span className="muted">-</span>}</td>
                <td>{u.telegram ? fmtNum(u.telegram) : <span className="muted">-</span>}</td>
                <td>
                  <button type="button" className={`badge ${u.policy_free ? "danger" : "ok"}`} disabled={saving === u.id} onClick={() => togglePolicy(u)} title={t("با زدن، محدودیت فقط-سلام برای این کاربر برداشته یا برگردانده می‌شود", "Click to lift or restore the Salam-only limit for this user")}>
                    {saving === u.id ? "…" : u.policy_free ? t("برداشته شده", "Lifted") : t("فعال", "On")}
                  </button>
                </td>
                <td title={u.last_login ? t(`${fmtDateTime(u.last_login)}، روی هم ${fmtNum(u.logins || 0)} ورود`, `${fmtDateTime(u.last_login)}, ${fmtNum(u.logins || 0)} logins in total`) : undefined}>{fmtRelative(u.last_login || 0)}</td>
                <td title={u.last_active_chat ? fmtDateTime(u.last_active_chat) : undefined}>{fmtRelative(u.last_active_chat)}</td>
                <td title={u.last_active_agent ? fmtDateTime(u.last_active_agent) : undefined}>{fmtRelative(u.last_active_agent)}</td>
                <td title={u.last_active_api ? fmtDateTime(u.last_active_api) : undefined}>{fmtRelative(u.last_active_api)}</td>
              </tr>
            ))}
            {!data && !error && (
              <tr>
                <td colSpan={22} className="empty-cell">
                  <span className="list-loading inline" role="status">
                    <span className="spinner" />
                    <span>{t("در حال بارگذاری کاربران…", "Loading users…")}</span>
                  </span>
                </td>
              </tr>
            )}
            {data && data.users.length === 0 && !loading && (
              <tr>
                <td colSpan={22} className="empty-cell">
                  {t("کاربری پیدا نشد.", "No users found.")}
                </td>
              </tr>
            )}
          </tbody>
        </table>
      </div>
      <div className="pager">
        <span className="muted">
          {data ? t(`${fmtNum(from)} تا ${fmtNum(to)} از ${fmtNum(total)}`, `${fmtNum(from)} to ${fmtNum(to)} of ${fmtNum(total)}`) : t("در حال بارگذاری…", "Loading…")}
        </span>
        <div>
          <button type="button" className="button ghost small" disabled={offset === 0 || loading} onClick={() => setOffset(Math.max(0, offset - PAGE_SIZE))}>
            {t("قبلی", "Previous")}
          </button>
          <button type="button" className="button ghost small" disabled={offset + PAGE_SIZE >= total || loading} onClick={() => setOffset(offset + PAGE_SIZE)}>
            {t("بعدی", "Next")}
          </button>
        </div>
      </div>
    </section>
  );
}

function Events({ picked, onPick }: { picked: Picked; onPick: (p: Picked) => void }) {
  const [tool, setTool] = useState("");
  const [client, setClient] = useState("");
  const [applied, setApplied] = useState({ tool: "", client: "" });
  const [events, setEvents] = useState<AppEvent[]>([]);
  const [loading, setLoading] = useState(true);
  const [hasMore, setHasMore] = useState(false);
  const [error, setError] = useState("");
  const seq = useRef(0);

  const load = useCallback(
    async (before?: number) => {
      const id = ++seq.current;
      setLoading(true);
      setError("");
      const f: EventFilters = { user: picked?.id, tool: applied.tool || undefined, client: applied.client || undefined, before };
      try {
        const r = await api.adminEvents(f);
        if (id !== seq.current) return;
        setEvents((cur) => (before ? [...cur, ...r.events] : r.events));
        setHasMore(r.events.length >= PAGE_SIZE);
      } catch (e) {
        if (id === seq.current) setError(errText(e, t("بارگذاری رویدادها ناموفق بود.", "Could not load events.")));
      } finally {
        if (id === seq.current) setLoading(false);
      }
    },
    [picked, applied],
  );

  useEffect(() => {
    setEvents([]);
    load();
  }, [load]);

  const submit = (e: FormEvent) => {
    e.preventDefault();
    setApplied({ tool: tool.trim(), client: client.trim() });
  };

  const clear = () => {
    setTool("");
    setClient("");
    setApplied({ tool: "", client: "" });
  };

  return (
    <section className="card panel">
      <div className="panel-head">
        <h3>{t("رویدادها", "Events")}</h3>
      </div>
      <form className="filters" onSubmit={submit}>
        {picked && (
          <span className="chip">
            {t("کاربر:", "User:")} <bdi>{picked.name}</bdi>
            <button type="button" className="icon-button tiny" onClick={() => onPick(null)} aria-label={t("برداشتن فیلتر کاربر", "Remove user filter")}>
              <IconClose width={14} height={14} />
            </button>
          </span>
        )}
        <label className="filter-field">
          <span className="sr-only">{t("نام ابزار", "Tool name")}</span>
          <input dir="ltr" value={tool} onChange={(e) => setTool(e.target.value)} placeholder="tool" />
        </label>
        <label className="filter-field">
          <span className="sr-only">{t("نام برنامه", "App name")}</span>
          <input dir="auto" value={client} onChange={(e) => setClient(e.target.value)} placeholder={t("نام برنامه", "App name")} />
        </label>
        <button type="submit" className="button ghost small">
          {t("اعمال فیلتر", "Apply filter")}
        </button>
        {(applied.tool || applied.client) && (
          <button type="button" className="link" onClick={clear}>
            {t("پاک کردن", "Clear")}
          </button>
        )}
      </form>
      {error && (
        <div className="form-error" role="alert">
          {error}
        </div>
      )}
      <EventList
        events={events}
        loading={loading}
        hasMore={hasMore}
        onMore={() => load(events[events.length - 1]?.id)}
        showUser
        showClient
        onUser={(id, name) => onPick({ id, name })}
        empty={t("رویدادی با این فیلترها پیدا نشد.", "No events match these filters.")}
      />
    </section>
  );
}

function RequestsFeed({ picked, onPick }: { picked: Picked; onPick: (p: Picked) => void }) {
  const [project, setProject] = useState("");
  const [keyId, setKeyId] = useState("");
  const [applied, setApplied] = useState<{ project: string; key?: number }>({ project: "" });
  const [requests, setRequests] = useState<ApiRequest[]>([]);
  const [loading, setLoading] = useState(true);
  const [hasMore, setHasMore] = useState(false);
  const [error, setError] = useState("");
  const seq = useRef(0);

  const load = useCallback(
    async (before?: number) => {
      const id = ++seq.current;
      setLoading(true);
      setError("");
      const f: RequestFilters = { user: picked?.id, project: applied.project || undefined, key: applied.key, before };
      try {
        const r = await api.adminRequests(f);
        if (id !== seq.current) return;
        setRequests((cur) => (before ? [...cur, ...r.requests] : r.requests));
        setHasMore(r.requests.length >= PAGE_SIZE);
      } catch (e) {
        if (id === seq.current) setError(errText(e, t("بارگذاری درخواست‌ها ناموفق بود.", "Could not load requests.")));
      } finally {
        if (id === seq.current) setLoading(false);
      }
    },
    [picked, applied],
  );

  useEffect(() => {
    setRequests([]);
    load();
  }, [load]);

  const submit = (e: FormEvent) => {
    e.preventDefault();
    const k = Number.parseInt(keyId.trim(), 10);
    setApplied({ project: project.trim(), key: Number.isFinite(k) && k > 0 ? k : undefined });
  };

  const clear = () => {
    setProject("");
    setKeyId("");
    setApplied({ project: "" });
  };

  const byKey = (id: number) => {
    setKeyId(String(id));
    setApplied((a) => ({ ...a, key: id }));
  };

  const byProject = (name: string) => {
    setProject(name);
    setApplied((a) => ({ ...a, project: name }));
  };

  return (
    <section className="card panel">
      <div className="panel-head">
        <h3>{t("درخواست‌های API", "API requests")}</h3>
      </div>
      <form className="filters" onSubmit={submit}>
        {picked && (
          <span className="chip">
            {t("کاربر:", "User:")} <bdi>{picked.name}</bdi>
            <button type="button" className="icon-button tiny" onClick={() => onPick(null)} aria-label={t("برداشتن فیلتر کاربر", "Remove user filter")}>
              <IconClose width={14} height={14} />
            </button>
          </span>
        )}
        <label className="filter-field">
          <span className="sr-only">{t("نام پروژه", "Project name")}</span>
          <input dir="auto" value={project} onChange={(e) => setProject(e.target.value)} placeholder={t("نام پروژه", "Project name")} />
        </label>
        <label className="filter-field">
          <span className="sr-only">{t("شناسه‌ی کلید", "Key ID")}</span>
          <input dir="ltr" inputMode="numeric" value={keyId} onChange={(e) => setKeyId(e.target.value.replace(/[۰-۹]/g, (d) => String("۰۱۲۳۴۵۶۷۸۹".indexOf(d))).replace(/[^0-9]/g, ""))} placeholder={t("شناسه‌ی کلید", "Key ID")} />
        </label>
        <button type="submit" className="button ghost small">
          {t("اعمال فیلتر", "Apply filter")}
        </button>
        {(applied.project || applied.key) && (
          <button type="button" className="link" onClick={clear}>
            {t("پاک کردن", "Clear")}
          </button>
        )}
      </form>
      {error && (
        <div className="form-error" role="alert">
          {error}
        </div>
      )}
      <RequestList
        requests={requests}
        loading={loading}
        hasMore={hasMore}
        onMore={() => load(requests[requests.length - 1]?.id)}
        showUser
        onUser={(id, name) => onPick({ id, name })}
        onKey={byKey}
        onProject={byProject}
        empty={t("درخواستی با این فیلترها پیدا نشد.", "No requests match these filters.")}
      />
    </section>
  );
}

const PLAN_LABELS: { id: PlanId; label: string }[] = [
  { id: "free", label: t("رایگان", "Free") },
  { id: "plus", label: t("پلاس", "Plus") },
  { id: "pro", label: t("حرفه‌ای", "Pro") },
  { id: "unlimited", label: t("نامحدود", "Unlimited") },
];

const PAY_WORDS: Record<Payment["status"], string> = {
  pending: t("در انتظار", "Pending"),
  verifying: t("در حال تایید", "Verifying"),
  paid: t("پرداخت شده", "Paid"),
  failed: t("ناموفق", "Failed"),
  canceled: t("لغو شده", "Canceled"),
};

const GATEWAY_WORDS: Record<string, string> = { zarinpal: t("زرین‌پال", "Zarinpal"), zibal: t("زیبال", "Zibal") };

function toman(n: number): string {
  return t(`${fmtNum(n)} تومان`, `${fmtNum(n)} toman`);
}

function Subscriptions({ picked, notify }: { picked: Picked; notify: (msg: string) => void }) {
  const [rows, setRows] = useState<AdminSubscription[] | null>(null);
  const [active, setActive] = useState(0);
  const [income, setIncome] = useState(0);
  const [latest, setLatest] = useState(false);
  const [error, setError] = useState("");
  const [busy, setBusy] = useState<number | null>(null);
  const [email, setEmail] = useState("");
  const [plan, setPlan] = useState<PlanId>("pro");
  const [days, setDays] = useState("");
  const [note, setNote] = useState("");
  const [granting, setGranting] = useState(false);

  const load = useCallback(async () => {
    setError("");
    setRows(null);
    try {
      const r = await api.adminSubscriptions(latest, picked?.id);
      setRows(r.subscriptions);
      setActive(r.active);
      setIncome(r.income);
    } catch (e) {
      setError(errText(e, t("بارگذاری اشتراک‌ها ناموفق بود.", "Could not load subscriptions.")));
    }
  }, [latest, picked]);

  useEffect(() => {
    void load();
  }, [load]);

  const grant = async (e: FormEvent) => {
    e.preventDefault();
    const who = email.trim();
    if (!who) return;
    setGranting(true);
    try {
      await api.adminGrant({ email: who, plan, days: days.trim() ? Number(days) : undefined, note: note.trim() || undefined });
      notify(t("اشتراک داده شد.", "Subscription granted."));
      setEmail("");
      setNote("");
      setDays("");
      await load();
    } catch (err) {
      notify(errText(err, t("دادن اشتراک ناموفق بود.", "Could not grant the subscription.")));
    } finally {
      setGranting(false);
    }
  };

  const cancel = async (row: AdminSubscription) => {
    const ok = await confirmDialog({
      title: t("لغو اشتراک", "Cancel subscription"),
      subject: row.user?.email ?? row.plan_name,
      message: (
        <>
          {t("اشتراک", "This user's")} <Subject>{row.plan_name}</Subject> {t("این کاربر لغو می‌شود و به پلن رایگان برمی‌گردد.", "subscription will be canceled and they go back to the free plan.")}
        </>
      ),
      confirmLabel: t("لغو اشتراک", "Cancel subscription"),
      icon: "alert",
    });
    if (!ok) return;
    setBusy(row.id);
    try {
      await api.adminCancelSubscription(row.id);
      notify(t("اشتراک لغو شد.", "Subscription canceled."));
      await load();
    } catch (e) {
      notify(errText(e, t("لغو اشتراک ناموفق بود.", "Could not cancel the subscription.")));
    } finally {
      setBusy(null);
    }
  };

  return (
    <section className="card panel">
      <div className="panel-head">
        <div className="panel-title">
          <h3>{t("اشتراک‌ها", "Subscriptions")}{picked ? t(" این کاربر", " for this user") : ""}</h3>
          {!picked && active > 0 && <span className="badge ok">{fmtNum(active)} {t("اشتراک فعال", "active")}</span>}
        </div>
        <Segmented
          label={t("نما", "View")}
          options={[
            { id: "active", label: t("فعال", "Active") },
            { id: "latest", label: t("آخرین اشتراک هر کاربر", "Latest per user") },
          ]}
          value={latest ? "latest" : "active"}
          onChange={(v) => setLatest(v === "latest")}
        />
      </div>
      <form className="grant" onSubmit={grant}>
        <div className="grant-head">
          <h4>{t("دادن اشتراک رایگان", "Grant a free subscription")}</h4>
          <p className="muted">{t("اشتراک فعال کاربر جایگزین می‌شود و اگر همین پلن را داشته باشد، به مدت باقی‌مانده‌اش اضافه می‌شود.", "Replaces the user's active subscription. If they already have this plan, the days are added to what is left.")}</p>
        </div>
        <div className="grant-grid">
          <label className="grant-field">
            <span>{t("ایمیل کاربر", "User email")}</span>
            <input type="email" dir="ltr" value={email} onChange={(e) => setEmail(e.target.value)} placeholder="user@example.com" required />
          </label>
          <label className="grant-field">
            <span>{t("پلن", "Plan")}</span>
            <div className="select">
              <select value={plan} onChange={(e) => setPlan(e.target.value as PlanId)}>
                {PLAN_LABELS.map((o) => (
                  <option key={o.id} value={o.id}>
                    {o.label}
                  </option>
                ))}
              </select>
              <IconChevronDown width={16} height={16} />
            </div>
          </label>
          <label className="grant-field">
            <span>
              {t("تعداد روز", "Days")} <span className="opt">{t("اختیاری", "optional")}</span>
            </span>
            <input type="number" inputMode="numeric" min="1" max="3650" value={days} onChange={(e) => setDays(e.target.value)} placeholder={t("پیش‌فرض پلن", "Plan default")} />
          </label>
          <label className="grant-field">
            <span>
              {t("یادداشت", "Note")} <span className="opt">{t("اختیاری", "optional")}</span>
            </span>
            <input type="text" value={note} onChange={(e) => setNote(e.target.value)} placeholder={t("چرا رایگان داده شد", "Why it was given for free")} />
          </label>
        </div>
        <div className="grant-actions">
          <button type="submit" className="button primary grant-go" disabled={granting}>
            {granting ? <span className="spinner" /> : <IconBadgeCheck width={18} height={18} />}
            {t("دادن اشتراک", "Grant subscription")}
          </button>
        </div>
      </form>
      {error && (
        <div className="form-error" role="alert">
          {error}
        </div>
      )}
      {rows === null && !error ? (
        <div className="list-loading" role="status">
          <span className="spinner" />
          <span className="sr-only">{t("در حال بارگذاری", "Loading")}</span>
        </div>
      ) : (
        <div className="table-wrap scroll-y">
          <table className="data-table">
            <thead>
              <tr>
                <th scope="col">{t("کاربر", "User")}</th>
                <th scope="col">{t("پلن", "Plan")}</th>
                <th scope="col">{t("از", "From")}</th>
                <th scope="col">{t("تا", "To")}</th>
                <th scope="col">{t("راه", "Source")}</th>
                <th scope="col">{t("مبلغ", "Amount")}</th>
                <th scope="col">{t("وضعیت", "Status")}</th>
                <th scope="col">{t("یادداشت", "Note")}</th>
                <th scope="col">
                  <span className="sr-only">{t("کارها", "Actions")}</span>
                </th>
              </tr>
            </thead>
            <tbody>
              {rows?.map((r) => (
                <tr key={r.id}>
                  <td>
                    <bdi>{r.user?.name}</bdi>
                    <span className="cell-sub" dir="ltr">
                      {r.user?.email}
                    </span>
                  </td>
                  <td>{r.plan_name}</td>
                  <td>{fmtDate(r.started)}</td>
                  <td>{r.expires === 0 ? t("بدون پایان", "No end") : fmtDate(r.expires)}</td>
                  <td>{r.source === "admin" ? t("هدیه‌ی مدیر", "Admin gift") : t("پرداخت", "Payment")}</td>
                  <td>{r.price === 0 ? "-" : toman(r.price)}</td>
                  <td>
                    <span className={`badge ${r.status === "active" ? "ok" : ""}`}>{r.status === "active" ? t("فعال", "Active") : r.status === "canceled" ? t("لغو شده", "Canceled") : t("جایگزین شده", "Replaced")}</span>
                  </td>
                  <td>
                    <bdi>{r.note}</bdi>
                  </td>
                  <td>
                    {r.status === "active" && (
                      <button type="button" className="link danger" disabled={busy === r.id} onClick={() => void cancel(r)}>
                        {t("لغو", "Cancel")}
                      </button>
                    )}
                  </td>
                </tr>
              ))}
              {rows?.length === 0 && (
                <tr>
                  <td colSpan={9} className="muted">
                    {t("هنوز اشتراکی ثبت نشده است.", "No subscriptions yet.")}
                  </td>
                </tr>
              )}
            </tbody>
          </table>
        </div>
      )}
      <div className="panel-foot muted">{t("مجموع پرداخت‌های موفق:", "Total successful payments:")} {toman(income)}</div>
    </section>
  );
}

function Payments({ picked }: { picked: Picked }) {
  const [rows, setRows] = useState<Payment[] | null>(null);
  const [income, setIncome] = useState(0);
  const [status, setStatus] = useState("");
  const [error, setError] = useState("");

  useEffect(() => {
    let live = true;
    setRows(null);
    setError("");
    api
      .adminPayments({ status: status || undefined, user: picked?.id })
      .then((r) => {
        if (!live) return;
        setRows(r.payments);
        setIncome(r.income);
      })
      .catch((e) => live && setError(errText(e, t("بارگذاری پرداخت‌ها ناموفق بود.", "Could not load payments."))));
    return () => {
      live = false;
    };
  }, [status, picked]);

  return (
    <section className="card panel">
      <div className="panel-head">
        <h3>{t("پرداخت‌ها", "Payments")}{picked ? t(" این کاربر", " for this user") : ""}</h3>
        <Segmented
          label={t("وضعیت", "Status")}
          options={[
            { id: "", label: t("همه", "All") },
            { id: "paid", label: t("پرداخت شده", "Paid") },
            { id: "pending", label: t("در انتظار", "Pending") },
            { id: "failed", label: t("ناموفق", "Failed") },
            { id: "canceled", label: t("لغو شده", "Canceled") },
          ]}
          value={status}
          onChange={setStatus}
        />
      </div>
      {error && (
        <div className="form-error" role="alert">
          {error}
        </div>
      )}
      {rows === null && !error ? (
        <div className="list-loading" role="status">
          <span className="spinner" />
          <span className="sr-only">{t("در حال بارگذاری", "Loading")}</span>
        </div>
      ) : (
        <div className="table-wrap scroll-y">
          <table className="data-table">
            <thead>
              <tr>
                <th scope="col">{t("کاربر", "User")}</th>
                <th scope="col">{t("پلن", "Plan")}</th>
                <th scope="col">{t("مبلغ", "Amount")}</th>
                <th scope="col">{t("درگاه", "Gateway")}</th>
                <th scope="col">{t("وضعیت", "Status")}</th>
                <th scope="col">{t("کد پیگیری", "Reference")}</th>
                <th scope="col">{t("کارت", "Card")}</th>
                <th scope="col">{t("زمان", "Time")}</th>
              </tr>
            </thead>
            <tbody>
              {rows?.map((r) => (
                <tr key={r.id}>
                  <td>
                    <bdi>{r.user?.name}</bdi>
                    <span className="cell-sub" dir="ltr">
                      {r.user?.email}
                    </span>
                  </td>
                  <td>{r.plan_name}</td>
                  <td>{toman(r.amount)}</td>
                  <td>{GATEWAY_WORDS[r.gateway] ?? r.gateway}</td>
                  <td>
                    <span className={`badge ${r.status === "paid" ? "ok" : r.status === "pending" || r.status === "verifying" ? "pending" : "danger"}`} title={r.message}>
                      {PAY_WORDS[r.status]}
                    </span>
                  </td>
                  <td dir="ltr">{r.ref || "-"}</td>
                  <td dir="ltr">{r.card || "-"}</td>
                  <td>{fmtDateTime(r.paid > 0 ? r.paid : r.created)}</td>
                </tr>
              ))}
              {rows?.length === 0 && (
                <tr>
                  <td colSpan={8} className="muted">
                    {t("پرداختی با این فیلتر نیست.", "No payments match this filter.")}
                  </td>
                </tr>
              )}
            </tbody>
          </table>
        </div>
      )}
      <div className="panel-foot muted">{t("مجموع پرداخت‌های موفق:", "Total successful payments:")} {toman(income)}</div>
    </section>
  );
}

function Conversations({ picked }: { picked: Picked }) {
  const [chats, setChats] = useState<Chat[] | null>(null);
  const [error, setError] = useState("");

  useEffect(() => {
    setChats(null);
    setError("");
    if (!picked) return;
    let live = true;
    api
      .adminUserChats(picked.id)
      .then((r) => live && setChats(r.chats))
      .catch((e) => live && setError(errText(e, t("بارگذاری گفت‌وگوها ناموفق بود.", "Could not load chats."))));
    return () => {
      live = false;
    };
  }, [picked]);

  return (
    <section className="card panel">
      <div className="panel-head">
        <h3>{t("گفت‌وگوها", "Chats")}{picked ? t(" این کاربر", " for this user") : ""}</h3>
      </div>
      {!picked ? (
        <p className="muted">{t("برای دیدن گفت‌وگوها یک کاربر را از جدول کاربران انتخاب کنید.", "Pick a user from the users table to see their chats.")}</p>
      ) : error ? (
        <div className="form-error" role="alert">
          {error}
        </div>
      ) : chats === null ? (
        <div className="list-loading" role="status">
          <span className="spinner" />
          <span className="sr-only">{t("در حال بارگذاری", "Loading")}</span>
        </div>
      ) : (
        <div className="table-wrap scroll-y">
          <table className="data-table">
            <thead>
              <tr>
                <th scope="col">{t("عنوان", "Title")}</th>
                <th scope="col">{t("نوع", "Type")}</th>
                <th scope="col">{t("آخرین بروزرسانی", "Last updated")}</th>
              </tr>
            </thead>
            <tbody>
              {chats.map((c) => (
                <tr key={c.id}>
                  <td className="cell-clip">
                    <a className="link" href={`/admin/chats/${c.id}`} target="_blank" rel="noopener" title={t("باز کردن کامل این گفت‌وگو در زبانه‌ی تازه", "Open the full chat in a new tab")}>
                      <bdi>{c.title}</bdi>
                    </a>
                  </td>
                  <td>{c.agent ? t("ایجنت", "Agent") : t("وب", "Web")}</td>
                  <td title={fmtDateTime(c.updated)}>{fmtRelative(c.updated)}</td>
                </tr>
              ))}
              {chats.length === 0 && (
                <tr>
                  <td colSpan={3} className="empty-cell">
                    {t("گفت‌وگویی نیست.", "No chats.")}
                  </td>
                </tr>
              )}
            </tbody>
          </table>
        </div>
      )}
    </section>
  );
}

function policyBadges(k: AdminKey) {
  return (
    <span className="badge-row">
      {k.salam_prompt ? <span className="badge ok">{t("فقط سلام", "Salam only")}</span> : <span className="badge danger">{t("پرامپت سفارشی", "Custom prompt")}</span>}
      {!k.salam_tools && <span className="badge">{t("بدون MCP", "No MCP")}</span>}
      {k.salam_prompt && k.system_prompt && <span className="badge">{t("دستور اپراتور", "Operator note")}</span>}
    </span>
  );
}

function KeyPolicyEditor({ k, onSaved, onClose, notify }: { k: AdminKey; onSaved: (k: AdminKey) => void; onClose: () => void; notify: (msg: string) => void }) {
  const [salam, setSalam] = useState(k.salam_prompt);
  const [tools, setTools] = useState(k.salam_tools);
  const [prompt, setPrompt] = useState(k.system_prompt);
  const [saving, setSaving] = useState(false);

  const save = async (e: FormEvent) => {
    e.preventDefault();
    setSaving(true);
    try {
      const r = await api.adminUpdateKey(k.id, { salam_prompt: salam, salam_tools: tools, system_prompt: prompt });
      onSaved(r.key);
      notify(t("تنظیم کلید ذخیره شد.", "Key settings saved."));
    } catch (err) {
      notify(errText(err, t("ذخیره‌ی تنظیم کلید ناموفق بود.", "Could not save the key settings.")));
    } finally {
      setSaving(false);
    }
  };

  return (
    <form className="grant key-policy" onSubmit={save}>
      <div className="grant-head">
        <h4>
          {t("تنظیم کلید", "Settings for key")} <bdi className="strong">{k.name}</bdi>
        </h4>
        <p className="muted">{t("پیش‌فرض هر کلید: دستیار فقط با سلام کار می‌کند و ابزارهای MCP سلام روشن است. این تنظیم فقط با این کلید عوض می‌شود.", "By default a key answers in Salam only, with the Salam MCP tools on. These switches only change this key.")}</p>
      </div>
      <Checkbox
        checked={salam}
        onChange={setSalam}
        label={t("پرامپت سیستمی سلام", "Salam system prompt")}
        hint={t("خاموش: پرامپت و سیاست سلام برداشته می‌شود و فقط پرامپت زیر به کار می‌رود، با پروفایل ساده‌ی موتور.", "Off: the Salam prompt and policy are removed and only the prompt below is used, on the engine's plain profile.")}
      />
      <Checkbox
        checked={tools}
        onChange={setTools}
        label={t("ابزارهای MCP", "MCP tools")}
        hint={t("خاموش: دستیار هیچ ابزاری صدا نمی‌زند (نه salam_check و نه جست‌وجو).", "Off: the assistant calls no tools at all (no salam_check, no search).")}
      />
      <label className="grant-field">
        <span>
          {salam ? t("دستور اپراتور", "Operator instructions") : t("پرامپت سیستمی کامل", "Full system prompt")} <span className="opt">{t("اختیاری", "optional")}</span>
        </span>
        <textarea dir="auto" rows={6} value={prompt} onChange={(e) => setPrompt(e.target.value)} placeholder={salam ? t("بعد از سیاست سلام اضافه می‌شود و نمی‌تواند آن را بردارد.", "Added after the Salam policy; it cannot lift it.") : t("همه‌ی پرامپت سیستمی این کلید.", "The whole system prompt for this key.")} />
      </label>
      <div className="grant-actions key-policy-actions">
        <button type="submit" className="button primary" disabled={saving}>
          {saving ? <span className="spinner" /> : null}
          {t("ذخیره", "Save")}
        </button>
        <button type="button" className="button ghost" onClick={onClose}>
          {t("بستن", "Close")}
        </button>
      </div>
    </form>
  );
}

function KeyPolicies({ picked, notify }: { picked: Picked; notify: (msg: string) => void }) {
  const [keys, setKeys] = useState<AdminKey[] | null>(null);
  const [ready, setReady] = useState(true);
  const [error, setError] = useState("");
  const [custom, setCustom] = useState(false);
  const [open, setOpen] = useState<number | null>(null);
  const [more, setMore] = useState(false);
  const [loadingMore, setLoadingMore] = useState(false);

  useEffect(() => {
    let live = true;
    setKeys(null);
    setError("");
    setMore(false);
    api
      .adminKeys({ user: picked?.id, custom })
      .then((r) => {
        if (!live) return;
        setKeys(r.keys);
        setReady(r.plain_ready);
        setMore(r.keys.length >= 50);
      })
      .catch((e) => live && setError(errText(e, t("بارگذاری کلیدها ناموفق بود.", "Could not load keys."))));
    return () => {
      live = false;
    };
  }, [picked, custom]);

  const loadMore = async () => {
    if (!keys || keys.length === 0) return;
    setLoadingMore(true);
    try {
      const r = await api.adminKeys({ user: picked?.id, custom, before: keys[keys.length - 1].id });
      setKeys((cur) => [...(cur ?? []), ...r.keys]);
      setMore(r.keys.length >= 50);
    } catch (e) {
      notify(errText(e, t("بارگذاری کلیدها ناموفق بود.", "Could not load keys.")));
    } finally {
      setLoadingMore(false);
    }
  };

  return (
    <section className="card panel">
      <div className="panel-head">
        <h3>{t("کلیدهای API", "API keys")}{picked ? t(" این کاربر", " for this user") : ""}</h3>
        <Segmented
          label={t("نما", "View")}
          options={[
            { id: "all", label: t("همه", "All") },
            { id: "custom", label: t("تنظیم‌شده", "Customized") },
          ]}
          value={custom ? "custom" : "all"}
          onChange={(v) => setCustom(v === "custom")}
        />
      </div>
      {!ready && (
        <p className="policy-warn" role="status">
          {t("موتور پروفایل ساده ندارد (AI_ENGINE_AGENT_RAW)، پس کلیدهایی که پرامپت سلامشان خاموش است هنوز با پروفایل سلام پاسخ می‌گیرند.", "The engine has no plain profile (AI_ENGINE_AGENT_RAW), so keys with the Salam prompt off are still answered by the Salam profile.")}
        </p>
      )}
      {error && (
        <div className="form-error" role="alert">
          {error}
        </div>
      )}
      {keys === null && !error ? (
        <div className="list-loading" role="status">
          <span className="spinner" />
          <span className="sr-only">{t("در حال بارگذاری", "Loading")}</span>
        </div>
      ) : (
        <div className="table-wrap scroll-y">
          <table className="data-table">
            <thead>
              <tr>
                <th scope="col">{t("کاربر", "User")}</th>
                <th scope="col">{t("کلید", "Key")}</th>
                <th scope="col">{t("آخرین استفاده", "Last used")}</th>
                <th scope="col">{t("درخواست", "Requests")}</th>
                <th scope="col">{t("رفتار", "Behavior")}</th>
                <th scope="col">
                  <span className="sr-only">{t("کارها", "Actions")}</span>
                </th>
              </tr>
            </thead>
            <tbody>
              {keys?.map((k) => (
                <Fragment key={k.id}>
                  <tr className={k.revoked ? "revoked" : ""}>
                    <td>
                      <bdi>{k.user.name}</bdi>
                      <span className="cell-sub" dir="ltr">
                        {k.user.email}
                      </span>
                    </td>
                    <td>
                      <bdi>{k.name}</bdi>
                      <span className="cell-sub cell-mono" dir="ltr">
                        {k.prefix}…{k.revoked ? ` ${t("(باطل)", "(revoked)")}` : ""}
                      </span>
                    </td>
                    <td title={k.last_used ? fmtDateTime(k.last_used) : undefined}>{fmtRelative(k.last_used)}</td>
                    <td>{fmtNum(k.requests)}</td>
                    <td title={k.policy_at ? fmtDateTime(k.policy_at) : undefined}>{policyBadges(k)}</td>
                    <td>
                      <button type="button" className="button ghost small" aria-expanded={open === k.id} onClick={() => setOpen(open === k.id ? null : k.id)} aria-label={t(`تنظیم کلید ${k.name}`, `Settings for key ${k.name}`)}>
                        {t("تنظیم", "Settings")}
                      </button>
                    </td>
                  </tr>
                  {open === k.id && (
                    <tr className="key-policy-row">
                      <td colSpan={6}>
                        <KeyPolicyEditor
                          k={k}
                          notify={notify}
                          onClose={() => setOpen(null)}
                          onSaved={(saved) => setKeys((cur) => (cur ? cur.map((x) => (x.id === saved.id ? saved : x)) : cur))}
                        />
                      </td>
                    </tr>
                  )}
                </Fragment>
              ))}
              {keys && keys.length === 0 && (
                <tr>
                  <td colSpan={6} className="empty-cell">
                    {custom ? t("کلید تنظیم‌شده‌ای نیست.", "No customized keys.") : t("کلیدی ساخته نشده است.", "No keys yet.")}
                  </td>
                </tr>
              )}
            </tbody>
          </table>
        </div>
      )}
      {loadingMore && (
        <div className="list-loading" role="status">
          <span className="spinner" />
          <span className="sr-only">{t("در حال بارگذاری", "Loading")}</span>
        </div>
      )}
      {more && !loadingMore && (
        <button type="button" className="button ghost more" onClick={loadMore}>
          {t("نمایش موارد بیشتر", "Show more")}
        </button>
      )}
    </section>
  );
}

function AppsTable({ picked, notify }: { picked: Picked; notify: (msg: string) => void }) {
  const [apps, setApps] = useState<AdminApp[] | null>(null);
  const [error, setError] = useState("");
  const [busy, setBusy] = useState<number | null>(null);
  const [more, setMore] = useState(false);
  const [loadingMore, setLoadingMore] = useState(false);

  useEffect(() => {
    let live = true;
    setApps(null);
    setError("");
    setMore(false);
    api
      .adminApps(picked?.id)
      .then((r) => {
        if (!live) return;
        setApps(r.apps);
        setMore(r.apps.length >= 50);
      })
      .catch((e) => live && setError(errText(e, t("بارگذاری برنامه‌ها ناموفق بود.", "Could not load apps."))));
    return () => {
      live = false;
    };
  }, [picked]);

  const loadMore = async () => {
    if (!apps || apps.length === 0) return;
    setLoadingMore(true);
    try {
      const r = await api.adminApps(picked?.id, apps[apps.length - 1].id);
      setApps((cur) => [...(cur ?? []), ...r.apps]);
      setMore(r.apps.length >= 50);
    } catch (e) {
      notify(errText(e, t("بارگذاری برنامه‌ها ناموفق بود.", "Could not load apps.")));
    } finally {
      setLoadingMore(false);
    }
  };

  const revoke = async (a: AdminApp) => {
    const ok = await confirmDialog({
      title: t("قطع اتصال برنامه", "Disconnect app"),
      subject: a.client.name,
      message: (
        <>
          {t("اتصال این برنامه برای", "This app will be disconnected for")} <Subject>{a.user.name}</Subject>{t(" قطع می‌شود.", ".")}
        </>
      ),
      confirmLabel: t("قطع اتصال", "Disconnect"),
      icon: "alert",
    });
    if (!ok) return;
    setBusy(a.id);
    try {
      await api.adminRevokeApp(a.id);
      setApps((cur) => (cur ? cur.map((x) => (x.id === a.id ? { ...x, revoked: true } : x)) : cur));
      notify(t("اتصال قطع شد.", "App disconnected."));
    } catch (e) {
      notify(errText(e, t("قطع اتصال ناموفق بود.", "Could not disconnect the app.")));
    } finally {
      setBusy(null);
    }
  };

  return (
    <section className="card panel">
      <div className="panel-head">
        <h3>{t("برنامه‌های متصل", "Connected apps")}{picked ? t(" این کاربر", " for this user") : ""}</h3>
      </div>
      {error && (
        <div className="form-error" role="alert">
          {error}
        </div>
      )}
      {apps === null && !error ? (
        <div className="list-loading" role="status">
          <span className="spinner" />
          <span className="sr-only">{t("در حال بارگذاری", "Loading")}</span>
        </div>
      ) : (
        <div className="table-wrap scroll-y">
          <table className="data-table">
            <thead>
              <tr>
                <th scope="col">{t("کاربر", "User")}</th>
                <th scope="col">{t("برنامه", "App")}</th>
                <th scope="col">{t("میزبان", "Host")}</th>
                <th scope="col">{t("دسترسی", "Access")}</th>
                <th scope="col">{t("اتصال", "Connected")}</th>
                <th scope="col">{t("آخرین استفاده", "Last used")}</th>
                <th scope="col">{t("فراخوانی", "Calls")}</th>
                <th scope="col">{t("وضعیت", "Status")}</th>
                <th scope="col">
                  <span className="sr-only">{t("کارها", "Actions")}</span>
                </th>
              </tr>
            </thead>
            <tbody>
              {apps?.map((a) => (
                <tr key={a.id} className={a.revoked ? "revoked" : ""}>
                  <td>
                    <bdi>{a.user.name}</bdi>
                    <span className="cell-sub" dir="ltr">
                      {a.user.email}
                    </span>
                  </td>
                  <td>
                    <bdi>{a.client.name}</bdi>
                    <span className="cell-sub">{a.client.kind === "cimd" ? t("تأییدشده", "Verified") : t("تأییدنشده", "Unverified")}</span>
                  </td>
                  <td dir="ltr" className="cell-ltr">
                    {a.client.host}
                  </td>
                  <td dir="ltr" className="cell-ltr">
                    {a.scope}
                  </td>
                  <td title={fmtDateTime(a.created)}>{fmtDate(a.created)}</td>
                  <td title={a.last_used ? fmtDateTime(a.last_used) : undefined}>{fmtRelative(a.last_used)}</td>
                  <td>{fmtNum(a.calls)}</td>
                  <td>{a.revoked ? <span className="badge">{t("قطع‌شده", "Disconnected")}</span> : <span className="badge ok">{t("فعال", "Active")}</span>}</td>
                  <td>
                    {!a.revoked && (
                      <button type="button" className="button danger small" disabled={busy === a.id} onClick={() => revoke(a)} aria-label={t(`قطع اتصال ${a.client.name} برای ${a.user.name}`, `Disconnect ${a.client.name} for ${a.user.name}`)}>
                        {busy === a.id ? <span className="spinner" /> : t("قطع اتصال", "Disconnect")}
                      </button>
                    )}
                  </td>
                </tr>
              ))}
              {apps && apps.length === 0 && (
                <tr>
                  <td colSpan={9} className="empty-cell">
                    {t("برنامه‌ی متصلی نیست.", "No connected apps.")}
                  </td>
                </tr>
              )}
            </tbody>
          </table>
        </div>
      )}
      {loadingMore && (
        <div className="list-loading" role="status">
          <span className="spinner" />
          <span className="sr-only">{t("در حال بارگذاری", "Loading")}</span>
        </div>
      )}
      {more && !loadingMore && (
        <button type="button" className="button ghost more" onClick={loadMore}>
          {t("نمایش موارد بیشتر", "Show more")}
        </button>
      )}
    </section>
  );
}

function TelegramSessions({ picked, onPick }: { picked: Picked; onPick: (p: Picked) => void }) {
  const [data, setData] = useState<AdminTelegram | null>(null);
  const [error, setError] = useState("");

  useEffect(() => {
    let live = true;
    const load = () =>
      api
        .adminTelegram()
        .then((r) => live && (setData(r), setError("")))
        .catch((e) => live && setError(errText(e, t("بارگذاری نشست‌های تلگرام ناموفق بود.", "Could not load Telegram sessions."))));
    load();
    const timer = window.setInterval(load, 15000);
    return () => {
      live = false;
      window.clearInterval(timer);
    };
  }, []);

  const online = (seen: number) => !!data && seen > data.now - 300;
  const links = (data?.links ?? []).filter((l) => !picked || l.user.id === picked.id);
  const guests = picked ? [] : data?.guests ?? [];
  const who = (name: string, username: string) => (
    <>
      <bdi>{name || "-"}</bdi>
      {username && (
        <span className="cell-sub" dir="ltr">
          @{username}
        </span>
      )}
    </>
  );
  const state = (seen: number, answering: boolean) => (
    <>
      {answering ? <span className="badge ok">{t("در حال پاسخ", "Answering")}</span> : online(seen) ? <span className="badge ok">{t("آنلاین", "Online")}</span> : <span className="muted">{seen ? fmtRelative(seen) : "-"}</span>}
    </>
  );

  return (
    <section className="card panel">
      <div className="panel-head">
        <h3>{t("نشست‌های تلگرام", "Telegram sessions")}</h3>
        {data && <span className="muted">{t(`${fmtNum(data.online_links)} کاربر و ${fmtNum(data.online_guests)} مهمان آنلاین (۵ دقیقه‌ی اخیر)`, `${fmtNum(data.online_links)} users and ${fmtNum(data.online_guests)} guests online (last 5 minutes)`)}</span>}
      </div>
      {error && (
        <div className="form-error" role="alert">
          {error}
        </div>
      )}
      <div className="table-wrap scroll-y">
        <table className="data-table">
          <thead>
            <tr>
              <th scope="col">{t("تلگرام", "Telegram")}</th>
              <th scope="col">{t("حساب سلام", "Salam account")}</th>
              <th scope="col">{t("وضعیت", "Status")}</th>
              <th scope="col">{t("گفت‌وگو", "Chats")}</th>
              <th scope="col">{t("پیام", "Messages")}</th>
              <th scope="col">{t("زبان", "Language")}</th>
              <th scope="col">{t("اتصال", "Connected")}</th>
            </tr>
          </thead>
          <tbody>
            {links.map((l) => (
              <tr key={`l${l.tg_id}`}>
                <td>{who(l.name, l.username)}</td>
                <td>
                  <button type="button" className="link" onClick={() => onPick(picked?.id === l.user.id ? null : { id: l.user.id, name: l.user.name })} title={t("نمایش فعالیت این کاربر", "Show this user's activity")}>
                    <bdi>{l.user.name}</bdi>
                  </button>
                  <span className="cell-sub" dir="ltr">
                    {l.user.email}
                  </span>
                </td>
                <td>{state(l.last_seen, l.answering)}</td>
                <td>{fmtNum(l.chats)}</td>
                <td>{fmtNum(l.messages)}</td>
                <td>{l.lang === "en" ? "English" : "فارسی"}</td>
                <td title={fmtDateTime(l.created)}>{fmtDate(l.created)}</td>
              </tr>
            ))}
            {guests.map((g) => (
              <tr key={`g${g.tg_id}`}>
                <td>{who(g.name, g.username)}</td>
                <td className="muted">{t("مهمان", "Guest")}</td>
                <td>{state(g.last_seen, g.answering)}</td>
                <td>-</td>
                <td>{fmtNum(g.messages)}</td>
                <td>{g.lang === "en" ? "English" : "فارسی"}</td>
                <td title={fmtDateTime(g.created)}>{fmtDate(g.created)}</td>
              </tr>
            ))}
            {data && links.length === 0 && guests.length === 0 && (
              <tr>
                <td colSpan={7} className="empty-cell">
                  {t("هنوز کسی از تلگرام استفاده نکرده است.", "Nobody has used Telegram yet.")}
                </td>
              </tr>
            )}
            {!data && !error && (
              <tr>
                <td colSpan={7} className="empty-cell">
                  <span className="list-loading inline" role="status">
                    <span className="spinner" />
                  </span>
                </td>
              </tr>
            )}
          </tbody>
        </table>
      </div>
    </section>
  );
}

function shortAgent(ua: string): string {
  const browser = /Edg\/[\d.]+|OPR\/[\d.]+|Firefox\/[\d.]+|Chrome\/[\d.]+|Version\/[\d.]+ Safari|Safari\/[\d.]+/.exec(ua)?.[0] ?? "";
  const os = /Windows NT [\d.]+|Mac OS X [\d_]+|Android [\d.]+|iPhone OS [\d_]+|Linux/.exec(ua)?.[0]?.replace(/_/g, ".") ?? "";
  return [browser.replace("Version/", "Safari ").replace(" Safari", ""), os].filter(Boolean).join(t("، ", ", ")) || ua.slice(0, 60);
}

function GuestThread({ id }: { id: number }) {
  const [msgs, setMsgs] = useState<GuestMessage[] | null>(null);
  const [error, setError] = useState("");
  useEffect(() => {
    let live = true;
    api
      .adminGuest(id)
      .then((r) => live && setMsgs(r.messages))
      .catch((e) => live && setError(errText(e, t("بارگذاری گفت‌وگو ناموفق بود.", "Loading the chat failed."))));
    return () => {
      live = false;
    };
  }, [id]);
  if (error)
    return (
      <div className="form-error" role="alert">
        {error}
      </div>
    );
  if (!msgs)
    return (
      <div className="list-loading" role="status">
        <span className="spinner" />
      </div>
    );
  return (
    <div className="guest-thread">
      {msgs.map((m) => (
        <div key={m.id} className={`guest-msg ${m.role}`}>
          <span className="guest-who">
            {m.role === "user" ? t("مهمان", "Guest") : t("سِلامِت", "Salamat")} · <span title={fmtDateTime(m.created)}>{fmtRelative(m.created)}</span>
            {m.status !== "done" && <span className="muted"> ({m.status})</span>}
          </span>
          <div dir="auto">{m.role === "user" ? <p className="guest-text">{m.body}</p> : <Markdown text={m.body} />}</div>
        </div>
      ))}
      {msgs.length === 0 && <p className="muted">{t("پیامی نیست.", "No messages.")}</p>}
    </div>
  );
}

function Guests() {
  const [rows, setRows] = useState<AdminGuest[] | null>(null);
  const [total, setTotal] = useState(0);
  const [offset, setOffset] = useState(0);
  const [open, setOpen] = useState<number | null>(null);
  const [error, setError] = useState("");
  useEffect(() => {
    let live = true;
    api
      .adminGuests(offset)
      .then((r) => {
        if (!live) return;
        setRows(r.guests);
        setTotal(r.total);
      })
      .catch((e) => live && setError(errText(e, t("بارگذاری مهمان‌ها ناموفق بود.", "Loading guests failed."))));
    return () => {
      live = false;
    };
  }, [offset]);
  return (
    <section className="card panel">
      <div className="panel-head">
        <h3>{t("گفت‌وگوهای مهمان سایت", "Website guest chats")}</h3>
      </div>
      {error ? (
        <div className="form-error" role="alert">
          {error}
        </div>
      ) : rows === null ? (
        <div className="list-loading" role="status">
          <span className="spinner" />
        </div>
      ) : (
        <>
          <div className="table-wrap scroll-y">
            <table className="data-table">
              <thead>
                <tr>
                  <th scope="col">{t("اولین پرسش", "First question")}</th>
                  <th scope="col">{t("پیام", "Messages")}</th>
                  <th scope="col">{t("زبان", "Language")}</th>
                  <th scope="col">IP</th>
                  <th scope="col">{t("شروع", "Started")}</th>
                  <th scope="col">{t("آخرین پیام", "Last message")}</th>
                </tr>
              </thead>
              <tbody>
                {rows.map((g) => (
                  <Fragment key={g.id}>
                    <tr className={open === g.id ? "selected" : ""}>
                      <td className="cell-clip">
                        <button type="button" className="link" onClick={() => setOpen((o) => (o === g.id ? null : g.id))} aria-expanded={open === g.id} title={g.first}>
                          <bdi>{g.first || t("(بدون پیام)", "(no message)")}</bdi>
                        </button>
                      </td>
                      <td>{fmtNum(g.messages)}</td>
                      <td>{g.lang === "en" ? "English" : "فارسی"}</td>
                      <td dir="ltr" className="cell-ltr">
                        {g.ip || "-"}
                      </td>
                      <td title={fmtDateTime(g.created)}>{fmtRelative(g.created)}</td>
                      <td title={fmtDateTime(g.updated)}>{fmtRelative(g.updated)}</td>
                    </tr>
                    {open === g.id && (
                      <tr>
                        <td colSpan={6}>
                          <GuestThread id={g.id} />
                        </td>
                      </tr>
                    )}
                  </Fragment>
                ))}
                {rows.length === 0 && (
                  <tr>
                    <td colSpan={6} className="empty-cell">
                      {t("هنوز مهمانی از سایت نپرسیده است.", "No website guest has asked anything yet.")}
                    </td>
                  </tr>
                )}
              </tbody>
            </table>
          </div>
          <div className="pager">
            <span className="muted">
              {fmtNum(total === 0 ? 0 : offset + 1)} {t("تا", "to")} {fmtNum(Math.min(total, offset + rows.length))} {t("از", "of")} {fmtNum(total)}
            </span>
            <div>
              <button type="button" className="button ghost small" disabled={offset === 0} onClick={() => setOffset(Math.max(0, offset - PAGE_SIZE))}>
                {t("قبلی", "Previous")}
              </button>
              <button type="button" className="button ghost small" disabled={offset + PAGE_SIZE >= total} onClick={() => setOffset(offset + PAGE_SIZE)}>
                {t("بعدی", "Next")}
              </button>
            </div>
          </div>
        </>
      )}
    </section>
  );
}

const PLATFORM_DAYS = [
  { id: "7", label: t("۷ روز", "7 days") },
  { id: "30", label: t("۳۰ روز", "30 days") },
  { id: "0", label: t("همه", "All") },
];

function platformName(id: string): string {
  switch (id) {
    case "windows":
      return "Windows";
    case "macos":
      return "macOS";
    case "linux":
      return "Linux";
    case "other":
      return t("سایر سیستم‌ها", "Other systems");
    case "telegram":
      return t("ربات تلگرام", "Telegram bot");
    default:
      return t("نامشخص (وب و چت‌بات سایت)", "Unknown (website and chatbot)");
  }
}

function PlatformsPanel() {
  const [days, setDays] = useState("30");
  const [data, setData] = useState<AdminPlatforms | null>(null);
  const [error, setError] = useState("");

  useEffect(() => {
    let live = true;
    setError("");
    api
      .adminPlatforms(Number(days))
      .then((r) => live && setData(r))
      .catch((e) => live && setError(errText(e, t("بارگذاری آمار سیستم‌عامل‌ها ناموفق بود.", "Could not load the platform breakdown."))));
    return () => {
      live = false;
    };
  }, [days]);

  const total = data?.total ?? 0;
  const pct = (n: number) => (total === 0 ? "0" : fmtNum(Math.round((n / total) * 1000) / 10));
  return (
    <section className="card panel">
      <div className="panel-head">
        <h3>{t("سیستم‌عامل نشست‌ها", "Session platforms")}</h3>
        <Segmented label={t("بازه", "Period")} options={PLATFORM_DAYS} value={days} onChange={setDays} />
      </div>
      {error ? (
        <div className="form-error" role="alert">
          {error}
        </div>
      ) : data === null ? (
        <div className="list-loading" role="status">
          <span className="spinner" />
          <span className="sr-only">{t("در حال بارگذاری", "Loading")}</span>
        </div>
      ) : total === 0 ? (
        <p className="empty-list">{t("هنوز نشستی نیست.", "No sessions yet.")}</p>
      ) : (
        <>
          <ol className="top-list">
            {data.platforms.map((p) => (
              <li key={p.id}>
                <div className="top-row">
                  <span>{platformName(p.id)}</span>
                  <span className="top-count">
                    {fmtNum(p.count)} ({pct(p.count)}٪)
                  </span>
                </div>
                <span className="meter" aria-hidden="true">
                  <i style={{ width: `${(p.count / total) * 100}%` }} />
                </span>
              </li>
            ))}
          </ol>
          {data.systems.length > 0 && (
            <div className="table-wrap">
              <table className="data-table">
                <thead>
                  <tr>
                    <th scope="col">{t("سیستم‌عامل", "System")}</th>
                    <th scope="col">{t("معماری", "Architecture")}</th>
                    <th scope="col">{t("نشست", "Sessions")}</th>
                  </tr>
                </thead>
                <tbody>
                  {data.systems.map((x) => (
                    <tr key={`${x.os}|${x.version}|${x.arch}`}>
                      <td dir="ltr" className="cell-ltr">
                        {x.version || platformName(x.os)}
                      </td>
                      <td dir="ltr" className="cell-ltr">
                        {x.arch || "-"}
                      </td>
                      <td>{fmtNum(x.count)}</td>
                    </tr>
                  ))}
                </tbody>
              </table>
            </div>
          )}
        </>
      )}
    </section>
  );
}

function Sessions({ picked, notify }: { picked: Picked; notify: (msg: string) => void }) {
  const [rows, setRows] = useState<AdminSession[] | null>(null);
  const [total, setTotal] = useState(0);
  const [client, setClient] = useState("");
  const [offset, setOffset] = useState(0);
  const [error, setError] = useState("");
  const [busy, setBusy] = useState<number | null>(null);
  const [reload, setReload] = useState(0);

  useEffect(() => {
    setOffset(0);
  }, [client, picked]);

  useEffect(() => {
    let live = true;
    setError("");
    api
      .adminSessions({ user: picked?.id, client: client || undefined, offset })
      .then((r) => {
        if (!live) return;
        setRows(r.sessions);
        setTotal(r.total);
      })
      .catch((e) => live && setError(errText(e, t("بارگذاری نشست‌ها ناموفق بود.", "Could not load sessions."))));
    return () => {
      live = false;
    };
  }, [client, picked, offset, reload]);

  const revoke = async (s: AdminSession) => {
    const ok = await confirmDialog({
      title: t("بستن نشست", "End session"),
      subject: s.user.email,
      message: (
        <>
          {t("نشست", "This user's")} <Subject>{s.client === "agent" ? t("ایجنت", "Agent") : t("وب", "Web")}</Subject> {t("این کاربر بسته می‌شود و در اولین درخواست بعدی، حداکثر تا یک دقیقه، از حساب خارج می‌شود.", "session will be ended. They are signed out on their next request, within a minute at most.")}
        </>
      ),
      confirmLabel: t("بستن نشست", "End session"),
      icon: "alert",
    });
    if (!ok) return;
    setBusy(s.id);
    try {
      await api.revokeSession(s.id);
      notify(t("نشست بسته شد.", "Session ended."));
      setReload((n) => n + 1);
    } catch (e) {
      notify(errText(e, t("بستن نشست ناموفق بود.", "Could not end the session.")));
    } finally {
      setBusy(null);
    }
  };

  const revokeAll = async () => {
    if (!picked) return;
    const ok = await confirmDialog({
      title: t("خروج از همه‌ی نشست‌ها", "Sign out everywhere"),
      subject: picked.name,
      message: <>{t("همه‌ی نشست‌های وب و ایجنت این کاربر بسته می‌شود. او باید دوباره وارد شود.", "All of this user's web and agent sessions will be ended. They will have to sign in again.")}</>,
      confirmLabel: t("بستن همه", "End all"),
      icon: "alert",
    });
    if (!ok) return;
    try {
      const r = await api.revokeUserSessions(picked.id);
      notify(t(`${fmtNum(r.revoked)} نشست بسته شد.`, `${fmtNum(r.revoked)} sessions ended.`));
      setReload((n) => n + 1);
    } catch (e) {
      notify(errText(e, t("بستن نشست‌ها ناموفق بود.", "Could not end the sessions.")));
    }
  };

  return (
    <>
    {!picked && <PlatformsPanel />}
    <section className="card panel">
      <div className="panel-head">
        <h3>{t("نشست‌ها", "Sessions")}{picked ? t(" این کاربر", " for this user") : ""}</h3>
        <div className="row-actions">
          <Segmented
            label={t("نوع", "Type")}
            options={[
              { id: "", label: t("همه", "All") },
              { id: "web", label: t("وب", "Web") },
              { id: "agent", label: t("ایجنت", "Agent") },
            ]}
            value={client}
            onChange={setClient}
          />
          {picked && (
            <button type="button" className="button ghost small danger" onClick={revokeAll}>
              {t("خروج از همه‌ی نشست‌ها", "Sign out everywhere")}
            </button>
          )}
        </div>
      </div>
      {error ? (
        <div className="form-error" role="alert">
          {error}
        </div>
      ) : rows === null ? (
        <div className="list-loading" role="status">
          <span className="spinner" />
          <span className="sr-only">{t("در حال بارگذاری", "Loading")}</span>
        </div>
      ) : (
        <>
          <div className="table-wrap scroll-y">
            <table className="data-table">
              <thead>
                <tr>
                  <th scope="col">{t("کاربر", "User")}</th>
                  <th scope="col">{t("از", "Client")}</th>
                  <th scope="col">{t("نسخه", "Version")}</th>
                  <th scope="col">{t("سیستم‌عامل", "System")}</th>
                  <th scope="col">{t("مرورگر یا سیستم", "Browser or system")}</th>
                  <th scope="col">IP</th>
                  <th scope="col">{t("ورود", "Signed in")}</th>
                  <th scope="col">{t("آخرین استفاده", "Last used")}</th>
                  <th scope="col">
                    <span className="sr-only">{t("کارها", "Actions")}</span>
                  </th>
                </tr>
              </thead>
              <tbody>
                {rows.map((s) => (
                  <tr key={s.id}>
                    <td title={s.user.email}>
                      <bdi>{s.user.name}</bdi>
                    </td>
                    <td>{s.client === "agent" ? t("ایجنت", "Agent") : t("وب", "Web")}</td>
                    <td dir="ltr" className="cell-ltr">
                      {s.client_version || "-"}
                    </td>
                    <td dir="ltr" className="cell-ltr" title={s.arch}>
                      {s.os ? s.os_version || platformName(s.os) : "-"}
                    </td>
                    <td dir="ltr" className="cell-ltr cell-clip" title={s.agent}>
                      {shortAgent(s.agent)}
                    </td>
                    <td dir="ltr" className="cell-ltr">
                      {s.ip || "-"}
                    </td>
                    <td title={fmtDateTime(s.created)}>{fmtRelative(s.created)}</td>
                    <td title={s.last_used ? fmtDateTime(s.last_used) : undefined}>{s.last_used ? fmtRelative(s.last_used) : "-"}</td>
                    <td className="cell-actions">
                      {s.current ? (
                        <span className="muted">{t("نشست فعلی شما", "Your current session")}</span>
                      ) : (
                        <button type="button" className="button ghost small danger" disabled={busy === s.id} onClick={() => revoke(s)}>
                          {t("بستن نشست", "End session")}
                        </button>
                      )}
                    </td>
                  </tr>
                ))}
                {rows.length === 0 && (
                  <tr>
                    <td colSpan={9} className="empty-cell">
                      {t("نشست فعالی نیست.", "No active sessions.")}
                    </td>
                  </tr>
                )}
              </tbody>
            </table>
          </div>
          <div className="pager">
            <span className="muted">
              {fmtNum(total === 0 ? 0 : offset + 1)} {t("تا", "to")} {fmtNum(Math.min(total, offset + rows.length))} {t("از", "of")} {fmtNum(total)}
            </span>
            <div>
              <button type="button" className="button ghost small" disabled={offset === 0} onClick={() => setOffset(Math.max(0, offset - PAGE_SIZE))}>
                {t("قبلی", "Previous")}
              </button>
              <button type="button" className="button ghost small" disabled={offset + PAGE_SIZE >= total} onClick={() => setOffset(offset + PAGE_SIZE)}>
                {t("بعدی", "Next")}
              </button>
            </div>
          </div>
        </>
      )}
    </section>
    </>
  );
}

const KIND_LABEL: Record<string, string> = { image: t("تصویر", "Image"), text: t("متن یا کد", "Text or code"), archive: "ZIP", pdf: "PDF" };

async function saveUpload(f: AdminUpload, notify: (msg: string) => void) {
  try {
    const r = await api.adminFile(f.id);
    const bytes = Uint8Array.from(atob(r.data), (c) => c.charCodeAt(0));
    const url = URL.createObjectURL(new Blob([bytes], { type: r.mime }));
    const a = document.createElement("a");
    a.href = url;
    a.download = r.name;
    a.click();
    window.setTimeout(() => URL.revokeObjectURL(url), 10000);
  } catch (e) {
    notify(errText(e, t("دریافت فایل ناموفق بود.", "Could not download the file.")));
  }
}

function UploadPreview({ f }: { f: AdminUpload }) {
  const [src, setSrc] = useState("");
  useEffect(() => {
    let live = true;
    api
      .adminFile(f.id)
      .then((r) => live && setSrc(`data:${r.mime};base64,${r.data}`))
      .catch(() => {});
    return () => {
      live = false;
    };
  }, [f.id]);
  return src ? <img src={src} alt={f.name} className="upload-preview" /> : <span className="spinner tiny" />;
}

function Uploads({ picked, notify }: { picked: Picked; notify?: (msg: string) => void }) {
  const [rows, setRows] = useState<AdminUpload[] | null>(null);
  const [total, setTotal] = useState(0);
  const [kind, setKind] = useState("");
  const [offset, setOffset] = useState(0);
  const [error, setError] = useState("");
  const [open, setOpen] = useState<number | null>(null);
  const tell = notify ?? (() => {});

  useEffect(() => {
    setOffset(0);
  }, [kind, picked]);

  useEffect(() => {
    let live = true;
    setRows(null);
    setError("");
    api
      .adminFiles({ user: picked?.id, kind: kind || undefined, offset })
      .then((r) => {
        if (!live) return;
        setRows(r.files);
        setTotal(r.total);
      })
      .catch((e) => live && setError(errText(e, t("بارگذاری فایل‌ها ناموفق بود.", "Could not load files."))));
    return () => {
      live = false;
    };
  }, [kind, picked, offset]);

  return (
    <section className="card panel">
      <div className="panel-head">
        <h3>{t("فایل‌های بارگذاری‌شده", "Uploaded files")}{picked ? t(" این کاربر", " for this user") : ""}</h3>
        <Segmented
          label={t("نوع", "Type")}
          options={[
            { id: "", label: t("همه", "All") },
            { id: "image", label: t("تصویر", "Image") },
            { id: "text", label: t("متن و کد", "Text and code") },
            { id: "archive", label: "ZIP" },
          ]}
          value={kind}
          onChange={setKind}
        />
      </div>
      {error ? (
        <div className="form-error" role="alert">
          {error}
        </div>
      ) : rows === null ? (
        <div className="list-loading" role="status">
          <span className="spinner" />
          <span className="sr-only">{t("در حال بارگذاری", "Loading")}</span>
        </div>
      ) : (
        <>
          <div className="table-wrap scroll-y">
            <table className="data-table">
              <thead>
                <tr>
                  <th scope="col">{t("فایل", "File")}</th>
                  <th scope="col">{t("نوع", "Type")}</th>
                  <th scope="col">{t("حجم", "Size")}</th>
                  <th scope="col">{t("کاربر", "User")}</th>
                  <th scope="col">{t("از", "Client")}</th>
                  <th scope="col">{t("زمان", "Time")}</th>
                  <th scope="col">{t("مسیر روی سرور", "Server path")}</th>
                  <th scope="col">
                    <span className="sr-only">{t("کارها", "Actions")}</span>
                  </th>
                </tr>
              </thead>
              <tbody>
                {rows.map((f) => (
                  <Fragment key={f.id}>
                    <tr className={open === f.id ? "selected" : ""}>
                      <td className="cell-clip">
                        <button type="button" className="link" onClick={() => setOpen((o) => (o === f.id ? null : f.id))} aria-expanded={open === f.id} title={f.name}>
                          <bdi>{f.name}</bdi>
                        </button>
                      </td>
                      <td>{KIND_LABEL[f.kind] ?? f.kind}</td>
                      <td>{fmtSize(f.size)}</td>
                      <td title={f.user.email}>
                        <bdi>{f.user.name}</bdi>
                      </td>
                      <td>{f.agent ? t("ایجنت", "Agent") : t("وب", "Web")}</td>
                      <td title={fmtDateTime(f.created)}>{fmtRelative(f.created)}</td>
                      <td dir="ltr" className="cell-ltr cell-clip" title={f.path}>
                        {f.path}
                      </td>
                      <td className="cell-actions">
                        <div className="row-actions">
                          <button type="button" className="button ghost small" onClick={() => saveUpload(f, tell)}>
                            {t("دریافت", "Download")}
                          </button>
                          <a className="button ghost small" href={`/admin/chats/${f.chat_id}`} target="_blank" rel="noopener">
                            {t("گفت‌وگو", "Chat")}
                          </a>
                        </div>
                      </td>
                    </tr>
                    {open === f.id && (
                      <tr>
                        <td colSpan={8}>
                          {f.kind === "image" ? (
                            <UploadPreview f={f} />
                          ) : f.kind === "archive" ? (
                            <pre dir="ltr" className="upload-listing">
                              {f.listing || t("(بدون فایل متنی)", "(no text files)")}
                            </pre>
                          ) : (
                            <p className="muted">{t("برای دیدن محتوا فایل را دریافت کنید.", "Download the file to see its contents.")}</p>
                          )}
                        </td>
                      </tr>
                    )}
                  </Fragment>
                ))}
                {rows.length === 0 && (
                  <tr>
                    <td colSpan={8} className="empty-cell">
                      {t("فایلی بارگذاری نشده است.", "No files uploaded yet.")}
                    </td>
                  </tr>
                )}
              </tbody>
            </table>
          </div>
          <div className="pager">
            <span className="muted">
              {fmtNum(total === 0 ? 0 : offset + 1)} {t("تا", "to")} {fmtNum(Math.min(total, offset + rows.length))} {t("از", "of")} {fmtNum(total)}
            </span>
            <div>
              <button type="button" className="button ghost small" disabled={offset === 0} onClick={() => setOffset(Math.max(0, offset - PAGE_SIZE))}>
                {t("قبلی", "Previous")}
              </button>
              <button type="button" className="button ghost small" disabled={offset + PAGE_SIZE >= total} onClick={() => setOffset(offset + PAGE_SIZE)}>
                {t("بعدی", "Next")}
              </button>
            </div>
          </div>
        </>
      )}
    </section>
  );
}

export function NoAccess() {
  return (
    <div className="page">
      <section className="card empty-card">
        <div className="notice-icon info">
          <IconShield />
        </div>
        <h3>{t("دسترسی به این بخش ندارید", "You do not have access to this section")}</h3>
        <p className="muted">{t("این صفحه فقط برای مدیران دستیار سلام است. اگر فکر می‌کنید باید دسترسی داشته باشید، با مدیر سایت تماس بگیرید.", "This page is only for Salam AI Assistant admins. If you think you should have access, contact the site admin.")}</p>
        <a className="button ghost" href="/">
          {t("بازگشت به گفت‌وگوها", "Back to chats")}
        </a>
      </section>
    </div>
  );
}

export function Admin({ notify }: { notify: (msg: string) => void }) {
  const [summary, setSummary] = useState<AdminSummary | null>(null);
  const [error, setError] = useState("");
  const [forbidden, setForbidden] = useState(false);
  const [picked, setPicked] = useState<Picked>(null);

  const load = useCallback(() => {
    setError("");
    api
      .adminSummary()
      .then(setSummary)
      .catch((e) => {
        if (e instanceof ApiError && e.status === 403) setForbidden(true);
        else setError(errText(e, t("بارگذاری آمار ناموفق بود.", "Could not load stats.")));
      });
  }, []);

  useEffect(() => {
    load();
  }, [load]);

  if (forbidden) return <NoAccess />;

  const s = summary;
  const errRate = s && s.mcp_calls_24h > 0 ? Math.round((s.mcp_errors_24h / s.mcp_calls_24h) * 1000) / 10 : 0;
  const apiErrRate = s && s.api_requests_24h > 0 ? Math.round((s.api_errors_24h / s.api_requests_24h) * 1000) / 10 : 0;

  return (
    <div className="page">
      <header className="page-head">
        <h2>{t("مدیریت", "Admin")}</h2>
        <p className="muted">{t("نمای کلی استفاده از دستیار و سرور MCP سلام.", "An overview of how Salam AI Assistant and the MCP server are used.")}</p>
      </header>
      {error && (
        <div className="form-error" role="alert">
          {error}{" "}
          <button type="button" className="link" onClick={load}>
            {t("تلاش دوباره", "Try again")}
          </button>
        </div>
      )}
      {!s && !error && (
        <div className="loading-chat" role="status">
          <span className="spinner" />
          <span className="sr-only">{t("در حال بارگذاری", "Loading")}</span>
        </div>
      )}
      {s && (
        <>
          <div className="tiles">
            <Tile label={t("کاربران", "Users")} value={s.users} sub={t(`${fmtNum(s.users_7d)} در ۷ روز اخیر`, `${fmtNum(s.users_7d)} in the last 7 days`)} />
            <Tile label={t("گفت‌وگوها", "Chats")} value={s.chats} sub={t(`${fmtNum(s.messages)} پیام`, `${fmtNum(s.messages)} messages`)} />
            <Tile label={t("مهمان‌های سایت", "Website guests")} value={s.guests ?? 0} sub={t(`${fmtNum(s.guest_messages ?? 0)} پیام، ${fmtNum(s.guests_at_limit ?? 0)} به سقف رسیدند`, `${fmtNum(s.guest_messages ?? 0)} messages, ${fmtNum(s.guests_at_limit ?? 0)} hit the limit`)} />
            <Tile label={t("فایل‌های بارگذاری‌شده", "Uploaded files")} value={s.uploads ?? 0} sub={t(`${fmtNum(s.uploads_images ?? 0)} تصویر، ${fmtSize(s.uploads_bytes ?? 0)}`, `${fmtNum(s.uploads_images ?? 0)} images, ${fmtSize(s.uploads_bytes ?? 0)}`)} />
            <Tile label={t("برنامه‌های متصل", "Connected apps")} value={s.apps} />
            <Tile label={t("فراخوانی‌های MCP", "MCP calls")} value={s.mcp_calls} sub={t(`${fmtNum(s.mcp_calls_24h)} در ۲۴ ساعت اخیر`, `${fmtNum(s.mcp_calls_24h)} in the last 24 hours`)} />
            <Tile label={t("خطاهای MCP در ۲۴ ساعت", "MCP errors in 24 hours")} value={s.mcp_errors_24h} sub={t(`${fmtNum(errRate)}٪ از فراخوانی‌ها`, `${fmtNum(errRate)}% of calls`)} tone={s.mcp_errors_24h > 0 ? "danger" : undefined} />
            <Tile label={t("درخواست‌های API", "API requests")} value={s.api_requests} />
            <Tile label={t("درخواست‌های API در ۲۴ ساعت", "API requests in 24 hours")} value={s.api_requests_24h} />
            <Tile label={t("خطاهای API در ۲۴ ساعت", "API errors in 24 hours")} value={s.api_errors_24h} sub={t(`${fmtNum(apiErrRate)}٪ از درخواست‌ها`, `${fmtNum(apiErrRate)}% of requests`)} tone={s.api_errors_24h > 0 ? "danger" : undefined} />
          </div>
          {s.usage && s.usage.length > 0 && (
            <>
              <UsageTiles usage={s.usage} />
              <UsagePanel usage={s.usage} />
            </>
          )}
          <DailyPanel daily={s.daily} />
          <div className="split three">
            <TopList title={t("ابزارهای پرکاربرد", "Top tools")} items={s.top_tools.map((t) => ({ name: t.tool, count: t.count }))} ltr />
            <TopList title={t("برنامه‌های پرکاربرد", "Top apps")} items={s.top_clients} />
            <TopList title={t("پروژه‌های پرکاربرد API", "Top API projects")} items={s.top_projects} />
          </div>
        </>
      )}
      {!forbidden && (
        <>
          <Users picked={picked} onPick={setPicked} notify={notify} />
          <TelegramSessions picked={picked} onPick={setPicked} />
          {picked && (
            <div className="picked-bar" role="status">
              {t("نمایش رویدادها، درخواست‌های API و برنامه‌های", "Showing events, API requests and apps for")} <bdi className="strong">{picked.name}</bdi>
              <button type="button" className="link" onClick={() => setPicked(null)}>
                {t("نمایش همه", "Show all")}
              </button>
            </div>
          )}
          <Sessions picked={picked} notify={notify} />
          {!picked && <Guests />}
          <Conversations picked={picked} />
          <Subscriptions picked={picked} notify={notify} />
          <Payments picked={picked} />
          <Uploads picked={picked} notify={notify} />
          <Events picked={picked} onPick={setPicked} />
          <RequestsFeed picked={picked} onPick={setPicked} />
          <KeyPolicies picked={picked} notify={notify} />
          <AppsTable picked={picked} notify={notify} />
        </>
      )}
    </div>
  );
}
