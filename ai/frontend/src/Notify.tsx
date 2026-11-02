import { useEffect, useRef, useState } from "react";
import { agent, isDesktop, type AgentInfo, type NotifyKind, type NotifyPrefs } from "./desktop";
import { IconBell } from "./icons";
import { Checkbox } from "./Checkbox";
import { lang, t } from "./i18n";
import { chime, pushToast, setToastSound } from "./Toasts";

let prefs: NotifyPrefs = { enabled: true, sound: true, preview: true, lang: "fa" };

function apply(next: NotifyPrefs) {
  prefs = next;
  setToastSound(next.sound);
  return next;
}

function snippet(text: string, max = 150): string {
  const flat = text
    .replace(/```[\s\S]*?```/g, " ")
    .replace(/\[([^\]]*)\]\([^)]*\)/g, "$1")
    .replace(/[#>*`_]/g, "")
    .replace(/\s+/g, " ")
    .trim();
  return flat.length > max ? `${flat.slice(0, max - 1).trimEnd()}…` : flat;
}

export function announce(kind: NotifyKind, chat: string, text: string, mid?: number) {
  if (!prefs.enabled) return;
  const tone = kind === "failed" ? "failed" : kind === "network" ? "network" : "done";
  const title =
    kind === "failed"
      ? t("پاسخ ناموفق بود", "The reply failed")
      : kind === "network"
        ? t("ارتباط قطع شد", "Connection lost")
        : t("پاسخ آماده است", "Reply ready");
  const body =
    kind === "network"
      ? t("ارتباط با سرور برقرار نیست. پاسخ ممکن است همچنان برسد.", "Cannot reach the server. The reply may still arrive.")
      : prefs.preview
        ? snippet(text) || chat
        : undefined;
  pushToast({ tone, title, body: body || undefined });
  chime(tone);
  if (!isDesktop) return;
  agent.notify(kind, chat, text, mid).catch(() => {});
}

export function announceApproval(summary: string) {
  pushToast({
    tone: "approval",
    title: t("منتظر اجازه‌ی شما", "Waiting for your approval"),
    body: summary,
  });
  chime("approval");
}

export function syncNotifyLanguage() {
  if (!isDesktop) return;
  agent
    .notifyPrefs()
    .then((current) => {
      apply(current);
      if (current.lang !== lang) agent.setNotifyPrefs({ ...current, lang }).then(apply).catch(() => {});
    })
    .catch(() => {});
}

function failureHint(os: AgentInfo["os"] | undefined, reason: string): string {
  const base = t("اعلان نمایش داده نشد: ", "The notification could not be shown: ") + reason;
  if (os === "linux") {
    return base + t(" یک برنامه‌ی اعلان مثل dunst، mako یا سرویس اعلان میز کار خود را نصب و فعال کنید.", " Install and start a notification daemon such as dunst or mako, or the one your desktop provides.");
  }
  if (os === "windows") {
    return base + t(" تنظیمات Windows، بخش اعلان‌ها را بررسی کنید.", " Check Windows Settings, Notifications.");
  }
  if (os === "macos") {
    return base + t(" تنظیمات سیستم، بخش اعلان‌ها را بررسی کنید.", " Check System Settings, Notifications.");
  }
  return base;
}

export function NotifyMenu({ os }: { os: AgentInfo["os"] | undefined }) {
  const [open, setOpen] = useState(false);
  const [current, setCurrent] = useState<NotifyPrefs | null>(null);
  const [result, setResult] = useState("");
  const [testing, setTesting] = useState(false);
  const box = useRef<HTMLDivElement>(null);

  useEffect(() => {
    if (!open) return;
    agent
      .notifyPrefs()
      .then((p) => setCurrent(apply(p)))
      .catch(() => {});
    const away = (e: MouseEvent) => {
      if (box.current && !box.current.contains(e.target as Node)) setOpen(false);
    };
    const esc = (e: KeyboardEvent) => {
      if (e.key === "Escape") setOpen(false);
    };
    document.addEventListener("mousedown", away);
    document.addEventListener("keydown", esc);
    return () => {
      document.removeEventListener("mousedown", away);
      document.removeEventListener("keydown", esc);
    };
  }, [open]);

  const change = (patch: Partial<NotifyPrefs>) => {
    if (!current) return;
    const next = { ...current, ...patch, lang };
    setCurrent(next);
    apply(next);
    agent
      .setNotifyPrefs(next)
      .then((saved) => setCurrent(apply(saved)))
      .catch(() => {});
  };

  const test = async () => {
    setTesting(true);
    setResult("");
    pushToast({
      tone: "info",
      title: t("اعلان آزمایشی", "Test notification"),
      body: t("اگر این پیام را می‌بینید، اعلان‌های ایجنت سلام کار می‌کنند.", "If you can see this, Salam Agent notifications work."),
      seconds: 8,
    });
    chime("info");
    try {
      const shown = await agent.notify("test");
      setResult(
        shown === "shown"
          ? t("هم اعلان سیستم و هم اعلان داخل برنامه فرستاده شد.", "Both the system notification and the in-app notice were sent.")
          : t(
              "اعلان سیستم نمایش داده نشد؛ اعلان داخل برنامه کار می‌کند. تنظیمات اعلان سیستم را بررسی کنید.",
              "The system notification was not shown; the in-app notice works. Check your system notification settings.",
            ),
      );
    } catch (e) {
      setResult(failureHint(os, typeof e === "string" ? e : e instanceof Error ? e.message : ""));
    } finally {
      setTesting(false);
    }
  };

  const on = current?.enabled ?? true;
  return (
    <div className="notify" ref={box}>
      <button
        type="button"
        className={`notify-btn${on ? "" : " off"}`}
        onClick={() => setOpen((o) => !o)}
        aria-expanded={open}
        aria-label={t("اعلان‌ها", "Notifications")}
        title={t("اعلان‌ها", "Notifications")}
      >
        <IconBell width={16} height={16} />
      </button>
      {open && (
        <div className="notify-pop" role="dialog" aria-label={t("اعلان‌ها", "Notifications")}>
          <strong>{t("اعلان‌های سیستم", "System notifications")}</strong>
          <p>{t("وقتی پاسخ آماده شد، خطایی پیش آمد، ارتباط قطع شد یا دستیار منتظر اجازه‌ی شما بود، اگر پنجره جلوی چشم شما نباشد خبر می‌دهیم؛ حتی وقتی پنجره کوچک شده یا ایجنت در پس‌زمینه است.", "When a reply is ready, something fails, the connection drops, or the assistant waits for your approval, we let you know if this window is not in front of you, even when it is minimized or the agent is in the background.")}</p>
          <Checkbox checked={on} onChange={(v) => change({ enabled: v })} label={t("نمایش اعلان", "Show notifications")} disabled={!current} />
          <Checkbox checked={current?.sound ?? true} onChange={(v) => change({ sound: v })} label={t("پخش صدا", "Play a sound")} disabled={!current || !on} />
          <Checkbox
            checked={current?.preview ?? true}
            onChange={(v) => change({ preview: v })}
            label={t("نمایش پیش‌نمایش پاسخ", "Show a reply preview")}
            hint={t("عنوان گفت‌وگو و چند کلمه از پاسخ در اعلان دیده می‌شود.", "The chat title and the start of the reply appear in the notification.")}
            disabled={!current || !on}
          />
          <button type="button" className="button small ghost" onClick={test} disabled={testing}>
            {t("فرستادن اعلان آزمایشی", "Send a test notification")}
          </button>
          <p className="notify-live">{t("اعلان داخل برنامه همیشه، حتی وقتی پنجره در پیش‌زمینه است، گوشه‌ی صفحه نمایش داده می‌شود.", "An in-app notice also appears in the corner, even when this window is in front.")}</p>
          {result && <p className="notify-result">{result}</p>}
        </div>
      )}
    </div>
  );
}
