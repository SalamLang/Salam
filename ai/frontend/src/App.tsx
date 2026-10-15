import { useCallback, useEffect, useRef, useState } from "react";
import { api, ApiError, getToken, handleUnauthorized, setToken, streamMessage, type Chat, type Message, type User } from "./api";
import { Auth } from "./Auth";
import { Composer } from "./Composer";
import { IconGlobe, IconCode, IconMenu, IconMoon, IconPlus, IconSun, IconSwap, IconBook } from "./icons";
import { MessageView } from "./Messages";
import { Sidebar } from "./Sidebar";
import { useTheme } from "./theme";

const SUGGESTIONS = [
  { icon: IconCode, title: "اولین برنامه", text: "یک برنامه‌ی سلام بنویس که نام کاربر را بپرسد و به او خوشامد بگوید." },
  { icon: IconBook, title: "کتابخانه‌ی استاندارد", text: "چطور در سلام یک فایل متنی را خط به خط بخوانم؟ با مثال کامل." },
  { icon: IconSwap, title: "تبدیل کد", text: "این تابع پایتون را به سلام فارسی تبدیل کن:\n\ndef fib(n):\n    return n if n < 2 else fib(n-1) + fib(n-2)" },
  { icon: IconGlobe, title: "یک وب‌سرور", text: "با سلام یک وب‌سرور ساده بساز که در مسیر /hello متن «سلام دنیا» برگرداند." },
];

function chatIdFromPath(): string | null {
  const m = /^\/c\/([a-f0-9]{24})\/?$/.exec(location.pathname);
  return m ? m[1] : null;
}

function go(id: string | null, replace = false) {
  const url = id ? `/c/${id}` : "/";
  if (location.pathname === url) return;
  if (replace) history.replaceState(null, "", url);
  else history.pushState(null, "", url);
}

export function App() {
  const [theme, toggleTheme] = useTheme();
  const [user, setUser] = useState<User | null>(null);
  const [booting, setBooting] = useState(true);
  const [chats, setChats] = useState<Chat[]>([]);
  const [activeId, setActiveId] = useState<string | null>(chatIdFromPath());
  const [messages, setMessages] = useState<Message[]>([]);
  const [loadingChat, setLoadingChat] = useState(false);
  const [sidebar, setSidebar] = useState(false);
  const [toast, setToast] = useState("");
  const scroller = useRef<HTMLDivElement>(null);
  const stick = useRef(true);
  const pollTimer = useRef<number | null>(null);
  const streamCtrl = useRef<AbortController | null>(null);
  const activeRef = useRef<string | null>(activeId);
  activeRef.current = activeId;

  const notify = useCallback((msg: string) => {
    setToast(msg);
    window.setTimeout(() => setToast(""), 4000);
  }, []);

  const signOut = useCallback(() => {
    setToken("");
    setUser(null);
    setChats([]);
    setMessages([]);
    setActiveId(null);
    go(null, true);
  }, []);

  useEffect(() => {
    handleUnauthorized(signOut);
    if (!getToken()) {
      setBooting(false);
      return;
    }
    api
      .me()
      .then((r) => setUser(r.user))
      .catch(() => {})
      .finally(() => setBooting(false));
  }, [signOut]);

  useEffect(() => {
    if (!user) return;
    api.chats().then((r) => setChats(r.chats)).catch((e) => notify(e.message));
  }, [user, notify]);

  const stopPolling = () => {
    if (pollTimer.current) window.clearTimeout(pollTimer.current);
    pollTimer.current = null;
    streamCtrl.current?.abort();
    streamCtrl.current = null;
  };

  const poll = useCallback(
    (chatId: string, mid: number) => {
      stopPolling();
      const tick = async () => {
        if (activeRef.current !== chatId) return;
        try {
          const r = await api.poll(chatId, mid);
          if (activeRef.current !== chatId) return;
          setMessages((ms) => ms.map((m) => (m.id === mid ? { ...m, body: r.body, status: r.status, phase: r.phase } : m)));
          if (r.status === "pending") pollTimer.current = window.setTimeout(tick, 500);
        } catch (e) {
          if (e instanceof ApiError && e.status === 401) return;
          pollTimer.current = window.setTimeout(tick, 2000);
        }
      };
      pollTimer.current = window.setTimeout(tick, 300);
    },
    [],
  );

  const follow = useCallback(
    (chatId: string, mid: number) => {
      stopPolling();
      const ctrl = new AbortController();
      streamCtrl.current = ctrl;
      const update = (fn: (m: Message) => Message) => {
        if (ctrl.signal.aborted || activeRef.current !== chatId) return;
        setMessages((ms) => ms.map((m) => (m.id === mid ? fn(m) : m)));
      };
      let finished = false;
      streamMessage(
        chatId,
        mid,
        {
          delta: (t) => update((m) => ({ ...m, body: m.body + t, phase: "writing" })),
          reset: (t) => update((m) => ({ ...m, body: t, phase: "writing" })),
          phase: (p) => update((m) => ({ ...m, phase: p })),
          done: (body, status) => {
            finished = true;
            update((m) => ({ ...m, body, status, phase: "done" }));
          },
        },
        ctrl.signal,
      )
        .then((ok) => {
          if (!ok && !finished && !ctrl.signal.aborted) poll(chatId, mid);
        })
        .catch(() => {
          if (!finished && !ctrl.signal.aborted) poll(chatId, mid);
        });
    },
    [poll],
  );

  const openChat = useCallback(
    async (id: string | null, replace = false) => {
      stopPolling();
      setSidebar(false);
      setActiveId(id);
      go(id, replace);
      stick.current = true;
      if (!id) {
        setMessages([]);
        return;
      }
      setLoadingChat(true);
      try {
        const r = await api.chat(id);
        if (activeRef.current !== id) return;
        setMessages(r.messages);
        const pending = r.messages.find((m) => m.status === "pending");
        if (pending) follow(id, pending.id);
      } catch (e) {
        if (e instanceof ApiError && e.status === 404) {
          notify("این گفت‌وگو پیدا نشد.");
          setActiveId(null);
          go(null, true);
          setMessages([]);
        } else if (e instanceof ApiError) notify(e.message);
      } finally {
        setLoadingChat(false);
      }
    },
    [notify, follow],
  );

  useEffect(() => {
    if (!user) return;
    openChat(chatIdFromPath(), true);
    const onPop = () => openChat(chatIdFromPath(), true);
    window.addEventListener("popstate", onPop);
    return () => window.removeEventListener("popstate", onPop);
  }, [user, openChat]);

  useEffect(() => {
    const el = scroller.current;
    if (el && stick.current) el.scrollTop = el.scrollHeight;
  }, [messages]);

  const onScroll = () => {
    const el = scroller.current;
    if (!el) return;
    stick.current = el.scrollHeight - el.scrollTop - el.clientHeight < 120;
  };

  const busy = messages.some((m) => m.status === "pending");

  const send = async (text: string) => {
    let id = activeId;
    try {
      if (!id) {
        const r = await api.createChat();
        id = r.chat.id;
        setChats((cs) => [r.chat, ...cs]);
        setActiveId(id);
        activeRef.current = id;
        go(id);
      }
      const now = Math.floor(Date.now() / 1000);
      const tempUser: Message = { id: -now, role: "user", body: text, status: "done", created: now };
      setMessages((ms) => [...ms, tempUser]);
      stick.current = true;
      const r = await api.send(id, text);
      setMessages((ms) => [...ms.filter((m) => m.id !== tempUser.id), r.user, { ...r.assistant, phase: "thinking" }]);
      setChats((cs) => {
        const cur = cs.find((c) => c.id === id);
        const rest = cs.filter((c) => c.id !== id);
        return cur ? [{ ...cur, title: r.title, updated: now }, ...rest] : cs;
      });
      follow(id, r.assistant.id);
    } catch (e) {
      setMessages((ms) => ms.filter((m) => m.id > 0));
      notify(e instanceof ApiError ? e.message : "فرستادن پیام ناموفق بود.");
    }
  };

  const stop = async () => {
    if (!activeId) return;
    stopPolling();
    try {
      const r = await api.stop(activeId);
      setMessages((ms) => ms.map((m) => (m.status === "pending" ? { ...m, status: "done", body: r.body } : m)));
    } catch (e) {
      if (e instanceof ApiError) notify(e.message);
    }
  };

  const rename = async (id: string, title: string) => {
    try {
      const r = await api.renameChat(id, title);
      setChats((cs) => cs.map((c) => (c.id === id ? { ...c, title: r.title } : c)));
    } catch (e) {
      if (e instanceof ApiError) notify(e.message);
    }
  };

  const remove = async (id: string) => {
    try {
      await api.deleteChat(id);
      setChats((cs) => cs.filter((c) => c.id !== id));
      if (id === activeId) openChat(null);
    } catch (e) {
      if (e instanceof ApiError) notify(e.message);
    }
  };

  const logout = async () => {
    try {
      await api.logout();
    } catch {}
    signOut();
  };

  if (booting) {
    return (
      <div className="boot">
        <img src="/logo.svg" alt="" width={64} height={64} />
      </div>
    );
  }

  if (!user) return <Auth onAuth={setUser} theme={theme} toggleTheme={toggleTheme} />;

  const title = chats.find((c) => c.id === activeId)?.title || "دستیار سلام";
  const firstName = user.name.trim().split(/\s+/)[0];

  return (
    <div className="app">
      <Sidebar
        user={user}
        chats={chats}
        activeId={activeId}
        open={sidebar}
        theme={theme}
        onClose={() => setSidebar(false)}
        onNew={() => openChat(null)}
        onSelect={(id) => openChat(id)}
        onRename={rename}
        onDelete={remove}
        onLogout={logout}
        onToggleTheme={toggleTheme}
      />
      <main className="main">
        <header className="topbar">
          <button type="button" className="icon-button only-mobile" onClick={() => setSidebar(true)} aria-label="باز کردن فهرست">
            <IconMenu />
          </button>
          <h1 dir="auto">{title}</h1>
          <button type="button" className="icon-button only-mobile" onClick={() => openChat(null)} aria-label="گفت‌وگوی تازه">
            <IconPlus />
          </button>
          <button type="button" className="icon-button only-desktop" onClick={toggleTheme} aria-label="تغییر پوسته">
            {theme === "dark" ? <IconSun /> : <IconMoon />}
          </button>
        </header>
        <div className="scroller" ref={scroller} onScroll={onScroll}>
          {messages.length === 0 && !loadingChat ? (
            <div className="welcome">
              <img src="/logo.svg" alt="" width={64} height={64} className="welcome-logo" />
              <h2>سلام {firstName}! چطور کمکتان کنم؟</h2>
              <p className="muted">هر پرسشی درباره‌ی زبان برنامه‌نویسی سلام دارید بپرسید: نوشتن کد، کتابخانه‌ی استاندارد، رفع خطا یا تبدیل کد از زبان‌های دیگر.</p>
              <div className="suggestions">
                {SUGGESTIONS.map((s) => (
                  <button key={s.title} type="button" className="suggestion" onClick={() => send(s.text)}>
                    <span className="suggestion-icon">
                      <s.icon width={18} height={18} />
                    </span>
                    <strong>{s.title}</strong>
                    <span>{s.text.split("\n")[0]}</span>
                  </button>
                ))}
              </div>
            </div>
          ) : (
            <div className="thread">
              {loadingChat && messages.length === 0 && <div className="loading-chat"><span className="spinner" /></div>}
              {messages.map((m) => (
                <MessageView key={m.id} m={m} name={user.name} />
              ))}
            </div>
          )}
        </div>
        <Composer busy={busy} disabled={loadingChat} onSend={send} onStop={stop} autoFocus={!activeId} />
      </main>
      {toast && (
        <div className="toast" role="status">
          {toast}
        </div>
      )}
    </div>
  );
}
