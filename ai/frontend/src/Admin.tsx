import { useCallback, useEffect, useLayoutEffect, useRef, useState, type FormEvent } from "react";
import { api, ApiError, PAGE_SIZE, type AdminApp, type AdminSubscription, type AdminSummary, type AdminUser, type ApiRequest, type AppEvent, type EventFilters, type Payment, type PlanId, type RequestFilters, type UsagePeriod, type UsagePeriodKey, type UsageTotals } from "./api";
import { EventList } from "./Events";
import { fmtCompact, fmtCost, fmtDate, fmtDateTime, fmtDay, fmtMonth, fmtNum, fmtRelative, tokenSplit } from "./format";
import { IconBadgeCheck, IconChevronDown, IconClose, IconSearch, IconShield } from "./icons";
import { RequestList } from "./Requests";
import { confirmDialog, Subject } from "./Confirm";

type Picked = { id: number; name: string } | null;
type UsageMode = "all" | "chat" | "api";
type ChartMode = "calls" | "tokens";

const USAGE_MODES: { id: UsageMode; label: string }[] = [
  { id: "all", label: "همه" },
  { id: "chat", label: "گفت‌وگو" },
  { id: "api", label: "API" },
];

const CHART_MODES: { id: ChartMode; label: string }[] = [
  { id: "calls", label: "فراخوانی‌ها" },
  { id: "tokens", label: "توکن‌ها" },
];

const PERIOD_LABELS: Record<UsagePeriodKey, string> = {
  today: "امروز",
  week: "این هفته",
  month: "این ماه",
  last_month: "ماه قبل",
  all: "کل",
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
    const t = tokenSplit(pickUsage(p, "all"));
    return <Tile key={p.key} label={label} value={t.total} compact sub={`${fmtCompact(t.input)} ورودی، ${fmtCompact(t.output)} خروجی`} />;
  };
  const monthCost = month ? pickUsage(month, "all").cost : 0;
  return (
    <div className="tiles">
      {tile("توکن‌های امروز", today)}
      {tile(month ? `توکن‌های ${fmtMonth(month.from)}` : "توکن‌های این ماه", month)}
      {tile("توکن‌های کل", all)}
      {month && monthCost > 0 && <Tile label="هزینه‌ی این ماه" value={monthCost} display={fmtCost(monthCost)} sub={fmtMonth(month.from)} />}
    </div>
  );
}

function UsagePanel({ usage }: { usage: UsagePeriod[] }) {
  const [mode, setMode] = useState<UsageMode>("all");
  const rows = usage.map((p) => ({ p, t: pickUsage(p, mode) }));
  const showCost = rows.some((r) => r.t.cost > 0);
  const countLabel = mode === "chat" ? "پاسخ" : mode === "api" ? "درخواست" : "پاسخ و درخواست";
  return (
    <section className="card panel">
      <div className="panel-head">
        <h3>مصرف توکن</h3>
        <Segmented label="منبع مصرف" options={USAGE_MODES} value={mode} onChange={setMode} />
      </div>
      <div className="table-wrap">
        <table className="data-table">
          <thead>
            <tr>
              <th scope="col">دوره</th>
              <th scope="col">{countLabel}</th>
              <th scope="col">ورودی</th>
              <th scope="col">خروجی</th>
              <th scope="col">کش</th>
              <th scope="col">جمع</th>
              {showCost && <th scope="col">هزینه</th>}
            </tr>
          </thead>
          <tbody>
            {rows.map(({ p, t }) => {
              const sp = tokenSplit(t);
              const sub = periodSub(p);
              return (
                <tr key={p.key}>
                  <td title={p.key === "all" ? undefined : `${fmtDateTime(p.from)} تا ${fmtDateTime(p.to)}`}>
                    <span className="strong">{PERIOD_LABELS[p.key] || p.key}</span>
                    {sub && <span className="cell-sub">{sub}</span>}
                  </td>
                  <td>{fmtNum(t.count)}</td>
                  <CompactCell n={sp.input} />
                  <CompactCell n={sp.output} />
                  <CompactCell n={sp.cache} />
                  <td className="strong" title={exactTitle(sp.total)}>
                    {fmtCompact(sp.total)}
                  </td>
                  {showCost && <td>{t.cost > 0 ? fmtCost(t.cost) : <span className="muted">-</span>}</td>}
                </tr>
              );
            })}
          </tbody>
        </table>
      </div>
      <p className="muted small usage-note">
        ورودی شامل توکن‌های کش هم هست و ستون کش همان بخش را جدا نشان می‌دهد. خروجی شامل توکن‌های استدلال است. «همه» جمع گفت‌وگوهای سایت و درخواست‌های API است.
      </p>
    </section>
  );
}

function DailyPanel({ daily }: { daily: AdminSummary["daily"] }) {
  const [mode, setMode] = useState<ChartMode>("calls");
  return (
    <section className="card panel">
      <div className="panel-head">
        <h3>{mode === "tokens" ? "مصرف توکن در ۱۴ روز گذشته" : "فراخوانی‌های MCP و درخواست‌های API در ۱۴ روز گذشته"}</h3>
        <Segmented label="داده‌ی نمودار" options={CHART_MODES} value={mode} onChange={setMode} />
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
            ? `نمودار ${fmtNum(daily.length)} روز گذشته: روی هم ${fmtNum(totalA)} توکن ورودی و ${fmtNum(totalB)} توکن خروجی`
            : `نمودار ${fmtNum(daily.length)} روز گذشته: روی هم ${fmtNum(totalA)} فراخوانی MCP و ${fmtNum(totalB)} درخواست API`
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
            ? `${fmtDay(d.day)}: ${fmtNum(va(d))} توکن ورودی، ${fmtNum(vb(d))} توکن خروجی`
            : `${fmtDay(d.day)}: ${fmtNum(d.calls)} فراخوانی MCP، ${fmtNum(d.api)} درخواست API، ${fmtNum(d.users)} کاربر`;
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
                {fmtNum(va(hovered))} توکن ورودی
              </span>
              <span>
                <i className={`swatch ${cb}`} />
                {fmtNum(vb(hovered))} توکن خروجی
              </span>
            </>
          ) : (
            <>
              <span>
                <i className="swatch mcp" />
                {fmtNum(hovered.calls)} فراخوانی MCP
              </span>
              <span>
                <i className="swatch api" />
                {fmtNum(hovered.api)} درخواست API
              </span>
              <span>{fmtNum(hovered.users)} کاربر</span>
            </>
          )}
        </div>
      )}
      <ul className="chart-legend">
        <li>
          <span className={`swatch ${ca}`} aria-hidden="true" />
          {tokens ? "توکن‌های ورودی" : "فراخوانی‌های MCP"}
        </li>
        <li>
          <span className={`swatch ${cb}`} aria-hidden="true" />
          {tokens ? "توکن‌های خروجی" : "درخواست‌های API"}
        </li>
      </ul>
      <details className="chart-table">
        <summary>نمایش جدول داده‌ها</summary>
        <div className="table-wrap">
          <table className="data-table">
            <thead>
              {tokens ? (
                <tr>
                  <th scope="col">روز</th>
                  <th scope="col">توکن ورودی</th>
                  <th scope="col">توکن خروجی</th>
                  <th scope="col">جمع</th>
                </tr>
              ) : (
                <tr>
                  <th scope="col">روز</th>
                  <th scope="col">فراخوانی MCP</th>
                  <th scope="col">درخواست API</th>
                  <th scope="col">کاربر</th>
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
        <p className="empty-list">هنوز داده‌ای نیست.</p>
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

function Users({ picked, onPick }: { picked: Picked; onPick: (p: Picked) => void }) {
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
      .catch((e) => live && setError(errText(e, "بارگذاری کاربران ناموفق بود.")))
      .finally(() => live && setLoading(false));
    return () => {
      live = false;
    };
  }, [query, offset]);

  const total = data?.total ?? 0;
  const from = total === 0 ? 0 : offset + 1;
  const to = Math.min(total, offset + (data?.users.length ?? 0));

  return (
    <section className="card panel">
      <div className="panel-head">
        <h3>کاربران</h3>
        <label className="search">
          <IconSearch width={16} height={16} />
          <span className="sr-only">جست‌وجوی کاربر</span>
          <input type="search" value={q} onChange={(e) => setQ(e.target.value)} placeholder="جست‌وجوی نام یا ایمیل" />
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
              <th scope="col">نام</th>
              <th scope="col">ایمیل</th>
              <th scope="col">عضویت</th>
              <th scope="col">گفت‌وگو</th>
              <th scope="col">پیام</th>
              <th scope="col">برنامه</th>
              <th scope="col">فراخوانی MCP</th>
              <th scope="col">API</th>
              <th scope="col">توکن</th>
              <th scope="col">آخرین ورود</th>
              <th scope="col">آخرین فعالیت</th>
            </tr>
          </thead>
          <tbody>
            {data?.users.map((u) => (
              <tr key={u.id} className={picked?.id === u.id ? "selected" : ""}>
                <td>
                  <button type="button" className="link" onClick={() => onPick(picked?.id === u.id ? null : { id: u.id, name: u.name })} aria-pressed={picked?.id === u.id} title="نمایش فعالیت، درخواست‌ها و برنامه‌های این کاربر">
                    <bdi>{u.name}</bdi>
                  </button>
                </td>
                <td dir="ltr" className="cell-ltr">
                  {u.email}
                </td>
                <td title={fmtDateTime(u.created)}>{fmtDate(u.created)}</td>
                <td>{fmtNum(u.chats)}</td>
                <td>{fmtNum(u.messages)}</td>
                <td>{fmtNum(u.apps)}</td>
                <td>{fmtNum(u.mcp_calls)}</td>
                <td>{fmtNum(u.api_requests)}</td>
                <td title={exactTitle(u.tokens || 0)}>{fmtCompact(u.tokens || 0)}</td>
                <td title={u.last_login ? `${fmtDateTime(u.last_login)}، روی هم ${fmtNum(u.logins || 0)} ورود` : undefined}>{fmtRelative(u.last_login || 0)}</td>
                <td title={u.last_active ? fmtDateTime(u.last_active) : undefined}>{fmtRelative(u.last_active)}</td>
              </tr>
            ))}
            {data && data.users.length === 0 && (
              <tr>
                <td colSpan={11} className="empty-cell">
                  کاربری پیدا نشد.
                </td>
              </tr>
            )}
          </tbody>
        </table>
      </div>
      <div className="pager">
        <span className="muted">
          {fmtNum(from)} تا {fmtNum(to)} از {fmtNum(total)}
        </span>
        <div>
          <button type="button" className="button ghost small" disabled={offset === 0 || loading} onClick={() => setOffset(Math.max(0, offset - PAGE_SIZE))}>
            قبلی
          </button>
          <button type="button" className="button ghost small" disabled={offset + PAGE_SIZE >= total || loading} onClick={() => setOffset(offset + PAGE_SIZE)}>
            بعدی
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
        if (id === seq.current) setError(errText(e, "بارگذاری رویدادها ناموفق بود."));
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
        <h3>رویدادها</h3>
      </div>
      <form className="filters" onSubmit={submit}>
        {picked && (
          <span className="chip">
            کاربر: <bdi>{picked.name}</bdi>
            <button type="button" className="icon-button tiny" onClick={() => onPick(null)} aria-label="برداشتن فیلتر کاربر">
              <IconClose width={14} height={14} />
            </button>
          </span>
        )}
        <label className="filter-field">
          <span className="sr-only">نام ابزار</span>
          <input dir="ltr" value={tool} onChange={(e) => setTool(e.target.value)} placeholder="tool" />
        </label>
        <label className="filter-field">
          <span className="sr-only">نام برنامه</span>
          <input dir="auto" value={client} onChange={(e) => setClient(e.target.value)} placeholder="نام برنامه" />
        </label>
        <button type="submit" className="button ghost small">
          اعمال فیلتر
        </button>
        {(applied.tool || applied.client) && (
          <button type="button" className="link" onClick={clear}>
            پاک کردن
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
        empty="رویدادی با این فیلترها پیدا نشد."
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
        if (id === seq.current) setError(errText(e, "بارگذاری درخواست‌ها ناموفق بود."));
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
        <h3>درخواست‌های API</h3>
      </div>
      <form className="filters" onSubmit={submit}>
        {picked && (
          <span className="chip">
            کاربر: <bdi>{picked.name}</bdi>
            <button type="button" className="icon-button tiny" onClick={() => onPick(null)} aria-label="برداشتن فیلتر کاربر">
              <IconClose width={14} height={14} />
            </button>
          </span>
        )}
        <label className="filter-field">
          <span className="sr-only">نام پروژه</span>
          <input dir="auto" value={project} onChange={(e) => setProject(e.target.value)} placeholder="نام پروژه" />
        </label>
        <label className="filter-field">
          <span className="sr-only">شناسه‌ی کلید</span>
          <input dir="ltr" inputMode="numeric" value={keyId} onChange={(e) => setKeyId(e.target.value.replace(/[۰-۹]/g, (d) => String("۰۱۲۳۴۵۶۷۸۹".indexOf(d))).replace(/[^0-9]/g, ""))} placeholder="شناسه‌ی کلید" />
        </label>
        <button type="submit" className="button ghost small">
          اعمال فیلتر
        </button>
        {(applied.project || applied.key) && (
          <button type="button" className="link" onClick={clear}>
            پاک کردن
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
        empty="درخواستی با این فیلترها پیدا نشد."
      />
    </section>
  );
}

const PLAN_LABELS: { id: PlanId; label: string }[] = [
  { id: "free", label: "رایگان" },
  { id: "plus", label: "پلاس" },
  { id: "pro", label: "حرفه‌ای" },
  { id: "unlimited", label: "نامحدود" },
];

const PAY_WORDS: Record<Payment["status"], string> = {
  pending: "در انتظار",
  verifying: "در حال تایید",
  paid: "پرداخت شده",
  failed: "ناموفق",
  canceled: "لغو شده",
};

const GATEWAY_WORDS: Record<string, string> = { zarinpal: "زرین‌پال", zibal: "زیبال" };

function toman(n: number): string {
  return `${fmtNum(n)} تومان`;
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
      setError(errText(e, "بارگذاری اشتراک‌ها ناموفق بود."));
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
      notify("اشتراک داده شد.");
      setEmail("");
      setNote("");
      setDays("");
      await load();
    } catch (err) {
      notify(errText(err, "دادن اشتراک ناموفق بود."));
    } finally {
      setGranting(false);
    }
  };

  const cancel = async (row: AdminSubscription) => {
    const ok = await confirmDialog({
      title: "لغو اشتراک",
      subject: row.user?.email ?? row.plan_name,
      message: (
        <>
          اشتراک <Subject>{row.plan_name}</Subject> این کاربر لغو می‌شود و به پلن رایگان برمی‌گردد.
        </>
      ),
      confirmLabel: "لغو اشتراک",
      icon: "alert",
    });
    if (!ok) return;
    setBusy(row.id);
    try {
      await api.adminCancelSubscription(row.id);
      notify("اشتراک لغو شد.");
      await load();
    } catch (e) {
      notify(errText(e, "لغو اشتراک ناموفق بود."));
    } finally {
      setBusy(null);
    }
  };

  return (
    <section className="card panel">
      <div className="panel-head">
        <div className="panel-title">
          <h3>اشتراک‌ها{picked ? " این کاربر" : ""}</h3>
          {!picked && active > 0 && <span className="badge ok">{fmtNum(active)} اشتراک فعال</span>}
        </div>
        <Segmented
          label="نما"
          options={[
            { id: "active", label: "فعال" },
            { id: "latest", label: "آخرین اشتراک هر کاربر" },
          ]}
          value={latest ? "latest" : "active"}
          onChange={(v) => setLatest(v === "latest")}
        />
      </div>
      <form className="grant" onSubmit={grant}>
        <div className="grant-head">
          <h4>دادن اشتراک رایگان</h4>
          <p className="muted">اشتراک فعال کاربر جایگزین می‌شود و اگر همین پلن را داشته باشد، به مدت باقی‌مانده‌اش اضافه می‌شود.</p>
        </div>
        <div className="grant-grid">
          <label className="grant-field">
            <span>ایمیل کاربر</span>
            <input type="email" dir="ltr" value={email} onChange={(e) => setEmail(e.target.value)} placeholder="user@example.com" required />
          </label>
          <label className="grant-field">
            <span>پلن</span>
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
              تعداد روز <span className="opt">اختیاری</span>
            </span>
            <input type="number" inputMode="numeric" min="1" max="3650" value={days} onChange={(e) => setDays(e.target.value)} placeholder="پیش‌فرض پلن" />
          </label>
          <label className="grant-field">
            <span>
              یادداشت <span className="opt">اختیاری</span>
            </span>
            <input type="text" value={note} onChange={(e) => setNote(e.target.value)} placeholder="چرا رایگان داده شد" />
          </label>
        </div>
        <div className="grant-actions">
          <button type="submit" className="button primary grant-go" disabled={granting}>
            {granting ? <span className="spinner" /> : <IconBadgeCheck width={18} height={18} />}
            دادن اشتراک
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
          <span className="sr-only">در حال بارگذاری</span>
        </div>
      ) : (
        <div className="table-wrap scroll-y">
          <table className="data-table">
            <thead>
              <tr>
                <th scope="col">کاربر</th>
                <th scope="col">پلن</th>
                <th scope="col">از</th>
                <th scope="col">تا</th>
                <th scope="col">راه</th>
                <th scope="col">مبلغ</th>
                <th scope="col">وضعیت</th>
                <th scope="col">یادداشت</th>
                <th scope="col">
                  <span className="sr-only">کارها</span>
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
                  <td>{r.expires === 0 ? "بدون پایان" : fmtDate(r.expires)}</td>
                  <td>{r.source === "admin" ? "هدیه‌ی مدیر" : "پرداخت"}</td>
                  <td>{r.price === 0 ? "-" : toman(r.price)}</td>
                  <td>
                    <span className={`badge ${r.status === "active" ? "ok" : ""}`}>{r.status === "active" ? "فعال" : r.status === "canceled" ? "لغو شده" : "جایگزین شده"}</span>
                  </td>
                  <td>
                    <bdi>{r.note}</bdi>
                  </td>
                  <td>
                    {r.status === "active" && (
                      <button type="button" className="link danger" disabled={busy === r.id} onClick={() => void cancel(r)}>
                        لغو
                      </button>
                    )}
                  </td>
                </tr>
              ))}
              {rows?.length === 0 && (
                <tr>
                  <td colSpan={9} className="muted">
                    هنوز اشتراکی ثبت نشده است.
                  </td>
                </tr>
              )}
            </tbody>
          </table>
        </div>
      )}
      <div className="panel-foot muted">مجموع پرداخت‌های موفق: {toman(income)}</div>
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
      .catch((e) => live && setError(errText(e, "بارگذاری پرداخت‌ها ناموفق بود.")));
    return () => {
      live = false;
    };
  }, [status, picked]);

  return (
    <section className="card panel">
      <div className="panel-head">
        <h3>پرداخت‌ها{picked ? " این کاربر" : ""}</h3>
        <Segmented
          label="وضعیت"
          options={[
            { id: "", label: "همه" },
            { id: "paid", label: "پرداخت شده" },
            { id: "pending", label: "در انتظار" },
            { id: "failed", label: "ناموفق" },
            { id: "canceled", label: "لغو شده" },
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
          <span className="sr-only">در حال بارگذاری</span>
        </div>
      ) : (
        <div className="table-wrap scroll-y">
          <table className="data-table">
            <thead>
              <tr>
                <th scope="col">کاربر</th>
                <th scope="col">پلن</th>
                <th scope="col">مبلغ</th>
                <th scope="col">درگاه</th>
                <th scope="col">وضعیت</th>
                <th scope="col">کد پیگیری</th>
                <th scope="col">کارت</th>
                <th scope="col">زمان</th>
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
                    پرداختی با این فیلتر نیست.
                  </td>
                </tr>
              )}
            </tbody>
          </table>
        </div>
      )}
      <div className="panel-foot muted">مجموع پرداخت‌های موفق: {toman(income)}</div>
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
      .catch((e) => live && setError(errText(e, "بارگذاری برنامه‌ها ناموفق بود.")));
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
      notify(errText(e, "بارگذاری برنامه‌ها ناموفق بود."));
    } finally {
      setLoadingMore(false);
    }
  };

  const revoke = async (a: AdminApp) => {
    const ok = await confirmDialog({
      title: "قطع اتصال برنامه",
      subject: a.client.name,
      message: (
        <>
          اتصال این برنامه برای <Subject>{a.user.name}</Subject> قطع می‌شود.
        </>
      ),
      confirmLabel: "قطع اتصال",
      icon: "alert",
    });
    if (!ok) return;
    setBusy(a.id);
    try {
      await api.adminRevokeApp(a.id);
      setApps((cur) => (cur ? cur.map((x) => (x.id === a.id ? { ...x, revoked: true } : x)) : cur));
      notify("اتصال قطع شد.");
    } catch (e) {
      notify(errText(e, "قطع اتصال ناموفق بود."));
    } finally {
      setBusy(null);
    }
  };

  return (
    <section className="card panel">
      <div className="panel-head">
        <h3>برنامه‌های متصل{picked ? " این کاربر" : ""}</h3>
      </div>
      {error && (
        <div className="form-error" role="alert">
          {error}
        </div>
      )}
      {apps === null && !error ? (
        <div className="list-loading" role="status">
          <span className="spinner" />
          <span className="sr-only">در حال بارگذاری</span>
        </div>
      ) : (
        <div className="table-wrap scroll-y">
          <table className="data-table">
            <thead>
              <tr>
                <th scope="col">کاربر</th>
                <th scope="col">برنامه</th>
                <th scope="col">میزبان</th>
                <th scope="col">دسترسی</th>
                <th scope="col">اتصال</th>
                <th scope="col">آخرین استفاده</th>
                <th scope="col">فراخوانی</th>
                <th scope="col">وضعیت</th>
                <th scope="col">
                  <span className="sr-only">کارها</span>
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
                    <span className="cell-sub">{a.client.kind === "cimd" ? "تأییدشده" : "تأییدنشده"}</span>
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
                  <td>{a.revoked ? <span className="badge">قطع‌شده</span> : <span className="badge ok">فعال</span>}</td>
                  <td>
                    {!a.revoked && (
                      <button type="button" className="button danger small" disabled={busy === a.id} onClick={() => revoke(a)} aria-label={`قطع اتصال ${a.client.name} برای ${a.user.name}`}>
                        {busy === a.id ? <span className="spinner" /> : "قطع اتصال"}
                      </button>
                    )}
                  </td>
                </tr>
              ))}
              {apps && apps.length === 0 && (
                <tr>
                  <td colSpan={9} className="empty-cell">
                    برنامه‌ی متصلی نیست.
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
          <span className="sr-only">در حال بارگذاری</span>
        </div>
      )}
      {more && !loadingMore && (
        <button type="button" className="button ghost more" onClick={loadMore}>
          نمایش موارد بیشتر
        </button>
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
        <h3>دسترسی به این بخش ندارید</h3>
        <p className="muted">این صفحه فقط برای مدیران دستیار سلام است. اگر فکر می‌کنید باید دسترسی داشته باشید، با مدیر سایت تماس بگیرید.</p>
        <a className="button ghost" href="/">
          بازگشت به گفت‌وگوها
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
        else setError(errText(e, "بارگذاری آمار ناموفق بود."));
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
        <h2>مدیریت</h2>
        <p className="muted">نمای کلی استفاده از دستیار و سرور MCP سلام.</p>
      </header>
      {error && (
        <div className="form-error" role="alert">
          {error}{" "}
          <button type="button" className="link" onClick={load}>
            تلاش دوباره
          </button>
        </div>
      )}
      {!s && !error && (
        <div className="loading-chat" role="status">
          <span className="spinner" />
          <span className="sr-only">در حال بارگذاری</span>
        </div>
      )}
      {s && (
        <>
          <div className="tiles">
            <Tile label="کاربران" value={s.users} sub={`${fmtNum(s.users_7d)} در ۷ روز اخیر`} />
            <Tile label="گفت‌وگوها" value={s.chats} sub={`${fmtNum(s.messages)} پیام`} />
            <Tile label="برنامه‌های متصل" value={s.apps} />
            <Tile label="فراخوانی‌های MCP" value={s.mcp_calls} sub={`${fmtNum(s.mcp_calls_24h)} در ۲۴ ساعت اخیر`} />
            <Tile label="خطاهای MCP در ۲۴ ساعت" value={s.mcp_errors_24h} sub={`${fmtNum(errRate)}٪ از فراخوانی‌ها`} tone={s.mcp_errors_24h > 0 ? "danger" : undefined} />
            <Tile label="درخواست‌های API" value={s.api_requests} />
            <Tile label="درخواست‌های API در ۲۴ ساعت" value={s.api_requests_24h} />
            <Tile label="خطاهای API در ۲۴ ساعت" value={s.api_errors_24h} sub={`${fmtNum(apiErrRate)}٪ از درخواست‌ها`} tone={s.api_errors_24h > 0 ? "danger" : undefined} />
          </div>
          {s.usage && s.usage.length > 0 && (
            <>
              <UsageTiles usage={s.usage} />
              <UsagePanel usage={s.usage} />
            </>
          )}
          <DailyPanel daily={s.daily} />
          <div className="split three">
            <TopList title="ابزارهای پرکاربرد" items={s.top_tools.map((t) => ({ name: t.tool, count: t.count }))} ltr />
            <TopList title="برنامه‌های پرکاربرد" items={s.top_clients} />
            <TopList title="پروژه‌های پرکاربرد API" items={s.top_projects} />
          </div>
        </>
      )}
      {!forbidden && (
        <>
          <Users picked={picked} onPick={setPicked} />
          {picked && (
            <div className="picked-bar" role="status">
              نمایش رویدادها، درخواست‌های API و برنامه‌های <bdi className="strong">{picked.name}</bdi>
              <button type="button" className="link" onClick={() => setPicked(null)}>
                نمایش همه
              </button>
            </div>
          )}
          <Subscriptions picked={picked} notify={notify} />
          <Payments picked={picked} />
          <Events picked={picked} onPick={setPicked} />
          <RequestsFeed picked={picked} onPick={setPicked} />
          <AppsTable picked={picked} notify={notify} />
        </>
      )}
    </div>
  );
}
