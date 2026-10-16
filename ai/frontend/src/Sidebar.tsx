import { useState } from "react";
import type { Chat, User } from "./api";
import { isDesktop } from "./desktop";
import { IconChat, IconClose, IconEdit, IconKey, IconLogout, IconMonitor, IconMoon, IconPlug, IconPlus, IconShield, IconSun, IconTrash } from "./icons";
import type { Theme } from "./theme";
import { confirmDialog } from "./Confirm";

function groupOf(updated: number): string {
  const now = new Date();
  const start = new Date(now.getFullYear(), now.getMonth(), now.getDate()).getTime() / 1000;
  if (updated >= start) return "امروز";
  if (updated >= start - 86400) return "دیروز";
  if (updated >= start - 7 * 86400) return "۷ روز گذشته";
  if (updated >= start - 30 * 86400) return "۳۰ روز گذشته";
  return "قدیمی‌تر";
}

type Props = {
  user: User;
  chats: Chat[];
  activeId: string | null;
  open: boolean;
  theme: Theme;
  onClose: () => void;
  onNew: () => void;
  onSelect: (id: string) => void;
  onRename: (id: string, title: string) => void;
  onDelete: (id: string) => void;
  onLogout: () => void;
  onToggleTheme: () => void;
  page: string;
  onNavigate: (path: string) => void;
};

export function Sidebar(p: Props) {
  const [editing, setEditing] = useState<string | null>(null);
  const [draft, setDraft] = useState("");
  const groups: { name: string; items: Chat[] }[] = [];
  for (const c of p.chats) {
    const g = groupOf(c.updated);
    const last = groups[groups.length - 1];
    if (last && last.name === g) last.items.push(c);
    else groups.push({ name: g, items: [c] });
  }
  const commit = (id: string) => {
    const t = draft.trim();
    if (t) p.onRename(id, t);
    setEditing(null);
  };
  return (
    <>
      <div className={`scrim ${p.open ? "show" : ""}`} onClick={p.onClose} aria-hidden="true" />
      <aside className={`sidebar ${p.open ? "open" : ""}`} aria-label="فهرست گفت‌وگوها">
        <div className="sidebar-head">
          <a className="brand" href="/" onClick={(e) => (e.preventDefault(), p.onNew())}>
            <img src="/logo.svg" alt="" width={30} height={30} />
            <span>
              دستیار سلام
              <small>زبان برنامه‌نویسی فارسی</small>
            </span>
          </a>
          <button type="button" className="icon-button only-mobile" onClick={p.onClose} aria-label="بستن فهرست">
            <IconClose />
          </button>
        </div>
        <button type="button" className="button primary new-chat" onClick={p.onNew}>
          <IconPlus width={18} height={18} />
          گفت‌وگوی تازه
        </button>
        <nav className="chat-list">
          {p.chats.length === 0 && <p className="empty-list">هنوز گفت‌وگویی ندارید. اولین پرسش خود را بنویسید.</p>}
          {groups.map((g) => (
            <div key={g.name} className="chat-group">
              <h3>{g.name}</h3>
              {g.items.map((c) => (
                <div key={c.id} className={`chat-item ${c.id === p.activeId ? "active" : ""}`}>
                  {editing === c.id ? (
                    <input
                      className="rename"
                      value={draft}
                      autoFocus
                      maxLength={80}
                      onChange={(e) => setDraft(e.target.value)}
                      onBlur={() => commit(c.id)}
                      onKeyDown={(e) => {
                        if (e.key === "Enter") commit(c.id);
                        if (e.key === "Escape") setEditing(null);
                      }}
                    />
                  ) : (
                    <button type="button" className="chat-link" onClick={() => p.onSelect(c.id)} title={c.title}>
                      <IconChat width={16} height={16} />
                      <span dir="auto">{c.title}</span>
                    </button>
                  )}
                  <div className="chat-actions">
                    <button
                      type="button"
                      className="icon-button tiny"
                      aria-label="تغییر نام"
                      onClick={() => {
                        setDraft(c.title);
                        setEditing(c.id);
                      }}
                    >
                      <IconEdit width={15} height={15} />
                    </button>
                    <button
                      type="button"
                      className="icon-button tiny danger"
                      aria-label="حذف گفت‌وگو"
                      onClick={async () => {
                        const ok = await confirmDialog({
                          title: "حذف گفت‌وگو",
                          subject: c.title,
                          message: "این گفت‌وگو و همه‌ی پیام‌هایش برای همیشه حذف می‌شود و برگرداندنش ممکن نیست.",
                          confirmLabel: "حذف",
                        });
                        if (ok) p.onDelete(c.id);
                      }}
                    >
                      <IconTrash width={15} height={15} />
                    </button>
                  </div>
                </div>
              ))}
            </div>
          ))}
        </nav>
        <nav className="sidebar-links" aria-label="بخش‌های حساب">
          <a
            href="/apps"
            className={`chat-link ${p.page === "apps" ? "active" : ""}`}
            aria-current={p.page === "apps" ? "page" : undefined}
            onClick={(e) => (e.preventDefault(), p.onNavigate("/apps"))}
          >
            <IconPlug width={16} height={16} />
            <span>برنامه‌های متصل</span>
          </a>
          <a
            href="/keys"
            className={`chat-link ${p.page === "keys" ? "active" : ""}`}
            aria-current={p.page === "keys" ? "page" : undefined}
            onClick={(e) => (e.preventDefault(), p.onNavigate("/keys"))}
          >
            <IconKey width={16} height={16} />
            <span>کلیدهای API</span>
          </a>
          {!isDesktop && (
            <a href="https://www.salamlang.ir/agent/" className="chat-link" target="_blank" rel="noreferrer">
              <IconMonitor width={16} height={16} />
              <span>دریافت ایجنت سلام</span>
            </a>
          )}
          {p.user.admin && (
            <a
              href="/admin"
              className={`chat-link ${p.page === "admin" ? "active" : ""}`}
              aria-current={p.page === "admin" ? "page" : undefined}
              onClick={(e) => (e.preventDefault(), p.onNavigate("/admin"))}
            >
              <IconShield width={16} height={16} />
              <span>مدیریت</span>
            </a>
          )}
        </nav>
        <div className="sidebar-foot">
          <div className="avatar" aria-hidden="true">
            {p.user.name.trim().charAt(0)}
          </div>
          <div className="who">
            <strong dir="auto">{p.user.name}</strong>
            <span dir="ltr">{p.user.email}</span>
          </div>
          <button type="button" className="icon-button" onClick={p.onToggleTheme} aria-label="تغییر پوسته" title={p.theme === "dark" ? "پوسته‌ی روشن" : "پوسته‌ی تیره"}>
            {p.theme === "dark" ? <IconSun /> : <IconMoon />}
          </button>
          <button type="button" className="icon-button" onClick={p.onLogout} aria-label="خروج" title="خروج">
            <IconLogout />
          </button>
        </div>
      </aside>
    </>
  );
}
