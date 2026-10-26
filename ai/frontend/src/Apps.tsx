import { useCallback, useEffect, useState } from "react";
import { api, ApiError, PAGE_SIZE, type AppEvent, type ConnectedApp } from "./api";
import { EventList } from "./Events";
import { fmtDate, fmtDateTime, fmtNum, fmtRelative, initial } from "./format";
import { IconActivity, IconBadgeCheck, IconChevronDown, IconPlug } from "./icons";
import { CopyButton } from "./Markdown";
import { confirmDialog } from "./Confirm";
import { t } from "./i18n";

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
        setError(e instanceof ApiError ? e.message : t("بارگذاری فعالیت‌ها ناموفق بود.", "Could not load activity."));
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
        <span className="code-lang">{t("ترمینال", "Terminal")}</span>
        <CopyButton text={MCP_COMMAND} />
      </div>
      <pre dir="ltr">
        <code>{MCP_COMMAND}</code>
      </pre>
    </div>
  );
}

export function AppsPanel({ notify }: { notify: (msg: string) => void }) {
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
      setError(e instanceof ApiError ? e.message : t("بارگذاری برنامه‌ها ناموفق بود.", "Could not load apps."));
    }
  }, []);

  useEffect(() => {
    load();
  }, [load]);

  const revoke = async (app: ConnectedApp) => {
    const ok = await confirmDialog({
      title: t("قطع اتصال برنامه", "Disconnect app"),
      subject: app.client.name,
      message: t("این برنامه دیگر نمی‌تواند از ابزارهای سلام با حساب شما استفاده کند.", "This app will no longer be able to use Salam tools with your account."),
      confirmLabel: t("قطع اتصال", "Disconnect"),
      icon: "alert",
    });
    if (!ok) return;
    setRemoving(app.id);
    try {
      await api.revokeApp(app.id);
      setApps((cur) => (cur ? cur.filter((a) => a.id !== app.id) : cur));
      notify(t(`اتصال «${app.client.name}» قطع شد.`, `Disconnected "${app.client.name}".`));
    } catch (e) {
      notify(e instanceof ApiError ? e.message : t("قطع اتصال ناموفق بود.", "Could not disconnect the app."));
    } finally {
      setRemoving(null);
    }
  };

  return (
    <section className="apps-panel" id="apps">
      <header className="section-head">
        <h3>{t("برنامه‌های متصل (MCP)", "Connected apps (MCP)")}</h3>
        <p className="muted">
          {t(
            "برنامه‌هایی مانند Claude Code، Codex یا Cursor که با حساب سلام شما به ابزارهای سلام وصل شده‌اند. هر زمان بخواهید می‌توانید اتصالشان را قطع کنید.",
            "Apps such as Claude Code, Codex or Cursor that are connected to Salam tools through your Salam account. You can disconnect them at any time.",
          )}
        </p>
      </header>
      <section className="card empty-card">
        <div className="notice-icon info">
          <IconPlug />
        </div>
        <h3>{t("اتصال یک ابزار جدید", "Connect a new tool")}</h3>
        <p className="muted">
          {t(
            "ابزارهای سلام (بررسی و کامپایل کد، جست‌وجو در کتابخانه‌ی استاندارد و نمونه‌ها) را می‌توانید در دستیارهای برنامه‌نویسی مانند Claude Code، Codex یا Cursor به کار ببرید؛ در هر پلن، حتی پلن رایگان، و بدون نیاز به ساختن توکن دستی. برای Claude Code این فرمان را در ترمینال اجرا کنید:",
            "You can use Salam tools (checking and compiling code, searching the standard library and examples) in coding assistants such as Claude Code, Codex or Cursor, on every plan including the free one, with no manual token needed. For Claude Code, run this command in your terminal:",
          )}
        </p>
        <ConnectHelp />
        <p className="muted small">
          {t("در برنامه‌های دیگر نشانی ", "In other apps, add ")}
          <code className="inline-code" dir="ltr">https://mcp.salamlang.ir/mcp</code>
          {t(
            " را به عنوان سرور MCP اضافه کنید. با نخستین استفاده، صفحه‌ی ورود سلام باز می‌شود و پس از اجازه‌ی شما، برنامه در این فهرست دیده می‌شود.",
            " as an MCP server. On first use the Salam sign in page opens, and once you allow it the app shows up in this list.",
          )}
        </p>
      </section>
      {error && (
        <div className="form-error" role="alert">
          {error}{" "}
          <button type="button" className="link" onClick={load}>
            {t("تلاش دوباره", "Try again")}
          </button>
        </div>
      )}
      {apps === null && !error && (
        <div className="loading-chat" role="status">
          <span className="spinner" />
          <span className="sr-only">{t("در حال بارگذاری", "Loading")}</span>
        </div>
      )}
      {apps && apps.length === 0 && (
        <section className="card empty-card">
          <h3>{t("هنوز برنامه‌ای وصل نکرده‌اید", "You have not connected any apps yet")}</h3>
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
                        <span className="badge ok" title={t("نام این برنامه از روی نشانی آن بررسی شده است", "This app's name was verified from its address")}>
                          <IconBadgeCheck width={14} height={14} />
                          {t("تأییدشده", "Verified")}
                        </span>
                      ) : (
                        <span className="badge" title={t("نام این برنامه را خود برنامه اعلام کرده است", "This app's name was declared by the app itself")}>
                          {t("تأییدنشده", "Unverified")}
                        </span>
                      )}
                    </div>
                    <span className="app-host" dir="ltr">
                      {app.client.host}
                    </span>
                    <div className="app-meta">
                      <span title={fmtDateTime(app.created)}>{t(`متصل از ${fmtDate(app.created)}`, `Connected since ${fmtDate(app.created)}`)}</span>
                      <span title={app.last_used ? fmtDateTime(app.last_used) : undefined}>{t(`آخرین استفاده: ${fmtRelative(app.last_used)}`, `Last used: ${fmtRelative(app.last_used)}`)}</span>
                      <span>{t(`${fmtNum(app.calls)} فراخوانی`, `${fmtNum(app.calls)} calls`)}</span>
                    </div>
                  </div>
                  <div className="app-actions">
                    <button type="button" className="button ghost small" aria-expanded={expanded} aria-controls={expanded ? `activity-${app.id}` : undefined} onClick={() => setOpen(expanded ? null : app.id)}>
                      <IconActivity width={16} height={16} />
                      {t("فعالیت‌های اخیر", "Recent activity")}
                      <IconChevronDown width={16} height={16} className={`chevron ${expanded ? "up" : ""}`} />
                    </button>
                    <button type="button" className="button danger small" disabled={removing === app.id} onClick={() => revoke(app)}>
                      {removing === app.id ? <span className="spinner" /> : t("قطع اتصال", "Disconnect")}
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
    </section>
  );
}
