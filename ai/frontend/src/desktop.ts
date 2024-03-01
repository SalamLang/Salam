import { invoke, isTauri } from "@tauri-apps/api/core";
import { listen, type UnlistenFn } from "@tauri-apps/api/event";

export const isDesktop = isTauri();

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
};

export type AgentInfo = {
  version: string;
  os: "windows" | "macos" | "linux" | "other";
  os_version: string;
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

export const agent = {
  info: () => invoke<AgentInfo>("agent_info"),
  link: (token: string) => invoke<AgentInfo>("agent_link", { token }),
  unlink: (token: string) => invoke<AgentInfo>("agent_unlink", { token }),
  pickFolder: () => invoke<string | null>("agent_pick_folder"),
  setMode: (chat: string, mode: AgentMode) =>
    invoke<void>("agent_set_mode", { chat, mode }),
  decide: (callId: string, answer: AgentAnswer) =>
    invoke<boolean>("agent_decide", { callId, answer }),
  stop: (callId: string) => invoke<boolean>("agent_stop", { callId }),
  pending: () => invoke<Omit<AgentCall, "state" | "output">[]>("agent_pending"),
  projectContext: (workspace: string) =>
    invoke<ProjectContext>("agent_project_context", { workspace }),
  notify: (kind: NotifyKind, chat?: string, text?: string) =>
    invoke<"shown" | "skipped">("agent_notify", { kind, chat, text }),
  notifyPrefs: () => invoke<NotifyPrefs>("agent_notify_prefs"),
  setNotifyPrefs: (prefs: NotifyPrefs) =>
    invoke<NotifyPrefs>("agent_set_notify_prefs", { prefs }),
};

export function onAgent(handlers: {
  call: (c: AgentCall) => void;
  output: (callId: string, chunk: string) => void;
  connection: (online: boolean) => void;
  device: () => void;
}): () => void {
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
