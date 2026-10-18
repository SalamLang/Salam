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
  device_id?: number;
  workspace?: string;
};
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
  ms?: number;
};
export type Phase = "thinking" | "searching" | "writing" | "checking" | "done";
export type ThinkStep = { kind: "think"; text: string };
export type ToolStep = {
  kind: "tool";
  icon: string;
  title: string;
  detail: string;
  link: string;
  status: "running" | "done" | "error";
};
export type ActivityStep = ThinkStep | ToolStep;

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
  tokens: number;
  last_login: number;
  logins: number;
  last_active: number;
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
export function handleUnauthorized(fn: () => void) {
  onUnauthorized = fn;
}

async function call<T>(
  method: string,
  path: string,
  body?: unknown,
): Promise<T> {
  const headers: Record<string, string> = {};
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
      "ارتباط با سرور برقرار نشد. اتصال اینترنت را بررسی کنید.",
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
      data?.message || "خطایی پیش آمد. دوباره تلاش کنید.",
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
  const headers: Record<string, string> = { Accept: "text/event-stream" };
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
  createChat: (agent?: { device_id: string; workspace: string }) =>
    call<{ chat: Chat }>("POST", "/chats", agent),
  devices: () => call<{ devices: Device[] }>("GET", "/bridge/devices"),
  chat: (id: string) =>
    call<{ chat: Chat; messages: Message[] }>("GET", `/chats/${id}`),
  renameChat: (id: string, title: string) =>
    call<{ title: string }>("PATCH", `/chats/${id}`, { title }),
  deleteChat: (id: string) => call<{ ok: boolean }>("DELETE", `/chats/${id}`),
  send: (id: string, text: string) =>
    call<{ user: Message; assistant: Message; title: string }>(
      "POST",
      `/chats/${id}/messages`,
      { text },
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
