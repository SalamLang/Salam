import { useCallback, useRef, useState, type FormEvent } from "react";
import { useEffect } from "react";
import { api, ApiError, awaitProviderLogin, newPollSecret, rememberOAuth, setToken, type Plan, type User } from "./api";
import { IconCode, IconFigma, IconGithub, IconEye, IconGlobe, IconEyeOff, IconMoon, IconSun, IconSwap, IconBook, IconSpark } from "./icons";
import { fmtNum } from "./format";
import type { Theme } from "./theme";
import { isFa, setLang, t } from "./i18n";
import { autoDir } from "./dir";
import { isDesktop, useReturnReset } from "./desktop";

type Mode = "login" | "register";

export const LOGIN_ERRORS: Record<string, string> = {
  unavailable: t("ورود با این سرویس فعال نیست.", "Signing in with this service is not available."),
  denied: t("ورود لغو شد.", "Sign in was canceled."),
  expired: t("مهلت این ورود (۱۵ دقیقه) تمام شد. دوباره تلاش کنید.", "This sign in timed out after 15 minutes. Try again."),
  used: t("این ورود قبلا استفاده شده است. اگر وارد نشده‌اید دوباره تلاش کنید.", "This sign in was already used. If you are not signed in, try again."),
  missing: t("این ورود پیدا نشد. شاید پیوند قدیمی است. دوباره تلاش کنید.", "This sign in was not found. The link may be old. Try again."),
  unbound: t("ورود باید در همان مرورگری که شروع کردید تمام شود. دوباره از همان‌جا شروع کنید.", "Sign in has to finish in the browser where you started it. Start again from there."),
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
  const [on, setOn] = useState<{ github: boolean; figma: boolean } | null>(() => {
    try {
      const saved = JSON.parse(localStorage.getItem("salam-ai-providers") || "null");
      return saved && typeof saved.github === "boolean" && typeof saved.figma === "boolean" ? saved : null;
    } catch {
      return null;
    }
  });
  const [attempt, setAttempt] = useState<{ provider: "github" | "figma"; phase: "start" | "wait" } | null>(null);
  const seq = useRef(0);
  const cancel = useCallback(() => {
    seq.current += 1;
    setAttempt(null);
  }, []);
  useReturnReset(attempt?.phase === "wait", cancel);
  useEffect(() => {
    api
      .authProviders()
      .then((r) => {
        setOn(r);
        try {
          localStorage.setItem("salam-ai-providers", JSON.stringify(r));
        } catch {}
      })
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
    const mine = ++seq.current;
    setAttempt({ provider, phase: "start" });
    setError("");
    try {
      const next = nextPath();
      const poll = isDesktop ? newPollSecret() : "";
      const r = await api.startProvider(provider, next, poll);
      if (mine !== seq.current) return;
      rememberOAuth({ provider, state: r.state, purpose: "login", next });
      location.assign(r.url);
      setAttempt({ provider, phase: "wait" });
      if (poll) {
        awaitProviderLogin(poll, () => mine === seq.current)
          .then((done) => {
            if (mine !== seq.current) return;
            setToken(done.token);
            location.replace(done.next.startsWith("/") && !done.next.startsWith("//") ? done.next : "/");
          })
          .catch((e) => {
            if (mine !== seq.current) return;
            setAttempt(null);
            setError(e instanceof ApiError ? (LOGIN_ERRORS[e.code] ?? t("ورود انجام نشد. دوباره تلاش کنید.", "Sign in failed. Try again.")) : t("ورود انجام نشد. دوباره تلاش کنید.", "Sign in failed. Try again."));
          });
      }
    } catch (e) {
      if (mine !== seq.current) return;
      setAttempt(null);
      setError(e instanceof ApiError ? (LOGIN_ERRORS[e.code] ?? e.message) : t("ورود انجام نشد. دوباره تلاش کنید.", "Sign in failed. Try again."));
    }
  };
  return (
    <>
      <div className="auth-or" role="separator">
        <span>{t("یا", "or")}</span>
      </div>
      <div className="auth-providers">
        {on.github && (
          <button type="button" className="button ghost wide" disabled={attempt?.provider === "github" && attempt.phase === "start"} onClick={() => begin("github")}>
            {attempt?.provider === "github" ? <span className="spinner" /> : <IconGithub width={18} height={18} />}
            {t("ادامه با GitHub", "Continue with GitHub")}
          </button>
        )}
        {on.figma && (
          <button type="button" className="button ghost wide" disabled={attempt?.provider === "figma" && attempt.phase === "start"} onClick={() => begin("figma")}>
            {attempt?.provider === "figma" ? <span className="spinner" /> : <IconFigma width={18} height={18} />}
            {t("ادامه با Figma", "Continue with Figma")}
          </button>
        )}
        {attempt?.phase === "wait" && isDesktop && (
          <p className="auth-wait" role="status">
            {t("منتظر تایید در مرورگر...", "Waiting for approval in the browser...")}{" "}
            <button type="button" className="auth-wait-cancel" onClick={cancel}>
              {t("لغو", "Cancel")}
            </button>
          </p>
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
          <input value={name} dir={autoDir(name, t("مثلا سارا احمدی", "e.g. Sara Ahmadi"))} onChange={(e) => setName(e.target.value)} autoComplete="name" placeholder={t("مثلا سارا احمدی", "e.g. Sara Ahmadi")} maxLength={80} />
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

function tomanShort(n: number): string {
  if (n >= 1000000 && n % 1000000 === 0) return t(`${fmtNum(n / 1000000)} میلیون تومان`, `${fmtNum(n / 1000000)} million toman`);
  return t(`${fmtNum(n)} تومان`, `${fmtNum(n)} toman`);
}

function PlansTeaser() {
  const [open, setOpen] = useState(false);
  const [plans, setPlans] = useState<Plan[] | null>(null);
  const [error, setError] = useState("");
  useEffect(() => {
    if (!open || plans !== null) return;
    api
      .plans()
      .then((r) => setPlans(r.plans))
      .catch(() => setError(t("بارگذاری پلن‌ها ناموفق بود.", "Could not load plans.")));
  }, [open, plans]);
  return (
    <div className="auth-plans-teaser">
      <button type="button" className="link" onClick={() => setOpen((v) => !v)}>
        <IconSpark width={14} height={14} /> {open ? t("بستن پلن‌ها", "Close plans") : t("دیدن پلن‌ها و قیمت‌ها", "See plans and pricing")}
      </button>
      {open && (
        <div className="auth-plans-grid">
          {plans === null && !error && (
            <div className="list-loading" role="status">
              <span className="spinner" />
            </div>
          )}
          {error && <p className="muted">{error}</p>}
          {plans?.map((p) => (
            <div key={p.id} className="auth-plan-card">
              <strong>{isFa ? p.name : p.name_en || p.name}</strong>
              <span className="auth-plan-price">{p.price === 0 ? t("رایگان", "Free") : tomanShort(p.price)}</span>
              <span className="muted">{isFa ? p.note : p.note_en || p.note}</span>
            </div>
          ))}
          {plans && (
            <p className="muted auth-plans-cta">{t("برای خرید یک پلن، ابتدا ثبت‌نام یا وارد شوید.", "To buy a plan, sign up or sign in first.")}</p>
          )}
        </div>
      )}
    </div>
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
          <PlansTeaser />
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
