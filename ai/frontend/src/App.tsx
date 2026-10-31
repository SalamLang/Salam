import { useCallback, useEffect, useLayoutEffect, useMemo, useRef, useState } from "react";
import { api, ApiError, getToken, handleUnauthorized, setClientPlatform, setClientVersion, setToken, streamMessage, uiText, type Chat, type FileMeta, type Message, type OutgoingFile, type QueueItem, type User } from "./api";
import { Admin, NoAccess } from "./Admin";
import { AdminChat } from "./AdminChat";
import { confirmDialog } from "./Confirm";
import { AgentBar, useAgent } from "./Agent";
import { Memory } from "./Memory";
import { fileLoader, type Pending } from "./Attach";
import { Auth } from "./Auth";
import { Composer } from "./Composer";
import { QueuePanel } from "./Queue";
import { Connect } from "./Connect";
import { Connections, TelegramLink } from "./Telegram";
import { OAuthCallback, providerFromPath } from "./OAuthCallback";
import { agent, isDesktop, saveMode, savedMode, savedWorkspace, saveWorkspace, type AgentMode } from "./desktop";
import { Keys } from "./Keys";
import { announce } from "./Notify";
import { Plans } from "./Plans";
import { IconGlobe, IconCode, IconMenu, IconMoon, IconPlus, IconSun, IconSwap, IconBook, IconBug, IconFolder, IconSearch, IconTerminal } from "./icons";
import { MessageView } from "./Messages";
import { Sidebar } from "./Sidebar";
import { useTheme } from "./theme";
import { t } from "./i18n";
import { Checkbox } from "./Checkbox";

const SUGGESTIONS = [
  { icon: IconCode, title: t("اولین برنامه", "First program"), text: t("یک برنامه‌ی سلام بنویس که نام کاربر را بپرسد و به او خوشامد بگوید.", "Write a Salam program that asks for the user's name and greets them.") },
  { icon: IconBook, title: t("کتابخانه‌ی استاندارد", "Standard library"), text: t("چطور در سلام یک فایل متنی را خط به خط بخوانم؟ با مثال کامل.", "How do I read a text file line by line in Salam? Show a complete example.") },
  { icon: IconSwap, title: t("تبدیل کد", "Convert code"), text: t("این تابع پایتون را به سلام فارسی تبدیل کن:\n\ndef fib(n):\n    return n if n < 2 else fib(n-1) + fib(n-2)", "Convert this Python function to Salam:\n\ndef fib(n):\n    return n if n < 2 else fib(n-1) + fib(n-2)") },
  { icon: IconGlobe, title: t("یک وب‌سرور", "A web server"), text: t("با سلام یک وب‌سرور ساده بساز که در مسیر /hello متن «سلام دنیا» برگرداند.", "Build a simple web server in Salam that returns \"Hello, world\" at /hello.") },
];

const AGENT_SUGGESTIONS = [
  { icon: IconSearch, title: t("آشنایی با پروژه", "Explore the project"), text: t("این پروژه را بررسی کن و کوتاه توضیح بده چه کار می‌کند و ساختارش چیست.", "Look through this project and briefly explain what it does and how it is structured.") },
  { icon: IconBug, title: t("رفع خطا", "Fix errors"), text: t("تست‌های این پروژه را اجرا کن و اگر خطایی بود علتش را پیدا و درست کن.", "Run this project's tests, and if anything fails, find the cause and fix it.") },
  { icon: IconTerminal, title: t("داکر", "Docker"), text: t("کانتینرهای داکر در حال اجرا را نشان بده و بگو کدام‌ها مشکل دارند.", "Show the running Docker containers and tell me which ones have problems.") },
  { icon: IconFolder, title: t("مستندسازی", "Documentation"), text: t("برای این پروژه یک README کوتاه و کاربردی بنویس.", "Write a short, practical README for this project.") },
];

function chatIdFromPath(): string | null {
  const m = /^\/c\/([a-f0-9]{24})\/?$/.exec(location.pathname);
  return m ? m[1] : null;
}

type Route = "chat" | "connect" | "telegram" | "connections" | "keys" | "plans" | "memory" | "admin" | "admin-chat";

function adminChatIdFromPath(): string {
  return /^\/admin\/chats\/([a-f0-9]{24})\/?$/.exec(location.pathname)?.[1] ?? "";
}

function routeFromPath(): Route {
  const p = location.pathname.replace(/\/+$/, "");
  if (p === "/connect") return "connect";
  if (p === "/telegram") return "telegram";
  if (p === "/connections") return "connections";
  if (p === "/apps") return "connections";
  if (p === "/keys") return "keys";
  if (p === "/plans") return "plans";
  if (p === "/memory") return "memory";
  if (p === "/admin") return "admin";
  if (adminChatIdFromPath()) return "admin-chat";
  return "chat";
}

const PAGE_TITLES: Record<Route, string> = {
  chat: t("دستیار سلام", "Salam AI"),
  connect: t("اتصال برنامه", "Connect an app"),
  telegram: t("اتصال تلگرام", "Connect Telegram"),
  connections: t("اتصال‌ها و برنامه‌ها", "Connections and apps"),
  keys: t("کلیدهای API", "API keys"),
  plans: t("اشتراک‌ها", "Plans"),
  memory: t("حافظه", "Memory"),
  admin: t("مدیریت", "Admin"),
  "admin-chat": t("گفت‌وگو در مدیریت", "Admin chat view"),
};

const DRAFT_KEY = "salam-ai-draft";

function takeDraft(): string {
  const params = new URLSearchParams(location.search);
  const q = (params.get("q") ?? "").slice(0, 2000);
  if (q) {
    params.delete("q");
    const rest = params.toString();
    history.replaceState(null, "", location.pathname + (rest ? `?${rest}` : "") + location.hash);
    try {
      sessionStorage.setItem(DRAFT_KEY, q);
    } catch {}
    return q;
  }
  try {
    return sessionStorage.getItem(DRAFT_KEY) ?? "";
  } catch {
    return "";
  }
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
  const [draft, setDraft] = useState(takeDraft);
  const dropDraft = useCallback(() => {
    try {
      sessionStorage.removeItem(DRAFT_KEY);
    } catch {}
    setDraft("");
  }, []);
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
  const [queue, setQueue] = useState<QueueItem[]>([]);
  const [cloudOk, setCloudOk] = useState(false);
  const [cloud, setCloud] = useState(() => {
    try {
      return localStorage.getItem("salam-ai-cloud") === "1";
    } catch {
      return false;
    }
  });
  const scroller = useRef<HTMLDivElement>(null);
  const pageScroller = useRef<HTMLDivElement>(null);
  const stick = useRef(true);
  const pollTimer = useRef<number | null>(null);
  const streamCtrl = useRef<AbortController | null>(null);
  const followed = useRef(0);
  const queueRef = useRef<QueueItem[]>([]);
  const activeRef = useRef<string | null>(activeId);
  activeRef.current = activeId;
  queueRef.current = queue;
  const messagesRef = useRef<Message[]>(messages);
  messagesRef.current = messages;
  const chatsRef = useRef<Chat[]>(chats);
  chatsRef.current = chats;
  const announced = useRef(new Set<number>());
  const watching = useRef(new Set<number>());
  const netFails = useRef(0);
  const signedIn = user !== null;
  useEffect(() => {
    if (!signedIn || isDesktop) return;
    api
      .integrations()
      .then((r) => setCloudOk(r.sandbox))
      .catch(() => {});
  }, [signedIn]);
  const toggleCloud = (on: boolean) => {
    setCloud(on);
    try {
      localStorage.setItem("salam-ai-cloud", on ? "1" : "0");
    } catch {}
  };
  const onConnect = route === "connect" || route === "telegram";

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

  const finish = useCallback((chatId: string, mid: number, status: Message["status"], body: string) => {
    if (!isDesktop || status === "pending" || announced.current.has(mid)) return;
    announced.current.add(mid);
    const title = chatsRef.current.find((c) => c.id === chatId)?.title ?? "";
    if (status === "error") announce("failed", title, uiText(body));
    else announce("done", title, body);
  }, []);

  const watchAway = useCallback(() => {
    const chatId = activeRef.current;
    const pending = messagesRef.current.find((m) => m.role === "assistant" && m.status === "pending");
    if (!isDesktop || !chatId || !pending || pending.id <= 0 || watching.current.has(pending.id)) return;
    const mid = pending.id;
    const until = Date.now() + 30 * 60 * 1000;
    watching.current.add(mid);
    const tick = async () => {
      if (announced.current.has(mid) || activeRef.current === chatId || Date.now() > until) {
        watching.current.delete(mid);
        return;
      }
      try {
        const r = await api.poll(chatId, mid);
        if (r.status !== "pending") {
          watching.current.delete(mid);
          finish(chatId, mid, r.status, r.body);
          return;
        }
      } catch (e) {
        if (e instanceof ApiError && e.status === 401) {
          watching.current.delete(mid);
          return;
        }
      }
      window.setTimeout(tick, 3000);
    };
    window.setTimeout(tick, 3000);
  }, [finish]);

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
          netFails.current = 0;
          setMessages((ms) =>
            ms.map((m) =>
              m.id === mid
                ? { ...m, body: r.body, status: r.status, phase: r.phase, activity: r.activity && r.activity.length > 0 ? r.activity : m.activity }
                : m,
            ),
          );
          if (r.status === "pending") pollTimer.current = window.setTimeout(tick, 500);
          else finish(chatId, mid, r.status, r.body);
        } catch (e) {
          if (e instanceof ApiError && e.status === 401) return;
          netFails.current += 1;
          if (netFails.current === 3) announce("network", chatsRef.current.find((c) => c.id === chatId)?.title ?? "", "");
          pollTimer.current = window.setTimeout(tick, 2000);
        }
      };
      pollTimer.current = window.setTimeout(tick, 300);
    },
    [finish],
  );

  const follow = useCallback(
    (chatId: string, mid: number) => {
      stopPolling();
      followed.current = mid;
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
            if (followed.current === mid) followed.current = 0;
            finish(chatId, mid, status, body);
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
    [poll, finish],
  );

  const applyChat = useCallback((r: Awaited<ReturnType<typeof api.chat>>) => {
    const byMid = new Map<number, NonNullable<Message["calls"]>>();
    for (const c of r.calls ?? []) {
      const list = byMid.get(c.mid) ?? [];
      list.push(c);
      byMid.set(c.mid, list);
    }
    setMessages(
      r.messages.map((m) =>
        m.role === "assistant" && byMid.has(m.id) ? { ...m, calls: byMid.get(m.id) } : m,
      ),
    );
    setQueue(r.queue ?? []);
    return r.messages.find((m) => m.status === "pending");
  }, []);

  const syncChat = useCallback(
    async (id: string) => {
      try {
        const r = await api.chat(id);
        if (activeRef.current !== id) return;
        const pending = applyChat(r);
        if (pending && followed.current !== pending.id) follow(id, pending.id);
      } catch {}
    },
    [applyChat, follow],
  );

  const openChat = useCallback(
    async (id: string | null, replace = false) => {
      if (id !== activeRef.current) watchAway();
      stopPolling();
      netFails.current = 0;
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
      setQueue([]);
      setLoadingChat(true);
      try {
        const r = await api.chat(id);
        if (activeRef.current !== id) return;
        const pending = applyChat(r);
        if (pending) follow(id, pending.id);
      } catch (e) {
        if (e instanceof ApiError && e.status === 404) {
          notify(t("این گفت‌وگو پیدا نشد.", "This chat was not found."));
          setActiveId(null);
          go(null, true);
          setMessages([]);
        } else if (e instanceof ApiError) notify(e.message);
      } finally {
        setLoadingChat(false);
      }
    },
    [notify, follow, watchAway, applyChat],
  );

  const leaveChat = () => {
    watchAway();
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
        watchAway();
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

  useLayoutEffect(() => {
    const el = scroller.current;
    if (!el) return;
    if (messages.length === 0) {
      el.scrollTop = 0;
      stick.current = true;
    } else if (stick.current) {
      el.scrollTop = el.scrollHeight;
    }
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
  const agentVersion = tools.info?.version ?? "";
  useEffect(() => {
    if (agentVersion) setClientVersion(agentVersion);
  }, [agentVersion]);
  const agentOs = tools.info?.os ?? "";
  const agentOsVersion = tools.info?.os_version ?? "";
  const agentArch = tools.info?.arch ?? "";
  useEffect(() => {
    if (agentOs) setClientPlatform(agentOs, agentOsVersion, agentArch);
  }, [agentOs, agentOsVersion, agentArch]);

  useEffect(() => {
    if (!signedIn) return;
    const check = () => {
      if (document.visibilityState === "visible") api.me().catch(() => {});
    };
    const t = window.setInterval(check, 60000);
    window.addEventListener("focus", check);
    document.addEventListener("visibilitychange", check);
    return () => {
      window.clearInterval(t);
      window.removeEventListener("focus", check);
      document.removeEventListener("visibilitychange", check);
    };
  }, [signedIn]);

  const loadFile = useMemo(() => fileLoader((fid) => api.chatFile(activeId ?? "", fid)), [activeId]);
  const waitingElsewhere = tools.calls.find((c) => c.state === "pending" && c.chat !== activeId);

  const pickWorkspace = async () => {
    try {
      const path = await agent.pickFolder();
      if (path) {
        setWorkspace(path);
        saveWorkspace(path);
      }
    } catch {
      notify(t("انتخاب پوشه ناموفق بود.", "Could not choose the folder."));
    }
  };

  const changeMode = async (m: AgentMode) => {
    if (m === mode) return;
    if (m === "everything") {
      const ok = await confirmDialog({
        title: t("همه‌چیز را خودکار کنم؟", "Allow everything automatically?"),
        message: t(
          "دستیار می‌تواند بیرون از پوشه‌ی کاری هم فایل بسازد و تغییر بدهد و هر فرمانی را بدون پرسیدن از شما اجرا کند. این حالت فقط برای کاری مناسب است که کاملاً به آن و به منبعش اطمینان دارید.",
          "The assistant can create and change files anywhere on this computer, and run any command without asking you. Only use this for a task and a source you fully trust.",
        ),
        confirmLabel: t("بله، همه‌چیز مجاز باشد", "Yes, allow everything"),
        cancelLabel: t("نه، پیش از تغییر بپرس", "No, keep asking"),
        tone: "danger",
        icon: "alert",
      });
      if (!ok) return;
    }
    setMode(m);
    if (activeId) {
      saveMode(activeId, m);
      agent.setMode(activeId, m).catch(() => {});
    }
  };

  const send = async (text: string, files: OutgoingFile[] = [], previews: Pending[] = []): Promise<boolean> => {
    let id = activeId;
    try {
      if (!id) {
        const device = tools.info?.device;
        if (isDesktop && workspace && !device) {
          notify(t("این رایانه هنوز به دستیار وصل نشده است. چند لحظه بعد دوباره تلاش کنید.", "This computer is not connected to the assistant yet. Please try again in a moment."));
          return false;
        }
        const r = await api.createChat(isDesktop && workspace && device ? { device_id: device.id, workspace } : !isDesktop && cloudOk && cloud ? { cloud: true } : undefined);
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
      const local: FileMeta[] = previews.map((p, i) => ({ id: -(i + 1), name: p.file.name, kind: p.kind, mime: p.file.type, size: p.file.size, url: p.preview }));
      const tempUser: Message = { id: -now, role: "user", body: text, status: "done", created: now, files: local.length > 0 ? local : undefined };
      setMessages((ms) => [...ms, tempUser]);
      stick.current = true;
      const chat = chats.find((c) => c.id === id);
      const folder = chat ? (chat.agent ? chat.workspace || "" : "") : isDesktop ? workspace : "";
      const project = isDesktop && folder ? await agent.projectContext(folder).catch(() => undefined) : undefined;
      const r = await api.send(id, text, files, project);
      const user = r.user.files ? { ...r.user, files: r.user.files.map((f, i) => ({ ...f, url: local[i]?.url })) } : r.user;
      setMessages((ms) => [...ms.filter((m) => m.id !== tempUser.id), user, { ...r.assistant, phase: "thinking" }]);
      setChats((cs) => {
        const cur = cs.find((c) => c.id === id);
        const rest = cs.filter((c) => c.id !== id);
        return cur ? [{ ...cur, title: r.title, updated: now }, ...rest] : cs;
      });
      follow(id, r.assistant.id);
      return true;
    } catch (e) {
      setMessages((ms) => ms.filter((m) => m.id > 0));
      notify(e instanceof ApiError ? e.message : t("فرستادن پیام ناموفق بود.", "Could not send the message."));
      return false;
    }
  };

  const folderFor = (id: string) => {
    const chat = chats.find((c) => c.id === id);
    return chat ? (chat.agent ? chat.workspace || "" : "") : isDesktop ? workspace : "";
  };

  const enqueue = async (text: string, files: OutgoingFile[] = []): Promise<boolean> => {
    const id = activeId;
    if (!id) return false;
    try {
      const folder = folderFor(id);
      const project = isDesktop && folder ? await agent.projectContext(folder).catch(() => undefined) : undefined;
      const r = await api.enqueue(id, text, files, project);
      setQueue(r.queue);
      return true;
    } catch (e) {
      notify(e instanceof ApiError ? e.message : t("افزودن به صف ناموفق بود.", "Could not add the message to the queue."));
      return false;
    }
  };

  const unqueue = async (qid: number) => {
    if (!activeId) return;
    try {
      const r = await api.unqueue(activeId, qid);
      setQueue(r.queue);
    } catch (e) {
      if (e instanceof ApiError) notify(e.message);
    }
  };

  const clearQueue = async () => {
    if (!activeId) return;
    try {
      const r = await api.clearQueue(activeId);
      setQueue(r.queue);
    } catch (e) {
      if (e instanceof ApiError) notify(e.message);
    }
  };

  const runQueued = async (qid: number) => {
    const id = activeId;
    if (!id) return;
    stopPolling();
    try {
      await api.runQueued(id, qid);
    } catch (e) {
      if (e instanceof ApiError) notify(e.message);
    }
    await syncChat(id);
  };

  const hasQueue = queue.length > 0;
  useEffect(() => {
    if (!activeId || !hasQueue) return;
    const id = activeId;
    const timer = window.setInterval(async () => {
      try {
        const r = await api.queue(id);
        if (activeRef.current !== id) return;
        const before = queueRef.current;
        setQueue(r.queue);
        if (r.queue.length < before.length || r.queue[0]?.id !== before[0]?.id) syncChat(id);
      } catch {}
    }, 2000);
    return () => window.clearInterval(timer);
  }, [activeId, hasQueue, syncChat]);

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

  if (providerFromPath()) return <OAuthCallback theme={theme} toggleTheme={toggleTheme} />;

  if (route === "telegram") return <TelegramLink user={user} onAuth={onAuth} onSwitchAccount={signOut} theme={theme} toggleTheme={toggleTheme} />;

  if (route === "connect") return <Connect user={user} onAuth={onAuth} onSwitchAccount={signOut} theme={theme} toggleTheme={toggleTheme} />;

  if (!user) return <Auth onAuth={onAuth} theme={theme} toggleTheme={toggleTheme} />;

  const title = route === "chat" ? chats.find((c) => c.id === activeId)?.title || t("دستیار سلام", "Salam AI") : PAGE_TITLES[route];
  const firstName = user.name.trim().split(/\s+/)[0];

  return (
    <div className="app">
      <Sidebar
        user={user}
        chats={chats}
        activeId={route === "chat" ? activeId : null}
        page={route === "admin-chat" ? "admin" : route}
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
          <button type="button" className="icon-button only-mobile" onClick={() => setSidebar(true)} aria-label={t("باز کردن فهرست", "Open menu")}>
            <IconMenu />
          </button>
          <h1 dir="auto">{uiText(title)}</h1>
          <button type="button" className="icon-button only-mobile" onClick={() => openChat(null)} aria-label={t("گفت‌وگوی تازه", "New chat")}>
            <IconPlus />
          </button>
          <button type="button" className="icon-button only-desktop" onClick={toggleTheme} aria-label={t("تغییر پوسته", "Toggle theme")}>
            {theme === "dark" ? <IconSun /> : <IconMoon />}
          </button>
        </header>
        {route === "admin-chat" ? (
          <div className="scroller page-scroller">{user.admin ? <AdminChat id={adminChatIdFromPath()} /> : <NoAccess />}</div>
        ) : route === "connections" || route === "keys" || route === "plans" || route === "memory" || route === "admin" ? (
          <div className="scroller page-scroller" ref={pageScroller}>
            {route === "connections" ? <Connections notify={notify} /> : route === "keys" ? <Keys notify={notify} /> : route === "plans" ? <Plans notify={notify} /> : route === "memory" ? <Memory notify={notify} /> : user.admin ? <Admin notify={notify} /> : <NoAccess />}
          </div>
        ) : (
          <>
            <div className="scroller" ref={scroller} onScroll={onScroll}>
              {messages.length === 0 && !loadingChat ? (
                <div className="welcome">
                  <img src="/logo.svg" alt="" width={64} height={64} className="welcome-logo" />
                  <h2>{t(`سلام ${firstName}! چطور کمکتان کنم؟`, `Hi ${firstName}! How can I help?`)}</h2>
                  <p className="muted">
                    {agentChat
                      ? t(
                          "دستیار روی پوشه‌ی کاری شما کار می‌کند: فایل‌ها را می‌خواند، تغییر می‌دهد، فرمان اجرا می‌کند و تست‌ها را می‌گیرد. پیش از هر تغییر از شما اجازه می‌گیرد.",
                          "The assistant works in your project folder: it reads and edits files, runs commands and runs tests. It asks for your permission before every change.",
                        )
                      : t(
                          "هر پرسشی درباره‌ی زبان برنامه‌نویسی سلام دارید بپرسید: نوشتن کد، کتابخانه‌ی استاندارد، رفع خطا یا تبدیل کد از زبان‌های دیگر.",
                          "Ask anything about the Salam programming language: writing code, the standard library, fixing errors, or converting code from other languages.",
                        )}
                  </p>
                  {cloudOk && !agentChat && !activeId && (
                    <Checkbox
                      className="cloud-toggle"
                      checked={cloud}
                      onChange={toggleCloud}
                      label={t("اجرای دستورها در محیط ابری خصوصی (git، node، python، کامپایلر سلام، GitHub و Figma)", "Run commands in a private cloud sandbox (git, node, python, the Salam compiler, GitHub and Figma)")}
                    />
                  )}
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
                    <MessageView key={m.id} m={m} name={user.name} loadFile={loadFile} calls={m.role === "assistant" ? callsFor(m.id) : undefined} onDecide={tools.decide} onStop={tools.stop} />
                  ))}
                </div>
              )}
            </div>
            {waitingElsewhere && (
              <button type="button" className="agent-waiting" onClick={() => openChat(waitingElsewhere.chat)}>
                {t("یک کار در گفت‌وگوی دیگری منتظر اجازه‌ی شماست. نمایش", "A task in another chat is waiting for your permission. Show")}
              </button>
            )}
            {activeId && <QueuePanel items={queue} working={busy} onRun={runQueued} onRemove={unqueue} onClear={clearQueue} />}
            <Composer
              busy={busy}
              queueing={queue.length > 0}
              onQueue={activeId ? enqueue : undefined}
              disabled={loadingChat}
              onSend={send}
              onStop={stop}
              autoFocus={!activeId}
              draft={draft}
              onDraftUsed={dropDraft}
              placeholder={agentChat ? t("بگویید روی این پروژه چه کاری انجام شود…", "Tell me what to do in this project…") : undefined}
              note={agentChat ? t("دستیار پیش از تغییر فایل یا اجرای فرمان از شما اجازه می‌گیرد، مگر حالت خودکار یا «همه‌چیز» را انتخاب کنید.", "The assistant asks before changing files or running commands, unless you choose automatic or \"everything\" mode.") : undefined}
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
