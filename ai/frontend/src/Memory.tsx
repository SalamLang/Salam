import { useCallback, useEffect, useState } from "react";
import { api, ApiError, type MemoryChat, type MemoryItem, type MemoryOverview } from "./api";
import { fmtDate, fmtDateTime } from "./format";
import { IconChevronDown, IconMemory, IconTrash } from "./icons";
import { Markdown } from "./Markdown";
import { confirmDialog, Subject } from "./Confirm";
import { t } from "./i18n";
import { Checkbox } from "./Checkbox";

function errText(e: unknown, fallback: string): string {
  return e instanceof ApiError ? e.message : fallback;
}

function download(name: string, text: string) {
  const url = URL.createObjectURL(new Blob([text], { type: "text/markdown;charset=utf-8" }));
  const a = document.createElement("a");
  a.href = url;
  a.download = name;
  document.body.appendChild(a);
  a.click();
  a.remove();
  setTimeout(() => URL.revokeObjectURL(url), 1000);
}

function MemoryRow({ item, onForget, notify }: { item: MemoryItem; onForget: (id: number) => void; notify: (msg: string) => void }) {
  const [open, setOpen] = useState(false);
  const [full, setFull] = useState<MemoryItem | null>(null);
  const [busy, setBusy] = useState(false);

  const toggle = async () => {
    const next = !open;
    setOpen(next);
    if (next && !full) {
      try {
        setFull((await api.memoryItem(item.id)).memory);
      } catch (e) {
        notify(errText(e, t("بارگذاری این خاطره ناموفق بود.", "Could not load this memory.")));
      }
    }
  };

  const forget = async () => {
    const ok = await confirmDialog({
      title: t("فراموش کردن", "Forget"),
      subject: item.title,
      message: (
        <>
          <Subject>{item.title}</Subject> {t("از حافظه پاک می‌شود و در گفت‌وگوهای بعدی به کار نمی‌رود.", "is removed from memory and will not be used in later chats.")}
        </>
      ),
      confirmLabel: t("فراموش کن", "Forget it"),
      icon: "alert",
    });
    if (!ok) return;
    setBusy(true);
    try {
      await api.forgetMemory(item.id);
      onForget(item.id);
    } catch (e) {
      notify(errText(e, t("پاک کردن ناموفق بود.", "Could not forget it.")));
      setBusy(false);
    }
  };

  return (
    <li className={`memory-item ${open ? "open" : ""}`}>
      <div className="memory-row">
        <button type="button" className="memory-toggle" aria-expanded={open} onClick={toggle}>
          <IconChevronDown width={16} height={16} />
          <span>
            <bdi className="strong">{item.title}</bdi>
            {item.description && <span className="muted" dir="auto">{item.description}</span>}
          </span>
        </button>
        <span className="memory-date muted" title={fmtDateTime(item.updated)}>
          {fmtDate(item.updated)}
        </span>
        <button type="button" className="icon-button" disabled={busy} onClick={forget} aria-label={t(`فراموش کردن ${item.title}`, `Forget ${item.title}`)}>
          <IconTrash width={16} height={16} />
        </button>
      </div>
      {open && (
        <div className="memory-body" dir="auto">
          {full ? (
            <>
              <Markdown text={full.body ?? ""} />
              <button type="button" className="button ghost small" onClick={() => download(`${full.slug}.md`, full.markdown ?? "")}>
                {t("دریافت فایل ‎.md", "Download .md")}
              </button>
            </>
          ) : (
            <span className="spinner" role="status" />
          )}
        </div>
      )}
    </li>
  );
}

function ChatCard({ chat, onForget, notify }: { chat: MemoryChat; onForget: (id: number) => void; notify: (msg: string) => void }) {
  const getIndex = async () => {
    try {
      const r = await api.chatMemory(chat.id);
      download(`index.md`, r.index);
    } catch (e) {
      notify(errText(e, t("دریافت فایل ناموفق بود.", "Could not download the file.")));
    }
  };

  return (
    <section className="card panel memory-chat">
      <div className="panel-head">
        <div className="panel-title">
          <h3>
            <a className="link" href={`/c/${chat.id}`}>
              <bdi>{chat.title}</bdi>
            </a>
          </h3>
          <span className="muted" title={fmtDateTime(chat.updated)}>
            {fmtDate(chat.updated)}
          </span>
        </div>
        <button type="button" className="button ghost small" onClick={getIndex}>
          index.md
        </button>
      </div>
      {chat.summary && <p className="memory-summary" dir="auto">{chat.summary}</p>}
      {chat.memories.length > 0 ? (
        <ul className="memory-list">
          {chat.memories.map((m) => (
            <MemoryRow key={m.id} item={m} onForget={onForget} notify={notify} />
          ))}
        </ul>
      ) : (
        <p className="muted">{t("از این گفت‌وگو فقط خلاصه مانده است.", "Only a summary is kept from this chat.")}</p>
      )}
    </section>
  );
}

export function Memory({ notify }: { notify: (msg: string) => void }) {
  const [data, setData] = useState<MemoryOverview | null>(null);
  const [error, setError] = useState("");
  const [saving, setSaving] = useState(false);

  const load = useCallback(async () => {
    setError("");
    try {
      setData(await api.memory());
    } catch (e) {
      setError(errText(e, t("بارگذاری حافظه ناموفق بود.", "Could not load memory.")));
    }
  }, []);

  useEffect(() => {
    void load();
  }, [load]);

  const toggle = async (enabled: boolean) => {
    setSaving(true);
    try {
      await api.setMemory(enabled);
      setData((d) => (d ? { ...d, enabled } : d));
      notify(enabled ? t("حافظه روشن شد.", "Memory is on.") : t("حافظه خاموش شد.", "Memory is off."));
    } catch (e) {
      notify(errText(e, t("ذخیره ناموفق بود.", "Could not save.")));
    } finally {
      setSaving(false);
    }
  };

  const clear = async () => {
    const ok = await confirmDialog({
      title: t("پاک کردن همه‌ی حافظه", "Clear all memory"),
      message: t("همه‌ی خلاصه‌ها و خاطره‌های گفت‌وگوهایتان پاک می‌شود. خود گفت‌وگوها می‌مانند.", "Every summary and memory of your chats is deleted. The chats themselves stay."),
      confirmLabel: t("پاک کردن همه", "Clear all"),
      icon: "alert",
    });
    if (!ok) return;
    try {
      await api.clearMemory();
      notify(t("حافظه پاک شد.", "Memory cleared."));
      await load();
    } catch (e) {
      notify(errText(e, t("پاک کردن ناموفق بود.", "Could not clear memory.")));
    }
  };

  const forget = (id: number) => setData((d) => (d ? { ...d, chats: d.chats.map((c) => ({ ...c, memories: c.memories.filter((m) => m.id !== id) })) } : d));

  return (
    <div className="page">
      <header className="page-head">
        <h2>{t("حافظه", "Memory")}</h2>
        <p className="muted">{t("دستیار پس از هر گفت‌وگو خلاصه‌ای از آن و نکته‌های ماندنی‌اش را نگه می‌دارد و در گفت‌وگوهای تازه از آن‌ها کمک می‌گیرد. هر خاطره را می‌توانید ببینید، دریافت کنید یا پاک کنید.", "After each chat the assistant keeps a summary of it and the things worth remembering, and uses them in new chats. You can read, download or delete each memory.")}</p>
      </header>
      {error && (
        <div className="form-error" role="alert">
          {error}{" "}
          <button type="button" className="link" onClick={load}>
            {t("تلاش دوباره", "Try again")}
          </button>
        </div>
      )}
      {!data && !error && (
        <div className="loading-chat" role="status">
          <span className="spinner" />
          <span className="sr-only">{t("در حال بارگذاری", "Loading")}</span>
        </div>
      )}
      {data && (
        <>
          <section className="card panel memory-controls">
            <Checkbox
              checked={data.enabled}
              disabled={saving}
              onChange={toggle}
              label={t("گفت‌وگوهایم را به خاطر بسپار", "Remember my chats")}
              hint={t("خاموش که باشد، چیز تازه‌ای نگه داشته نمی‌شود و حافظه در گفت‌وگوها به کار نمی‌رود.", "When off, nothing new is kept and memory is not used in chats.")}
            />
            {!data.service && <p className="policy-warn">{t("حافظه فعلاً روی سرور خاموش است.", "Memory is turned off on the server for now.")}</p>}
            <div className="memory-actions">
              <button type="button" className="button ghost small" onClick={() => download("index.md", data.index)}>
                {t("دریافت index.md همه‌ی گفت‌وگوها", "Download index.md for all chats")}
              </button>
              <button type="button" className="button danger small" onClick={clear} disabled={data.chats.length === 0}>
                {t("پاک کردن همه", "Clear all")}
              </button>
            </div>
          </section>
          {data.chats.length === 0 ? (
            <section className="card empty-card">
              <div className="notice-icon info">
                <IconMemory />
              </div>
              <h3>{t("هنوز چیزی به خاطر سپرده نشده", "Nothing remembered yet")}</h3>
              <p className="muted">{t("کمی پس از پایان هر گفت‌وگو، خلاصه و خاطره‌هایش اینجا می‌آید.", "A little after each chat ends, its summary and memories show up here.")}</p>
            </section>
          ) : (
            data.chats.map((c) => <ChatCard key={c.id} chat={c} onForget={forget} notify={notify} />)
          )}
        </>
      )}
    </div>
  );
}
