import { useEffect, useMemo, useState } from "react";
import { api, ApiError, uiText, type AdminChatOwner, type Chat, type Message } from "./api";
import { NoAccess } from "./Admin";
import { fileLoader } from "./Attach";
import { fmtDateTime, fmtNum } from "./format";
import { MessageView } from "./Messages";
import { Trace } from "./Trace";
import { t } from "./i18n";

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
        document.title = t(`${r.chat.title} | مدیریت دستیار سلام`, `${uiText(r.chat.title)} | Salam AI admin`);
      })
      .catch((e) => {
        if (!live) return;
        if (e instanceof ApiError && e.status === 403) setForbidden(true);
        else setError(e instanceof Error && e.message ? e.message : t("بارگذاری گفت‌وگو ناموفق بود.", "Could not load the chat."));
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
        <span className="sr-only">{t("در حال بارگذاری", "Loading")}</span>
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
          {t("بازگشت به مدیریت", "Back to admin")}
        </a>
        <h2 dir="auto">{data.chat.title}</h2>
        <dl className="admin-chat-meta">
          {data.user && (
            <div>
              <dt>{t("کاربر", "User")}</dt>
              <dd>
                <bdi>{data.user.name}</bdi> <span dir="ltr" className="muted">{data.user.email}</span>
              </dd>
            </div>
          )}
          <div>
            <dt>{t("نوع", "Type")}</dt>
            <dd>{data.chat.agent ? t("ایجنت دسکتاپ", "Desktop agent") : t("وب", "Web")}</dd>
          </div>
          {data.chat.workspace && (
            <div>
              <dt>{t("پوشه‌ی کاری", "Project folder")}</dt>
              <dd dir="ltr" className="cell-ltr">
                {data.chat.workspace}
              </dd>
            </div>
          )}
          <div>
            <dt>{t("پیام‌ها", "Messages")}</dt>
            <dd>{fmtNum(data.messages.length)}</dd>
          </div>
          <div>
            <dt>{t("آخرین بروزرسانی", "Last updated")}</dt>
            <dd>{fmtDateTime(data.chat.updated)}</dd>
          </div>
        </dl>
        {answers.length > 0 && (
          <button type="button" className="button ghost small" onClick={() => setTraced(allOpen ? new Set() : new Set(answers.map((m) => m.id)))}>
            {allOpen ? t("بستن همه‌ی جزئیات", "Close all details") : t("باز کردن جزئیات همه‌ی پاسخ‌ها", "Open details for all answers")}
          </button>
        )}
      </header>
      {data.messages.map((m) => (
        <div key={m.id} className="admin-turn">
          <MessageView m={m} name={data.user?.name ?? ""} calls={m.calls} loadFile={loadFile} />
          {m.role === "assistant" && m.status !== "done" && (
            <p className={`admin-turn-status ${m.status}`}>{m.status === "pending" ? t("این پاسخ هنوز تمام نشده است.", "This answer is not finished yet.") : t("این پاسخ با خطا تمام شد.", "This answer ended with an error.")}</p>
          )}
          {m.role === "assistant" && (
            <div className="admin-turn-trace">
              <button type="button" className="link small-link" aria-expanded={traced.has(m.id)} onClick={() => toggle(m.id)}>
                {traced.has(m.id) ? t("بستن جزئیات کامل", "Close full details") : t("همه‌ی جزئیات: فکرها، کدها، فرمان‌ها و خروجی‌ها", "All details: thoughts, code, commands and output")}
              </button>
              {traced.has(m.id) && <Trace messageId={m.id} />}
            </div>
          )}
        </div>
      ))}
      {data.messages.length === 0 && <p className="muted">{t("پیامی در این گفت‌وگو نیست.", "There are no messages in this chat.")}</p>}
    </div>
  );
}
