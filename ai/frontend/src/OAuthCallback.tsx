import { useEffect, useState } from "react";
import { api, ApiError, setToken, takeOAuth } from "./api";
import { LOGIN_ERRORS } from "./Auth";
import { Notice, Shell } from "./Connect";
import type { Theme } from "./theme";
import { t } from "./i18n";

export function providerFromPath(): string {
  return /^\/auth\/callback\/(github|figma)\/?$/.exec(location.pathname)?.[1] ?? "";
}

function safe(next: string | undefined): string {
  return next && next.startsWith("/") && !next.startsWith("//") ? next : "/";
}

export function OAuthCallback({ theme, toggleTheme }: { theme: Theme; toggleTheme: () => void }) {
  const [problem, setProblem] = useState("");

  useEffect(() => {
    const provider = providerFromPath();
    const q = new URLSearchParams(location.search);
    const pending = takeOAuth();
    const code = q.get("code") ?? "";
    const state = q.get("state") ?? "";
    const back = (reason: string) => {
      if (pending?.purpose === "connect") location.replace(`/connections?connected=${provider}&result=failed`);
      else location.replace(`/?login_error=${encodeURIComponent(reason)}${pending?.next ? `&next=${encodeURIComponent(pending.next)}` : ""}`);
    };
    if (!provider || !state || !pending || pending.provider !== provider || pending.state !== state) {
      if (pending?.purpose === "connect") back("expired");
      else setProblem(LOGIN_ERRORS.expired);
      return;
    }
    api
      .completeProvider(provider, code, state, q.get("error") ?? "")
      .then((r) => {
        if (r.purpose === "connect") {
          location.replace(`/connections?connected=${provider}&result=${r.ok ? "ok" : "failed"}`);
          return;
        }
        if (r.token) setToken(r.token);
        location.replace(safe(r.next || pending.next));
      })
      .catch((e) => {
        if (e instanceof ApiError && e.status === 0) setProblem(e.message);
        else back(e instanceof ApiError ? e.code : "failed");
      });
  }, []);

  return (
    <Shell theme={theme} toggleTheme={toggleTheme}>
      {problem ? (
        <Notice tone="danger" title={t("ورود انجام نشد", "Sign in failed")}>
          <p className="muted">{problem}</p>
          <a className="button primary wide" href="/">
            {t("بازگشت به صفحه‌ی ورود", "Back to sign in")}
          </a>
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
