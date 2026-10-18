import { useCallback, useEffect, useState, type ReactNode } from "react";
import { api, ApiError, type ConsentRequest, type User } from "./api";
import { AuthForm } from "./Auth";
import { initial } from "./format";
import { IconAlert, IconBadgeCheck, IconCheck, IconLink, IconMoon, IconSun } from "./icons";
import type { Theme } from "./theme";

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
  access_denied: "دسترسی داده نشد و اتصال لغو شد.",
  invalid_request: "درخواستی که برنامه فرستاده کامل یا درست نیست.",
  invalid_client: "این برنامه شناخته نشد.",
  unauthorized_client: "این برنامه اجازه‌ی چنین درخواستی را ندارد.",
  invalid_scope: "برنامه دسترسی‌ای خواسته که سلام آن را نمی‌شناسد.",
  unsupported_response_type: "روش درخواست این برنامه پشتیبانی نمی‌شود.",
  server_error: "در سرور سلام خطایی پیش آمد.",
  temporarily_unavailable: "سرویس سلام موقتا در دسترس نیست. کمی بعد دوباره تلاش کنید.",
};

function Shell({ theme, toggleTheme, children }: { theme: Theme; toggleTheme: () => void; children: ReactNode }) {
  return (
    <div className="connect">
      <button type="button" className="icon-button theme-corner" onClick={toggleTheme} aria-label="تغییر پوسته">
        {theme === "dark" ? <IconSun /> : <IconMoon />}
      </button>
      <main className="connect-inner">{children}</main>
      <p className="auth-foot">
        <a href="https://www.salamlang.ir" target="_blank" rel="noreferrer">
          salamlang.ir
        </a>
        <span>·</span>
        <a href="https://www.salamlang.ir/learn/" target="_blank" rel="noreferrer">
          آموزش سلام
        </a>
      </p>
    </div>
  );
}

function Notice({ tone = "info", title, children }: { tone?: "info" | "danger"; title: string; children?: ReactNode }) {
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
    const t = window.setTimeout(() => setLate(true), 4000);
    return () => window.clearTimeout(t);
  }, []);
  return (
    <section className="auth-card connect-card notice-card" aria-live="polite">
      <div className={`notice-icon ${approved ? "ok" : "info"}`}>{approved ? <IconCheck /> : <IconLink />}</div>
      <h2>{approved ? "اجازه داده شد" : "درخواست رد شد"}</h2>
      <p className="muted">
        در حال بازگشت به <bdi className="strong">{request.client.name}</bdi>
        {request.redirect_host && (
          <>
            {" "}
            (<span dir="ltr">{request.redirect_host}</span>)
          </>
        )}{" "}
        ...
      </p>
      <span className="spinner connect-spinner" aria-hidden="true" />
      {late && <p className="connect-hint">اگر برنامه خودش باز شد یا کارتان تمام است، می‌توانید این پنجره را ببندید.</p>}
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
      else setState({ kind: "failed", message: e instanceof ApiError ? e.message : "خطایی پیش آمد." });
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
      else setError(e instanceof ApiError ? e.message : "خطایی پیش آمد. دوباره تلاش کنید.");
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
          <span className="sr-only">در حال بارگذاری درخواست</span>
        </div>,
      );
    case "missing":
      return shell(
        <Notice tone="danger" title="درخواست اتصال پیدا نشد">
          <p className="muted">این نشانی کامل نیست. اتصال را دوباره از برنامه‌ای که می‌خواهید به سلام وصل کنید آغاز کنید.</p>
          <a className="button ghost wide" href="/">
            رفتن به دستیار سلام
          </a>
        </Notice>,
      );
    case "expired":
      return shell(
        <Notice tone="danger" title="این درخواست منقضی شده است">
          <p className="muted">درخواست‌های اتصال فقط چند دقیقه معتبرند. به برنامه برگردید و اتصال را دوباره آغاز کنید.</p>
        </Notice>,
      );
    case "used":
      return shell(
        <Notice title="به این درخواست پاسخ داده شده است">
          <p className="muted">این درخواست اتصال قبلا تأیید یا رد شده است. اگر برنامه هنوز منتظر است، اتصال را از همان برنامه دوباره آغاز کنید.</p>
          <p className="connect-hint">می‌توانید این پنجره را ببندید.</p>
        </Notice>,
      );
    case "failed":
      return shell(
        <Notice tone="danger" title="درخواست بارگذاری نشد">
          <p className="muted">{state.message}</p>
          <button type="button" className="button primary wide" onClick={load}>
            تلاش دوباره
          </button>
        </Notice>,
      );
    case "oauth-error":
      return shell(
        <Notice tone="danger" title="اتصال انجام نشد">
          <p className="muted">{ERROR_TEXT[state.code] || "در روند اتصال برنامه به حساب سلام خطایی پیش آمد."}</p>
          {state.description && (
            <p className="connect-detail" dir="auto">
              {state.description}
            </p>
          )}
          <p className="connect-code">
            کد خطا: <code dir="ltr">{state.code}</code>
          </p>
          <p className="connect-hint">به برنامه برگردید و اتصال را دوباره آغاز کنید.</p>
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
            <bdi className="strong">{client.name}</bdi> می‌خواهد به حساب سلام شما وصل شود. برای ادامه وارد شوید.
          </p>
        </div>
        <AuthForm compact onAuth={onAuth} intro="با حساب سلام خود وارد شوید یا یک حساب تازه بسازید." />
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
        <bdi>{client.name}</bdi> می‌خواهد به حساب سلام شما دسترسی داشته باشد
      </h2>
      <p className="connect-host">
        <span dir="ltr">{client.host}</span>
        {client.kind === "cimd" && (
          <span className="badge ok" title="نام این برنامه از روی نشانی آن بررسی شده است">
            <IconBadgeCheck width={14} height={14} />
            تأییدشده
          </span>
        )}
      </p>
      {client.kind === "dcr" && (
        <div className="warning" role="note">
          <IconAlert width={18} height={18} />
          <p>
            نام «<bdi>{client.name}</bdi>» را خود برنامه اعلام کرده و سلام آن را تأیید نکرده است. فقط اگر همین الان خودتان این اتصال را آغاز کرده‌اید ادامه دهید.
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
          شما نیستید؟ تغییر حساب
        </button>
      </div>
      <h3 className="scope-head">این برنامه می‌تواند:</h3>
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
        استفاده‌ی این برنامه از ابزارهای سلام در حساب شما ثبت می‌شود و هر زمان بخواهید می‌توانید از بخش «برنامه‌های متصل» اتصال آن را قطع کنید.
      </p>
      {request.redirect_host && (
        <p className="connect-note">
          پس از پاسخ شما به <span dir="ltr">{request.redirect_host}</span> بازگردانده می‌شوید.
        </p>
      )}
      {error && (
        <div className="form-error" role="alert">
          {error}
        </div>
      )}
      <div className="connect-actions">
        <button type="button" className="button ghost" disabled={busy !== ""} onClick={() => answer(request, false)}>
          {busy === "deny" ? <span className="spinner" /> : "رد کردن"}
        </button>
        <button type="button" className="button primary" disabled={busy !== ""} onClick={() => answer(request, true)}>
          {busy === "approve" ? <span className="spinner" /> : "اجازه می‌دهم"}
        </button>
      </div>
    </section>,
  );
}
