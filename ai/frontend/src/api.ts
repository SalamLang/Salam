export type User = { id: number; name: string; email: string };
export type Chat = { id: string; title: string; updated: number };
export type Message = {
  id: number;
  role: "user" | "assistant";
  body: string;
  status: "done" | "pending" | "error";
  created: number;
  phase?: Phase;
};
export type Phase = "thinking" | "searching" | "writing" | "done";

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
  createChat: () => call<{ chat: Chat }>("POST", "/chats"),
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
    call<{ id: number; body: string; status: Message["status"]; phase: Phase }>(
      "GET",
      `/chats/${id}/messages/${mid}`,
    ),
  stop: (id: string) => call<{ body: string }>("POST", `/chats/${id}/stop`),
};
