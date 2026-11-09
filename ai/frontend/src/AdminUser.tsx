import { useEffect, useState } from "react";
import { api, ApiError, type UserProfile } from "./api";
import { AppsTable, Conversations, Events, KeyPolicies, NoAccess, Payments, RequestsFeed, Sessions, Subscriptions, TelegramSessions, Tile, Uploads, openLink, type Picked } from "./Admin";
import { fmtCompact, fmtDate, fmtDateTime, fmtNum, fmtRelative } from "./format";
import { t } from "./i18n";

type Tab = "summary" | "account" | "chats" | "purchases" | "api" | "telegram" | "mcp" | "files";

const TABS: { id: Tab; label: () => string }[] = [
  { id: "summary", label: () => t("خلاصه", "Summary") },
  { id: "account", label: () => t("حساب و نشست‌ها", "Account and sessions") },
  { id: "chats", label: () => t("گفت‌وگوها", "Chats") },
  { id: "purchases", label: () => t("اشتراک و خرید", "Plan and purchases") },
  { id: "api", label: () => "API" },
  { id: "telegram", label: () => t("تلگرام", "Telegram") },
  { id: "mcp", label: () => t("MCP و برنامه‌ها", "MCP and apps") },
  { id: "files", label: () => t("فایل‌ها", "Files") },
];

function tabFromPath(id: number): Tab {
  const m = new RegExp(`^/admin/users/${id}/([a-z]+)/?$`).exec(location.pathname);
  return TABS.find((x) => x.id === m?.[1])?.id ?? "summary";
}

function userPath(id: number, tab: Tab): string {
  return tab === "summary" ? `/admin/users/${id}` : `/admin/users/${id}/${tab}`;
}

function Row({ label, children }: { label: string; children: React.ReactNode }) {
  return (
    <div>
      <dt>{label}</dt>
      <dd>{children}</dd>
    </div>
  );
}

function when(ts: number): string {
  return ts > 0 ? fmtDateTime(ts) : "-";
}

function ago(ts: number): string {
  return ts > 0 ? fmtRelative(ts) : t("هیچ‌وقت", "Never");
}

function Summary({ p, go }: { p: UserProfile; go: (tab: Tab) => void }) {
  const c = p.counts;
  const free = p.plan.id === "free";
  const tokensIn = (c.api_tokens_in ?? 0) + (c.chat_tokens_in ?? 0);
  const tokensOut = (c.api_tokens_out ?? 0) + (c.chat_tokens_out ?? 0);
  const left = p.plan.expires > 0 ? p.plan.expires - Math.floor(Date.now() / 1000) : 0;
  return (
    <>
      <section className="card panel">
        <div className="panel-head">
          <h3>{t("اطلاعات کاربر", "User")}</h3>
          <span className={`badge ${free ? "" : "ok"}`}>{free ? t("رایگان", "Free") : p.plan.name}</span>
        </div>
        <dl className="admin-chat-meta">
          <Row label={t("نام", "Name")}>
            <bdi>{p.name}</bdi>
          </Row>
          <Row label={t("ایمیل", "Email")}>
            <span dir="ltr">{p.email}</span>
          </Row>
          <Row label={t("شناسه", "Id")}>
            <span dir="ltr">{p.id}</span>
          </Row>
          <Row label={t("ثبت‌نام", "Signed up")}>
            <span title={fmtDateTime(p.created)}>{fmtDate(p.created)}</span>
          </Row>
          <Row label={t("آخرین ورود", "Last sign-in")}>{c.last_login > 0 ? <span title={when(c.last_login)}>{ago(c.last_login)}</span> : "-"}</Row>
          <Row label={t("آخرین فعالیت", "Last seen")}>{c.last_seen > 0 ? <span title={when(c.last_seen)}>{ago(c.last_seen)}</span> : "-"}</Row>
          <Row label={t("پلن فعلی", "Current plan")}>
            {free
              ? t("رایگان", "Free")
              : p.plan.expires === 0
                ? t(`${p.plan.name}، بدون انقضا`, `${p.plan.name}, no expiry`)
                : t(`${p.plan.name}، تا ${fmtDateTime(p.plan.expires)}`, `${p.plan.name}, until ${fmtDateTime(p.plan.expires)}`)}
            {!free && left > 0 && <span className="muted"> ({fmtRelative(p.plan.expires)})</span>}
          </Row>
          {p.identities.length > 0 && (
            <Row label={t("ورود با", "Signs in with")}>
              {p.identities.map((i) => (
                <span key={i.provider + i.login} className="badge" dir="ltr">
                  {i.provider}: {i.login || i.email}
                </span>
              ))}
            </Row>
          )}
        </dl>
      </section>

      <section className="card panel">
        <div className="panel-head">
          <h3>{t("فعالیت", "Activity")}</h3>
        </div>
        <div className="tiles tiles-4">
          <Tile label={t("گفت‌وگوها", "Chats")} value={c.chats} sub={t(`${fmtNum(c.messages)} پیام کاربر`, `${fmtNum(c.messages)} user messages`)} />
          <Tile label={t("درخواست API", "API requests")} value={c.api_requests} sub={t(`${fmtNum(c.api_requests_24h)} در ۲۴ ساعت`, `${fmtNum(c.api_requests_24h)} in 24 hours`)} />
          <Tile label={t("فراخوانی MCP", "MCP calls")} value={c.mcp_calls} sub={c.mcp_last > 0 ? ago(c.mcp_last) : undefined} />
          <Tile label={t("توکن‌ها", "Tokens")} value={tokensIn + tokensOut} compact sub={t(`${fmtCompact(tokensIn)} ورودی، ${fmtCompact(tokensOut)} خروجی`, `${fmtCompact(tokensIn)} in, ${fmtCompact(tokensOut)} out`)} />
        </div>
        <div className="tiles tiles-4">
          <Tile label={t("نشست‌های فعال", "Active sessions")} value={c.sessions} sub={t(`${fmtNum(c.logins)} ورود روی هم`, `${fmtNum(c.logins)} sign-ins in total`)} />
          <Tile label={t("کلیدهای API", "API keys")} value={c.keys} sub={c.keys_revoked ? t(`${fmtNum(c.keys_revoked)} باطل‌شده`, `${fmtNum(c.keys_revoked)} revoked`) : undefined} />
          <Tile label={t("برنامه‌های متصل", "Connected apps")} value={c.apps} />
          <Tile label={t("فایل‌ها", "Files")} value={c.files} />
        </div>
      </section>

      <section className="card panel">
        <div className="panel-head">
          <h3>{t("اشتراک و خرید", "Plan and purchases")}</h3>
          <button type="button" className="button small" onClick={() => go("purchases")}>
            {t("جزئیات", "Details")}
          </button>
        </div>
        <div className="tiles tiles-4">
          <Tile label={t("اشتراک‌ها تا امروز", "Subscriptions so far")} value={c.subscriptions} />
          <Tile label={t("پرداخت موفق", "Paid")} value={c.payments_paid} sub={c.paid_total > 0 ? t(`${fmtNum(c.paid_total)} تومان`, `${fmtNum(c.paid_total)} toman`) : undefined} />
          <Tile label={t("تلاش ناموفق", "Failed attempts")} value={c.payments_failed} tone={c.payments_failed > 0 ? "danger" : undefined} />
          <Tile label={t("تلاش کل خرید", "All purchase attempts")} value={c.payments_attempts} sub={c.last_payment > 0 ? ago(c.last_payment) : undefined} />
        </div>
      </section>

      <section className="card panel">
        <div className="panel-head">
          <h3>{t("کانال‌ها و اتصال‌ها", "Channels and connections")}</h3>
        </div>
        <dl className="admin-chat-meta">
          <Row label={t("تلگرام", "Telegram")}>
            {p.telegram.length === 0
              ? t("وصل نیست", "Not linked")
              : p.telegram.map((x) => (
                  <span key={x.tg_id} className="badge" dir="auto">
                    {x.username ? `@${x.username}` : x.name || x.tg_id}
                  </span>
                ))}
          </Row>
          <Row label={t("اتصال‌های بیرونی", "External accounts")}>
            {p.integrations.length === 0
              ? "-"
              : p.integrations.map((x) => (
                  <span key={x.provider} className="badge" dir="ltr">
                    {x.provider}: {x.login}
                  </span>
                ))}
          </Row>
          <Row label={t("دستگاه‌های ایجنت", "Agent devices")}>{fmtNum(c.devices)}</Row>
          <Row label={t("گفت‌وگوهای ایجنت", "Agent chats")}>{fmtNum(c.agent_chats)}</Row>
          <Row label={t("گفت‌وگوهای تلگرام", "Telegram chats")}>{fmtNum(c.telegram_chats)}</Row>
          <Row label={t("پروژه‌ها", "Projects")}>{fmtNum(c.projects)}</Row>
          <Row label={t("حافظه‌ها", "Memories")}>{fmtNum(c.memories)}</Row>
          <Row label={t("لینک‌های اشتراک‌گذاری", "Share links")}>{fmtNum(c.shares)}</Row>
          <Row label={t("آخرین گفت‌وگو", "Last chat")}>{c.last_chat > 0 ? <span title={when(c.last_chat)}>{ago(c.last_chat)}</span> : "-"}</Row>
        </dl>
      </section>
    </>
  );
}

function AccountTab({ p, picked, notify }: { p: UserProfile; picked: Picked; notify: (msg: string, ok?: boolean) => void }) {
  return (
    <>
      <section className="card panel">
        <div className="panel-head">
          <h3>{t("حساب", "Account")}</h3>
        </div>
        <dl className="admin-chat-meta">
          <Row label={t("نام", "Name")}>
            <bdi>{p.name}</bdi>
          </Row>
          <Row label={t("ایمیل", "Email")}>
            <span dir="ltr">{p.email}</span>
          </Row>
          <Row label={t("ثبت‌نام", "Signed up")}>{when(p.created)}</Row>
          <Row label={t("آخرین ورود", "Last sign-in")}>{when(p.counts.last_login)}</Row>
          {p.identities.map((i) => (
            <Row key={i.provider + i.login} label={t(`ورود با ${i.provider}`, `${i.provider} sign-in`)}>
              <span dir="ltr">{i.login || i.email}</span> <span className="muted">{fmtDate(i.created)}</span>
            </Row>
          ))}
          {p.integrations.map((i) => (
            <Row key={i.provider} label={t(`اتصال ${i.provider}`, `${i.provider} connection`)}>
              <span dir="ltr">{i.login || "-"}</span>
            </Row>
          ))}
        </dl>
      </section>
      <Sessions picked={picked} notify={notify} />
    </>
  );
}

export function AdminUser({ id, notify, onNavigate }: { id: number; notify: (msg: string, ok?: boolean) => void; onNavigate: (path: string) => void }) {
  const [p, setP] = useState<UserProfile | null>(null);
  const [error, setError] = useState("");
  const [forbidden, setForbidden] = useState(false);
  const tab = tabFromPath(id);

  useEffect(() => {
    let live = true;
    setP(null);
    setError("");
    api
      .adminUserProfile(id)
      .then((r) => live && setP(r))
      .catch((e) => {
        if (!live) return;
        if (e instanceof ApiError && e.status === 403) setForbidden(true);
        else setError(e instanceof ApiError ? e.message : t("بارگذاری پروفایل ناموفق بود.", "Could not load the profile."));
      });
    return () => {
      live = false;
    };
  }, [id]);

  if (forbidden) return <NoAccess />;
  const picked: Picked = p ? { id: p.id, name: p.name } : null;
  const go = (next: Tab) => onNavigate(userPath(id, next));

  return (
    <div className="page admin-page">
      <header className="page-head">
        <a className="link" href="/admin/users/accounts" onClick={(e) => openLink(e, "/admin/users/accounts", onNavigate)}>
          {t("بازگشت به کاربران", "Back to users")}
        </a>
        <h2 dir="auto">{p ? p.name : t("پروفایل کاربر", "User profile")}</h2>
        {p && (
          <p className="muted">
            <bdi dir="ltr">{p.email}</bdi>
          </p>
        )}
      </header>
      {error && (
        <div className="form-error" role="alert">
          {error}
        </div>
      )}
      {!p && !error && (
        <div className="loading-chat" role="status">
          <span className="spinner" />
          <span className="sr-only">{t("در حال بارگذاری", "Loading")}</span>
        </div>
      )}
      {p && (
        <>
          <nav className="admin-subtabs" role="tablist" aria-label={t("بخش‌های پروفایل", "Profile sections")}>
            {TABS.map((x) => (
              <a key={x.id} role="tab" aria-selected={x.id === tab} href={userPath(id, x.id)} className={`admin-subtab${x.id === tab ? " active" : ""}`} onClick={(e) => openLink(e, userPath(id, x.id), onNavigate)}>
                {x.label()}
              </a>
            ))}
          </nav>
          <div className="admin-panel" role="tabpanel">
            {tab === "summary" && <Summary p={p} go={go} />}
            {tab === "account" && <AccountTab p={p} picked={picked} notify={notify} />}
            {tab === "chats" && <Conversations picked={picked} />}
            {tab === "purchases" && (
              <>
                <Subscriptions picked={picked} notify={notify} history userEmail={p.email} />
                <Payments picked={picked} />
              </>
            )}
            {tab === "api" && (
              <>
                <KeyPolicies picked={picked} notify={notify} />
                <RequestsFeed picked={picked} onPick={() => {}} />
              </>
            )}
            {tab === "telegram" && <TelegramSessions picked={picked} onPick={() => {}} />}
            {tab === "mcp" && (
              <>
                <AppsTable picked={picked} notify={notify} />
                <Events picked={picked} onPick={() => {}} />
              </>
            )}
            {tab === "files" && <Uploads picked={picked} notify={notify} />}
          </div>
        </>
      )}
    </div>
  );
}
