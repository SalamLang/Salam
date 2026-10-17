import { useState, type FormEvent } from "react";
import { api, ApiError, setToken, type User } from "./api";
import { IconCode, IconEye, IconEyeOff, IconMoon, IconSun, IconSwap, IconBook } from "./icons";
import type { Theme } from "./theme";

type Mode = "login" | "register";

export function AuthForm({ onAuth, compact = false, intro }: { onAuth: (u: User) => void; compact?: boolean; intro?: string }) {
  const [mode, setMode] = useState<Mode>("login");
  const [name, setName] = useState("");
  const [email, setEmail] = useState("");
  const [password, setPassword] = useState("");
  const [show, setShow] = useState(false);
  const [busy, setBusy] = useState(false);
  const [error, setError] = useState("");

  const submit = async (e: FormEvent) => {
    e.preventDefault();
    setError("");
    if (mode === "register" && name.trim().length < 2) return setError("نام خود را بنویسید.");
    if (!email.includes("@")) return setError("نشانی ایمیل درست نیست.");
    if (password.length < 8) return setError("گذرواژه باید دست‌کم ۸ نویسه باشد.");
    setBusy(true);
    try {
      const res = mode === "login" ? await api.login(email, password) : await api.register(name.trim(), email, password);
      setToken(res.token);
      onAuth(res.user);
    } catch (err) {
      setError(err instanceof ApiError ? err.message : "خطایی پیش آمد.");
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
      <h2>{mode === "login" ? "ورود به حساب" : "ساخت حساب تازه"}</h2>
      <p className="muted">{intro ?? (mode === "login" ? "خوش برگشتید! برای ادامه وارد شوید." : "برای گفت‌وگو با دستیار سلام یک حساب بسازید.")}</p>
      <div className="tabs" role="tablist">
        <button type="button" role="tab" aria-selected={mode === "login"} className={mode === "login" ? "active" : ""} onClick={() => switchMode("login")}>
          ورود
        </button>
        <button type="button" role="tab" aria-selected={mode === "register"} className={mode === "register" ? "active" : ""} onClick={() => switchMode("register")}>
          ثبت‌نام
        </button>
      </div>
      {mode === "register" && (
        <label className="field">
          <span>نام</span>
          <input value={name} onChange={(e) => setName(e.target.value)} autoComplete="name" placeholder="مثلا سارا احمدی" maxLength={80} />
        </label>
      )}
      <label className="field">
        <span>ایمیل</span>
        <input type="email" dir="ltr" value={email} onChange={(e) => setEmail(e.target.value)} autoComplete="email" placeholder="you@example.com" />
      </label>
      <label className="field">
        <span>گذرواژه</span>
        <div className="password">
          <input
            type={show ? "text" : "password"}
            dir="ltr"
            value={password}
            onChange={(e) => setPassword(e.target.value)}
            autoComplete={mode === "login" ? "current-password" : "new-password"}
            placeholder="دست‌کم ۸ نویسه"
          />
          <button type="button" className="icon-button" onClick={() => setShow((s) => !s)} aria-label={show ? "پنهان کردن گذرواژه" : "نمایش گذرواژه"}>
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
        {busy ? <span className="spinner" /> : mode === "login" ? "ورود" : "ساخت حساب"}
      </button>
      <p className="switch">
        {mode === "login" ? "حساب ندارید؟ " : "حساب دارید؟ "}
        <button type="button" className="link" onClick={() => switchMode(mode === "login" ? "register" : "login")}>
          {mode === "login" ? "ثبت‌نام کنید" : "وارد شوید"}
        </button>
      </p>
    </form>
  );
}

export function Auth({ onAuth, theme, toggleTheme }: { onAuth: (u: User) => void; theme: Theme; toggleTheme: () => void }) {
  return (
    <div className="auth">
      <button type="button" className="icon-button theme-corner" onClick={toggleTheme} aria-label="تغییر پوسته">
        {theme === "dark" ? <IconSun /> : <IconMoon />}
      </button>
      <section className="auth-brand">
        <div className="auth-brand-inner">
          <img src="/logo.svg" alt="" className="auth-logo" width={72} height={72} />
          <h1>دستیار هوشمند سلام</h1>
          <p className="lead">همراه شما برای یادگیری و نوشتن برنامه با سلام، نخستین زبان برنامه‌نویسی ایرانی.</p>
          <ul className="features">
            <li>
              <span className="feature-icon"><IconCode /></span>
              <div>
                <strong>کد سلام بنویسید</strong>
                <span>از یک ایده تا برنامه‌ی کامل، به فارسی یا انگلیسی</span>
              </div>
            </li>
            <li>
              <span className="feature-icon"><IconBook /></span>
              <div>
                <strong>کتابخانه‌ی استاندارد را بشناسید</strong>
                <span>پاسخ‌ها بر پایه‌ی مستندات واقعی سلام</span>
              </div>
            </li>
            <li>
              <span className="feature-icon"><IconSwap /></span>
              <div>
                <strong>کد خود را به سلام ببرید</strong>
                <span>تبدیل از پایتون، جاوااسکریپت، C و زبان‌های دیگر</span>
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
          <a href="https://editor.salamlang.ir" target="_blank" rel="noreferrer">ویرایشگر آنلاین</a>
          <span>·</span>
          <a href="https://www.salamlang.ir/learn/" target="_blank" rel="noreferrer">آموزش سلام</a>
        </p>
      </section>
    </div>
  );
}
