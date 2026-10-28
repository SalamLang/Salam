import { useState, type FormEvent } from "react";
import { useEffect } from "react";
import { api, ApiError, rememberOAuth, setToken, type User } from "./api";
import { IconCode, IconFigma, IconGithub, IconEye, IconGlobe, IconEyeOff, IconMoon, IconSun, IconSwap, IconBook } from "./icons";
import type { Theme } from "./theme";
import { isFa, setLang, t } from "./i18n";

type Mode = "login" | "register";

export const LOGIN_ERRORS: Record<string, string> = {
  unavailable: t("ورود با این سرویس فعال نیست.", "Signing in with this service is not available."),
  denied: t("ورود لغو شد.", "Sign in was canceled."),
  expired: t("نشست ورود منقضی شد. دوباره تلاش کنید.", "The sign in expired. Try again."),
  failed: t("سرویس ورود پاسخ درستی نداد. دوباره تلاش کنید.", "The sign in service did not answer correctly. Try again."),
  server: t("خطایی در سرور پیش آمد. دوباره تلاش کنید.", "Something went wrong on the server. Try again."),
  email_exists: t("با این ایمیل قبلا حساب ساخته شده است. با ایمیل و گذرواژه وارد شوید.", "An account with this email already exists. Sign in with your email and password."),
  no_email: t("این حساب ایمیل تأییدشده‌ای ندارد. در آن سرویس یک ایمیل تأییدشده اضافه کنید یا با ایمیل ثبت‌نام کنید.", "This account has no verified email. Add a verified email there, or sign up with email."),
};

function nextPath(): string {
  const p = new URLSearchParams(location.search).get("next");
  if (p && p.startsWith("/") && !p.startsWith("//")) return p;
  if (location.pathname !== "/" && location.pathname !== "/auth/done") return location.pathname + location.search;
  return "";
}

function ProviderButtons({ setError }: { setError: (m: string) => void }) {
  const [on, setOn] = useState<{ github: boolean; figma: boolean } | null>(null);
  const [busy, setBusy] = useState("");
  useEffect(() => {
    api
      .authProviders()
      .then(setOn)
      .catch(() => {});
    const code = new URLSearchParams(location.search).get("login_error");
    if (code) {
      setError(LOGIN_ERRORS[code] ?? t("ورود انجام نشد.", "Sign in failed."));
      const p = new URLSearchParams(location.search);
      p.delete("login_error");
      p.delete("next");
      const rest = p.toString();
      history.replaceState(null, "", location.pathname + (rest ? `?${rest}` : ""));
    }
  }, [setError]);
  if (!on || (!on.github && !on.figma)) return null;
  const begin = async (provider: "github" | "figma") => {
    setBusy(provider);
    setError("");
    try {
      const next = nextPath();
      const r = await api.startProvider(provider, next);
      rememberOAuth({ provider, state: r.state, purpose: "login", next });
      location.assign(r.url);
    } catch (e) {
      setBusy("");
      setError(e instanceof ApiError ? (LOGIN_ERRORS[e.code] ?? e.message) : t("ورود انجام نشد.", "Sign in failed."));
    }
  };
  return (
    <>
      <div className="auth-or" role="separator">
        <span>{t("یا", "or")}</span>
      </div>
      <div className="auth-providers">
        {on.github && (
          <button type="button" className="button ghost wide" disabled={busy !== ""} onClick={() => begin("github")}>
            {busy === "github" ? <span className="spinner" /> : <IconGithub width={18} height={18} />}
            {t("ادامه با GitHub", "Continue with GitHub")}
          </button>
        )}
        {on.figma && (
          <button type="button" className="button ghost wide" disabled={busy !== ""} onClick={() => begin("figma")}>
            {busy === "figma" ? <span className="spinner" /> : <IconFigma width={18} height={18} />}
            {t("ادامه با Figma", "Continue with Figma")}
          </button>
        )}
      </div>
    </>
  );
}

export function AuthForm({ onAuth, compact = false, intro }: { onAuth: (u: User) => void; compact?: boolean; intro?: string }) {
  const [mode, setMode] = useState<Mode>(() => (new URLSearchParams(location.search).get("signup") ? "register" : "login"));
  const [name, setName] = useState("");
  const [email, setEmail] = useState("");
  const [password, setPassword] = useState("");
  const [show, setShow] = useState(false);
  const [busy, setBusy] = useState(false);
  const [error, setError] = useState("");

  const submit = async (e: FormEvent) => {
    e.preventDefault();
    setError("");
    if (mode === "register" && name.trim().length < 2) return setError(t("نام خود را بنویسید.", "Enter your name."));
    if (!email.includes("@")) return setError(t("نشانی ایمیل درست نیست.", "That email address is not valid."));
    if (password.length < 8) return setError(t("گذرواژه باید دست‌کم ۸ نویسه باشد.", "Password must be at least 8 characters."));
    setBusy(true);
    try {
      const res = mode === "login" ? await api.login(email, password) : await api.register(name.trim(), email, password);
      setToken(res.token);
      onAuth(res.user);
    } catch (err) {
      setError(err instanceof ApiError ? err.message : t("خطایی پیش آمد.", "Something went wrong."));
    } finally {
      setBusy(false);
    }
  };

  const switchMode = (m: Mode) => {
    setMode(m);
    setError("");
  };

  return (
    <form className={`auth-card${compact ? " compact" : ""}`} onSubmit={submit} noValidate>
      <img src="/logo.svg" alt="" className="auth-card-logo" width={44} height={44} />
      <h2>{mode === "login" ? t("ورود به حساب", "Sign in") : t("ساخت حساب تازه", "Create an account")}</h2>
      <p className="muted">{intro ?? (mode === "login" ? t("خوش برگشتید! برای ادامه وارد شوید.", "Welcome back! Sign in to continue.") : t("برای گفت‌وگو با دستیار سلام یک حساب بسازید.", "Create an account to chat with Salam AI."))}</p>
      <div className="tabs" role="tablist">
        <button type="button" role="tab" aria-selected={mode === "login"} className={mode === "login" ? "active" : ""} onClick={() => switchMode("login")}>
          {t("ورود", "Sign in")}
        </button>
        <button type="button" role="tab" aria-selected={mode === "register"} className={mode === "register" ? "active" : ""} onClick={() => switchMode("register")}>
          {t("ثبت‌نام", "Sign up")}
        </button>
      </div>
      {mode === "register" && (
        <label className="field">
          <span>{t("نام", "Name")}</span>
          <input value={name} onChange={(e) => setName(e.target.value)} autoComplete="name" placeholder={t("مثلا سارا احمدی", "e.g. Sara Ahmadi")} maxLength={80} />
        </label>
      )}
      <label className="field">
        <span>{t("ایمیل", "Email")}</span>
        <input type="email" dir="ltr" value={email} onChange={(e) => setEmail(e.target.value)} autoComplete="email" placeholder="you@example.com" />
      </label>
      <label className="field">
        <span>{t("گذرواژه", "Password")}</span>
        <div className="password">
          <input
            type={show ? "text" : "password"}
            dir="ltr"
            value={password}
            onChange={(e) => setPassword(e.target.value)}
            autoComplete={mode === "login" ? "current-password" : "new-password"}
            placeholder={t("دست‌کم ۸ نویسه", "At least 8 characters")}
          />
          <button type="button" className="icon-button" onClick={() => setShow((s) => !s)} aria-label={show ? t("پنهان کردن گذرواژه", "Hide password") : t("نمایش گذرواژه", "Show password")}>
            {show ? <IconEyeOff width={18} height={18} /> : <IconEye width={18} height={18} />}
          </button>
        </div>
      </label>
      {error && (
        <div className="form-error" role="alert">
          {error}
        </div>
      )}
      <button type="submit" className="button primary wide" disabled={busy}>
        {busy ? <span className="spinner" /> : mode === "login" ? t("ورود", "Sign in") : t("ساخت حساب", "Create account")}
      </button>
      <ProviderButtons setError={setError} />
      <p className="switch">
        {mode === "login" ? t("حساب ندارید؟ ", "No account? ") : t("حساب دارید؟ ", "Have an account? ")}
        <button type="button" className="link" onClick={() => switchMode(mode === "login" ? "register" : "login")}>
          {mode === "login" ? t("ثبت‌نام کنید", "Sign up") : t("وارد شوید", "Sign in")}
        </button>
      </p>
    </form>
  );
}

export function Auth({ onAuth, theme, toggleTheme }: { onAuth: (u: User) => void; theme: Theme; toggleTheme: () => void }) {
  return (
    <div className="auth">
      <button type="button" className="icon-button theme-corner" onClick={toggleTheme} aria-label={t("تغییر پوسته", "Toggle theme")}>
        {theme === "dark" ? <IconSun /> : <IconMoon />}
      </button>
      <section className="auth-brand">
        <div className="auth-brand-inner">
          <img src="/logo.svg" alt="" className="auth-logo" width={72} height={72} />
          <h1>{t("دستیار هوشمند سلام", "Salam AI")}</h1>
          <p className="lead">{t("همراه شما برای یادگیری و نوشتن برنامه با سلام، نخستین زبان برنامه‌نویسی ایرانی.", "Your companion for learning and writing programs in Salam, the first Iranian programming language.")}</p>
          <ul className="features">
            <li>
              <span className="feature-icon"><IconCode /></span>
              <div>
                <strong>{t("کد سلام بنویسید", "Write Salam code")}</strong>
                <span>{t("از یک ایده تا برنامه‌ی کامل، به فارسی یا انگلیسی", "From an idea to a full program, in Persian or English")}</span>
              </div>
            </li>
            <li>
              <span className="feature-icon"><IconBook /></span>
              <div>
                <strong>{t("کتابخانه‌ی استاندارد را بشناسید", "Explore the standard library")}</strong>
                <span>{t("پاسخ‌ها بر پایه‌ی مستندات واقعی سلام", "Answers grounded in the real Salam docs")}</span>
              </div>
            </li>
            <li>
              <span className="feature-icon"><IconSwap /></span>
              <div>
                <strong>{t("کد خود را به سلام ببرید", "Bring your code to Salam")}</strong>
                <span>{t("تبدیل از پایتون، جاوااسکریپت، C و زبان‌های دیگر", "Convert from Python, JavaScript, C and other languages")}</span>
              </div>
            </li>
          </ul>
        </div>
      </section>
      <section className="auth-panel">
        <AuthForm onAuth={onAuth} />
        <p className="auth-foot">
          <a href="https://www.salamlang.ir" target="_blank" rel="noreferrer">salamlang.ir</a>
          <span>·</span>
          <a href="https://editor.salamlang.ir" target="_blank" rel="noreferrer">{t("ویرایشگر آنلاین", "Online editor")}</a>
          <span>·</span>
          <a href="https://www.salamlang.ir/learn/" target="_blank" rel="noreferrer">{t("آموزش سلام", "Learn Salam")}</a>
          <span>·</span>
          <button type="button" className="link" onClick={() => setLang(isFa ? "en" : "fa")} aria-label={t("تغییر زبان به انگلیسی", "Switch to Persian")}>
            <IconGlobe width={14} height={14} /> {isFa ? "English" : "فارسی"}
          </button>
        </p>
      </section>
    </div>
  );
}
