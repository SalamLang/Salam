import { useCallback, useEffect, useRef, useState, type FormEvent, type ReactNode } from "react";
import { api, ApiError, rememberOAuth, type Integration, type Integrations, type TelegramLinkInfo, type TelegramStatus, type User } from "./api";
import { AuthForm } from "./Auth";
import { Notice, Shell } from "./Connect";
import { AppsPanel } from "./Apps";
import { confirmDialog } from "./Confirm";
import { fmtDate, fmtNum, initial } from "./format";
import { IconFigma, IconGithub, IconKey, IconTelegram, IconTerminal } from "./icons";
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
            <IconTelegram width={18} height={18} />
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
    icon: <IconGithub width={22} height={22} />,
    about: t("مخازن عمومی و خصوصی را بخواند، کلون کند، شاخه و درخواست ادغام بسازد و push کند.", "Read and clone public and private repositories, create branches and pull requests, and push."),
    help: "https://github.com/settings/tokens/new?scopes=repo,workflow,admin:org,admin:repo_hook,admin:public_key,gist,notifications,user,project,write:discussion,read:packages,write:packages,codespace&description=Salam%20AI",
    helpText: t("ساخت توکن شخصی (دسترسی‌ها از قبل انتخاب شده)", "Create a personal access token (permissions preselected)"),
    placeholder: "ghp_... / github_pat_...",
    access: t("دسترسی خواسته‌شده: مخزن‌ها (عمومی و خصوصی)، سازمان‌ها (مدیریت کامل، از جمله تیم‌ها و عضویت‌ها)، workflowها، gist، اعلان‌ها، پروژه‌ها و بسته‌ها. حذف مخزن هرگز.", "Access asked for: repositories (public and private), organizations (full management, including teams and memberships), workflows, gists, notifications, projects and packages. Never repository deletion."),
  },
  figma: {
    name: "Figma",
    icon: <IconFigma width={22} height={22} />,
    about: t("فایل‌ها و گره‌های طراحی را بخواند، تصویرها را صادر کند و کامنت بگذارد تا از طراحی کد بسازد.", "Read design files and nodes, export images and leave comments so it can build code from a design."),
    help: "https://www.figma.com/settings",
    helpText: t("ساخت توکن شخصی (Settings، Security)", "Create a personal access token (Settings, Security)"),
    placeholder: "figd_...",
    access: t("دسترسی پیش‌فرض OAuth: خواندن فایل‌ها، نسخه‌ها، کتابخانه‌ها و پوشه‌ها، کامنت‌ها و منابع توسعه‌دهنده (خواندن و نوشتن) و وب‌هوک‌ها.", "Default OAuth access: files, versions, libraries and folders (read), comments and dev resources (read and write) and webhooks."),
  },
} as const;

function ConnCard({ id, icon, title, on, label, highlight, children }: { id: string; icon: ReactNode; title: string; on: boolean; label: string; highlight?: boolean; children: ReactNode }) {
  const ref = useRef<HTMLElement>(null);
  useEffect(() => {
    if (highlight) ref.current?.scrollIntoView({ block: "center" });
  }, [highlight]);
  return (
    <section ref={ref} id={id} className={`conn-card ${highlight ? "highlight" : ""}`}>
      <header className="conn-head">
        <span className={`conn-icon ${on ? "on" : ""}`} aria-hidden="true">
          {icon}
        </span>
        <h3>{title}</h3>
        <span className={`badge ${on ? "ok" : ""}`}>{label}</span>
      </header>
      <div className="conn-body">{children}</div>
    </section>
  );
}

function Account({ name, handle, note, since, busy, onRemove }: { name: string; handle?: string; note?: string; since?: number; busy: boolean; onRemove: () => void }) {
  return (
    <li className="conn-account">
      <span className="conn-avatar" aria-hidden="true">
        {initial(name || handle || "?")}
      </span>
      <div className="conn-account-main">
        <strong>
          <bdi>{name || handle}</bdi>
        </strong>
        <span className="conn-account-sub">
          {handle && (
            <span dir="ltr" className="conn-handle">
              {handle}
            </span>
          )}
          {note && <span>{note}</span>}
          {since ? <span>{t(`متصل از ${fmtDate(since)}`, `Connected since ${fmtDate(since)}`)}</span> : null}
        </span>
      </div>
      <button type="button" className="button danger small" disabled={busy} onClick={onRemove}>
        {busy ? <span className="spinner" /> : t("جدا کردن", "Disconnect")}
      </button>
    </li>
  );
}

function IntegrationCard({ item, storage, highlight, notify, onChange }: { item: Integration; storage: boolean; highlight: boolean; notify: (msg: string) => void; onChange: () => void }) {
  const info = PROVIDERS[item.provider];
  const [token, setToken] = useState("");
  const [busy, setBusy] = useState("");
  const [replacing, setReplacing] = useState(false);

  const save = async (e: FormEvent) => {
    e.preventDefault();
    setBusy("save");
    try {
      const r = await api.saveIntegration(item.provider, token.trim());
      setToken("");
      setReplacing(false);
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
      rememberOAuth({ provider: item.provider, state: r.state, purpose: "connect", next: "" });
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

  const connectForm = (
    <>
      {item.oauth && (
        <button type="button" className="button primary conn-wide" disabled={busy !== ""} onClick={oauth}>
          {busy === "oauth" ? <span className="spinner" /> : t(`اتصال با ${info.name}`, `Connect with ${info.name}`)}
        </button>
      )}
      {item.oauth && <p className="conn-note">{info.access}</p>}
      <form className="conn-form" onSubmit={save}>
        <label className="field">
          <span>{item.oauth ? t("یا با توکن شخصی", "Or with a personal access token") : t("توکن شخصی", "Personal access token")}</span>
          <input type="password" value={token} onChange={(e) => setToken(e.target.value)} dir="ltr" placeholder={info.placeholder} autoComplete="off" spellCheck={false} />
        </label>
        <button type="submit" className="button primary" disabled={busy !== "" || token.trim().length < 20}>
          {busy === "save" ? <span className="spinner" /> : <IconKey width={18} height={18} />}
          {t("وصل کن", "Connect")}
        </button>
      </form>
      <a className="conn-help" href={info.help} target="_blank" rel="noreferrer">
        {info.helpText}
      </a>
    </>
  );

  return (
    <ConnCard id={`connect-${item.provider}`} icon={info.icon} title={info.name} on={item.connected} label={item.connected ? t("وصل است", "Connected") : t("وصل نیست", "Not connected")} highlight={highlight}>
      <p className="conn-about">{info.about}</p>
      {item.connected && item.expired && (
        <p className="policy-warn">{t(`${info.name} این اتصال را رد می‌کند، یعنی توکن لغو یا منقضی شده است. دوباره وصلش کنید تا دستیار به حساب شما دسترسی داشته باشد.`, `${info.name} rejects this connection: the token was revoked or has expired. Connect it again so the assistant can use your account.`)}</p>
      )}
      {item.connected && (
        <ul className="conn-accounts">
          <Account name={item.login} note={item.scope && item.scope !== "token" ? item.scope : undefined} since={item.created} busy={busy === "remove"} onRemove={remove} />
        </ul>
      )}
      {item.connected && !replacing && storage && (
        <button type="button" className="link small-link conn-link" onClick={() => setReplacing(true)}>
          {t("استفاده از حساب یا توکن دیگر", "Use a different account or token")}
        </button>
      )}
      {(!item.connected || replacing) && (storage ? connectForm : <p className="muted small">{t("ذخیره‌ی امن توکن‌ها روی این سرور فعال نیست.", "Secure token storage is not available on this server.")}</p>)}
    </ConnCard>
  );
}

export function Connections({ notify }: { notify: (msg: string) => void }) {
  const [integrations, setIntegrations] = useState<Integrations | null>(null);
  const [status, setStatus] = useState<TelegramStatus | null>(null);
  const [error, setError] = useState("");
  const [busy, setBusy] = useState("");
  const [waiting, setWaiting] = useState<{ have: number; until: number } | null>(null);
  const focus = new URLSearchParams(location.search).get("connect") ?? "";

  const loadIntegrations = useCallback(async () => {
    try {
      setIntegrations(await api.integrations());
    } catch {}
  }, []);

  const load = useCallback(async () => {
    try {
      const r = await api.telegramStatus();
      setStatus(r);
      setError("");
      return r;
    } catch (e) {
      setError(e instanceof ApiError ? e.message : t("بارگذاری ناموفق بود.", "Could not load."));
      return null;
    }
  }, []);

  useEffect(() => {
    loadIntegrations();
    load();
    const p = new URLSearchParams(location.search);
    if (p.get("connected")) {
      notify(p.get("result") === "ok" ? t("اتصال انجام شد.", "Connected.") : t("اتصال انجام نشد. دوباره تلاش کنید.", "The connection did not complete. Try again."));
      history.replaceState(null, "", "/connections");
    }
    const refresh = () => {
      load();
      loadIntegrations();
    };
    window.addEventListener("focus", refresh);
    document.addEventListener("visibilitychange", refresh);
    return () => {
      window.removeEventListener("focus", refresh);
      document.removeEventListener("visibilitychange", refresh);
    };
  }, [load, loadIntegrations, notify]);

  useEffect(() => {
    if (!waiting) return;
    const timer = window.setInterval(async () => {
      const r = await load();
      if ((r && r.links.length > waiting.have) || Date.now() > waiting.until) {
        setWaiting(null);
      }
    }, 3000);
    return () => window.clearInterval(timer);
  }, [waiting, load]);

  const connect = async () => {
    setBusy("connect");
    try {
      const r = await api.telegramCode();
      window.open(r.url, "_blank", "noopener");
      setWaiting({ have: status?.links.length ?? 0, until: Date.now() + 180000 });
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

  const links = status?.links ?? [];

  return (
    <div className="page">
      <header className="page-head">
        <h2>{t("اتصال‌ها و برنامه‌ها", "Connections and apps")}</h2>
        <p className="muted">{t("تلگرام، GitHub، Figma و برنامه‌هایی مانند Claude Code که به حساب سلام شما وصل‌اند.", "Telegram, GitHub, Figma and apps such as Claude Code connected to your Salam account.")}</p>
      </header>
      {error && (
        <div className="form-error" role="alert">
          {error}{" "}
          <button type="button" className="link" onClick={load}>
            {t("تلاش دوباره", "Try again")}
          </button>
        </div>
      )}
      <div className="conn-grid">
        <ConnCard id="connect-telegram" icon={<IconTelegram width={22} height={22} />} title={t("ربات تلگرام", "Telegram bot")} on={links.length > 0} label={links.length > 0 ? t(`${fmtNum(links.length)} حساب وصل است`, `${fmtNum(links.length)} connected`) : t("وصل نیست", "Not connected")} highlight={focus === "telegram"}>
          <p className="conn-about">
            {t(
              "با ربات تلگرام هر جا که هستید با دستیار سلام گفت‌وگو کنید. گفت‌وگوهای تلگرام و سایت یکی هستند: چیزی که در تلگرام می‌پرسید در تاریخچه‌ی سایت هم دیده می‌شود. می‌توانید چند حساب تلگرام را هم‌زمان وصل کنید.",
              "Chat with Salam AI from anywhere through the Telegram bot. Telegram and the website share one history: what you ask in Telegram shows up here too. You can connect several Telegram accounts at once.",
            )}
          </p>
          {status && !status.enabled && <p className="muted small">{t("ربات تلگرام فعلا غیرفعال است.", "The Telegram bot is switched off right now.")}</p>}
          {links.length > 0 && (
            <ul className="conn-accounts">
              {links.map((l) => (
                <Account key={l.id} name={l.name || l.id} handle={l.username ? `@${l.username}` : undefined} since={l.created} busy={busy === l.id} onRemove={() => remove(l.id, l.name || l.id)} />
              ))}
            </ul>
          )}
          {status?.enabled && (
            <div className="conn-actions">
              <button type="button" className="button primary" disabled={busy === "connect"} onClick={connect}>
                {busy === "connect" ? <span className="spinner" /> : links.length > 0 ? t("وصل کردن تلگرام دیگر", "Connect another Telegram") : t("وصل کردن تلگرام", "Connect Telegram")}
              </button>
              <a className="button ghost" href={`https://t.me/${status.bot}`} target="_blank" rel="noreferrer">
                <span dir="ltr">@{status.bot}</span>
              </a>
            </div>
          )}
          {waiting && (
            <p className="conn-waiting" role="status">
              <span className="spinner" /> {t("منتظر تأیید در تلگرام... پس از زدن «بله، وصل کن» در ربات، این صفحه خودش به‌روز می‌شود.", "Waiting for you in Telegram... after you tap Yes in the bot, this page updates by itself.")}
            </p>
          )}
        </ConnCard>
        {integrations?.integrations.map((item) => (
          <IntegrationCard key={item.provider} item={item} storage={integrations.storage} highlight={focus === item.provider} notify={notify} onChange={loadIntegrations} />
        ))}
        {integrations?.sandbox && (
          <ConnCard id="sandbox" icon={<IconTerminal width={22} height={22} />} title={t("محیط اجرای ابری", "Cloud sandbox")} on label={t("فعال", "On")}>
            <p className="conn-about">{t("هر گفت‌وگو یک محیط لینوکسی خصوصی دارد که دستورها در آن اجرا می‌شوند. در تلگرام همه‌ی گفت‌وگوها و در سایت گفت‌وگوهایی که گزینه‌اش را روشن کنید از آن استفاده می‌کنند.", "Every chat has its own private Linux sandbox where commands run. In Telegram every chat uses it; on the site, chats where you turn the option on do.")}</p>
          </ConnCard>
        )}
      </div>
      <AppsPanel notify={notify} />
    </div>
  );
}
