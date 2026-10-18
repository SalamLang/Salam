import { useCallback, useEffect, useLayoutEffect, useRef, useState } from "react";
import { api, ApiError, getToken, handleUnauthorized, setToken, streamMessage, type Chat, type Message, type User } from "./api";
import { Admin, NoAccess } from "./Admin";
import { AgentBar, useAgent } from "./Agent";
import { Apps } from "./Apps";
import { Auth } from "./Auth";
import { Composer } from "./Composer";
import { Connect } from "./Connect";
import { agent, isDesktop, saveMode, savedMode, savedWorkspace, saveWorkspace, type AgentMode } from "./desktop";
import { Keys } from "./Keys";
import { IconGlobe, IconCode, IconMenu, IconMoon, IconPlus, IconSun, IconSwap, IconBook, IconBug, IconFolder, IconSearch, IconTerminal } from "./icons";
import { MessageView } from "./Messages";
import { Sidebar } from "./Sidebar";
import { useTheme } from "./theme";

const SUGGESTIONS = [
  { icon: IconCode, title: "اولین برنامه", text: "یک برنامه‌ی سلام بنویس که نام کاربر را بپرسد و به او خوشامد بگوید." },
  { icon: IconBook, title: "کتابخانه‌ی استاندارد", text: "چطور در سلام یک فایل متنی را خط به خط بخوانم؟ با مثال کامل." },
  { icon: IconSwap, title: "تبدیل کد", text: "این تابع پایتون را به سلام فارسی تبدیل کن:\n\ndef fib(n):\n    return n if n < 2 else fib(n-1) + fib(n-2)" },
  { icon: IconGlobe, title: "یک وب‌سرور", text: "با سلام یک وب‌سرور ساده بساز که در مسیر /hello متن «سلام دنیا» برگرداند." },
];

const AGENT_SUGGESTIONS = [
  { icon: IconSearch, title: "آشنایی با پروژه", text: "این پروژه را بررسی کن و کوتاه توضیح بده چه کار می‌کند و ساختارش چیست." },
  { icon: IconBug, title: "رفع خطا", text: "تست‌های این پروژه را اجرا کن و اگر خطایی بود علتش را پیدا و درست کن." },
  { icon: IconTerminal, title: "داکر", text: "کانتینرهای داکر در حال اجرا را نشان بده و بگو کدام‌ها مشکل دارند." },
  { icon: IconFolder, title: "مستندسازی", text: "برای این پروژه یک README کوتاه و کاربردی بنویس." },
];

function chatIdFromPath(): string | null {
  const m = /^\/c\/([a-f0-9]{24})\/?$/.exec(location.pathname);
  return m ? m[1] : null;
}

type Route = "chat" | "connect" | "apps" | "keys" | "admin";

function routeFromPath(): Route {
  const p = location.pathname.replace(/\/+$/, "");
  if (p === "/connect") return "connect";
  if (p === "/apps") return "apps";
  if (p === "/keys") return "keys";
  if (p === "/admin") return "admin";
  return "chat";
}

const PAGE_TITLES: Record<Route, string> = {
  chat: "دستیار سلام",
  connect: "اتصال برنامه",
  apps: "برنامه‌های متصل",
  keys: "کلیدهای API",
  admin: "مدیریت",
};

function go(id: string | null, replace = false) {
  const url = id ? `/c/${id}` : "/";
  if (location.pathname === url) return;
  if (replace) history.replaceState(null, "", url);
  else history.pushState(null, "", url);
}

export function App() {
  const [theme, toggleTheme] = useTheme();
  const [user, setUser] = useState<User | null>(null);
  const [route, setRoute] = useState<Route>(routeFromPath);
  const [booting, setBooting] = useState(true);
  const [chats, setChats] = useState<Chat[]>([]);
  const [activeId, setActiveId] = useState<string | null>(chatIdFromPath());
  const [messages, setMessages] = useState<Message[]>([]);
  const [loadingChat, setLoadingChat] = useState(false);
  const [sidebar, setSidebar] = useState(false);
  const [toast, setToast] = useState("");
  const [workspace, setWorkspace] = useState(isDesktop ? savedWorkspace() : "");
  const [mode, setMode] = useState<AgentMode>("ask");
  const scroller = useRef<HTMLDivElement>(null);
  const pageScroller = useRef<HTMLDivElement>(null);
  const stick = useRef(true);
  const pollTimer = useRef<number | null>(null);
  const streamCtrl = useRef<AbortController | null>(null);
  const activeRef = useRef<string | null>(activeId);
  activeRef.current = activeId;
  const messagesRef = useRef<Message[]>(messages);
  messagesRef.current = messages;
  const signedIn = user !== null;
  const onConnect = route === "connect";

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
    if (routeFromPath() === "chat") go(null, true);
  }, []);

  const onAuth = useCallback((u: User) => {
    setUser(u);
    api
      .me()
      .then((r) => setUser(r.user))
      .catch(() => {});
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
    if (!signedIn || onConnect) return;
    api.chats().then((r) => setChats(r.chats)).catch((e) => notify(e.message));
  }, [signedIn, onConnect, notify]);

  const midFor = useCallback((chat: string) => {
    if (chat !== activeRef.current) return undefined;
    return messagesRef.current.find((m) => m.role === "assistant" && m.status === "pending")?.id;
  }, []);
  const tools = useAgent(isDesktop && signedIn, midFor, notify);

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
          setMessages((ms) =>
            ms.map((m) =>
              m.id === mid
                ? { ...m, body: r.body, status: r.status, phase: r.phase, activity: r.activity && r.activity.length > 0 ? r.activity : m.activity }
                : m,
            ),
          );
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
          activity: (steps) => update((m) => ({ ...m, activity: steps })),
          done: (body, status, activity, ms) => {
            finished = true;
            update((m) => ({
              ...m,
              body,
              status,
              phase: "done",
              activity: activity && activity.length > 0 ? activity : m.activity,
              ms: ms ?? m.ms,
            }));
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
      setRoute("chat");
      setActiveId(id);
      go(id, replace);
      stick.current = true;
      if (isDesktop) {
        const m = id ? savedMode(id) : "ask";
        setMode(m);
        if (id) agent.setMode(id, m).catch(() => {});
      }
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

  const leaveChat = () => {
    stopPolling();
    setSidebar(false);
    setActiveId(null);
    setMessages([]);
  };

  const navigate = (path: string) => {
    leaveChat();
    if (location.pathname !== path) history.pushState(null, "", path);
    setRoute(routeFromPath());
  };

  useEffect(() => {
    if (!signedIn) return;
    if (routeFromPath() === "chat") openChat(chatIdFromPath(), true);
    const onPop = () => {
      if (routeFromPath() === "chat") openChat(chatIdFromPath(), true);
      else {
        stopPolling();
        setSidebar(false);
        setActiveId(null);
        setMessages([]);
        setRoute(routeFromPath());
      }
    };
    window.addEventListener("popstate", onPop);
    return () => window.removeEventListener("popstate", onPop);
  }, [signedIn, openChat]);

  useEffect(() => {
    if (signedIn) return;
    const onPop = () => setRoute(routeFromPath());
    window.addEventListener("popstate", onPop);
    return () => window.removeEventListener("popstate", onPop);
  }, [signedIn]);

  useEffect(() => {
    const el = scroller.current;
    if (el && stick.current) el.scrollTop = el.scrollHeight;
  }, [messages, tools.calls]);

  useLayoutEffect(() => {
    if (pageScroller.current) pageScroller.current.scrollTop = 0;
    window.scrollTo(0, 0);
  }, [route]);

  const onScroll = () => {
    const el = scroller.current;
    if (!el) return;
    stick.current = el.scrollHeight - el.scrollTop - el.clientHeight < 120;
  };

  const busy = messages.some((m) => m.status === "pending");
  const activeChat = chats.find((c) => c.id === activeId);
  const agentChat = isDesktop && (activeChat ? activeChat.agent === true : workspace !== "");
  const lastAssistant = [...messages].reverse().find((m) => m.role === "assistant")?.id;
  const callsFor = (mid: number) => tools.calls.filter((c) => c.chat === activeId && (c.mid === mid || (c.mid === undefined && mid === lastAssistant)));
  const waitingElsewhere = tools.calls.find((c) => c.state === "pending" && c.chat !== activeId);

  const pickWorkspace = async () => {
    try {
      const path = await agent.pickFolder();
      if (path) {
        setWorkspace(path);
        saveWorkspace(path);
      }
    } catch {
      notify("انتخاب پوشه ناموفق بود.");
    }
  };

  const changeMode = (m: AgentMode) => {
    setMode(m);
    if (activeId) {
      saveMode(activeId, m);
      agent.setMode(activeId, m).catch(() => {});
    }
  };

  const send = async (text: string) => {
    let id = activeId;
    try {
      if (!id) {
        const device = tools.info?.device;
        if (isDesktop && workspace && !device) {
          notify("این رایانه هنوز به دستیار وصل نشده است. چند لحظه بعد دوباره تلاش کنید.");
          return;
        }
        const r = await api.createChat(isDesktop && workspace && device ? { device_id: device.id, workspace } : undefined);
        id = r.chat.id;
        if (r.chat.agent) {
          saveMode(id, mode);
          await agent.setMode(id, mode).catch(() => {});
        }
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

  if (route === "connect") return <Connect user={user} onAuth={onAuth} onSwitchAccount={signOut} theme={theme} toggleTheme={toggleTheme} />;

  if (!user) return <Auth onAuth={onAuth} theme={theme} toggleTheme={toggleTheme} />;

  const title = route === "chat" ? chats.find((c) => c.id === activeId)?.title || "دستیار سلام" : PAGE_TITLES[route];
  const firstName = user.name.trim().split(/\s+/)[0];

  return (
    <div className="app">
      <Sidebar
        user={user}
        chats={chats}
        activeId={route === "chat" ? activeId : null}
        page={route}
        onNavigate={navigate}
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
        {route === "apps" || route === "keys" || route === "admin" ? (
          <div className="scroller page-scroller" ref={pageScroller}>
            {route === "apps" ? <Apps notify={notify} /> : route === "keys" ? <Keys notify={notify} /> : user.admin ? <Admin notify={notify} /> : <NoAccess />}
          </div>
        ) : (
          <>
            <div className="scroller" ref={scroller} onScroll={onScroll}>
              {messages.length === 0 && !loadingChat ? (
                <div className="welcome">
                  <img src="/logo.svg" alt="" width={64} height={64} className="welcome-logo" />
                  <h2>سلام {firstName}! چطور کمکتان کنم؟</h2>
                  <p className="muted">
                    {agentChat
                      ? "دستیار روی پوشه‌ی کاری شما کار می‌کند: فایل‌ها را می‌خواند، تغییر می‌دهد، فرمان اجرا می‌کند و تست‌ها را می‌گیرد. پیش از هر تغییر از شما اجازه می‌گیرد."
                      : "هر پرسشی درباره‌ی زبان برنامه‌نویسی سلام دارید بپرسید: نوشتن کد، کتابخانه‌ی استاندارد، رفع خطا یا تبدیل کد از زبان‌های دیگر."}
                  </p>
                  <div className="suggestions">
                    {(agentChat ? AGENT_SUGGESTIONS : SUGGESTIONS).map((s) => (
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
                    <MessageView key={m.id} m={m} name={user.name} calls={m.role === "assistant" ? callsFor(m.id) : undefined} onDecide={tools.decide} onStop={tools.stop} />
                  ))}
                </div>
              )}
            </div>
            {waitingElsewhere && (
              <button type="button" className="agent-waiting" onClick={() => openChat(waitingElsewhere.chat)}>
                یک کار در گفت‌وگوی دیگری منتظر اجازه‌ی شماست. نمایش
              </button>
            )}
            <Composer
              busy={busy}
              disabled={loadingChat}
              onSend={send}
              onStop={stop}
              autoFocus={!activeId}
              placeholder={agentChat ? "بگویید روی این پروژه چه کاری انجام شود…" : undefined}
              note={agentChat ? "دستیار پیش از تغییر فایل یا اجرای فرمان از شما اجازه می‌گیرد، مگر حالت خودکار را انتخاب کنید." : undefined}
              bar={
                isDesktop && (!activeChat || activeChat.agent) ? (
                  <AgentBar
                    info={tools.info}
                    workspace={activeChat ? activeChat.workspace || "" : workspace}
                    locked={activeChat !== undefined}
                    elsewhere={activeChat?.agent === true && tools.info?.device !== null && tools.info?.device !== undefined && String(activeChat.device_id) !== tools.info.device.id}
                    mode={mode}
                    onPick={pickWorkspace}
                    onClear={() => {
                      setWorkspace("");
                      saveWorkspace("");
                    }}
                    onMode={changeMode}
                  />
                ) : undefined
              }
            />
          </>
        )}
      </main>
      {toast && (
        <div className="toast" role="status">
          {toast}
        </div>
      )}
    </div>
  );
}
