import { lazy, Suspense, useCallback, useEffect, useLayoutEffect, useMemo, useRef, useState } from "react";
import { api, ApiError, getToken, handleUnauthorized, setClientPlatform, setClientVersion, setToken, streamMessage, uiText, type Chat, type ChatStart, type FileMeta, type Message, type OutgoingFile, type Project, type ProjectInput, type QueueItem, type Subscription, type User } from "./api";
import { Admin, NoAccess } from "./Admin";
import { captureFromUrl } from "./campaign";
import { AdminChat } from "./AdminChat";
import { AdminUser } from "./AdminUser";
import { confirmDialog } from "./Confirm";
import { AgentBar, useAgent } from "./Agent";
import { Memory } from "./Memory";
import { fileLoader, type Pending } from "./Attach";
import { Auth } from "./Auth";
import { Composer } from "./Composer";
import { getEffort } from "./effort";
import { QueuePanel } from "./Queue";
import { Connect } from "./Connect";
import { Connections, TelegramLink, integrationsLoader } from "./Telegram";
import { OAuthCallback, providerFromPath } from "./OAuthCallback";
import { agent, isDesktop, saveMode, savedMode, savedWorkspace, saveWorkspace, mention, type AgentMode, type PathStatus } from "./desktop";
import { extraPathProblem, pathErrorText, projectPathProblem, projectPathUnset } from "./paths";
import { Keys } from "./Keys";
import { announce, announceGuest } from "./Notify";
import { Plans } from "./Plans";
import { Usage } from "./Usage";
import { Select } from "./Select";
import { IconArrowLeft, IconBook, IconBug, IconCode, IconFolder, IconGlobe, IconMenu, IconMoon, IconPlus, IconSearch, IconShare, IconSun, IconSwap, IconTerminal } from "./icons";
import { MessageView } from "./Messages";
import { Sidebar } from "./Sidebar";
import { ProjectDialog, ProjectPage, ProjectsHome, confirmDeleteProject } from "./Projects";
import { useTheme } from "./theme";
import { t } from "./i18n";
import { Checkbox } from "./Checkbox";
import { pushToast } from "./Toasts";

const ShareDialog = lazy(() => import("./ShareDialog").then((m) => ({ default: m.ShareDialog })));
const SharedPage = lazy(() => import("./SharedPage").then((m) => ({ default: m.SharedPage })));

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

function shareTokenFromPath(): string | null {
  return /^\/s\/([a-f0-9]{40})\/?$/.exec(location.pathname)?.[1] ?? null;
}

type Route = "share" | "chat" | "projects" | "project" | "others" | "connect" | "telegram" | "connections" | "keys" | "plans" | "usage" | "memory" | "admin" | "admin-user" | "admin-chat";

type Place = { route: Route; chat: string | null; project: string | null };

function adminUserIdFromPath(): number {
  return Number(/^\/admin\/users\/(\d+)(\/[a-z]+)?\/?$/.exec(location.pathname)?.[1] ?? 0);
}

function adminChatIdFromPath(): string {
  return /^\/admin\/chats\/([a-f0-9]{24})\/?$/.exec(location.pathname)?.[1] ?? "";
}

function placeFromPath(): Place {
  const p = location.pathname.replace(/\/+$/, "") || "/";
  if (shareTokenFromPath()) return { route: "share", chat: null, project: null };
  if (adminUserIdFromPath()) return { route: "admin-user", chat: null, project: null };
  if (adminChatIdFromPath()) return { route: "admin-chat", chat: adminChatIdFromPath(), project: null };
  if (p === "/connect") return { route: "connect", chat: null, project: null };
  if (p === "/telegram") return { route: "telegram", chat: null, project: null };
  if (p === "/connections" || p === "/apps") return { route: "connections", chat: null, project: null };
  if (p === "/keys") return { route: "keys", chat: null, project: null };
  if (p === "/plans") return { route: "plans", chat: null, project: null };
  if (p === "/usage") return { route: "usage", chat: null, project: null };
  if (p === "/memory") return { route: "memory", chat: null, project: null };
  if (p === "/admin" || /^\/admin\/(overview|users|chats|billing|api|events)(\/[a-z]+)?$/.test(p)) return { route: "admin", chat: null, project: null };
  const inside = /^\/p\/([a-f0-9]{24})\/c\/([a-f0-9]{24})$/.exec(p);
  if (inside) return { route: "chat", chat: inside[2], project: inside[1] };
  const fresh = /^\/p\/([a-f0-9]{24})\/new$/.exec(p);
  if (fresh) return { route: "chat", chat: null, project: fresh[1] };
  const project = /^\/p\/([a-f0-9]{24})$/.exec(p);
  if (project) return { route: "project", chat: null, project: project[1] };
  const otherChat = /^\/others\/c\/([a-f0-9]{24})$/.exec(p);
  if (otherChat) return { route: "chat", chat: otherChat[1], project: null };
  if (p === "/others/new") return { route: "chat", chat: null, project: null };
  if (p === "/others") return { route: "others", chat: null, project: null };
  const chat = /^\/c\/([a-f0-9]{24})$/.exec(p);
  if (chat) return { route: "chat", chat: chat[1], project: null };
  return { route: "projects", chat: null, project: null };
}

function routeFromPath(): Route {
  return placeFromPath().route;
}

const INITIAL_PLACE: Place = new URLSearchParams(location.search).has("q")
  ? { route: "chat", chat: null, project: null }
  : placeFromPath();

captureFromUrl();

const PAGE_TITLES: Record<Route, string> = {
  share: t("گفت‌وگوی اشتراکی", "Shared chat"),
  chat: t("دستیار سلام", "Salam AI"),
  projects: t("پروژه‌ها", "Projects"),
  project: t("پروژه", "Project"),
  others: t("سایر گفت‌وگوها", "Others"),
  connect: t("اتصال برنامه", "Connect an app"),
  telegram: t("اتصال تلگرام", "Connect Telegram"),
  connections: t("اتصال‌ها و برنامه‌ها", "Connections and apps"),
  keys: t("کلیدهای API", "API keys"),
  plans: t("اشتراک‌ها", "Plans"),
  usage: t("مصرف و توکن‌ها", "Usage and tokens"),
  memory: t("حافظه", "Memory"),
  admin: t("مدیریت", "Admin"),
  "admin-user": t("پروفایل کاربر", "User profile"),
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

function goUrl(url: string, replace = false) {
  if (location.pathname === url) return;
  if (replace) history.replaceState(null, "", url);
  else history.pushState(null, "", url);
}

function chatPath(id: string, project: string | null | undefined): string {
  if (project === undefined) return `/c/${id}`;
  return project ? `/p/${project}/c/${id}` : `/others/c/${id}`;
}

function goChat(id: string, project: string | null | undefined, replace = false) {
  goUrl(chatPath(id, project), replace);
}

function goNew(project: string | null, replace = false) {
  goUrl(project ? `/p/${project}/new` : "/others/new", replace);
}

export function App() {
  const [theme, toggleTheme] = useTheme();
  const [user, setUser] = useState<User | null>(null);
  const [plan, setPlan] = useState<Subscription | null>(null);
  const [draft, setDraft] = useState(takeDraft);
  const dropDraft = useCallback(() => {
    try {
      sessionStorage.removeItem(DRAFT_KEY);
    } catch {}
    setDraft("");
  }, []);
  const [route, setRoute] = useState<Route>(INITIAL_PLACE.route);
  const [booting, setBooting] = useState(true);
  const [chats, setChats] = useState<Chat[]>([]);
  const [projects, setProjects] = useState<Project[]>([]);
  const [projectsReady, setProjectsReady] = useState(false);
  const [projectScope, setProjectScope] = useState<string | null>(INITIAL_PLACE.project);
  const [projectDialog, setProjectDialog] = useState<{ mode: "create" } | { mode: "edit"; project: Project } | null>(null);
  const [projectBusy, setProjectBusy] = useState(false);
  const [activeId, setActiveId] = useState<string | null>(INITIAL_PLACE.chat);
  const [messages, setMessages] = useState<Message[]>([]);
  const [loadingChat, setLoadingChat] = useState(false);
  const [sidebar, setSidebar] = useState(false);
  const [workspace, setWorkspace] = useState(isDesktop ? savedWorkspace() : "");
  const [newExtra, setNewExtra] = useState("");
  const [mode, setMode] = useState<AgentMode>("ask");
  const [queue, setQueue] = useState<QueueItem[]>([]);
  const [cloudOk, setCloudOk] = useState(false);
  const [shareOpen, setShareOpen] = useState(false);
  const guestSeen = useRef(new Map<string, number>());
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
  const initialRouteDone = useRef(false);
  const signedIn = user !== null;
  useEffect(() => {
    if (!signedIn || isDesktop) return;
    const known = integrationsLoader.peek();
    if (known) setCloudOk(known.sandbox);
    integrationsLoader
      .load()
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

  const notify = useCallback((msg: string, ok?: boolean) => {
    pushToast({ tone: ok ? "done" : "failed", title: msg });
  }, []);

  const signOut = useCallback(() => {
    setToken("");
    setUser(null);
    setPlan(null);
    setChats([]);
    setMessages([]);
    setActiveId(null);
    if (routeFromPath() === "chat") goUrl("/", true);
  }, []);

  const loadPlan = useCallback((announceFree: boolean) => {
    api
      .subscription()
      .then((r) => {
        setPlan(r.subscription);
        if (announceFree && r.subscription.plan.id === "free") {
          notify(t("حساب شما روی پلن رایگان است. از منوی «اشتراک‌ها» می‌توانید پلن‌های پولی را ببینید.", "Your account is on the free plan. See the paid plans under Plans in the menu."), true);
        }
      })
      .catch(() => {});
  }, [notify]);

  const onAuth = useCallback((u: User) => {
    setUser(u);
    loadPlan(true);
  }, [loadPlan]);

  useEffect(() => {
    handleUnauthorized(signOut);
    if (!getToken()) {
      setBooting(false);
      return;
    }
    api
      .me()
      .then((r) => {
        setUser(r.user);
        loadPlan(false);
      })
      .catch(() => {})
      .finally(() => setBooting(false));
  }, [signOut]);

  useEffect(() => {
    if (!signedIn || onConnect) return;
    let stopped = false;
    const tick = (first: boolean) => {
      if (document.visibilityState !== "visible") return;
      api
        .chats()
        .then((r) => {
          if (!stopped) setChats(r.chats);
        })
        .catch((e) => {
          if (first) notify(e.message);
        });
      api
        .projects()
        .then((r) => {
          if (!stopped) {
            setProjects(r.projects);
            setProjectsReady(true);
          }
        })
        .catch(() => {
          if (!stopped) setProjectsReady(true);
        });
    };
    tick(true);
    const timer = window.setInterval(() => tick(false), 10000);
    const onVisible = () => {
      if (document.visibilityState === "visible") tick(false);
    };
    document.addEventListener("visibilitychange", onVisible);
    return () => {
      stopped = true;
      window.clearInterval(timer);
      document.removeEventListener("visibilitychange", onVisible);
    };
  }, [signedIn, onConnect, notify]);

  const midFor = useCallback((chat: string) => {
    if (chat !== activeRef.current) return undefined;
    return messagesRef.current.find((m) => m.role === "assistant" && m.status === "pending")?.id;
  }, []);
  const tools = useAgent(isDesktop && signedIn, midFor, notify);

  const finish = useCallback((chatId: string, mid: number, status: Message["status"], body: string) => {
    if (!isDesktop || status === "pending") return;
    if (announced.current.has(mid)) return;
    announced.current.add(mid);
    const title = chatsRef.current.find((c) => c.id === chatId)?.title ?? "";
    if (status === "error") announce("failed", title, uiText(body), mid);
    else announce("done", title, body, mid);
  }, []);

  const watchAway = useCallback(() => {
    const chatId = activeRef.current;
    const pending = messagesRef.current.find((m) => m.role === "assistant" && m.status === "pending");
    if (!isDesktop || !chatId || !pending || pending.id <= 0 || watching.current.has(pending.id)) return;
    const mid = pending.id;
    const until = Date.now() + 12 * 60 * 60 * 1000;
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
          else {
            setChats((cs) => cs.map((c) => (c.id === chatId ? { ...c, state: "idle", phase: undefined, last_error: r.status === "error" } : c)));
            finish(chatId, mid, r.status, r.body);
          }
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
    (chatId: string, mid: number, title = "") => {
      stopPolling();
      followed.current = mid;
      if (isDesktop) {
        const named = title || chatsRef.current.find((c) => c.id === chatId)?.title || "";
        agent.watchReply(chatId, mid, getToken(), named).catch(() => {});
      }
      const ctrl = new AbortController();
      streamCtrl.current = ctrl;
      const update = (fn: (m: Message) => Message) => {
        if (ctrl.signal.aborted || activeRef.current !== chatId) return;
        setMessages((ms) => ms.map((m) => (m.id === mid ? fn(m) : m)));
      };
      let finished = false;
      const opened = Date.now();
      const again = () => {
        if (finished || ctrl.signal.aborted) return;
        if (Date.now() - opened > 60000 && activeRef.current === chatId) {
          window.setTimeout(() => {
            if (ctrl.signal.aborted || activeRef.current !== chatId) return;
            setMessages((ms) => ms.map((m) => (m.id === mid ? { ...m, body: "" } : m)));
            follow(chatId, mid, title);
          }, 500);
        } else poll(chatId, mid);
      };
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
            setChats((cs) => cs.map((c) => (c.id === chatId ? { ...c, state: "idle", phase: undefined, last_error: status === "error" } : c)));
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
          if (!ok) again();
        })
        .catch(() => {
          again();
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
    setChats((cs) =>
      cs.some((c) => c.id === r.chat.id)
        ? cs.map((c) =>
            c.id === r.chat.id
              ? { ...c, title: r.chat.title, project_id: r.chat.project_id, agent: r.chat.agent, cloud: r.chat.cloud, device_id: r.chat.device_id, workspace: r.chat.workspace, shared: r.chat.shared }
              : c,
          )
        : [r.chat, ...cs],
    );
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
    async (id: string | null, replace = false, projectId?: string | null) => {
      if (id !== activeRef.current) watchAway();
      stopPolling();
      netFails.current = 0;
      setSidebar(false);
      setRoute("chat");
      setActiveId(id);
      const listed = id ? chatsRef.current.find((c) => c.id === id) : undefined;
      const known = id === null ? (projectId ?? null) : projectId !== undefined ? projectId : listed?.project_id;
      setProjectScope(known || null);
      if (id) goChat(id, known, replace);
      else goNew(known ?? null, replace);
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
        const actual = r.chat.project_id ?? "";
        if (actual !== (known ?? "")) {
          setProjectScope(actual || null);
          goChat(id, actual, true);
        }
        const pending = applyChat(r);
        if (pending) follow(id, pending.id);
      } catch (e) {
        if (e instanceof ApiError && e.status === 404) {
          notify(t("این گفت‌وگو پیدا نشد.", "This chat was not found."));
          setActiveId(null);
          goNew(null, true);
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
    setProjectScope(null);
    setMessages([]);
  };

  const navigate = (path: string) => {
    leaveChat();
    if (location.pathname !== path) history.pushState(null, "", path);
    const place = placeFromPath();
    setRoute(place.route);
    setProjectScope(place.project);
  };

  useEffect(() => {
    if (!signedIn) return;
    const askedByLink = !initialRouteDone.current && INITIAL_PLACE.route === "chat";
    initialRouteDone.current = true;
    if (routeFromPath() === "chat" || askedByLink) {
      const place = placeFromPath();
      openChat(place.chat, true, /^\/c\//.test(location.pathname) ? undefined : (place.project ?? ""));
    }
    const onPop = () => {
      const place = placeFromPath();
      if (place.route === "chat") {
        openChat(place.chat, true, /^\/c\//.test(location.pathname) ? undefined : (place.project ?? ""));
      } else {
        watchAway();
        stopPolling();
        setSidebar(false);
        setActiveId(null);
        setProjectScope(place.project);
        setMessages([]);
        setRoute(place.route);
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
  const scopedProject = projects.find((p) => p.id === (activeChat ? activeChat.project_id : projectScope));
  const projectChat = isDesktop && scopedProject !== undefined && (activeChat ? activeChat.agent === true : true);
  const projectFolder = scopedProject?.path ?? "";
  const effWorkspace = projectChat ? projectFolder : activeChat ? activeChat.workspace || "" : workspace;
  const extraNow = activeChat ? activeChat.extra_path || "" : newExtra;
  const agentChat = isDesktop && (activeChat ? activeChat.agent === true : projectChat ? projectFolder !== "" : workspace !== "");
  const chatElsewhere = activeChat?.agent === true && tools.info?.device !== null && tools.info?.device !== undefined && String(activeChat.device_id) !== tools.info.device.id;
  const mentionRoot = agentChat && !chatElsewhere ? effWorkspace : "";
  const lastAssistant = [...messages].reverse().find((m) => m.role === "assistant")?.id;
  const callsFor = (mid: number) => tools.calls.filter((c) => c.chat === activeId && (c.mid === mid || (c.mid === undefined && mid === lastAssistant)));
  const agentVersion = tools.info?.version ?? "";
  useEffect(() => {
    if (agentVersion) setClientVersion(agentVersion);
  }, [agentVersion]);
  const agentOs = tools.info?.os ?? "";
  const agentOsVersion = tools.info?.os_version ?? "";
  const agentArch = tools.info?.arch ?? "";
  const agentOsId = tools.info?.os_id ?? "";
  const agentOsBuild = tools.info?.os_build ?? "";
  useEffect(() => {
    if (agentOs) setClientPlatform(agentOs, agentOsVersion, agentArch, agentOsId, agentOsBuild);
  }, [agentOs, agentOsVersion, agentArch, agentOsId, agentOsBuild]);

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

  const sharedChats = chats.filter((c) => c.shared?.includes("collab")).map((c) => c.id).join(",");
  useEffect(() => {
    if (!signedIn || !sharedChats) return;
    let stopped = false;
    const look = async () => {
      if (stopped || document.visibilityState !== "visible") return;
      for (const id of sharedChats.split(",")) {
        try {
          const r = await api.chat(id);
          if (stopped) return;
          const newest = r.messages.filter((m) => m.guest).reduce((n, m) => Math.max(n, m.id), 0);
          const seen = guestSeen.current.get(id);
          guestSeen.current.set(id, newest);
          if (seen === undefined || newest <= seen) continue;
          const last = r.messages.find((m) => m.id === newest);
          const title = chatsRef.current.find((c) => c.id === id)?.title ?? r.chat.title;
          if (last) announceGuest(last.guest ?? "", title, last.body);
          if (id === activeRef.current) {
            const pending = applyChat(r);
            if (pending && followed.current !== pending.id) follow(id, pending.id);
          }
        } catch {}
      }
    };
    look();
    const timer = window.setInterval(look, 5000);
    return () => {
      stopped = true;
      window.clearInterval(timer);
    };
  }, [signedIn, sharedChats, applyChat, follow]);

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

  const pickExtra = async () => {
    try {
      const path = await agent.pickFolder();
      if (!path) return;
      if (!activeChat) {
        setNewExtra(path);
        return;
      }
      await api.setChatExtra(activeChat.id, path);
      setChats((cs) => cs.map((c) => (c.id === activeChat.id ? { ...c, extra_path: path } : c)));
    } catch (e) {
      notify(pathErrorText(e) ?? (e instanceof ApiError ? e.message : t("انتخاب پوشه ناموفق بود.", "Could not choose the folder.")));
    }
  };

  const clearExtra = async () => {
    if (!activeChat) {
      setNewExtra("");
      return;
    }
    try {
      await api.setChatExtra(activeChat.id, "");
      setChats((cs) => cs.map((c) => (c.id === activeChat.id ? { ...c, extra_path: undefined } : c)));
    } catch (e) {
      notify(pathErrorText(e) ?? (e instanceof ApiError ? e.message : t("برداشتن پوشه ناموفق بود.", "Could not remove the folder.")));
    }
  };

  const folderProblem = async (folder: string, extra: string): Promise<string> => {
    if (!folder) return projectPathUnset();
    const status = await agent.pathStatus(folder).catch((): PathStatus => "ok");
    if (status !== "ok") return projectPathProblem(status);
    if (extra) {
      const more = await agent.pathStatus(extra).catch((): PathStatus => "ok");
      if (more !== "ok") return extraPathProblem();
    }
    return "";
  };

  const chatFolder = (chat: Chat): string => (chat.agent ? projects.find((p) => p.id === chat.project_id)?.path || chat.workspace || "" : "");

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
    if (projectChat) {
      const problem = await folderProblem(projectFolder, extraNow);
      if (problem) {
        notify(problem);
        return false;
      }
    }
    try {
      if (!id) {
        const device = tools.info?.device;
        if (isDesktop && effWorkspace && !device) {
          notify(t("این رایانه هنوز به دستیار وصل نشده است. چند لحظه بعد دوباره تلاش کنید.", "This computer is not connected to the assistant yet. Please try again in a moment."));
          return false;
        }
        const start: ChatStart = {};
        if (isDesktop && effWorkspace && device) {
          start.device_id = device.id;
          start.workspace = effWorkspace;
          if (extraNow) start.extra_path = extraNow;
        } else if (!isDesktop && cloudOk && cloud) {
          start.cloud = true;
        }
        if (projectScope) start.project_id = projectScope;
        const r = await api.createChat(start);
        id = r.chat.id;
        if (r.chat.agent) {
          saveMode(id, mode);
          await agent.setMode(id, mode).catch(() => {});
        }
        setChats((cs) => [r.chat, ...cs]);
        setNewExtra("");
        setActiveId(id);
        activeRef.current = id;
        goChat(id, projectScope ?? null);
      }
      const now = Math.floor(Date.now() / 1000);
      const local: FileMeta[] = previews.map((p, i) => ({ id: -(i + 1), name: p.file.name, kind: p.kind, mime: p.file.type, size: p.file.size, url: p.preview }));
      const effort = getEffort();
      const tempUser: Message = { id: -now, role: "user", body: text, status: "done", created: now, files: local.length > 0 ? local : undefined, effort };
      setMessages((ms) => [...ms, tempUser]);
      stick.current = true;
      const chat = chats.find((c) => c.id === id);
      const folder = chat ? chatFolder(chat) : isDesktop ? effWorkspace : "";
      const project = isDesktop && folder ? await agent.projectContext(folder).catch(() => undefined) : undefined;
      const r = await api.send(id, text, files, project, effort);
      const user = r.user.files ? { ...r.user, files: r.user.files.map((f, i) => ({ ...f, url: local[i]?.url })) } : r.user;
      setMessages((ms) => [...ms.filter((m) => m.id !== tempUser.id), user, { ...r.assistant, phase: "thinking" }]);
      setChats((cs) => {
        const cur = cs.find((c) => c.id === id);
        const rest = cs.filter((c) => c.id !== id);
        return cur ? [{ ...cur, title: r.title, updated: now, state: "working", phase: "thinking", last_error: false }, ...rest] : cs;
      });
      follow(id, r.assistant.id, r.title);
      return true;
    } catch (e) {
      setMessages((ms) => ms.filter((m) => m.id > 0));
      notify(pathErrorText(e) ?? (e instanceof ApiError ? e.message : t("فرستادن پیام ناموفق بود.", "Could not send the message.")));
      return false;
    }
  };

  const folderFor = (id: string) => {
    const chat = chats.find((c) => c.id === id);
    return chat ? chatFolder(chat) : isDesktop ? effWorkspace : "";
  };

  const enqueue = async (text: string, files: OutgoingFile[] = []): Promise<boolean> => {
    const id = activeId;
    if (!id) return false;
    try {
      const folder = folderFor(id);
      const project = isDesktop && folder ? await agent.projectContext(folder).catch(() => undefined) : undefined;
      const r = await api.enqueue(id, text, files, project, getEffort());
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
    if (isDesktop) agent.unwatchReply(activeId).catch(() => {});
    try {
      const r = await api.stop(activeId);
      setMessages((ms) => ms.map((m) => (m.status === "pending" ? { ...m, status: "done", body: r.body } : m)));
      setChats((cs) => cs.map((c) => (c.id === activeId ? { ...c, state: "idle", phase: undefined } : c)));
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
      if (isDesktop) agent.unwatchReply(id).catch(() => {});
      setChats((cs) => cs.filter((c) => c.id !== id));
      if (id === activeId) navigate(projectScope ? `/p/${projectScope}` : "/others");
    } catch (e) {
      if (e instanceof ApiError) notify(e.message);
    }
  };

  const refreshProjects = useCallback(() => {
    api
      .projects()
      .then((r) => {
        setProjects(r.projects);
        setProjectsReady(true);
      })
      .catch(() => {});
  }, []);

  const saveProject = async (input: ProjectInput) => {
    setProjectBusy(true);
    try {
      if (projectDialog?.mode === "edit") {
        const r = await api.updateProject(projectDialog.project.id, input);
        setProjects((ps) => ps.map((p) => (p.id === r.project.id ? { ...p, ...r.project, created: p.created, chats: p.chats, live: p.live, last: p.last } : p)));
        if (r.project.path !== undefined) {
          const moved = r.project.path;
          setChats((cs) => cs.map((c) => (c.project_id === r.project.id && c.agent ? { ...c, workspace: moved } : c)));
        }
        setProjectDialog(null);
        refreshProjects();
      } else {
        const r = await api.createProject(input);
        setProjects((ps) => [r.project, ...ps]);
        setProjectDialog(null);
        navigate(`/p/${r.project.id}`);
        refreshProjects();
      }
    } catch (e) {
      notify(pathErrorText(e) ?? (e instanceof ApiError ? e.message : t("ذخیره‌ی پروژه ناموفق بود.", "Could not save the project.")));
    } finally {
      setProjectBusy(false);
    }
  };

  const removeProject = async (project: Project) => {
    if (!(await confirmDeleteProject(project))) return;
    try {
      await api.deleteProject(project.id);
      setProjects((ps) => ps.filter((p) => p.id !== project.id));
      setChats((cs) => cs.map((c) => (c.project_id === project.id ? { ...c, project_id: undefined } : c)));
      if (projectScope === project.id) navigate("/");
      refreshProjects();
    } catch (e) {
      if (e instanceof ApiError) notify(e.message);
    }
  };

  const moveChatTo = async (id: string, projectId: string) => {
    try {
      await api.moveChat(id, projectId);
      setChats((cs) => cs.map((c) => (c.id === id ? { ...c, project_id: projectId || undefined } : c)));
      if (id === activeId) {
        setProjectScope(projectId || null);
        goChat(id, projectId || null, true);
      }
      refreshProjects();
    } catch (e) {
      if (e instanceof ApiError) notify(e.message);
    }
  };

  const openProject = (id: string) => navigate(`/p/${id}`);
  const openOthers = () => navigate("/others");
  const newChatHere = () => openChat(null, false, projectScope ?? "");

  const logout = async () => {
    try {
      await api.logout();
    } catch {}
    signOut();
  };

  const sharedToken = route === "share" ? shareTokenFromPath() : null;
  if (sharedToken) {
    return (
      <Suspense
        fallback={
          <div className="boot">
            <img src="/logo.svg" alt="" width={64} height={64} />
          </div>
        }
      >
        <SharedPage token={sharedToken} theme={theme} toggleTheme={toggleTheme} />
      </Suspense>
    );
  }

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

  const activeProject = projects.find((p) => p.id === projectScope);
  const sidebarChats = route === "projects" ? chats : projectScope ? chats.filter((c) => c.project_id === projectScope) : chats.filter((c) => !c.project_id);
  const title =
    route === "chat"
      ? chats.find((c) => c.id === activeId)?.title ||
        (activeProject ? t(`گفت‌وگوی تازه در ${activeProject.name}`, `New chat in ${activeProject.name}`) : t("گفت‌وگوی تازه", "New chat"))
      : route === "project"
        ? activeProject?.name || PAGE_TITLES.project
        : PAGE_TITLES[route];
  const firstName = user.name.trim().split(/\s+/)[0];
  const backTo = route === "chat" ? (projectScope ? `/p/${projectScope}` : "/others") : route === "project" || route === "others" ? "/" : "";

  return (
    <div className="app">
      <Sidebar
        user={user}
        plan={plan}
        projects={projects}
        activeProject={projectScope}
        page={route === "admin-chat" || route === "admin-user" ? "admin" : route}
        onNavigate={navigate}
        open={sidebar}
        onClose={() => setSidebar(false)}
        onNew={newChatHere}
        onLogout={logout}
        onOpenProjects={() => navigate("/")}
        onOpenProject={openProject}
        onOpenOthers={openOthers}
        onNewProject={() => setProjectDialog({ mode: "create" })}
      />
      <main className="main">
        <header className="topbar">
          <button type="button" className="icon-button only-mobile" onClick={() => setSidebar(true)} aria-label={t("باز کردن فهرست", "Open menu")}>
            <IconMenu />
          </button>
          {backTo && (
            <button type="button" className="icon-button back-button" onClick={() => navigate(backTo)} aria-label={t("بازگشت", "Back")} title={t("بازگشت", "Back")}>
              <IconArrowLeft width={18} height={18} />
            </button>
          )}
          <h1 dir="auto">{uiText(title)}</h1>
          {route === "chat" && activeId && (
            <div className="project-move" title={t("انتقال به پروژه", "Move to a project")}>
              <IconFolder width={15} height={15} />
              <Select
                className="project-select"
                value={activeChat?.project_id ?? ""}
                options={[{ value: "", label: t("سایر گفت‌وگوها", "Others") }, ...projects.map((p) => ({ value: p.id, label: p.name }))]}
                ariaLabel={t("انتقال گفت‌وگو به پروژه", "Move chat to a project")}
                searchLabel={t("جست‌وجوی پروژه", "Search projects")}
                emptyLabel={t("پروژه‌ای پیدا نشد", "No project found")}
                onChange={(v) => activeChat && moveChatTo(activeChat.id, v)}
              />
            </div>
          )}
          {route === "chat" && activeId && (
            <button
              type="button"
              className={`icon-button share-button${activeChat?.shared && activeChat.shared.length > 0 ? " on" : ""}`}
              disabled={!activeChat}
              onClick={() => setShareOpen(true)}
              aria-label={t("اشتراک‌گذاری گفت‌وگو", "Share this chat")}
              title={activeChat ? t("اشتراک‌گذاری با پیوند", "Share with a link") : t("این گفت‌وگو را نمی‌شود به اشتراک گذاشت", "This chat cannot be shared")}
            >
              <IconShare width={18} height={18} />
              <span className="share-button-label">{t("اشتراک", "Share")}</span>
            </button>
          )}
          <button type="button" className="icon-button only-mobile" onClick={newChatHere} aria-label={t("گفت‌وگوی تازه", "New chat")}>
            <IconPlus />
          </button>
          <button type="button" className="icon-button" onClick={toggleTheme} aria-label={t("تغییر پوسته", "Toggle theme")}>
            {theme === "dark" ? <IconSun /> : <IconMoon />}
          </button>
        </header>
        {route === "projects" ? (
          <div className="scroller page-scroller" ref={pageScroller}>
            <ProjectsHome
              projects={projects}
              chats={chats}
              onOpenProject={openProject}
              onOpenChat={(id, pid) => openChat(id, false, pid)}
              onOpenOthers={openOthers}
              onNewProject={() => setProjectDialog({ mode: "create" })}
              onEditProject={(project) => setProjectDialog({ mode: "edit", project })}
              onDeleteProject={removeProject}
            />
          </div>
        ) : route === "project" ? (
          <div className="scroller page-scroller" ref={pageScroller}>
            {activeProject ? (
              <ProjectPage
                project={activeProject}
                chats={sidebarChats}
                onOpenChat={(id, pid) => openChat(id, false, pid)}
                onNewChat={() => openChat(null, false, activeProject.id)}
                onEdit={() => setProjectDialog({ mode: "edit", project: activeProject })}
                onDelete={() => removeProject(activeProject)}
                onRenameChat={rename}
                onDeleteChat={remove}
              />
            ) : (
              <div className="page">
                <div className="card empty-card">
                  <h3>{!projectsReady ? t("در حال بارگذاری پروژه…", "Loading project…") : t("این پروژه پیدا نشد.", "This project was not found.")}</h3>
                  <button type="button" className="button ghost" onClick={() => navigate("/")}>{t("بازگشت به پروژه‌ها", "Back to projects")}</button>
                </div>
              </div>
            )}
          </div>
        ) : route === "others" ? (
          <div className="scroller page-scroller" ref={pageScroller}>
            <ProjectPage
              project={null}
              chats={chats.filter((c) => !c.project_id)}
              onOpenChat={(id, pid) => openChat(id, false, pid)}
              onNewChat={() => openChat(null, false, "")}
              onRenameChat={rename}
              onDeleteChat={remove}
            />
          </div>
        ) : route === "admin-user" ? (
          <div className="scroller page-scroller">{user.admin ? <AdminUser id={adminUserIdFromPath()} notify={notify} onNavigate={navigate} /> : <NoAccess />}</div>
        ) : route === "admin-chat" ? (
          <div className="scroller page-scroller">{user.admin ? <AdminChat id={adminChatIdFromPath()} /> : <NoAccess />}</div>
        ) : route === "connections" || route === "keys" || route === "plans" || route === "usage" || route === "memory" || route === "admin" ? (
          <div className="scroller page-scroller" ref={pageScroller}>
            {route === "connections" ? <Connections notify={notify} /> : route === "keys" ? <Keys notify={notify} /> : route === "plans" ? <Plans notify={notify} /> : route === "usage" ? <Usage onNavigate={navigate} /> : route === "memory" ? <Memory notify={notify} /> : user.admin ? <Admin notify={notify} onNavigate={navigate} /> : <NoAccess />}
          </div>
        ) : (
          <>
            <div className="scroller" ref={scroller} onScroll={onScroll}>
              {messages.length === 0 && !loadingChat ? (
                <div className="welcome">
                  <img src="/logo.svg" alt="" width={64} height={64} className="welcome-logo" />
                  <h2>{t(`سلام ${firstName}! چطور کمکتان کنم؟`, `Hi ${firstName}! How can I help?`)}</h2>
                  {activeProject && (
                    <p className="welcome-project">
                      <IconFolder width={16} height={16} />
                      {t("در پروژه‌ی", "In project")} <strong dir="auto">{activeProject.name}</strong>
                    </p>
                  )}
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
                  {messages.map((m) =>
                    m.guest && m.role === "user" ? (
                      <div key={m.id} className="shared-msg">
                        <span className="shared-guest">
                          {t("از راه پیوند", "Via link")}: <bdi dir="auto">{m.guest}</bdi>
                        </span>
                        <MessageView m={m} name={m.guest} loadFile={loadFile} />
                      </div>
                    ) : (
                      <MessageView key={m.id} m={m} name={user.name} loadFile={loadFile} calls={m.role === "assistant" ? callsFor(m.id) : undefined} cloud={activeChat?.cloud === true} onDecide={tools.decide} onStop={tools.stop} />
                    ),
                  )}
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
              workspace={mentionRoot}
              placeholder={agentChat ? t("بگویید روی این پروژه چه کاری انجام شود…", "Tell me what to do in this project…") : undefined}
              note={agentChat ? t("دستیار پیش از تغییر فایل یا اجرای فرمان از شما اجازه می‌گیرد، مگر حالت خودکار یا «همه‌چیز» را انتخاب کنید.", "The assistant asks before changing files or running commands, unless you choose automatic or \"everything\" mode.") : undefined}
              bar={
                isDesktop && (!activeChat || activeChat.agent) ? (
                  <AgentBar
                    info={tools.info}
                    workspace={effWorkspace}
                    locked={activeChat !== undefined}
                    inProject={projectChat}
                    extra={extraNow}
                    onExtraPick={pickExtra}
                    onExtraClear={clearExtra}
                    elsewhere={activeChat?.agent === true && tools.info?.device !== null && tools.info?.device !== undefined && String(activeChat.device_id) !== tools.info.device.id}
                    mode={mode}
                    onPick={pickWorkspace}
                    onClear={() => {
                      if (workspace) mention.close(workspace).catch(() => {});
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
      {projectDialog && (
        <ProjectDialog
          project={projectDialog.mode === "edit" ? projectDialog.project : null}
          busy={projectBusy}
          running={projectDialog.mode === "edit" && (projects.find((p) => p.id === projectDialog.project.id)?.live ?? 0) > 0}
          onSave={saveProject}
          onClose={() => setProjectDialog(null)}
        />
      )}
      {shareOpen && activeId && activeChat && (
        <Suspense fallback={null}>
          <ShareDialog
            chatId={activeId}
            title={activeChat.title}
            onClose={() => setShareOpen(false)}
            onChange={(kinds) => setChats((cs) => cs.map((c) => (c.id === activeId ? { ...c, shared: kinds.length > 0 ? kinds : undefined } : c)))}
          />
        </Suspense>
      )}
    </div>
  );
}
