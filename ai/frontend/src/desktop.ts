import { invoke, isTauri } from "@tauri-apps/api/core";
import { listen, type UnlistenFn } from "@tauri-apps/api/event";
import { useEffect } from "react";

type SalamBridge = {
  invoke<T>(command: string, args?: Record<string, unknown>): Promise<T>;
  listen(event: string, handler: (data: unknown) => void): () => void;
};

const salam = (window as unknown as { salam?: SalamBridge }).salam;

export const isDesktop = isTauri() || Boolean(salam);

function call<T>(command: string, args?: Record<string, unknown>): Promise<T> {
  return salam ? salam.invoke<T>(command, args) : invoke<T>(command, args);
}

export type AgentMode = "ask" | "auto" | "readonly" | "everything";
export type AgentAnswer = "allow" | "always" | "deny";
export type CallState = "pending" | "running" | "done" | "error" | "denied";
export type CallKind = "read" | "write" | "exec";

export type AgentCall = {
  call_id: string;
  chat: string;
  tool: string;
  kind: CallKind;
  summary: string;
  detail: string;
  lang: string;
  outside: boolean;
  cwd: string;
  state: CallState;
  output: string;
  image?: boolean;
  image_data?: string;
  image_mime?: string;
};

export type AgentInfo = {
  version: string;
  os: "windows" | "macos" | "linux" | "other";
  os_version: string;
  os_id?: string;
  os_build?: string;
  arch: string;
  device: { id: string; name: string } | null;
  connected: boolean;
};

export type ProjectContext = {
  instructions_file: string;
  instructions: string;
  memory_index: string;
  memories: number;
};

export type NotifyKind = "done" | "failed" | "network" | "test";

export type NotifyPrefs = {
  enabled: boolean;
  sound: boolean;
  preview: boolean;
  lang: "fa" | "en";
};

export type PathStatus = "ok" | "missing" | "not_folder" | "invalid";

export const agent = {
  info: () => call<AgentInfo>("agent_info"),
  openUrl: (url: string) => call<void>("agent_open_url", { url }),
  link: (token: string) => call<AgentInfo>("agent_link", { token }),
  unlink: (token: string) => call<AgentInfo>("agent_unlink", { token }),
  pickFolder: () => call<string | null>("agent_pick_folder"),
  pathStatus: (path: string) => call<PathStatus>("agent_path_status", { path }),
  setMode: (chat: string, mode: AgentMode) =>
    call<void>("agent_set_mode", { chat, mode }),
  decide: (callId: string, answer: AgentAnswer) =>
    call<boolean>("agent_decide", { callId, answer }),
  stop: (callId: string) => call<boolean>("agent_stop", { callId }),
  pending: () => call<Omit<AgentCall, "state" | "output">[]>("agent_pending"),
  projectContext: (workspace: string) =>
    call<ProjectContext>("agent_project_context", { workspace }),
  notify: (kind: NotifyKind, chat?: string, text?: string, mid?: number) =>
    call<"shown" | "skipped">("agent_notify", { kind, chat, text, mid }),
  watchReply: (chat: string, mid: number, token: string, title: string) =>
    call<void>("agent_watch_reply", { chat, mid, token, title }),
  unwatchReply: (chat: string) => call<void>("agent_unwatch_reply", { chat }),
  notifyPrefs: () => call<NotifyPrefs>("agent_notify_prefs"),
  setNotifyPrefs: (prefs: NotifyPrefs) =>
    call<NotifyPrefs>("agent_set_notify_prefs", { prefs }),
};

export type ChatgptProgress = {
  phase: "reading" | "uploading" | "processing";
  done: number;
  total: number;
  bytes: number;
};

export const nativeChatgptImport = isTauri();

export const chatgptExport = {
  pick: () => call<string | null>("chatgpt_pick_export"),
  run: (path: string, token: string) =>
    call<{ import: { id: number } }>("chatgpt_import", { path, token }),
  onProgress: async (
    handler: (p: ChatgptProgress) => void,
  ): Promise<() => void> =>
    listen<ChatgptProgress>("chatgpt-import", (e) => handler(e.payload)),
};

export type MentionHit = {
  path: string;
  is_dir: boolean;
  name_at: number;
  ranges: [number, number][];
};

export type MentionReply = {
  hits: MentionHit[];
  indexing: boolean;
  count: number;
  revision: number;
  stale: boolean;
};

export const mention = {
  open: (root: string) => call<unknown>("ws_index_open", { root }),
  close: (root: string) => call<void>("ws_index_close", { root }),
  search: (root: string, query: string, limit = 50) =>
    call<MentionReply>("ws_search", { root, query, limit }),
  picked: (root: string, path: string) =>
    call<void>("ws_mention_picked", { root, path }),
  onUpdated: (handler: () => void): (() => void) => {
    if (salam) return salam.listen("ws-index-updated", () => handler());
    const sub: Promise<UnlistenFn> = listen("ws-index-updated", () =>
      handler(),
    );
    return () => {
      sub.then((off) => off()).catch(() => {});
    };
  },
};

const EXTERNAL = /^(https?:|mailto:)/i;

export function openExternal(url: string): void {
  if (isDesktop) {
    call<void>("agent_open_url", { url }).catch(() => {
      // Older agents have no such command: a same-window navigation still
      // reaches on_navigation, which opens the browser and cancels the load.
      // Only http(s) and mailto reach here, so the app cannot leave its origin.
      window.location.href = url;
    });
    return;
  }
  window.open(url, "_blank", "noopener");
}

/**
 * In the desktop app a `target="_blank"` link does nothing on macOS: the
 * WKWebView never asks the app to open the new window, so the click is lost.
 * Route every external link through the native command instead; the browser
 * build keeps its normal anchors.
 */
export function installLinkHandler(): void {
  if (!isDesktop) return;
  document.addEventListener(
    "click",
    (event) => {
      if (event.defaultPrevented || event.button !== 0) return;
      if (event.metaKey || event.ctrlKey || event.shiftKey || event.altKey)
        return;
      const target = event.target as Element | null;
      const anchor = target?.closest?.("a[href]") as HTMLAnchorElement | null;
      if (!anchor) return;
      const href = anchor.getAttribute("href") || "";
      if (!EXTERNAL.test(href)) return;
      event.preventDefault();
      event.stopPropagation();
      openExternal(href);
    },
    true,
  );
}

export function onAgent(handlers: {
  call: (c: AgentCall) => void;
  output: (callId: string, chunk: string) => void;
  connection: (online: boolean) => void;
  device: () => void;
}): () => void {
  if (salam) {
    const stops = [
      salam.listen("agent://call", (data) => handlers.call(data as AgentCall)),
      salam.listen("agent://output", (data) => {
        const { call_id, chunk } = data as { call_id: string; chunk: string };
        handlers.output(call_id, chunk);
      }),
      salam.listen("agent://connection", (data) =>
        handlers.connection(data as boolean),
      ),
      salam.listen("agent://device", () => handlers.device()),
    ];
    return () => {
      for (const stop of stops) stop();
    };
  }
  const subs: Promise<UnlistenFn>[] = [
    listen<AgentCall>("agent://call", (e) => handlers.call(e.payload)),
    listen<{ call_id: string; chunk: string }>("agent://output", (e) =>
      handlers.output(e.payload.call_id, e.payload.chunk),
    ),
    listen<boolean>("agent://connection", (e) =>
      handlers.connection(e.payload),
    ),
    listen("agent://device", () => handlers.device()),
  ];
  return () => {
    for (const s of subs) s.then((off) => off()).catch(() => {});
  };
}

const MODE_KEY = "salam-agent-mode:";
const WORKSPACE_KEY = "salam-agent-workspace";

function read(key: string): string {
  try {
    return localStorage.getItem(key) || "";
  } catch {
    return "";
  }
}

function write(key: string, value: string) {
  try {
    if (value) localStorage.setItem(key, value);
    else localStorage.removeItem(key);
  } catch {}
}

export function savedMode(chat: string): AgentMode {
  const m = read(MODE_KEY + chat);
  return m === "auto" || m === "readonly" || m === "everything" ? m : "ask";
}

export function saveMode(chat: string, mode: AgentMode) {
  write(MODE_KEY + chat, mode === "ask" ? "" : mode);
}

export function savedWorkspace(): string {
  return read(WORKSPACE_KEY);
}

export function saveWorkspace(path: string) {
  write(WORKSPACE_KEY, path);
}

export function folderName(path: string): string {
  const parts = path.split(/[\\/]+/).filter(Boolean);
  return parts[parts.length - 1] || path;
}

export function useReturnReset(
  active: boolean,
  reset: () => void,
  timeoutMs = 120000,
): void {
  useEffect(() => {
    if (!active) return;
    const armed = Date.now() + 1500;
    const back = () => {
      if (Date.now() >= armed && document.visibilityState !== "hidden") reset();
    };
    const pageshow = (e: PageTransitionEvent) => {
      if (e.persisted) reset();
    };
    window.addEventListener("focus", back);
    document.addEventListener("visibilitychange", back);
    window.addEventListener("pageshow", pageshow);
    const timer = window.setTimeout(reset, timeoutMs);
    return () => {
      window.removeEventListener("focus", back);
      document.removeEventListener("visibilitychange", back);
      window.removeEventListener("pageshow", pageshow);
      window.clearTimeout(timer);
    };
  }, [active, reset, timeoutMs]);
}
