import { useEffect, useMemo, useState } from "react";
import { api, ApiError, type AdminChatOwner, type Chat, type Message } from "./api";
import { NoAccess } from "./Admin";
import { fileLoader } from "./Attach";
import { fmtDateTime, fmtNum } from "./format";
import { MessageView } from "./Messages";
import { Trace } from "./Trace";

type Loaded = { chat: Chat; user?: AdminChatOwner; messages: Message[] };

export function AdminChat({ id }: { id: string }) {
  const [data, setData] = useState<Loaded | null>(null);
  const [error, setError] = useState("");
  const [forbidden, setForbidden] = useState(false);
  const [traced, setTraced] = useState<Set<number>>(new Set());
  const loadFile = useMemo(() => fileLoader((fid) => api.adminChatFile(id, fid)), [id]);

  useEffect(() => {
    let live = true;
    setData(null);
    setError("");
    api
      .adminChat(id)
      .then((r) => {
        if (!live) return;
        const byMid = new Map<number, NonNullable<Message["calls"]>>();
        for (const c of r.calls ?? []) {
          const list = byMid.get(c.mid) ?? [];
          list.push(c);
          byMid.set(c.mid, list);
        }
        const messages = r.messages.map((m) => (m.role === "assistant" && byMid.has(m.id) ? { ...m, calls: byMid.get(m.id) } : m));
        setData({ chat: r.chat, user: r.user, messages });
        document.title = `${r.chat.title} | مدیریت دستیار سلام`;
      })
      .catch((e) => {
        if (!live) return;
        if (e instanceof ApiError && e.status === 403) setForbidden(true);
        else setError(e instanceof Error && e.message ? e.message : "بارگذاری گفت‌وگو ناموفق بود.");
      });
    return () => {
      live = false;
    };
  }, [id]);

  if (forbidden) return <NoAccess />;
  if (error)
    return (
      <div className="thread">
        <div className="form-error" role="alert">
          {error}
        </div>
      </div>
    );
  if (!data)
    return (
      <div className="loading-chat" role="status">
        <span className="spinner" />
        <span className="sr-only">در حال بارگذاری</span>
      </div>
    );

  const answers = data.messages.filter((m) => m.role === "assistant");
  const allOpen = answers.length > 0 && answers.every((m) => traced.has(m.id));
  const toggle = (mid: number) =>
    setTraced((cur) => {
      const next = new Set(cur);
      if (next.has(mid)) next.delete(mid);
      else next.add(mid);
      return next;
    });

  return (
    <div className="thread admin-thread">
      <header className="admin-chat-head card">
        <a className="link small-link" href="/admin">
          بازگشت به مدیریت
        </a>
        <h2 dir="auto">{data.chat.title}</h2>
        <dl className="admin-chat-meta">
          {data.user && (
            <div>
              <dt>کاربر</dt>
              <dd>
                <bdi>{data.user.name}</bdi> <span dir="ltr" className="muted">{data.user.email}</span>
              </dd>
            </div>
          )}
          <div>
            <dt>نوع</dt>
            <dd>{data.chat.agent ? "ایجنت دسکتاپ" : "وب"}</dd>
          </div>
          {data.chat.workspace && (
            <div>
              <dt>پوشه‌ی کاری</dt>
              <dd dir="ltr" className="cell-ltr">
                {data.chat.workspace}
              </dd>
            </div>
          )}
          <div>
            <dt>پیام‌ها</dt>
            <dd>{fmtNum(data.messages.length)}</dd>
          </div>
          <div>
            <dt>آخرین بروزرسانی</dt>
            <dd>{fmtDateTime(data.chat.updated)}</dd>
          </div>
        </dl>
        {answers.length > 0 && (
          <button type="button" className="button ghost small" onClick={() => setTraced(allOpen ? new Set() : new Set(answers.map((m) => m.id)))}>
            {allOpen ? "بستن همه‌ی جزئیات" : "باز کردن جزئیات همه‌ی پاسخ‌ها"}
          </button>
        )}
      </header>
      {data.messages.map((m) => (
        <div key={m.id} className="admin-turn">
          <MessageView m={m} name={data.user?.name ?? ""} calls={m.calls} loadFile={loadFile} />
          {m.role === "assistant" && m.status !== "done" && (
            <p className={`admin-turn-status ${m.status}`}>{m.status === "pending" ? "این پاسخ هنوز تمام نشده است." : "این پاسخ با خطا تمام شد."}</p>
          )}
          {m.role === "assistant" && (
            <div className="admin-turn-trace">
              <button type="button" className="link small-link" aria-expanded={traced.has(m.id)} onClick={() => toggle(m.id)}>
                {traced.has(m.id) ? "بستن جزئیات کامل" : "همه‌ی جزئیات: فکرها، کدها، فرمان‌ها و خروجی‌ها"}
              </button>
              {traced.has(m.id) && <Trace messageId={m.id} />}
            </div>
          )}
        </div>
      ))}
      {data.messages.length === 0 && <p className="muted">پیامی در این گفت‌وگو نیست.</p>}
    </div>
  );
}
