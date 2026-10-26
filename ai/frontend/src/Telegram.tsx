import { useCallback, useEffect, useRef, useState, type FormEvent } from "react";
import { api, ApiError, type Integration, type Integrations, type TelegramLinkInfo, type TelegramStatus, type User } from "./api";
import { AuthForm } from "./Auth";
import { Notice, Shell } from "./Connect";
import { confirmDialog } from "./Confirm";
import { fmtDate } from "./format";
import { IconCheck, IconKey, IconSend } from "./icons";
import type { Theme } from "./theme";
import { t } from "./i18n";

type Props = {
  user: User | null;
  onAuth: (u: User) => void;
  onSwitchAccount: () => void;
  theme: Theme;
  toggleTheme: () => void;
};

type State =
  | { kind: "loading" }
  | { kind: "missing" }
  | { kind: "expired" }
  | { kind: "ready"; info: TelegramLinkInfo }
  | { kind: "done"; info: TelegramLinkInfo }
  | { kind: "failed"; message: string };

function codeParam(): string {
  return new URLSearchParams(location.search).get("code") ?? "";
}

export function TelegramLink({ user, onAuth, onSwitchAccount, theme, toggleTheme }: Props) {
  const [state, setState] = useState<State>(() => (codeParam() ? { kind: "loading" } : { kind: "missing" }));
  const [busy, setBusy] = useState(false);
  const [error, setError] = useState("");

  const load = useCallback(async () => {
    const code = codeParam();
    if (!code) return;
    setState({ kind: "loading" });
    try {
      setState({ kind: "ready", info: await api.telegramInfo(code) });
    } catch (e) {
      if (e instanceof ApiError && e.status === 404) setState({ kind: "expired" });
      else setState({ kind: "failed", message: e instanceof ApiError ? e.message : t("خطایی پیش آمد.", "Something went wrong.") });
    }
  }, []);

  useEffect(() => {
    load();
  }, [load]);

  const confirm = async (info: TelegramLinkInfo) => {
    setError("");
    setBusy(true);
    try {
      await api.telegramLink(codeParam());
      history.replaceState(null, "", "/telegram");
      setState({ kind: "done", info });
    } catch (e) {
      if (e instanceof ApiError && e.status === 401) return;
      if (e instanceof ApiError && e.status === 404) setState({ kind: "expired" });
      else setError(e instanceof ApiError ? e.message : t("خطایی پیش آمد. دوباره تلاش کنید.", "Something went wrong. Try again."));
    } finally {
      setBusy(false);
    }
  };

  const shell = (children: React.ReactNode) => (
    <Shell theme={theme} toggleTheme={toggleTheme}>
      {children}
    </Shell>
  );

  switch (state.kind) {
    case "loading":
      return shell(
        <div className="loading-chat" role="status">
          <span className="spinner" />
          <span className="sr-only">{t("در حال بارگذاری", "Loading")}</span>
        </div>,
      );
    case "missing":
      return shell(
        <Notice tone="danger" title={t("پیوند اتصال پیدا نشد", "Link not found")}>
          <p className="muted">{t("این نشانی کامل نیست. در ربات تلگرام دستور /login را بفرستید و از دکمه‌ی آن وارد شوید.", "This link is incomplete. Send /login to the Telegram bot and use its button.")}</p>
          <a className="button ghost wide" href="/">
            {t("رفتن به دستیار سلام", "Go to Salam AI")}
          </a>
        </Notice>,
      );
    case "expired":
      return shell(
        <Notice tone="danger" title={t("این پیوند منقضی شده است", "This link has expired")}>
          <p className="muted">{t("پیوندهای ورود فقط ۱۵ دقیقه معتبرند و یک بار مصرف می‌شوند. در ربات تلگرام دوباره /login را بفرستید.", "Sign in links are valid for 15 minutes and work once. Send /login to the Telegram bot again.")}</p>
        </Notice>,
      );
    case "failed":
      return shell(
        <Notice tone="danger" title={t("پیوند بارگذاری نشد", "Could not load the link")}>
          <p className="muted">{state.message}</p>
          <button type="button" className="button primary wide" onClick={load}>
            {t("تلاش دوباره", "Try again")}
          </button>
        </Notice>,
      );
    case "done":
      return shell(
        <Notice title={t("تلگرام وصل شد", "Telegram connected")}>
          <p className="muted">
            {t("تلگرام ", "Telegram ")}
            <bdi className="strong">{state.info.name}</bdi>
            {t(" به حساب شما وصل شد. از این به بعد گفت‌وگوهایتان در تاریخچه‌ی حساب ذخیره می‌شود.", " is now connected to your account. Your chats are saved in your account history from now on.")}
          </p>
          <a className="button primary wide" href={`https://t.me/${state.info.bot}`}>
            {t("بازگشت به تلگرام", "Back to Telegram")}
          </a>
          <a className="button ghost wide" href="/">
            {t("رفتن به دستیار سلام", "Go to Salam AI")}
          </a>
        </Notice>,
      );
  }

  const info = state.info;
  const who = (
    <>
      <bdi className="strong">{info.name}</bdi>
      {info.username && <span dir="ltr"> @{info.username}</span>}
    </>
  );

  if (!user) {
    return shell(
      <>
        <div className="connect-context">
          <span className="client-avatar small" aria-hidden="true">
            <IconSend width={18} height={18} />
          </span>
          <p>
            {t("تلگرام ", "Telegram ")}
            {who}
            {t(" می‌خواهد به حساب سلام شما وصل شود. برای ادامه وارد شوید یا ثبت‌نام کنید.", " wants to connect to your Salam account. Sign in or sign up to continue.")}
          </p>
        </div>
        <AuthForm compact onAuth={onAuth} intro={t("با حساب سلام وارد شوید یا یک حساب تازه بسازید.", "Sign in with your Salam account or create a new one.")} />
      </>,
    );
  }

  return shell(
    <section className="auth-card connect-card" aria-labelledby="tg-title">
      <h2 id="tg-title">{t("اتصال تلگرام به حساب سلام", "Connect Telegram to your Salam account")}</h2>
      <p className="muted">
        {t("این تلگرام به حساب شما وصل می‌شود: ", "This Telegram will be connected to your account: ")}
        {who}
      </p>
      <div className="warning" role="note">
        <p>{t("فقط اگر همین الان خودتان در ربات تلگرام /login را فرستاده‌اید ادامه دهید. هر کسی با این تلگرام می‌تواند گفت‌وگوهای حساب شما را ببیند و با آن‌ها ادامه دهد.", "Only continue if you just sent /login to the Telegram bot yourself. Whoever uses this Telegram can see your account's chats and continue them.")}</p>
      </div>
      <div className="account-box">
        <div className="who">
          <strong dir="auto">{user.name}</strong>
          <span dir="ltr">{user.email}</span>
        </div>
        <button type="button" className="link small-link" onClick={onSwitchAccount}>
          {t("شما نیستید؟ تغییر حساب", "Not you? Switch account")}
        </button>
      </div>
      {error && (
        <div className="form-error" role="alert">
          {error}
        </div>
      )}
      <div className="connect-actions">
        <a className="button ghost" href="/">
          {t("لغو", "Cancel")}
        </a>
        <button type="button" className="button primary" disabled={busy} onClick={() => confirm(info)}>
          {busy ? <span className="spinner" /> : t("وصل کن", "Connect")}
        </button>
      </div>
    </section>,
  );
}

const PROVIDERS = {
  github: {
    name: "GitHub",
    about: t("مخازن عمومی و خصوصی را بخواند، کلون کند، شاخه و درخواست ادغام بسازد و push کند.", "Read and clone public and private repositories, create branches and pull requests, and push."),
    help: "https://github.com/settings/tokens/new?scopes=repo,read:org,workflow&description=Salam%20AI",
    helpText: t("ساخت توکن شخصی (دسترسی repo)", "Create a personal access token (repo scope)"),
    placeholder: "ghp_... / github_pat_...",
  },
  figma: {
    name: "Figma",
    about: t("فایل‌ها و گره‌های طراحی را بخواند، تصویرها را صادر کند و کامنت بگذارد تا از طراحی کد بسازد.", "Read design files and nodes, export images and leave comments so it can build code from a design."),
    help: "https://www.figma.com/settings",
    helpText: t("ساخت توکن شخصی (Settings، Security)", "Create a personal access token (Settings, Security)"),
    placeholder: "figd_...",
  },
} as const;

function IntegrationCard({ item, storage, highlight, notify, onChange }: { item: Integration; storage: boolean; highlight: boolean; notify: (msg: string) => void; onChange: () => void }) {
  const info = PROVIDERS[item.provider];
  const [token, setToken] = useState("");
  const [busy, setBusy] = useState("");
  const ref = useRef<HTMLElement>(null);

  useEffect(() => {
    if (highlight) ref.current?.scrollIntoView({ block: "center" });
  }, [highlight]);

  const save = async (e: FormEvent) => {
    e.preventDefault();
    setBusy("save");
    try {
      const r = await api.saveIntegration(item.provider, token.trim());
      setToken("");
      notify(t(`${info.name} وصل شد: ${r.login}`, `${info.name} connected: ${r.login}`));
      onChange();
    } catch (err) {
      notify(err instanceof ApiError ? err.message : t("وصل کردن ناموفق بود.", "Could not connect."));
    } finally {
      setBusy("");
    }
  };

  const oauth = async () => {
    setBusy("oauth");
    try {
      const r = await api.integrationOauth(item.provider);
      location.assign(r.url);
    } catch (err) {
      notify(err instanceof ApiError ? err.message : t("وصل کردن ناموفق بود.", "Could not connect."));
      setBusy("");
    }
  };

  const remove = async () => {
    const ok = await confirmDialog({
      title: t(`جدا کردن ${info.name}`, `Disconnect ${info.name}`),
      subject: item.login,
      message: t("دستیار دیگر به این حساب دسترسی ندارد. توکن از سرور سلام پاک می‌شود.", "The assistant will no longer have access to this account. The token is deleted from the Salam server."),
      confirmLabel: t("جدا کردن", "Disconnect"),
      icon: "alert",
    });
    if (!ok) return;
    setBusy("remove");
    try {
      await api.removeIntegration(item.provider);
      notify(t(`${info.name} جدا شد.`, `${info.name} disconnected.`));
      onChange();
    } catch (err) {
      notify(err instanceof ApiError ? err.message : t("جدا کردن ناموفق بود.", "Could not disconnect."));
    } finally {
      setBusy("");
    }
  };

  return (
    <section ref={ref} className={`card integration-card ${highlight ? "highlight" : ""}`} id={`connect-${item.provider}`}>
      <div className="app-title">
        <strong>{info.name}</strong>
        <span className={`badge ${item.connected ? "ok" : ""}`}>{item.connected ? t("وصل است", "Connected") : t("وصل نیست", "Not connected")}</span>
      </div>
      <p className="muted">{info.about}</p>
      {item.connected ? (
        <>
          <p>
            {t("حساب: ", "Account: ")}
            <bdi className="strong" dir="ltr">
              {item.login}
            </bdi>
          </p>
          <div className="integration-actions">
            <button type="button" className="button danger small" disabled={busy !== ""} onClick={remove}>
              {busy === "remove" ? <span className="spinner" /> : t("جدا کردن", "Disconnect")}
            </button>
          </div>
        </>
      ) : !storage ? (
        <p className="muted small">{t("ذخیره‌ی امن توکن‌ها روی این سرور فعال نیست.", "Secure token storage is not available on this server.")}</p>
      ) : (
        <>
          {item.oauth && (
            <div className="integration-actions">
              <button type="button" className="button primary" disabled={busy !== ""} onClick={oauth}>
                {busy === "oauth" ? <span className="spinner" /> : t(`ورود با ${info.name}`, `Sign in with ${info.name}`)}
              </button>
            </div>
          )}
          <form className="integration-form" onSubmit={save}>
            <label className="field">
              <span>{t("یا توکن شخصی", "Or a personal access token")}</span>
              <input type="password" value={token} onChange={(e) => setToken(e.target.value)} dir="ltr" placeholder={info.placeholder} autoComplete="off" spellCheck={false} />
            </label>
            <button type="submit" className="button primary" disabled={busy !== "" || token.trim().length < 20}>
              {busy === "save" ? <span className="spinner" /> : <IconKey width={18} height={18} />}
              {t("وصل کن", "Connect")}
            </button>
          </form>
          <p className="muted small">
            <a href={info.help} target="_blank" rel="noreferrer">
              {info.helpText}
            </a>
          </p>
        </>
      )}
    </section>
  );
}

export function Connections({ notify }: { notify: (msg: string) => void }) {
  const [integrations, setIntegrations] = useState<Integrations | null>(null);
  const focus = new URLSearchParams(location.search).get("connect") ?? "";

  const loadIntegrations = useCallback(async () => {
    try {
      setIntegrations(await api.integrations());
    } catch {}
  }, []);

  useEffect(() => {
    loadIntegrations();
    const p = new URLSearchParams(location.search);
    const done = p.get("connected");
    if (done) {
      const result = p.get("result");
      notify(result === "ok" ? t("اتصال انجام شد.", "Connected.") : t("اتصال انجام نشد. دوباره تلاش کنید.", "The connection did not complete. Try again."));
      history.replaceState(null, "", "/connections");
    }
  }, [loadIntegrations, notify]);

  const [status, setStatus] = useState<TelegramStatus | null>(null);
  const [error, setError] = useState("");
  const [busy, setBusy] = useState("");

  const load = useCallback(async () => {
    setError("");
    try {
      setStatus(await api.telegramStatus());
    } catch (e) {
      setError(e instanceof ApiError ? e.message : t("بارگذاری ناموفق بود.", "Could not load."));
    }
  }, []);

  useEffect(() => {
    load();
  }, [load]);

  const connect = async () => {
    setBusy("connect");
    try {
      const r = await api.telegramCode();
      window.open(r.url, "_blank", "noopener");
    } catch (e) {
      notify(e instanceof ApiError ? e.message : t("ساخت پیوند ناموفق بود.", "Could not create the link."));
    } finally {
      setBusy("");
    }
  };

  const remove = async (id: string, name: string) => {
    const ok = await confirmDialog({
      title: t("جدا کردن تلگرام", "Disconnect Telegram"),
      subject: name,
      message: t("این تلگرام دیگر به حساب شما وصل نیست و به‌عنوان مهمان ادامه می‌دهد. گفت‌وگوهای قبلی در حساب شما می‌مانند.", "This Telegram will no longer be connected to your account and becomes a guest. Your earlier chats stay in your account."),
      confirmLabel: t("جدا کردن", "Disconnect"),
      icon: "alert",
    });
    if (!ok) return;
    setBusy(id);
    try {
      await api.telegramUnlink(id);
      setStatus((cur) => (cur ? { ...cur, links: cur.links.filter((l) => l.id !== id) } : cur));
      notify(t("تلگرام جدا شد.", "Telegram disconnected."));
    } catch (e) {
      notify(e instanceof ApiError ? e.message : t("جدا کردن ناموفق بود.", "Could not disconnect."));
    } finally {
      setBusy("");
    }
  };

  return (
    <div className="page">
      <header className="page-head">
        <h2>{t("اتصال‌ها", "Connections")}</h2>
        <p className="muted">{t("برنامه‌ها و سرویس‌هایی که به حساب سلام شما وصل‌اند.", "Apps and services connected to your Salam account.")}</p>
      </header>
      {error && (
        <div className="form-error" role="alert">
          {error}{" "}
          <button type="button" className="link" onClick={load}>
            {t("تلاش دوباره", "Try again")}
          </button>
        </div>
      )}
      <section className="card empty-card">
        <div className="notice-icon info">
          <IconSend />
        </div>
        <h3>{t("ربات تلگرام", "Telegram bot")}</h3>
        <p className="muted">
          {t(
            "با ربات تلگرام هر جا که هستید با دستیار سلام گفت‌وگو کنید. گفت‌وگوهای تلگرام و سایت یکی هستند: چیزی که در تلگرام می‌پرسید در تاریخچه‌ی سایت هم دیده می‌شود.",
            "Chat with Salam AI from anywhere through the Telegram bot. Telegram and the website share one history: what you ask in Telegram shows up here too.",
          )}
        </p>
        {status && !status.enabled && <p className="muted small">{t("ربات تلگرام فعلا غیرفعال است.", "The Telegram bot is switched off right now.")}</p>}
        {status?.enabled && (
          <button type="button" className="button primary" disabled={busy === "connect"} onClick={connect}>
            {busy === "connect" ? <span className="spinner" /> : t("وصل کردن یک تلگرام", "Connect a Telegram")}
          </button>
        )}
        {status?.enabled && (
          <p className="muted small">
            {t("یا در تلگرام ", "Or open ")}
            <a href={`https://t.me/${status.bot}`} target="_blank" rel="noreferrer" dir="ltr">
              @{status.bot}
            </a>
            {t(" را باز کنید و /login را بفرستید.", " in Telegram and send /login.")}
          </p>
        )}
      </section>
      {integrations && (
        <div className="integration-grid">
          {integrations.integrations.map((item) => (
            <IntegrationCard key={item.provider} item={item} storage={integrations.storage} highlight={focus === item.provider} notify={notify} onChange={loadIntegrations} />
          ))}
          {integrations.sandbox && (
            <section className="card">
              <div className="app-title">
                <strong>{t("محیط اجرای ابری", "Cloud sandbox")}</strong>
                <span className="badge ok">{t("فعال", "On")}</span>
              </div>
              <p className="muted">{t("هر گفت‌وگو یک محیط لینوکسی خصوصی دارد که دستورها در آن اجرا می‌شوند. در تلگرام همه‌ی گفت‌وگوها و در سایت گفت‌وگوهایی که گزینه‌اش را روشن کنید از آن استفاده می‌کنند.", "Every chat has its own private Linux sandbox where commands run. In Telegram every chat uses it; on the site, chats where you turn the option on do.")}</p>
            </section>
          )}
        </div>
      )}
      {status && status.links.length > 0 && (
        <ul className="app-list">
          {status.links.map((l) => (
            <li key={l.id} className="card app-card">
              <div className="app-row">
                <span className="client-avatar" aria-hidden="true">
                  <IconCheck width={18} height={18} />
                </span>
                <div className="app-main">
                  <div className="app-title">
                    <strong>
                      <bdi>{l.name || l.id}</bdi>
                    </strong>
                    {l.username && (
                      <span className="app-host" dir="ltr">
                        @{l.username}
                      </span>
                    )}
                  </div>
                  <div className="app-meta">
                    <span>{t(`متصل از ${fmtDate(l.created)}`, `Connected since ${fmtDate(l.created)}`)}</span>
                  </div>
                </div>
                <div className="app-actions">
                  <button type="button" className="button danger small" disabled={busy === l.id} onClick={() => remove(l.id, l.name || l.id)}>
                    {busy === l.id ? <span className="spinner" /> : t("جدا کردن", "Disconnect")}
                  </button>
                </div>
              </div>
            </li>
          ))}
        </ul>
      )}
    </div>
  );
}
