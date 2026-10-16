import { useCallback, useEffect, useState } from "react";
import { api, ApiError, PAGE_SIZE, type AppEvent, type ConnectedApp } from "./api";
import { EventList } from "./Events";
import { fmtDate, fmtDateTime, fmtNum, fmtRelative, initial } from "./format";
import { IconActivity, IconBadgeCheck, IconChevronDown, IconPlug } from "./icons";
import { CopyButton } from "./Markdown";
import { confirmDialog } from "./Confirm";

export const MCP_COMMAND = "claude mcp add --transport http salam https://mcp.salamlang.ir/mcp";

function AppActivity({ appId }: { appId: number }) {
  const [events, setEvents] = useState<AppEvent[]>([]);
  const [loading, setLoading] = useState(true);
  const [hasMore, setHasMore] = useState(false);
  const [error, setError] = useState("");

  const load = useCallback(
    async (before?: number) => {
      setLoading(true);
      setError("");
      try {
        const r = await api.appEvents(appId, before);
        setEvents((cur) => (before ? [...cur, ...r.events] : r.events));
        setHasMore(r.events.length >= PAGE_SIZE);
      } catch (e) {
        setError(e instanceof ApiError ? e.message : "بارگذاری فعالیت‌ها ناموفق بود.");
      } finally {
        setLoading(false);
      }
    },
    [appId],
  );

  useEffect(() => {
    load();
  }, [load]);

  return (
    <div className="app-activity">
      {error && (
        <div className="form-error" role="alert">
          {error}
        </div>
      )}
      <EventList events={events} loading={loading} hasMore={hasMore} onMore={() => load(events[events.length - 1]?.id)} />
    </div>
  );
}

export function ConnectHelp() {
  return (
    <div className="code-block connect-command">
      <div className="code-head">
        <span className="code-lang">ترمینال</span>
        <CopyButton text={MCP_COMMAND} />
      </div>
      <pre dir="ltr">
        <code>{MCP_COMMAND}</code>
      </pre>
    </div>
  );
}

export function Apps({ notify }: { notify: (msg: string) => void }) {
  const [apps, setApps] = useState<ConnectedApp[] | null>(null);
  const [error, setError] = useState("");
  const [open, setOpen] = useState<number | null>(null);
  const [removing, setRemoving] = useState<number | null>(null);

  const load = useCallback(async () => {
    setError("");
    try {
      const r = await api.apps();
      setApps(r.apps);
    } catch (e) {
      setError(e instanceof ApiError ? e.message : "بارگذاری برنامه‌ها ناموفق بود.");
    }
  }, []);

  useEffect(() => {
    load();
  }, [load]);

  const revoke = async (app: ConnectedApp) => {
    const ok = await confirmDialog({
      title: "قطع اتصال برنامه",
      subject: app.client.name,
      message: "این برنامه دیگر نمی‌تواند از ابزارهای سلام با حساب شما استفاده کند.",
      confirmLabel: "قطع اتصال",
      icon: "alert",
    });
    if (!ok) return;
    setRemoving(app.id);
    try {
      await api.revokeApp(app.id);
      setApps((cur) => (cur ? cur.filter((a) => a.id !== app.id) : cur));
      notify(`اتصال «${app.client.name}» قطع شد.`);
    } catch (e) {
      notify(e instanceof ApiError ? e.message : "قطع اتصال ناموفق بود.");
    } finally {
      setRemoving(null);
    }
  };

  return (
    <div className="page">
      <header className="page-head">
        <h2>برنامه‌های متصل</h2>
        <p className="muted">برنامه‌هایی مانند Claude Code، Codex یا Cursor که با حساب سلام شما به ابزارهای سلام وصل شده‌اند. هر زمان بخواهید می‌توانید اتصالشان را قطع کنید.</p>
      </header>
      {error && (
        <div className="form-error" role="alert">
          {error}{" "}
          <button type="button" className="link" onClick={load}>
            تلاش دوباره
          </button>
        </div>
      )}
      {apps === null && !error && (
        <div className="loading-chat" role="status">
          <span className="spinner" />
          <span className="sr-only">در حال بارگذاری</span>
        </div>
      )}
      {apps && apps.length === 0 && (
        <section className="card empty-card">
          <div className="notice-icon info">
            <IconPlug />
          </div>
          <h3>هنوز برنامه‌ای وصل نکرده‌اید</h3>
          <p className="muted">
            ابزارهای سلام (بررسی و کامپایل کد، جست‌وجو در کتابخانه‌ی استاندارد و نمونه‌ها) را می‌توانید در دستیارهای برنامه‌نویسی مانند Claude Code، Codex یا Cursor به کار ببرید. برای Claude Code
            این فرمان را در ترمینال اجرا کنید:
          </p>
          <ConnectHelp />
          <p className="muted small">
            در برنامه‌های دیگر نشانی <code className="inline-code" dir="ltr">https://mcp.salamlang.ir/mcp</code> را به عنوان سرور MCP اضافه کنید. با نخستین استفاده، صفحه‌ی ورود سلام باز می‌شود و پس از اجازه‌ی شما، برنامه در این فهرست دیده
            می‌شود.
          </p>
        </section>
      )}
      {apps && apps.length > 0 && (
        <ul className="app-list">
          {apps.map((app) => {
            const expanded = open === app.id;
            return (
              <li key={app.id} className="card app-card">
                <div className="app-row">
                  <span className="client-avatar" aria-hidden="true">
                    {initial(app.client.name)}
                  </span>
                  <div className="app-main">
                    <div className="app-title">
                      <strong>
                        <bdi>{app.client.name}</bdi>
                      </strong>
                      {app.client.kind === "cimd" ? (
                        <span className="badge ok" title="نام این برنامه از روی نشانی آن بررسی شده است">
                          <IconBadgeCheck width={14} height={14} />
                          تأییدشده
                        </span>
                      ) : (
                        <span className="badge" title="نام این برنامه را خود برنامه اعلام کرده است">
                          تأییدنشده
                        </span>
                      )}
                    </div>
                    <span className="app-host" dir="ltr">
                      {app.client.host}
                    </span>
                    <div className="app-meta">
                      <span title={fmtDateTime(app.created)}>متصل از {fmtDate(app.created)}</span>
                      <span title={app.last_used ? fmtDateTime(app.last_used) : undefined}>آخرین استفاده: {fmtRelative(app.last_used)}</span>
                      <span>{fmtNum(app.calls)} فراخوانی</span>
                    </div>
                  </div>
                  <div className="app-actions">
                    <button type="button" className="button ghost small" aria-expanded={expanded} aria-controls={expanded ? `activity-${app.id}` : undefined} onClick={() => setOpen(expanded ? null : app.id)}>
                      <IconActivity width={16} height={16} />
                      فعالیت‌های اخیر
                      <IconChevronDown width={16} height={16} className={`chevron ${expanded ? "up" : ""}`} />
                    </button>
                    <button type="button" className="button danger small" disabled={removing === app.id} onClick={() => revoke(app)}>
                      {removing === app.id ? <span className="spinner" /> : "قطع اتصال"}
                    </button>
                  </div>
                </div>
                {expanded && (
                  <div id={`activity-${app.id}`}>
                    <AppActivity appId={app.id} />
                  </div>
                )}
              </li>
            );
          })}
        </ul>
      )}
    </div>
  );
}
