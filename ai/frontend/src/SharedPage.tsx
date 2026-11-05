import { useCallback, useEffect, useLayoutEffect, useRef, useState } from "react";
import { api, ApiError, type Message, type PublicShare } from "./api";
import { isFa, setLang, t } from "./i18n";
import { IconGlobe, IconMoon, IconSend, IconSun } from "./icons";
import { MessageView } from "./Messages";
import { toPersianDigits, type Theme } from "./theme";

const NAME_KEY = "salam-ai-guest-name";
const MAX_CHARS = 2000;

function savedName(): string {
  try {
    const have = localStorage.getItem(NAME_KEY);
    if (have) return have;
    const made = isFa ? `مهمان ${toPersianDigits(100 + Math.floor(Math.random() * 900))}` : `Guest ${100 + Math.floor(Math.random() * 900)}`;
    localStorage.setItem(NAME_KEY, made);
    return made;
  } catch {
    return isFa ? "مهمان" : "Guest";
  }
}

function useNoindex() {
  useEffect(() => {
    const meta = document.createElement("meta");
    meta.name = "robots";
    meta.content = "noindex, nofollow, noarchive";
    document.head.appendChild(meta);
    const referrer = document.createElement("meta");
    referrer.name = "referrer";
    referrer.content = "no-referrer";
    document.head.appendChild(referrer);
    const previous = document.title;
    document.title = t("گفت‌وگوی اشتراکی | دستیار سلام", "Shared chat | Salam AI");
    return () => {
      meta.remove();
      referrer.remove();
      document.title = previous;
    };
  }, []);
}

const NOTICE: Record<string, { fa: string; en: string }> = {
  gone: { fa: "این پیوند وجود ندارد یا صاحب آن را حذف کرده است.", en: "This link does not exist or the owner removed it." },
  expired: { fa: "این پیوند منقضی شده است.", en: "This link has expired." },
  paused: { fa: "صاحب گفت‌وگو فعلا ارسال پیام را متوقف کرده است. هنوز می‌توانید گفت‌وگو را ببینید.", en: "The owner has paused sending for now. You can still read the chat." },
  limit: { fa: "فعلا امکان ارسال پیام از این پیوند نیست. بعدا دوباره سر بزنید.", en: "Sending from this link is not possible right now. Please come back later." },
  cap: { fa: "سقف پیام‌های این پیوند پر شده است.", en: "This link has used up its message limit." },
  daily: { fa: "سقف پیام‌های امروز این پیوند پر شده است. فردا دوباره امتحان کنید.", en: "This link reached today's message limit. Try again tomorrow." },
};

type Props = { token: string; theme: Theme; toggleTheme: () => void };

export function SharedPage({ token, theme, toggleTheme }: Props) {
  useNoindex();
  const [data, setData] = useState<PublicShare | null>(null);
  const [messages, setMessages] = useState<Message[]>([]);
  const [fatal, setFatal] = useState("");
  const [name, setName] = useState(savedName);
  const [text, setText] = useState("");
  const [sending, setSending] = useState(false);
  const [problem, setProblem] = useState("");
  const scroller = useRef<HTMLDivElement>(null);
  const stick = useRef(true);
  const timer = useRef<number | null>(null);
  const messagesRef = useRef<Message[]>([]);
  messagesRef.current = messages;
  const alive = useRef(true);

  const merge = useCallback((incoming: Message[]) => {
    if (incoming.length === 0) return;
    setMessages((cur) => {
      const byId = new Map(cur.map((m) => [m.id, m] as const));
      for (const m of incoming) byId.set(m.id, { ...byId.get(m.id), ...m });
      return [...byId.values()].sort((a, b) => a.id - b.id);
    });
  }, []);

  const pull = useCallback(async () => {
    const have = messagesRef.current;
    let after = 0;
    for (const m of have) {
      if (m.status === "pending") break;
      after = Math.max(after, m.id);
    }
    try {
      const r = await api.sharedChat(token, after);
      if (!alive.current) return;
      setData(r);
      setFatal("");
      if (!r.share.readable) setMessages([]);
      else merge(r.messages.map((m) => (m.status === "pending" ? { ...m, phase: r.phase } : m)));
    } catch (e) {
      if (!alive.current) return;
      if (e instanceof ApiError && (e.status === 404 || e.status === 410)) setFatal("gone");
    }
  }, [token, merge]);

  useEffect(() => {
    alive.current = true;
    const loop = async () => {
      await pull();
      if (!alive.current) return;
      const pending = messagesRef.current.some((m) => m.status === "pending");
      const wait = document.visibilityState !== "visible" ? 15000 : pending ? 1100 : 4000;
      timer.current = window.setTimeout(loop, wait);
    };
    loop();
    return () => {
      alive.current = false;
      if (timer.current) window.clearTimeout(timer.current);
    };
  }, [pull]);

  useLayoutEffect(() => {
    const el = scroller.current;
    if (el && stick.current) el.scrollTop = el.scrollHeight;
  }, [messages]);

  const onScroll = () => {
    const el = scroller.current;
    if (el) stick.current = el.scrollHeight - el.scrollTop - el.clientHeight < 120;
  };

  const share = data?.share;
  const busy = messages.some((m) => m.status === "pending");
  const canWrite = share?.collab === true;
  const state = fatal || share?.state || "ok";
  const notice = state !== "ok" ? NOTICE[state] : undefined;

  const send = async () => {
    const body = text.trim();
    if (!body || sending || busy || !canWrite) return;
    setSending(true);
    setProblem("");
    try {
      try {
        localStorage.setItem(NAME_KEY, name);
      } catch {}
      const r = await api.sharedSend(token, body, name);
      setText("");
      stick.current = true;
      merge([r.user, { ...r.assistant, phase: "thinking" }]);
    } catch (e) {
      if (e instanceof ApiError) {
        setProblem(e.message);
        if (e.status !== 409) pull();
      } else setProblem(t("فرستادن پیام ناموفق بود.", "Could not send the message."));
    } finally {
      setSending(false);
    }
  };

  return (
    <div className="shared-page">
      <header className="shared-top">
        <a className="brand shared-brand" href="/">
          <img src="/logo.svg" alt="" width={28} height={28} />
          <span>{t("دستیار سلام", "Salam AI")}</span>
        </a>
        <div className="shared-title">
          <h1 dir="auto">{data?.chat.title || t("گفت‌وگوی اشتراکی", "Shared chat")}</h1>
          {share && share.readable && (
            <span className={`share-badge ${share.kind === "collab" ? "ok" : "view"}`}>{share.kind === "collab" ? t("همکاری", "Collaborative") : t("فقط دیدن", "View only")}</span>
          )}
        </div>
        <button type="button" className="icon-button" onClick={toggleTheme} aria-label={t("تغییر پوسته", "Toggle theme")}>
          {theme === "dark" ? <IconSun /> : <IconMoon />}
        </button>
        <button type="button" className="icon-button" onClick={() => setLang(isFa ? "en" : "fa")} aria-label={t("زبان", "Language")} title={isFa ? "English" : "فارسی"}>
          <IconGlobe />
        </button>
        <a className="button primary shared-login" href="/">
          {t("گفت‌وگوی خودم", "Chat for myself")}
        </a>
      </header>

      <div className="scroller" ref={scroller} onScroll={onScroll}>
        {!data && !fatal && (
          <div className="loading-chat">
            <span className="spinner" />
          </div>
        )}
        {notice && (
          <p className={`shared-notice ${state}`} role="status">
            {t(notice.fa, notice.en)}
          </p>
        )}
        {share?.readable && (
          <div className="thread">
            {messages.length === 0 && <p className="muted shared-empty">{t("هنوز پیامی نیست.", "No messages yet.")}</p>}
            {messages.map((m) => (
              <div key={m.id} className="shared-msg">
                {m.guest && m.role === "user" && (
                  <span className="shared-guest">
                    <bdi dir="auto">{m.guest}</bdi>
                  </span>
                )}
                <MessageView m={m} name={m.guest ?? ""} />
              </div>
            ))}
          </div>
        )}
      </div>

      {canWrite ? (
        <div className="composer-wrap shared-composer">
          <label className="shared-name">
            <span>{t("نام شما", "Your name")}</span>
            <input value={name} maxLength={24} onChange={(e) => setName(e.target.value)} dir="auto" aria-label={t("نام شما", "Your name")} />
          </label>
          <form
            className="composer"
            onSubmit={(e) => {
              e.preventDefault();
              send();
            }}
          >
            <div className="composer-row">
              <textarea
                dir="auto"
                rows={1}
                value={text}
                maxLength={MAX_CHARS}
                placeholder={busy ? t("دستیار در حال پاسخ است…", "The assistant is answering…") : t("پیام خود را بنویسید…", "Write your message…")}
                aria-label={t("پیام", "Message")}
                onChange={(e) => setText(e.target.value)}
                onKeyDown={(e) => {
                  if (e.key === "Enter" && !e.shiftKey && !e.nativeEvent.isComposing) {
                    e.preventDefault();
                    send();
                  }
                }}
              />
              <button type="submit" className="send" disabled={!text.trim() || sending || busy} aria-label={t("فرستادن", "Send")} title={t("فرستادن", "Send")}>
                {sending ? <span className="spinner tiny" /> : <IconSend />}
              </button>
            </div>
            {problem && (
              <p className="attach-problem" role="alert">
                {problem}
              </p>
            )}
          </form>
          <p className="disclaimer">{t("پیام‌های شما را دستیار جواب می‌دهد و صاحب گفت‌وگو و مهمان‌های دیگر آن را می‌بینند.", "The assistant answers your messages, and the owner and other guests can see them.")}</p>
        </div>
      ) : (
        share?.readable && (
          <p className="shared-readonly">
            {share.kind === "view" ? t("این صفحه فقط برای دیدن است و به‌روز می‌شود.", "This page is read only and updates by itself.") : t("ارسال پیام الان ممکن نیست.", "Sending is not possible right now.")}
          </p>
        )
      )}

      <footer className="shared-foot">
        <a href="https://ai.salamlang.ir" target="_blank" rel="noopener noreferrer">
          {t("ساخته‌شده با دستیار سلام", "Powered by Salam AI")}
        </a>
      </footer>
    </div>
  );
}
