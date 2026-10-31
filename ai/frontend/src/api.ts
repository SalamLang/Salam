import { isDesktop, type ProjectContext } from "./desktop";
import { isFa, lang, t } from "./i18n";
export type User = {
  id: number;
  name: string;
  email: string;
  admin?: boolean;
};
export type Chat = {
  id: string;
  title: string;
  updated: number;
  agent?: boolean;
  cloud?: boolean;
  device_id?: number;
  workspace?: string;
};
export type FileMeta = {
  id: number;
  name: string;
  kind: "image" | "text" | "pdf" | "archive";
  mime: string;
  size: number;
  url?: string;
};
export type Integration = {
  provider: "github" | "figma";
  connected: boolean;
  expired?: boolean;
  login: string;
  scope: string;
  created: number;
  oauth: boolean;
};
export type Integrations = {
  sandbox: boolean;
  storage: boolean;
  integrations: Integration[];
};
export type TelegramLinkInfo = { name: string; username: string; bot: string };
export type TelegramAccount = {
  id: string;
  name: string;
  username: string;
  created: number;
};
export type TelegramStatus = {
  enabled: boolean;
  bot: string;
  links: TelegramAccount[];
};
export type OutgoingFile = { name: string; data: string };
export type Capabilities = {
  image: boolean;
  pdf: boolean;
  max_files: number;
  max_total: number;
  max_image: number;
  max_pdf: number;
  max_text: number;
  max_archive: number;
  text: string[];
  text_names: string[];
};
export type AdminUpload = {
  id: number;
  chat_id: string;
  message_id: number;
  name: string;
  kind: FileMeta["kind"];
  mime: string;
  size: number;
  path: string;
  listing: string;
  created: number;
  user: { id: number; name: string; email: string };
  agent: boolean;
};
export type AdminGuest = {
  id: number;
  ip: string;
  lang: string;
  messages: number;
  created: number;
  updated: number;
  first: string;
};
export type GuestMessage = {
  id: number;
  role: "user" | "assistant";
  body: string;
  status: string;
  created: number;
};
export type AdminSession = {
  id: number;
  created: number;
  last_used: number;
  expires: number;
  ip: string;
  agent: string;
  client: "web" | "agent";
  client_version: string;
  os: string;
  os_version: string;
  arch: string;
  current: boolean;
  user: { id: number; name: string; email: string };
};
export type PlatformId =
  | "windows"
  | "macos"
  | "linux"
  | "other"
  | "telegram"
  | "unknown";
export type AdminPlatforms = {
  days: number;
  total: number;
  platforms: { id: PlatformId; count: number }[];
  systems: { os: string; version: string; arch: string; count: number }[];
};
export type AdminChatOwner = { id: number; name: string; email: string };
export type Device = {
  id: number;
  name: string;
  os: string;
  created: number;
  online: boolean;
};
export type Message = {
  id: number;
  role: "user" | "assistant";
  body: string;
  status: "done" | "pending" | "error";
  created: number;
  phase?: Phase;
  activity?: ActivityStep[];
  calls?: MessageCall[];
  ms?: number;
  files?: FileMeta[];
};
export type QueueItem = {
  id: number;
  text: string;
  created: number;
  files: {
    id: number;
    name: string;
    kind: string;
    mime: string;
    size: number;
  }[];
};
export type TraceItem =
  | { kind: "prompt"; text: string; at: number }
  | { kind: "think"; text: string; at: number }
  | { kind: "text"; text: string; synthetic: boolean; at: number }
  | {
      kind: "tool";
      tool: string;
      status: string;
      title: string;
      input: string;
      output: string;
      error: string;
      at: number;
      end: number;
    }
  | { kind: "file"; text: string }
  | { kind: "error"; text: string; at: number };
export type Phase = "thinking" | "searching" | "writing" | "checking" | "done";
export type ThinkStep = { kind: "think"; text: string };
export type ToolStep = {
  kind: "tool";
  tool?: string;
  icon: string;
  title: string;
  title_en?: string;
  detail: string;
  detail_en?: string;
  link: string;
  status: "running" | "done" | "error";
};
const UI_TEXT_EN: Record<string, string> = {
  "گفت‌وگوی تازه": "New chat",
  "دستیار الان نتوانست پاسخ بدهد. چند لحظه بعد دوباره بپرسید.":
    "The assistant could not answer just now. Please ask again in a moment.",
  "این نوبت بیش از حد طول کشید و متوقف شد. بنویسید «ادامه بده» تا از همین‌جا ادامه دهد.":
    'This turn ran too long and was stopped. Say "continue" to pick up where it left off.',
  "دستیار در دسترس نیست. چند لحظه بعد دوباره تلاش کنید.":
    "The assistant is not available. Please try again in a moment.",
};

export function uiText(s: string): string {
  if (isFa) return s;
  return UI_TEXT_EN[s] ?? s;
}

export type TextStep = { kind: "text"; text: string };
export type ActivityStep = ThinkStep | ToolStep | TextStep;
export type CallState = "pending" | "running" | "done" | "error" | "denied";
export type MessageCall = {
  call_id: string;
  chat: string;
  mid: number;
  tool: string;
  kind: "read" | "write" | "exec";
  summary: string;
  detail: string;
  lang: string;
  outside: boolean;
  cwd: string;
  state: CallState;
  output: string;
};

export type PlanId = "free" | "plus" | "pro" | "unlimited";
export type Plan = {
  id: PlanId;
  name: string;
  note: string;
  price: number;
  days: number;
  api_hourly: number;
  chat_hourly: number;
  parallel: number;
};
export type Gateway = "zarinpal" | "zibal";
export type Subscription = {
  plan: Plan;
  status: string;
  source: string;
  started: number;
  expires: number;
  api_used: number;
  chat_used: number;
};
export type Payment = {
  id: number;
  user_id: number;
  plan: PlanId;
  plan_name: string;
  gateway: Gateway;
  amount: number;
  status: "pending" | "verifying" | "paid" | "failed" | "canceled";
  token: string;
  ref: string;
  card: string;
  message: string;
  created: number;
  paid: number;
  user?: { id: number; name: string; email: string };
};
export type AdminSubscription = {
  id: number;
  user_id: number;
  plan: PlanId;
  plan_name: string;
  status: string;
  source: string;
  started: number;
  expires: number;
  price: number;
  note: string;
  payment_id: number;
  user?: { id: number; name: string; email: string };
};

export type ClientKind = "cimd" | "dcr";
export type OAuthClient = {
  id?: string;
  name: string;
  uri?: string;
  logo?: string;
  host: string;
  kind: ClientKind;
};
export type Scope = { name: string; title: string; description: string };
export type ConsentRequest = {
  id: string;
  client: OAuthClient;
  redirect_host: string;
  scopes: Scope[];
  resource: string;
  expires: number;
};
export type ConnectedApp = {
  id: number;
  client: OAuthClient;
  scope: string;
  resource: string;
  created: number;
  last_used: number;
  calls: number;
};
export type PersonRef = { id: number; name: string; email: string };
export type AppEvent = {
  id: number;
  created: number;
  method: string;
  tool: string;
  ok: boolean;
  ms: number;
  args: string;
  ip: string;
  agent: string;
  user?: PersonRef;
  client: { name: string; host: string };
};
export type UsageTotals = {
  count: number;
  input: number;
  output: number;
  reasoning: number;
  cache_read: number;
  cache_write: number;
  cost: number;
  cost_in: number;
  cost_out: number;
};
export type UsagePeriodKey = "today" | "week" | "month" | "last_month" | "all";
export type UsagePeriod = {
  key: UsagePeriodKey;
  from: number;
  to: number;
  chat: UsageTotals;
  api: UsageTotals;
};
export type RequestUsage = {
  input: number;
  output: number;
  reasoning: number;
  cache_read: number;
  cache_write: number;
  cost: number;
  tools: number;
};
export type AdminSummary = {
  users: number;
  users_7d: number;
  chats: number;
  messages: number;
  uploads?: number;
  guests?: number;
  guest_messages?: number;
  guests_at_limit?: number;
  uploads_images?: number;
  uploads_bytes?: number;
  apps: number;
  mcp_calls: number;
  mcp_calls_24h: number;
  mcp_errors_24h: number;
  api_requests: number;
  api_requests_24h: number;
  api_errors_24h: number;
  top_tools: { tool: string; count: number }[];
  top_clients: { name: string; count: number }[];
  top_projects: { name: string; count: number }[];
  daily: {
    day: string;
    calls: number;
    api: number;
    users: number;
    tokens_in: number;
    tokens_out: number;
  }[];
  usage: UsagePeriod[];
};
export type AdminUser = {
  id: number;
  name: string;
  email: string;
  created: number;
  chats: number;
  messages: number;
  apps: number;
  mcp_calls: number;
  api_requests: number;
  tokens_in: number;
  tokens_out: number;
  last_login: number;
  logins: number;
  images?: number;
  files?: number;
  sessions?: number;
  agent_version?: string;
  last_active_chat: number;
  last_active_agent: number;
  last_active_api: number;
  cost_in?: number;
  cost_out?: number;
  policy_free?: boolean;
  policy_system?: string;
  telegram?: number;
};
export type AdminTelegramLink = {
  tg_id: string;
  name: string;
  username: string;
  lang: string;
  created: number;
  last_seen: number;
  user: PersonRef;
  chats: number;
  messages: number;
  answering: boolean;
  chat_id: string;
};
export type AdminTelegramGuest = {
  tg_id: string;
  name: string;
  username: string;
  lang: string;
  created: number;
  last_seen: number;
  guest_id: number;
  messages: number;
  answering: boolean;
};
export type AdminTelegram = {
  links: AdminTelegramLink[];
  guests: AdminTelegramGuest[];
  online_links: number;
  online_guests: number;
  now: number;
};
export type AdminApp = {
  id: number;
  user: PersonRef;
  client: OAuthClient;
  scope: string;
  created: number;
  last_used: number;
  calls: number;
  revoked: boolean;
};
export type EventFilters = {
  user?: number;
  tool?: string;
  client?: string;
  before?: number;
};

export type ApiKey = {
  id: number;
  name: string;
  prefix: string;
  created: number;
  last_used: number;
  requests: number;
  custom?: boolean;
};
export type AdminKey = ApiKey & {
  revoked: boolean;
  salam_prompt: boolean;
  salam_tools: boolean;
  system_prompt: string;
  policy_at: number;
  policy_by: number;
  user: PersonRef;
};
export type MemoryItem = {
  id: number;
  chat: string;
  slug: string;
  title: string;
  description: string;
  created: number;
  updated: number;
  body?: string;
  markdown?: string;
};
export type MemoryChat = {
  id: string;
  title: string;
  summary: string;
  updated: number;
  memories: MemoryItem[];
};
export type MemoryOverview = {
  enabled: boolean;
  service: boolean;
  index: string;
  chats: MemoryChat[];
};
export type KeyPolicy = {
  salam_prompt?: boolean;
  salam_tools?: boolean;
  system_prompt?: string;
};
export type RequestStatus = "pending" | "done" | "error" | "aborted" | "lost";
export type ApiRequest = {
  id: number;
  created: number;
  key: { id: number; name: string };
  project: string;
  end_user: string;
  model: string;
  stream: boolean;
  status: RequestStatus;
  ms: number;
  prompt: string;
  response: string;
  prompt_chars: number;
  completion_chars: number;
  ip: string;
  agent: string;
  user?: PersonRef;
  usage: RequestUsage;
};
export type RequestFilters = {
  user?: number;
  project?: string;
  key?: number;
  before?: number;
};

export const PAGE_SIZE = 50;

const BASE = (
  import.meta.env.PUBLIC_API_URL || "https://ai-api.salamlang.ir"
).replace(/\/$/, "");
const TOKEN_KEY = "salam-ai-token";

export class ApiError extends Error {
  status: number;
  code: string;
  constructor(status: number, code: string, message: string) {
    super(message);
    this.status = status;
    this.code = code;
  }
}

const STATE_KEY = "salam-oauth-state";

export type PendingOAuth = {
  provider: string;
  state: string;
  purpose: "login" | "connect";
  next: string;
};

export function rememberOAuth(p: PendingOAuth) {
  try {
    localStorage.setItem(STATE_KEY, JSON.stringify(p));
  } catch {}
}

export function takeOAuth(): PendingOAuth | null {
  try {
    const raw = localStorage.getItem(STATE_KEY);
    localStorage.removeItem(STATE_KEY);
    return raw ? (JSON.parse(raw) as PendingOAuth) : null;
  } catch {
    return null;
  }
}

export function getToken(): string {
  try {
    return localStorage.getItem(TOKEN_KEY) || "";
  } catch {
    return "";
  }
}

export function setToken(token: string) {
  try {
    if (token) localStorage.setItem(TOKEN_KEY, token);
    else localStorage.removeItem(TOKEN_KEY);
  } catch {}
}

function query(params: Record<string, string | number | undefined>): string {
  const q = new URLSearchParams();
  for (const [k, v] of Object.entries(params)) {
    if (v !== undefined && v !== "") q.set(k, String(v));
  }
  const text = q.toString();
  return text ? `?${text}` : "";
}

let onUnauthorized: () => void = () => {};
let clientVersion = "";
export function setClientVersion(v: string) {
  clientVersion = /^[0-9A-Za-z.+-]{1,32}$/.test(v) ? v : "";
}

let clientPlatform = { os: "", version: "", arch: "" };
export function setClientPlatform(os: string, version: string, arch: string) {
  const plain = (v: string, max: number) =>
    /^[0-9A-Za-z.+_() -]+$/.test(v) && v.length <= max ? v : "";
  clientPlatform = {
    os: ["windows", "macos", "linux", "other"].includes(os) ? os : "",
    version: plain(version, 64),
    arch: plain(arch, 16),
  };
}

export function handleUnauthorized(fn: () => void) {
  onUnauthorized = fn;
}

async function call<T>(
  method: string,
  path: string,
  body?: unknown,
): Promise<T> {
  const headers: Record<string, string> = {
    "X-Salam-Client": isDesktop ? "agent" : "web",
    "X-Salam-Lang": lang,
  };
  if (clientVersion) headers["X-Salam-Version"] = clientVersion;
  if (clientPlatform.os) headers["X-Salam-OS"] = clientPlatform.os;
  if (clientPlatform.version)
    headers["X-Salam-OS-Version"] = clientPlatform.version;
  if (clientPlatform.arch) headers["X-Salam-Arch"] = clientPlatform.arch;
  const token = getToken();
  if (token) headers.Authorization = `Bearer ${token}`;
  if (body !== undefined) headers["Content-Type"] = "application/json";
  let res: Response;
  try {
    res = await fetch(BASE + path, {
      method,
      headers,
      body: body === undefined ? undefined : JSON.stringify(body),
    });
  } catch {
    throw new ApiError(
      0,
      "network",
      t(
        "ارتباط با سرور برقرار نشد. اتصال اینترنت را بررسی کنید.",
        "Could not reach the server. Check your internet connection.",
      ),
    );
  }
  let data: { error?: string; message?: string } | null = null;
  try {
    data = await res.json();
  } catch {}
  if (!res.ok) {
    if (res.status === 401 && path !== "/auth/login") onUnauthorized();
    throw new ApiError(
      res.status,
      data?.error || "server",
      data?.message ||
        t(
          "خطایی پیش آمد. دوباره تلاش کنید.",
          "Something went wrong. Please try again.",
        ),
    );
  }
  return data as unknown as T;
}

export type StreamHandlers = {
  delta: (text: string) => void;
  reset: (text: string) => void;
  phase: (phase: Phase) => void;
  activity: (steps: ActivityStep[]) => void;
  done: (
    body: string,
    status: Message["status"],
    activity?: ActivityStep[],
    ms?: number,
  ) => void;
};

export async function streamMessage(
  chatId: string,
  mid: number,
  on: StreamHandlers,
  signal: AbortSignal,
): Promise<boolean> {
  const headers: Record<string, string> = {
    Accept: "text/event-stream",
    "X-Salam-Lang": lang,
  };
  const token = getToken();
  if (token) headers.Authorization = `Bearer ${token}`;
  const res = await fetch(`${BASE}/chats/${chatId}/messages/${mid}/stream`, {
    headers,
    signal,
    cache: "no-store",
  });
  if (res.status === 401) {
    onUnauthorized();
    return false;
  }
  if (!res.ok || !res.body) return false;
  const reader = res.body.pipeThrough(new TextDecoderStream()).getReader();
  let buf = "";
  for (;;) {
    const { value, done } = await reader.read();
    if (done) return false;
    buf += value.replace(/\r\n/g, "\n");
    let cut = buf.indexOf("\n\n");
    while (cut >= 0) {
      const block = buf.slice(0, cut);
      buf = buf.slice(cut + 2);
      cut = buf.indexOf("\n\n");
      let event = "message";
      const data: string[] = [];
      for (const line of block.split("\n")) {
        if (line.startsWith("event:")) event = line.slice(6).trim();
        else if (line.startsWith("data:")) data.push(line.slice(5).trimStart());
      }
      if (data.length === 0) continue;
      let payload: {
        t?: string;
        phase?: Phase;
        body?: string;
        status?: Message["status"];
        steps?: ActivityStep[];
        activity?: ActivityStep[];
        ms?: number;
      };
      try {
        payload = JSON.parse(data.join("\n"));
      } catch {
        continue;
      }
      if (event === "delta") on.delta(payload.t ?? "");
      else if (event === "reset") on.reset(payload.t ?? "");
      else if (event === "phase" && payload.phase) on.phase(payload.phase);
      else if (event === "activity" && Array.isArray(payload.steps))
        on.activity(payload.steps);
      else if (event === "done") {
        on.done(
          payload.body ?? "",
          payload.status ?? "done",
          Array.isArray(payload.activity) ? payload.activity : undefined,
          payload.ms,
        );
        reader.cancel().catch(() => {});
        return true;
      }
    }
  }
}

export const api = {
  register: (name: string, email: string, password: string) =>
    call<{ token: string; user: User }>("POST", "/auth/register", {
      name,
      email,
      password,
    }),
  login: (email: string, password: string) =>
    call<{ token: string; user: User }>("POST", "/auth/login", {
      email,
      password,
    }),
  logout: () => call<{ ok: boolean }>("POST", "/auth/logout"),
  me: () => call<{ user: User }>("GET", "/me"),
  chats: () => call<{ chats: Chat[] }>("GET", "/chats"),
  createChat: (
    options?: { device_id: string; workspace: string } | { cloud: true },
  ) => call<{ chat: Chat }>("POST", "/chats", options),
  authProviders: () =>
    call<{ github: boolean; figma: boolean }>("GET", "/auth/providers"),
  startProvider: (provider: string, next: string) =>
    call<{ url: string; state: string }>(
      "POST",
      `/auth/provider/${provider}/start`,
      { next },
    ),
  completeProvider: (
    provider: string,
    code: string,
    state: string,
    error: string,
  ) =>
    call<{
      purpose: "login" | "connect";
      token?: string;
      next?: string;
      ok?: boolean;
    }>("POST", `/auth/provider/${provider}/complete`, { code, state, error }),
  integrations: () => call<Integrations>("GET", "/integrations"),
  saveIntegration: (provider: string, token: string) =>
    call<Integration>("POST", `/integrations/${provider}/token`, { token }),
  removeIntegration: (provider: string) =>
    call<{ ok: boolean }>("DELETE", `/integrations/${provider}`),
  integrationOauth: (provider: string) =>
    call<{ url: string; state: string }>(
      "POST",
      `/integrations/${provider}/oauth`,
    ),
  devices: () => call<{ devices: Device[] }>("GET", "/bridge/devices"),
  chat: (id: string) =>
    call<{
      chat: Chat;
      messages: Message[];
      calls?: MessageCall[];
      queue?: QueueItem[];
    }>("GET", `/chats/${id}`),
  renameChat: (id: string, title: string) =>
    call<{ title: string }>("PATCH", `/chats/${id}`, { title }),
  deleteChat: (id: string) => call<{ ok: boolean }>("DELETE", `/chats/${id}`),
  send: (
    id: string,
    text: string,
    files?: OutgoingFile[],
    project?: ProjectContext,
  ) =>
    call<{ user: Message; assistant: Message; title: string }>(
      "POST",
      `/chats/${id}/messages`,
      {
        text,
        ...(files && files.length > 0 ? { files } : {}),
        ...(project ? { project } : {}),
      },
    ),
  capabilities: () => call<Capabilities>("GET", "/capabilities"),
  memory: () => call<MemoryOverview>("GET", "/memory"),
  setMemory: (enabled: boolean) =>
    call<{ enabled: boolean }>("PATCH", "/memory", { enabled }),
  clearMemory: () => call<{ ok: boolean }>("DELETE", "/memory"),
  memoryItem: (id: number) =>
    call<{ memory: MemoryItem }>("GET", `/memory/${id}`),
  forgetMemory: (id: number) =>
    call<{ ok: boolean }>("DELETE", `/memory/${id}`),
  chatMemory: (id: string) =>
    call<{
      title: string;
      summary: string;
      index: string;
      memories: MemoryItem[];
    }>("GET", `/chats/${id}/memory`),
  adminFiles: (q: { user?: number; kind?: string; offset?: number }) =>
    call<{ files: AdminUpload[]; total: number }>(
      "GET",
      `/admin/files?${new URLSearchParams({ ...(q.user ? { user: String(q.user) } : {}), ...(q.kind ? { kind: q.kind } : {}), offset: String(q.offset ?? 0) })}`,
    ),
  adminSessions: (q: { user?: number; client?: string; offset?: number }) =>
    call<{ sessions: AdminSession[]; total: number }>(
      "GET",
      `/admin/sessions?${new URLSearchParams({ ...(q.user ? { user: String(q.user) } : {}), ...(q.client ? { client: q.client } : {}), offset: String(q.offset ?? 0) })}`,
    ),
  adminPlatforms: (days: number) =>
    call<AdminPlatforms>("GET", `/admin/platforms?days=${days}`),
  adminGuests: (offset: number) =>
    call<{ guests: AdminGuest[]; total: number }>(
      "GET",
      `/admin/guests?offset=${offset}`,
    ),
  adminGuest: (id: number) =>
    call<{ messages: GuestMessage[] }>("GET", `/admin/guests/${id}`),
  revokeSession: (id: number) =>
    call<{ ok: boolean }>("DELETE", `/admin/sessions/${id}`),
  revokeUserSessions: (id: number) =>
    call<{ revoked: number }>("DELETE", `/admin/users/${id}/sessions`),
  adminFile: (id: number) =>
    call<{ name: string; kind: string; mime: string; data: string }>(
      "GET",
      `/admin/files/${id}`,
    ),
  chatFile: (id: string, fid: number) =>
    call<{ name: string; kind: string; mime: string; data: string }>(
      "GET",
      `/chats/${id}/files/${fid}`,
    ),
  adminChatFile: (id: string, fid: number) =>
    call<{ name: string; kind: string; mime: string; data: string }>(
      "GET",
      `/admin/chats/${id}/files/${fid}`,
    ),
  poll: (id: string, mid: number) =>
    call<{
      id: number;
      body: string;
      status: Message["status"];
      phase: Phase;
      activity?: ActivityStep[];
    }>("GET", `/chats/${id}/messages/${mid}`),
  stop: (id: string) => call<{ body: string }>("POST", `/chats/${id}/stop`),
  queue: (id: string) =>
    call<{ queue: QueueItem[] }>("GET", `/chats/${id}/queue`),
  enqueue: (
    id: string,
    text: string,
    files?: OutgoingFile[],
    project?: ProjectContext,
  ) =>
    call<{ queue: QueueItem[] }>("POST", `/chats/${id}/queue`, {
      text,
      ...(files && files.length > 0 ? { files } : {}),
      ...(project ? { project } : {}),
    }),
  unqueue: (id: string, qid: number) =>
    call<{ queue: QueueItem[] }>("DELETE", `/chats/${id}/queue/${qid}`),
  clearQueue: (id: string) =>
    call<{ queue: QueueItem[] }>("DELETE", `/chats/${id}/queue`),
  runQueued: (id: string, qid: number) =>
    call<{ queue?: QueueItem[]; user?: Message; assistant?: Message }>(
      "POST",
      `/chats/${id}/queue/${qid}/now`,
    ),
  consentRequest: (id: string) =>
    call<{ request: ConsentRequest }>(
      "GET",
      `/oauth/requests/${encodeURIComponent(id)}`,
    ),
  approveRequest: (id: string) =>
    call<{ redirect: string }>(
      "POST",
      `/oauth/requests/${encodeURIComponent(id)}/approve`,
    ),
  denyRequest: (id: string) =>
    call<{ redirect: string }>(
      "POST",
      `/oauth/requests/${encodeURIComponent(id)}/deny`,
    ),
  telegramInfo: (code: string) =>
    call<TelegramLinkInfo>("GET", `/telegram/link/${encodeURIComponent(code)}`),
  telegramLink: (code: string) =>
    call<TelegramLinkInfo & { ok: boolean }>("POST", "/telegram/link", {
      code,
    }),
  telegramStatus: () => call<TelegramStatus>("GET", "/telegram"),
  telegramCode: () =>
    call<{ url: string; bot: string }>("POST", "/telegram/code"),
  telegramUnlink: (id: string) =>
    call<{ ok: boolean }>("DELETE", `/telegram/${encodeURIComponent(id)}`),
  apps: () => call<{ apps: ConnectedApp[] }>("GET", "/apps"),
  revokeApp: (id: number) => call<{ ok: boolean }>("DELETE", `/apps/${id}`),
  appEvents: (id: number, before?: number) =>
    call<{ events: AppEvent[] }>(
      "GET",
      `/apps/${id}/events${query({ before })}`,
    ),
  adminSummary: () => call<AdminSummary>("GET", "/admin/summary"),
  adminUsers: (q: string, offset: number) =>
    call<{ users: AdminUser[]; total: number }>(
      "GET",
      `/admin/users${query({ q, offset: offset || undefined })}`,
    ),
  adminTelegram: () => call<AdminTelegram>("GET", "/admin/telegram"),
  adminUserPolicy: (
    id: number,
    body: { policy_free?: boolean; policy_system?: string },
  ) =>
    call<{ user: { id: number; policy_free: boolean; policy_system: string } }>(
      "PATCH",
      `/admin/users/${id}/policy`,
      body,
    ),
  adminEvents: (f: EventFilters) =>
    call<{ events: AppEvent[] }>(
      "GET",
      `/admin/events${query({
        user: f.user,
        tool: f.tool,
        client: f.client,
        before: f.before,
      })}`,
    ),
  adminApps: (user?: number, before?: number) =>
    call<{ apps: AdminApp[] }>("GET", `/admin/apps${query({ user, before })}`),
  adminRevokeApp: (id: number) =>
    call<{ ok: boolean }>("DELETE", `/admin/apps/${id}`),
  adminKeys: (q: { user?: number; before?: number; custom?: boolean }) =>
    call<{ keys: AdminKey[]; plain_ready: boolean }>(
      "GET",
      `/admin/keys${query({ user: q.user, before: q.before, custom: q.custom ? 1 : undefined })}`,
    ),
  adminUpdateKey: (id: number, policy: KeyPolicy) =>
    call<{ key: AdminKey }>("PATCH", `/admin/keys/${id}`, policy),
  adminUserChats: (id: number) =>
    call<{ chats: Chat[] }>("GET", `/admin/users/${id}/chats`),
  adminChat: (id: string) =>
    call<{
      chat: Chat;
      user?: AdminChatOwner;
      messages: Message[];
      calls?: MessageCall[];
    }>("GET", `/admin/chats/${id}`),
  adminTrace: (id: number) =>
    call<{ trace: TraceItem[]; saved: boolean }>(
      "GET",
      `/admin/messages/${id}/trace`,
    ),
  plans: () => call<{ plans: Plan[]; gateways: Gateway[] }>("GET", "/plans"),
  subscription: () =>
    call<{ subscription: Subscription }>("GET", "/billing/subscription"),
  checkout: (plan: PlanId, gateway: Gateway) =>
    call<{ payment_id: number; gateway: Gateway; amount: number; url: string }>(
      "POST",
      "/billing/checkout",
      { plan, gateway },
    ),
  payments: () => call<{ payments: Payment[] }>("GET", "/billing/payments"),
  adminSubscriptions: (latest?: boolean, user?: number) =>
    call<{
      subscriptions: AdminSubscription[];
      active: number;
      income: number;
    }>(
      "GET",
      `/admin/subscriptions${query({ latest: latest ? 1 : undefined, user })}`,
    ),
  adminGrant: (body: {
    email?: string;
    user_id?: number;
    plan: PlanId;
    days?: number;
    note?: string;
  }) =>
    call<{ subscription_id: number; plan: PlanId }>(
      "POST",
      "/admin/subscriptions",
      body,
    ),
  adminCancelSubscription: (id: number) =>
    call<{ ok: boolean }>("DELETE", `/admin/subscriptions/${id}`),
  adminPayments: (f: { status?: string; user?: number; before?: number }) =>
    call<{ payments: Payment[]; income: number }>(
      "GET",
      `/admin/payments${query({ status: f.status, user: f.user, before: f.before })}`,
    ),
  keys: () => call<{ keys: ApiKey[] }>("GET", "/keys"),
  createKey: (name: string) =>
    call<{ key: ApiKey; secret: string }>("POST", "/keys", { name }),
  revokeKey: (id: number) => call<{ ok: boolean }>("DELETE", `/keys/${id}`),
  myRequests: (before?: number) =>
    call<{ requests: ApiRequest[] }>(
      "GET",
      `/keys/requests${query({ before })}`,
    ),
  adminRequests: (f: RequestFilters) =>
    call<{ requests: ApiRequest[] }>(
      "GET",
      `/admin/requests${query({
        user: f.user,
        project: f.project,
        key: f.key,
        before: f.before,
      })}`,
    ),
};
