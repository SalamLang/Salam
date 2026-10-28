import { useEffect, useRef, useState } from "react";
import { agent, isDesktop, type AgentInfo, type NotifyKind, type NotifyPrefs } from "./desktop";
import { IconBell } from "./icons";
import { Checkbox } from "./Checkbox";
import { lang, t } from "./i18n";

export function announce(kind: NotifyKind, chat: string, text: string) {
  if (!isDesktop) return;
  agent.notify(kind, chat, text).catch(() => {});
}

export function syncNotifyLanguage() {
  if (!isDesktop) return;
  agent
    .notifyPrefs()
    .then((p) => (p.lang === lang ? p : agent.setNotifyPrefs({ ...p, lang })))
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
  const [prefs, setPrefs] = useState<NotifyPrefs | null>(null);
  const [result, setResult] = useState("");
  const [testing, setTesting] = useState(false);
  const box = useRef<HTMLDivElement>(null);

  useEffect(() => {
    if (!open) return;
    agent.notifyPrefs().then(setPrefs).catch(() => {});
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
    if (!prefs) return;
    const next = { ...prefs, ...patch, lang };
    setPrefs(next);
    agent.setNotifyPrefs(next).then(setPrefs).catch(() => {});
  };

  const test = async () => {
    setTesting(true);
    setResult("");
    try {
      await agent.notify("test");
      setResult(t("فرستاده شد. اگر چیزی ندیدید، حالت «مزاحم نشوید» یا تنظیمات اعلان سیستم را بررسی کنید.", "Sent. If you saw nothing, check Do Not Disturb or your system notification settings."));
    } catch (e) {
      setResult(failureHint(os, typeof e === "string" ? e : e instanceof Error ? e.message : ""));
    } finally {
      setTesting(false);
    }
  };

  const on = prefs?.enabled ?? true;
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
          <p>{t("وقتی پاسخ آماده شد، خطایی پیش آمد، ارتباط قطع شد یا دستیار منتظر اجازه‌ی شما بود، اگر پنجره جلوی چشم شما نباشد خبر می‌دهیم.", "When a reply is ready, something fails, the connection drops, or the assistant waits for your approval, we let you know if this window is not in front.")}</p>
          <Checkbox checked={on} onChange={(v) => change({ enabled: v })} label={t("نمایش اعلان", "Show notifications")} disabled={!prefs} />
          <Checkbox checked={prefs?.sound ?? true} onChange={(v) => change({ sound: v })} label={t("پخش صدا", "Play a sound")} disabled={!prefs || !on} />
          <Checkbox
            checked={prefs?.preview ?? true}
            onChange={(v) => change({ preview: v })}
            label={t("نمایش پیش‌نمایش پاسخ", "Show a reply preview")}
            hint={t("عنوان گفت‌وگو و چند کلمه از پاسخ در اعلان دیده می‌شود.", "The chat title and the start of the reply appear in the notification.")}
            disabled={!prefs || !on}
          />
          <button type="button" className="button small ghost" onClick={test} disabled={testing}>
            {t("فرستادن اعلان آزمایشی", "Send a test notification")}
          </button>
          {result && <p className="notify-result">{result}</p>}
        </div>
      )}
    </div>
  );
}
