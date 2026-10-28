import { useCallback, useEffect, useState, type ReactNode } from "react";
import { api, ApiError, type ConsentRequest, type User } from "./api";
import { AuthForm } from "./Auth";
import { initial } from "./format";
import { IconAlert, IconBadgeCheck, IconCheck, IconLink, IconMoon, IconSun } from "./icons";
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
  | { kind: "ready"; request: ConsentRequest }
  | { kind: "missing" }
  | { kind: "expired" }
  | { kind: "used" }
  | { kind: "failed"; message: string }
  | { kind: "oauth-error"; code: string; description: string }
  | { kind: "redirecting"; request: ConsentRequest; approved: boolean };

function readParams() {
  const p = new URLSearchParams(location.search);
  return { id: p.get("request") || "", error: p.get("error") || "", description: p.get("error_description") || "" };
}

function initialState(): State {
  const p = readParams();
  if (p.error) return { kind: "oauth-error", code: p.error, description: p.description };
  if (!p.id) return { kind: "missing" };
  return { kind: "loading" };
}

const ERROR_TEXT: Record<string, string> = {
  access_denied: t("دسترسی داده نشد و اتصال لغو شد.", "Access was not granted and the connection was canceled."),
  invalid_request: t("درخواستی که برنامه فرستاده کامل یا درست نیست.", "The request the app sent is incomplete or invalid."),
  invalid_client: t("این برنامه شناخته نشد.", "This app was not recognized."),
  unauthorized_client: t("این برنامه اجازه‌ی چنین درخواستی را ندارد.", "This app is not allowed to make this request."),
  invalid_scope: t("برنامه دسترسی‌ای خواسته که سلام آن را نمی‌شناسد.", "The app asked for access that Salam does not recognize."),
  unsupported_response_type: t("روش درخواست این برنامه پشتیبانی نمی‌شود.", "This app's request method is not supported."),
  server_error: t("در سرور سلام خطایی پیش آمد.", "Something went wrong on the Salam server."),
  temporarily_unavailable: t("سرویس سلام موقتا در دسترس نیست. کمی بعد دوباره تلاش کنید.", "The Salam service is temporarily unavailable. Try again in a little while."),
};

export function Shell({ theme, toggleTheme, children }: { theme: Theme; toggleTheme: () => void; children: ReactNode }) {
  return (
    <div className="connect">
      <button type="button" className="icon-button theme-corner" onClick={toggleTheme} aria-label={t("تغییر پوسته", "Toggle theme")}>
        {theme === "dark" ? <IconSun /> : <IconMoon />}
      </button>
      <main className="connect-inner">{children}</main>
      <p className="auth-foot">
        <a href="https://www.salamlang.ir" target="_blank" rel="noreferrer">
          salamlang.ir
        </a>
        <span>·</span>
        <a href="https://www.salamlang.ir/learn/" target="_blank" rel="noreferrer">
          {t("آموزش سلام", "Learn Salam")}
        </a>
      </p>
    </div>
  );
}

export function Notice({ tone = "info", title, children }: { tone?: "info" | "danger"; title: string; children?: ReactNode }) {
  return (
    <section className="auth-card connect-card notice-card" aria-live="polite">
      <div className={`notice-icon ${tone}`}>{tone === "danger" ? <IconAlert /> : <IconLink />}</div>
      <h2>{title}</h2>
      {children}
    </section>
  );
}

function Redirecting({ request, approved }: { request: ConsentRequest; approved: boolean }) {
  const [late, setLate] = useState(false);
  useEffect(() => {
    const timer = window.setTimeout(() => setLate(true), 4000);
    return () => window.clearTimeout(timer);
  }, []);
  return (
    <section className="auth-card connect-card notice-card" aria-live="polite">
      <div className={`notice-icon ${approved ? "ok" : "info"}`}>{approved ? <IconCheck /> : <IconLink />}</div>
      <h2>{approved ? t("اجازه داده شد", "Access granted") : t("درخواست رد شد", "Request denied")}</h2>
      <p className="muted">
        {t("در حال بازگشت به ", "Returning to ")}<bdi className="strong">{request.client.name}</bdi>
        {request.redirect_host && (
          <>
            {" "}
            (<span dir="ltr">{request.redirect_host}</span>)
          </>
        )}{" "}
        ...
      </p>
      <span className="spinner connect-spinner" aria-hidden="true" />
      {late && <p className="connect-hint">{t("اگر برنامه خودش باز شد یا کارتان تمام است، می‌توانید این پنجره را ببندید.", "If the app opened by itself or you are done, you can close this window.")}</p>}
    </section>
  );
}

export function Connect({ user, onAuth, onSwitchAccount, theme, toggleTheme }: Props) {
  const [state, setState] = useState<State>(initialState);
  const [busy, setBusy] = useState<"" | "approve" | "deny">("");
  const [error, setError] = useState("");

  const load = useCallback(async () => {
    const { id } = readParams();
    if (!id) return;
    setState({ kind: "loading" });
    try {
      const r = await api.consentRequest(id);
      setState({ kind: "ready", request: r.request });
    } catch (e) {
      if (e instanceof ApiError && e.status === 404) setState({ kind: "expired" });
      else if (e instanceof ApiError && e.status === 410) setState({ kind: "used" });
      else setState({ kind: "failed", message: e instanceof ApiError ? e.message : t("خطایی پیش آمد.", "Something went wrong.") });
    }
  }, []);

  useEffect(() => {
    if (initialState().kind === "loading") load();
  }, [load]);

  const answer = async (request: ConsentRequest, approve: boolean) => {
    setError("");
    setBusy(approve ? "approve" : "deny");
    try {
      const r = approve ? await api.approveRequest(request.id) : await api.denyRequest(request.id);
      setState({ kind: "redirecting", request, approved: approve });
      location.assign(r.redirect);
    } catch (e) {
      if (e instanceof ApiError && e.status === 401) return;
      if (e instanceof ApiError && e.status === 404) setState({ kind: "expired" });
      else if (e instanceof ApiError && e.status === 410) setState({ kind: "used" });
      else setError(e instanceof ApiError ? e.message : t("خطایی پیش آمد. دوباره تلاش کنید.", "Something went wrong. Try again."));
    } finally {
      setBusy("");
    }
  };

  const shell = (children: ReactNode) => (
    <Shell theme={theme} toggleTheme={toggleTheme}>
      {children}
    </Shell>
  );

  switch (state.kind) {
    case "loading":
      return shell(
        <div className="loading-chat" role="status">
          <span className="spinner" />
          <span className="sr-only">{t("در حال بارگذاری درخواست", "Loading request")}</span>
        </div>,
      );
    case "missing":
      return shell(
        <Notice tone="danger" title={t("درخواست اتصال پیدا نشد", "Connection request not found")}>
          <p className="muted">{t("این نشانی کامل نیست. اتصال را دوباره از برنامه‌ای که می‌خواهید به سلام وصل کنید آغاز کنید.", "This link is incomplete. Start the connection again from the app you want to connect to Salam.")}</p>
          <a className="button ghost wide" href="/">
            {t("رفتن به دستیار سلام", "Go to Salam AI")}
          </a>
        </Notice>,
      );
    case "expired":
      return shell(
        <Notice tone="danger" title={t("این درخواست منقضی شده است", "This request has expired")}>
          <p className="muted">{t("درخواست‌های اتصال فقط چند دقیقه معتبرند. به برنامه برگردید و اتصال را دوباره آغاز کنید.", "Connection requests are only valid for a few minutes. Go back to the app and start the connection again.")}</p>
        </Notice>,
      );
    case "used":
      return shell(
        <Notice title={t("به این درخواست پاسخ داده شده است", "This request has already been answered")}>
          <p className="muted">{t("این درخواست اتصال قبلا تأیید یا رد شده است. اگر برنامه هنوز منتظر است، اتصال را از همان برنامه دوباره آغاز کنید.", "This connection request was already approved or denied. If the app is still waiting, start the connection again from that app.")}</p>
          <p className="connect-hint">{t("می‌توانید این پنجره را ببندید.", "You can close this window.")}</p>
        </Notice>,
      );
    case "failed":
      return shell(
        <Notice tone="danger" title={t("درخواست بارگذاری نشد", "Could not load the request")}>
          <p className="muted">{state.message}</p>
          <button type="button" className="button primary wide" onClick={load}>
            {t("تلاش دوباره", "Try again")}
          </button>
        </Notice>,
      );
    case "oauth-error":
      return shell(
        <Notice tone="danger" title={t("اتصال انجام نشد", "Connection failed")}>
          <p className="muted">{ERROR_TEXT[state.code] || t("در روند اتصال برنامه به حساب سلام خطایی پیش آمد.", "Something went wrong while connecting the app to your Salam account.")}</p>
          {state.description && (
            <p className="connect-detail" dir="auto">
              {state.description}
            </p>
          )}
          <p className="connect-code">
            {t("کد خطا: ", "Error code: ")}<code dir="ltr">{state.code}</code>
          </p>
          <p className="connect-hint">{t("به برنامه برگردید و اتصال را دوباره آغاز کنید.", "Go back to the app and start the connection again.")}</p>
        </Notice>,
      );
    case "redirecting":
      return shell(<Redirecting request={state.request} approved={state.approved} />);
  }

  const request = state.request;
  const client = request.client;

  if (!user) {
    return shell(
      <>
        <div className="connect-context">
          <span className="client-avatar small" aria-hidden="true">
            {initial(client.name)}
          </span>
          <p>
            <bdi className="strong">{client.name}</bdi>
            {t(" می‌خواهد به حساب سلام شما وصل شود. برای ادامه وارد شوید.", " wants to connect to your Salam account. Sign in to continue.")}
          </p>
        </div>
        <AuthForm compact onAuth={onAuth} intro={t("با حساب سلام خود وارد شوید یا یک حساب تازه بسازید.", "Sign in with your Salam account or create a new one.")} />
      </>,
    );
  }

  return shell(
    <section className="auth-card connect-card" aria-labelledby="consent-title">
      <div className="connect-logos" aria-hidden="true">
        <img src="/logo.svg" alt="" width={48} height={48} className="connect-logo" />
        <span className="connect-dots">
          <i />
          <i />
          <i />
        </span>
        <span className="client-avatar">{initial(client.name)}</span>
      </div>
      <h2 id="consent-title">
        <bdi>{client.name}</bdi>
        {t(" می‌خواهد به حساب سلام شما دسترسی داشته باشد", " wants to access your Salam account")}
      </h2>
      <p className="connect-host">
        <span dir="ltr">{client.host}</span>
        {client.kind === "cimd" && (
          <span className="badge ok" title={t("نام این برنامه از روی نشانی آن بررسی شده است", "This app's name was verified from its address")}>
            <IconBadgeCheck width={14} height={14} />
            {t("تأییدشده", "Verified")}
          </span>
        )}
      </p>
      {client.kind === "dcr" && (
        <div className="warning" role="note">
          <IconAlert width={18} height={18} />
          <p>
            {t("نام «", "The name \"")}
            <bdi>{client.name}</bdi>
            {t(
              "» را خود برنامه اعلام کرده و سلام آن را تأیید نکرده است. فقط اگر همین الان خودتان این اتصال را آغاز کرده‌اید ادامه دهید.",
              "\" was declared by the app itself and Salam has not verified it. Only continue if you started this connection yourself just now.",
            )}
          </p>
        </div>
      )}
      <div className="account-box">
        <div className="avatar" aria-hidden="true">
          {initial(user.name)}
        </div>
        <div className="who">
          <strong dir="auto">{user.name}</strong>
          <span dir="ltr">{user.email}</span>
        </div>
        <button type="button" className="link small-link" onClick={onSwitchAccount}>
          {t("شما نیستید؟ تغییر حساب", "Not you? Switch account")}
        </button>
      </div>
      <h3 className="scope-head">{t("این برنامه می‌تواند:", "This app will be able to:")}</h3>
      <ul className="scope-list">
        {request.scopes.map((s) => (
          <li key={s.name}>
            <span className="scope-icon">
              <IconCheck width={16} height={16} />
            </span>
            <div>
              <strong>{s.title}</strong>
              {s.description && <span>{s.description}</span>}
            </div>
          </li>
        ))}
      </ul>
      <p className="connect-note">
        {t(
          "استفاده‌ی این برنامه از ابزارهای سلام در حساب شما ثبت می‌شود و هر زمان بخواهید می‌توانید از صفحه‌ی «اتصال‌ها و برنامه‌ها» اتصال آن را قطع کنید.",
          "This app's use of Salam tools is logged in your account, and you can disconnect it at any time from Connections and apps.",
        )}
      </p>
      {request.redirect_host && (
        <p className="connect-note">
          {t("پس از پاسخ شما به ", "After you answer, you will be returned to ")}
          <span dir="ltr">{request.redirect_host}</span>
          {t(" بازگردانده می‌شوید.", ".")}
        </p>
      )}
      {error && (
        <div className="form-error" role="alert">
          {error}
        </div>
      )}
      <div className="connect-actions">
        <button type="button" className="button ghost" disabled={busy !== ""} onClick={() => answer(request, false)}>
          {busy === "deny" ? <span className="spinner" /> : t("رد کردن", "Deny")}
        </button>
        <button type="button" className="button primary" disabled={busy !== ""} onClick={() => answer(request, true)}>
          {busy === "approve" ? <span className="spinner" /> : t("اجازه می‌دهم", "Allow")}
        </button>
      </div>
    </section>,
  );
}
