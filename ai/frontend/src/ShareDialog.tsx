import { useCallback, useEffect, useId, useRef, useState } from "react";
import { createPortal } from "react-dom";
import { api, ApiError, type Share, type ShareKind, type ShareList, type ShareState } from "./api";
import { confirmDialog } from "./Confirm";
import { isDesktop } from "./desktop";
import { t } from "./i18n";
import { IconAlert, IconCheck, IconClose, IconCopy, IconEye, IconLink, IconPause, IconPlay, IconRefresh, IconTrash, IconUsers } from "./icons";
import { Select } from "./Select";
import { toPersianDigits } from "./theme";
import { pushToast } from "./Toasts";

type Props = {
  chatId: string;
  title: string;
  onClose: () => void;
  onChange: (kinds: ShareKind[]) => void;
};

type Hours = "1" | "24" | "168" | "0";

const REFRESH_MS = 4000;

const HOURS: { value: Hours; label: string }[] = [
  { value: "1", label: t("یک ساعت", "1 hour") },
  { value: "24", label: t("۲۴ ساعت", "24 hours") },
  { value: "168", label: t("۷ روز", "7 days") },
  { value: "0", label: t("بدون انقضا", "Never") },
];

const STATE_TEXT: Record<ShareState, string> = {
  ok: t("فعال", "Active"),
  paused: t("متوقف‌شده", "Paused"),
  limit: t("خودکار متوقف شد: سقف پیام شما", "Paused automatically: your message limit"),
  expired: t("منقضی‌شده", "Expired"),
  cap: t("سقف پیام این پیوند پر شد", "Message limit reached"),
  daily: t("سقف امروز پر شد", "Today's limit reached"),
};

function num(n: number): string {
  return toPersianDigits(n.toLocaleString());
}

function expiryText(expires: number): string {
  if (expires === 0) return t("بدون انقضا", "Never expires");
  const left = expires - Math.floor(Date.now() / 1000);
  if (left <= 0) return t("منقضی شده است", "Expired");
  if (left < 3600) return t(`${num(Math.max(1, Math.round(left / 60)))} دقیقه دیگر منقضی می‌شود`, `Expires in ${Math.max(1, Math.round(left / 60))} min`);
  if (left < 172800) return t(`${num(Math.round(left / 3600))} ساعت دیگر منقضی می‌شود`, `Expires in ${Math.round(left / 3600)} h`);
  return t(`${num(Math.round(left / 86400))} روز دیگر منقضی می‌شود`, `Expires in ${Math.round(left / 86400)} days`);
}

function nearestHours(expires: number): Hours {
  if (expires === 0) return "0";
  const left = (expires - Math.floor(Date.now() / 1000)) / 3600;
  if (left <= 2) return "1";
  if (left <= 36) return "24";
  return "168";
}

function openLink(url: string) {
  window.open(url, "_blank", "noopener,noreferrer");
}

async function copyText(text: string): Promise<boolean> {
  try {
    await navigator.clipboard.writeText(text);
    return true;
  } catch {
    const el = document.createElement("textarea");
    el.value = text;
    el.setAttribute("readonly", "");
    el.style.position = "fixed";
    el.style.opacity = "0";
    document.body.appendChild(el);
    el.select();
    let ok = false;
    try {
      ok = document.execCommand("copy");
    } catch {}
    el.remove();
    return ok;
  }
}

function CapField({ label, value, ceiling, disabled, onCommit }: { label: string; value: number; ceiling: number; disabled: boolean; onCommit: (n: number) => void }) {
  const [draft, setDraft] = useState(String(value));
  const focused = useRef(false);
  useEffect(() => {
    if (!focused.current) setDraft(String(value));
  }, [value]);
  const commit = () => {
    focused.current = false;
    const n = Math.max(0, Math.min(ceiling, Math.floor(Number(draft) || 0)));
    setDraft(String(n));
    if (n !== value) onCommit(n);
  };
  return (
    <label className="share-field">
      <span>{label}</span>
      <input
        type="number"
        inputMode="numeric"
        min={0}
        max={ceiling}
        value={draft}
        disabled={disabled}
        dir="ltr"
        onFocus={() => (focused.current = true)}
        onChange={(e) => setDraft(e.target.value)}
        onBlur={commit}
        onKeyDown={(e) => {
          if (e.key === "Enter") (e.target as HTMLInputElement).blur();
        }}
      />
    </label>
  );
}

type CardProps = {
  kind: ShareKind;
  share: Share | undefined;
  list: ShareList;
  busy: boolean;
  run: (action: () => Promise<ShareList>) => void;
  chatId: string;
};

function Card({ kind, share, list, busy, run, chatId }: CardProps) {
  const collab = kind === "collab";
  const blocked = collab && !list.collab_allowed;
  const [copied, setCopied] = useState(false);
  const heading = collab ? t("می‌تواند پیام بفرستد", "Can send messages") : t("فقط دیدن", "View only");
  const blurb = collab
    ? t("هر کس این پیوند را داشته باشد می‌تواند گفت‌وگو را ادامه بدهد و پیام بفرستد.", "Anyone with this link can continue the conversation and send messages.")
    : t("هر کس این پیوند را داشته باشد می‌تواند گفت‌وگو را زنده ببیند، اما نمی‌تواند چیزی بنویسد.", "Anyone with this link can follow the chat live but cannot write anything.");

  const copy = async () => {
    if (!share?.url) return;
    const ok = await copyText(share.url);
    if (ok) {
      setCopied(true);
      window.setTimeout(() => setCopied(false), 1800);
      pushToast({ tone: "done", title: t("پیوند کپی شد", "Link copied"), seconds: 2 });
    } else pushToast({ tone: "failed", title: t("کپی نشد. پیوند را دستی کپی کنید.", "Could not copy. Copy the link by hand."), seconds: 4 });
  };

  const regenerate = async () => {
    const ok = await confirmDialog({
      title: t("پیوند تازه بسازم؟", "Make a new link?"),
      message: t("پیوند فعلی بی‌درنگ از کار می‌افتد و هر کس آن را دارد دیگر دسترسی ندارد. باید پیوند تازه را دوباره بفرستید.", "The current link stops working at once, so everyone who has it loses access. You will need to share the new link."),
      confirmLabel: t("بله، پیوند تازه", "Yes, make a new link"),
      tone: "default",
      icon: "alert",
    });
    if (ok) run(() => api.regenerateShare(chatId, kind));
  };

  const remove = async () => {
    const ok = await confirmDialog({
      title: t("پیوند حذف شود؟", "Delete this link?"),
      message: t("پیوند برای همیشه از کار می‌افتد. شمارنده‌ی پیام‌های مهمان‌ها هم از این پنجره پاک می‌شود.", "The link stops working for good. Its guest counters are removed from this window too."),
      confirmLabel: t("حذف پیوند", "Delete link"),
    });
    if (ok) run(() => api.deleteShare(chatId, kind));
  };

  return (
    <section className={`share-card ${collab ? "collab" : "view"}`} aria-label={heading}>
      <header className="share-card-head">
        <span className="share-card-icon">{collab ? <IconUsers width={18} height={18} /> : <IconEye width={18} height={18} />}</span>
        <h3>{heading}</h3>
        {share && <span className={`share-badge ${share.state}`}>{STATE_TEXT[share.state]}</span>}
      </header>
      <p className="share-blurb">{blurb}</p>

      {collab && !blocked && (
        <p className="share-warning" role="note">
          <IconAlert width={16} height={16} />
          <span>{t("پیام‌هایی که مهمان‌ها می‌فرستند را دستیار جواب می‌دهد و از پیام‌ها و سهمیه‌ی پلن شما کم می‌شود. هر لحظه می‌توانید پیوند را متوقف یا حذف کنید.", "Messages guests send are answered by the assistant and count against your plan and limits. You can pause or delete the link at any moment.")}</span>
        </p>
      )}

      {blocked && (
        <p className="share-refusal" role="note">
          {list.collab_refusal === "device"
            ? t("برای گفت‌وگوهای متصل به رایانه‌ی شما پیوند همکاری ساخته نمی‌شود، چون مهمان می‌توانست روی رایانه‌ی شما فایل بخواند یا فرمان اجرا کند. پیوند فقط‌دیدنی ممکن است و جزئیات ابزارها در آن نمایش داده نمی‌شود.", "A collaborate link is not available for chats connected to your computer, because a guest could read files or run commands there. A view only link is possible and hides tool details.")
            : t("برای گفت‌وگوهای محیط ابری پیوند همکاری ساخته نمی‌شود، چون مهمان به حساب‌های متصل شما (مثل GitHub) دسترسی پیدا می‌کرد. پیوند فقط‌دیدنی ممکن است.", "A collaborate link is not available for cloud environment chats, because a guest would reach your connected accounts (like GitHub). A view only link is possible.")}
        </p>
      )}

      {!share && !blocked && (
        <button type="button" className="button primary" disabled={busy} onClick={() => run(() => api.createShare(chatId, kind))}>
          <IconLink width={16} height={16} />
          {t("ساخت پیوند", "Create link")}
        </button>
      )}

      {share && (
        <>
          {share.url ? (
            <div className="share-url-row">
              <input className="share-url" readOnly dir="ltr" value={share.url} onFocus={(e) => e.currentTarget.select()} aria-label={t("پیوند اشتراک", "Share link")} />
              <button type="button" className="button primary share-copy" onClick={copy}>
                {copied ? <IconCheck width={16} height={16} /> : <IconCopy width={16} height={16} />}
                {copied ? t("کپی شد", "Copied") : t("کپی پیوند", "Copy link")}
              </button>
              {isDesktop && (
                <button type="button" className="icon-button" onClick={() => openLink(share.url ?? "")} aria-label={t("باز کردن در مرورگر", "Open in browser")} title={t("باز کردن در مرورگر", "Open in browser")}>
                  <IconLink width={16} height={16} />
                </button>
              )}
            </div>
          ) : (
            <p className="share-refusal" role="note">
              {t("این سرور نمی‌تواند پیوند را دوباره نشان بدهد. برای دیدن پیوند تازه روی «پیوند تازه» بزنید.", "This server cannot show the link again. Use Regenerate to get a new link.")}
            </p>
          )}

          <div className="share-grid">
            <label className="share-field">
              <span>{t("اعتبار", "Lifetime")}</span>
              <Select<Hours> value={nearestHours(share.expires)} options={HOURS} ariaLabel={t("مدت اعتبار پیوند", "Link lifetime")} disabled={busy} onChange={(v) => run(() => api.patchShare(chatId, kind, { hours: Number(v) }))} />
              <small>{expiryText(share.expires)}</small>
            </label>
            {collab && (
              <>
                <CapField label={t("سقف کل پیام‌ها (۰ یعنی بی‌سقف)", "Total message cap (0 means none)")} value={share.max_messages} ceiling={list.ceiling} disabled={busy} onCommit={(n) => run(() => api.patchShare(chatId, kind, { max_messages: n }))} />
                <CapField label={t("سقف پیام در ۲۴ ساعت (۰ یعنی بی‌سقف)", "Daily cap (0 means none)")} value={share.max_daily} ceiling={list.ceiling} disabled={busy} onCommit={(n) => run(() => api.patchShare(chatId, kind, { max_daily: n }))} />
              </>
            )}
          </div>

          <dl className="share-stats">
            {collab ? (
              <>
                <div>
                  <dt>{t("پیام مهمان‌ها", "Guest messages")}</dt>
                  <dd>{num(share.messages)}</dd>
                </div>
                <div>
                  <dt>{t("امروز", "Today")}</dt>
                  <dd>{num(share.today)}</dd>
                </div>
                <div>
                  <dt>{t("توکن تقریبی", "Approx. tokens")}</dt>
                  <dd>{num(share.tokens)}</dd>
                </div>
              </>
            ) : (
              <div>
                <dt>{t("بازدید", "Visits")}</dt>
                <dd>{num(share.views)}</dd>
              </div>
            )}
          </dl>

          {collab && share.guests.length > 0 && (
            <ul className="share-guests" aria-label={t("مهمان‌ها", "Guests")}>
              {share.guests.map((g) => (
                <li key={g.name}>
                  <bdi dir="auto">{g.name}</bdi>
                  <span>{num(g.count)}</span>
                </li>
              ))}
            </ul>
          )}

          <div className="share-actions">
            {share.paused ? (
              <button type="button" className="button primary" disabled={busy} onClick={() => run(() => api.patchShare(chatId, kind, { paused: false }))}>
                <IconPlay width={16} height={16} />
                {t("ادامه", "Resume")}
              </button>
            ) : (
              <button type="button" className={`button ${collab ? "stop-solid" : "ghost"}`} disabled={busy || share.state === "expired"} onClick={() => run(() => api.patchShare(chatId, kind, { paused: true }))}>
                <IconPause width={16} height={16} />
                {collab ? t("توقف فوری", "Stop now") : t("توقف", "Pause")}
              </button>
            )}
            <button type="button" className="button ghost" disabled={busy} onClick={regenerate}>
              <IconRefresh width={16} height={16} />
              {t("پیوند تازه", "Regenerate")}
            </button>
            <button type="button" className="button danger" disabled={busy} onClick={remove}>
              <IconTrash width={16} height={16} />
              {t("حذف", "Delete")}
            </button>
          </div>
        </>
      )}
    </section>
  );
}

export function ShareDialog({ chatId, title, onClose, onChange }: Props) {
  const titleId = useId();
  const panel = useRef<HTMLDivElement>(null);
  const [list, setList] = useState<ShareList | null>(null);
  const [problem, setProblem] = useState("");
  const [busy, setBusy] = useState(false);
  const alive = useRef(true);
  const acting = useRef(false);

  const apply = useCallback(
    (next: ShareList) => {
      setList(next);
      onChange(next.shares.map((s) => s.kind));
    },
    [onChange],
  );

  const refresh = useCallback(async () => {
    try {
      const next = await api.shares(chatId);
      if (alive.current && !acting.current) apply(next);
    } catch (e) {
      if (alive.current && !acting.current && e instanceof ApiError && e.status === 404) setProblem(e.message);
    }
  }, [chatId, apply]);

  useEffect(() => {
    alive.current = true;
    refresh();
    const timer = window.setInterval(() => {
      if (document.visibilityState === "visible") refresh();
    }, REFRESH_MS);
    return () => {
      alive.current = false;
      window.clearInterval(timer);
    };
  }, [refresh]);

  useEffect(() => {
    const previous = document.activeElement as HTMLElement | null;
    const root = document.getElementById("root");
    const overflow = document.body.style.overflow;
    document.body.style.overflow = "hidden";
    root?.setAttribute("inert", "");
    root?.setAttribute("aria-hidden", "true");
    panel.current?.focus();
    return () => {
      document.body.style.overflow = overflow;
      root?.removeAttribute("inert");
      root?.removeAttribute("aria-hidden");
      if (previous && document.contains(previous)) previous.focus();
    };
  }, []);

  useEffect(() => {
    const onKey = (e: KeyboardEvent) => {
      if (e.key === "Escape" && !document.querySelector(".confirm-backdrop")) {
        e.preventDefault();
        onClose();
      }
    };
    document.addEventListener("keydown", onKey);
    return () => document.removeEventListener("keydown", onKey);
  }, [onClose]);

  const run = async (action: () => Promise<ShareList>) => {
    acting.current = true;
    setBusy(true);
    setProblem("");
    try {
      const next = await action();
      if (alive.current) apply(next);
    } catch (e) {
      if (alive.current) setProblem(e instanceof ApiError ? e.message : t("انجام نشد. دوباره تلاش کنید.", "That did not work. Please try again."));
    } finally {
      acting.current = false;
      if (alive.current) setBusy(false);
    }
  };

  const pressedOnBackdrop = useRef(false);
  const find = (kind: ShareKind) => list?.shares.find((s) => s.kind === kind);

  return createPortal(
    <div
      className="confirm-backdrop share-backdrop"
      onMouseDown={(e) => {
        pressedOnBackdrop.current = e.target === e.currentTarget;
      }}
      onMouseUp={(e) => {
        if (pressedOnBackdrop.current && e.target === e.currentTarget) onClose();
        pressedOnBackdrop.current = false;
      }}
    >
      <div ref={panel} tabIndex={-1} className="share-panel" role="dialog" aria-modal="true" aria-labelledby={titleId}>
        <header className="share-head">
          <div>
            <h2 id={titleId}>{t("اشتراک‌گذاری گفت‌وگو", "Share this chat")}</h2>
            <p className="share-subject" dir="auto" title={title}>
              {title}
            </p>
          </div>
          <button type="button" className="icon-button" onClick={onClose} aria-label={t("بستن", "Close")}>
            <IconClose />
          </button>
        </header>
        {problem && (
          <p className="share-problem" role="alert">
            {problem}
          </p>
        )}
        {!list ? (
          <div className="share-loading">
            <span className="spinner" />
          </div>
        ) : (
          <div className="share-cards">
            <Card kind="view" share={find("view")} list={list} busy={busy} run={run} chatId={chatId} />
            <Card kind="collab" share={find("collab")} list={list} busy={busy} run={run} chatId={chatId} />
          </div>
        )}
        <p className="share-footnote">{t("پیوندها در موتورهای جست‌وجو نمی‌آیند. فایل‌های پیوست‌شده، حساب شما و گفت‌وگوهای دیگرتان از راه پیوند دیده نمی‌شود.", "Links are kept out of search engines. Attached files, your account and your other chats are never visible through a link.")}</p>
      </div>
    </div>,
    document.body,
  );
}
