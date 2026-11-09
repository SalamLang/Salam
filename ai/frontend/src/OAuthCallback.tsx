import { useEffect, useState } from "react";
import { api, ApiError, rememberOAuth, setToken, takeOAuth } from "./api";
import { LOGIN_ERRORS } from "./Auth";
import { Notice, Shell } from "./Connect";
import type { Theme } from "./theme";
import { t } from "./i18n";

export function providerFromPath(): string {
  return /^\/auth\/callback\/(github|figma-mcp|figma)\/?$/.exec(location.pathname)?.[1] ?? "";
}

function safe(next: string | undefined): string {
  return next && next.startsWith("/") && !next.startsWith("//") ? next : "/";
}

type Problem = { code: string; text: string; handoff: boolean; next: string };

const RETRY = new Set(["missing", "used", "expired", "unbound", "denied", "failed", "server"]);

function explain(code: string, detail: string): string {
  const base = LOGIN_ERRORS[code] ?? t("ورود انجام نشد.", "Sign in failed.");
  return code === "denied" && detail ? `${base} ${t("پیام سرویس", "The service said")}: ${detail}` : base;
}

export function OAuthCallback({ theme, toggleTheme }: { theme: Theme; toggleTheme: () => void }) {
  const [problem, setProblem] = useState<Problem | null>(null);
  const [done, setDone] = useState("");
  const [again, setAgain] = useState(false);

  useEffect(() => {
    const provider = providerFromPath();
    const q = new URLSearchParams(location.search);
    const pending = takeOAuth();
    const code = q.get("code") ?? "";
    const state = q.get("state") ?? "";
    const next = pending?.next ?? "";
    const fail = (c: string, detail: string, handoff: boolean) => setProblem({ code: c, text: explain(c, detail), handoff, next });
    if (provider && code && !state && q.get("installation_id")) {
      api
        .startProvider(provider, "/")
        .then((r) => {
          rememberOAuth({ provider, state: r.state, purpose: "login", next: "/" });
          location.replace(r.url);
        })
        .catch(() => fail("missing", "", false));
      return;
    }
    if (!provider || !state) {
      fail(q.get("error") ? "denied" : "missing", q.get("error_description") ?? q.get("error") ?? "", false);
      return;
    }
    const bound = Boolean(pending && pending.provider === provider && pending.state === state);
    const mine = bound ? pending : null;
    api
      .completeProvider(provider, code, state, q.get("error") ?? "", q.get("error_description") ?? "", bound)
      .then((r) => {
        if (r.purpose === "handoff") {
          setDone(t("ورود انجام شد. به برنامه‌ی سلام برگردید.", "You are signed in. Go back to the Salam app."));
          return;
        }
        if (r.purpose === "connect") {
          if (mine?.purpose === "connect") {
            location.replace(`/connections?connected=${provider}&result=${r.ok ? "ok" : "failed"}${r.ok || !r.reason ? "" : `&reason=${encodeURIComponent(r.reason)}`}`);
          } else {
            setDone(r.ok ? t("حساب وصل شد. به برنامه برگردید.", "The account is connected. Go back to the app.") : (r.reason ?? t("وصل کردن ناموفق بود.", "Could not connect.")));
          }
          return;
        }
        if (r.token) setToken(r.token);
        location.replace(safe(r.next || mine?.next));
      })
      .catch((e) => {
        if (e instanceof ApiError && e.status === 0) fail("server", "", false);
        else if (mine?.purpose === "connect") location.replace(`/connections?connected=${provider}&result=failed`);
        else fail(e instanceof ApiError ? e.code : "failed", e instanceof ApiError ? e.detail : "", e instanceof ApiError && e.handoff);
      });
  }, []);

  const retry = async () => {
    const provider = providerFromPath() as "github" | "figma" | "";
    if (!provider || !problem) return;
    setAgain(true);
    try {
      const r = await api.startProvider(provider, problem.next);
      rememberOAuth({ provider, state: r.state, purpose: "login", next: problem.next });
      location.assign(r.url);
    } catch (e) {
      setAgain(false);
      setProblem({ ...problem, text: e instanceof ApiError ? (LOGIN_ERRORS[e.code] ?? e.message) : problem.text });
    }
  };

  return (
    <Shell theme={theme} toggleTheme={toggleTheme}>
      {problem ? (
        <Notice tone="danger" title={t("ورود انجام نشد", "Sign in failed")}>
          <p className="muted">{problem.text}</p>
          {problem.handoff ? (
            <p className="muted">{t("به برنامه‌ی سلام برگردید و دوباره تلاش کنید.", "Go back to the Salam app and try again.")}</p>
          ) : (
            <>
              {RETRY.has(problem.code) && providerFromPath() && (
                <button type="button" className="button primary wide" disabled={again} onClick={retry}>
                  {again && <span className="spinner" />}
                  {t("تلاش دوباره", "Try again")}
                </button>
              )}
              <a className={`button ${RETRY.has(problem.code) ? "ghost" : "primary"} wide`} href="/">
                {t("بازگشت به صفحه‌ی ورود", "Back to sign in")}
              </a>
            </>
          )}
        </Notice>
      ) : done ? (
        <Notice tone="info" title={t("انجام شد", "Done")}>
          <p className="muted">{done}</p>
        </Notice>
      ) : (
        <div className="loading-chat" role="status">
          <span className="spinner" />
          <span className="sr-only">{t("در حال ورود", "Signing in")}</span>
        </div>
      )}
    </Shell>
  );
}
