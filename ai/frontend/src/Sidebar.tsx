import { useState } from "react";
import type { Chat, User } from "./api";
import { isDesktop } from "./desktop";
import { IconChat, IconClose, IconEdit, IconGlobe, IconKey, IconLogout, IconMemory, IconMonitor, IconMoon, IconPlug, IconPlus, IconShield, IconSpark, IconSun, IconTrash } from "./icons";
import type { Theme } from "./theme";
import { confirmDialog } from "./Confirm";
import { isFa, setLang, t } from "./i18n";

function groupOf(updated: number): string {
  const now = new Date();
  const start = new Date(now.getFullYear(), now.getMonth(), now.getDate()).getTime() / 1000;
  if (updated >= start) return t("امروز", "Today");
  if (updated >= start - 86400) return t("دیروز", "Yesterday");
  if (updated >= start - 7 * 86400) return t("۷ روز گذشته", "Previous 7 days");
  if (updated >= start - 30 * 86400) return t("۳۰ روز گذشته", "Previous 30 days");
  return t("قدیمی‌تر", "Older");
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
    const title = draft.trim();
    if (title) p.onRename(id, title);
    setEditing(null);
  };
  return (
    <>
      <div className={`scrim ${p.open ? "show" : ""}`} onClick={p.onClose} aria-hidden="true" />
      <aside className={`sidebar ${p.open ? "open" : ""}`} aria-label={t("فهرست گفت‌وگوها", "Chat list")}>
        <div className="sidebar-head">
          <a className="brand" href="/" onClick={(e) => (e.preventDefault(), p.onNew())}>
            <img src="/logo.svg" alt="" width={30} height={30} />
            <span>
              {t("دستیار سلام", "Salam AI")}
              <small>{t("زبان برنامه‌نویسی ایرانی", "The Iranian programming language")}</small>
            </span>
          </a>
          <button type="button" className="icon-button only-mobile" onClick={p.onClose} aria-label={t("بستن فهرست", "Close menu")}>
            <IconClose />
          </button>
        </div>
        <button type="button" className="button primary new-chat" onClick={p.onNew}>
          <IconPlus width={18} height={18} />
          {t("گفت‌وگوی تازه", "New chat")}
        </button>
        <nav className="chat-list">
          {p.chats.length === 0 && <p className="empty-list">{t("هنوز گفت‌وگویی ندارید. اولین پرسش خود را بنویسید.", "No chats yet. Ask your first question.")}</p>}
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
                      aria-label={t("تغییر نام", "Rename")}
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
                      aria-label={t("حذف گفت‌وگو", "Delete chat")}
                      onClick={async () => {
                        const ok = await confirmDialog({
                          title: t("حذف گفت‌وگو", "Delete chat"),
                          subject: c.title,
                          message: t("این گفت‌وگو و همه‌ی پیام‌هایش برای همیشه حذف می‌شود و برگرداندنش ممکن نیست.", "This chat and all its messages will be deleted for good. This cannot be undone."),
                          confirmLabel: t("حذف", "Delete"),
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
        <nav className="sidebar-links" aria-label={t("بخش‌های حساب", "Account sections")}>
          <a
            href="/connections"
            className={`chat-link ${p.page === "connections" ? "active" : ""}`}
            aria-current={p.page === "connections" ? "page" : undefined}
            onClick={(e) => (e.preventDefault(), p.onNavigate("/connections"))}
          >
            <IconPlug width={16} height={16} />
            <span>{t("اتصال‌ها و برنامه‌ها", "Connections and apps")}</span>
          </a>
          <a
            href="/plans"
            className={`chat-link ${p.page === "plans" ? "active" : ""}`}
            aria-current={p.page === "plans" ? "page" : undefined}
            onClick={(e) => (e.preventDefault(), p.onNavigate("/plans"))}
          >
            <IconSpark width={16} height={16} />
            <span>{t("اشتراک‌ها", "Plans")}</span>
          </a>
          <a
            href="/keys"
            className={`chat-link ${p.page === "keys" ? "active" : ""}`}
            aria-current={p.page === "keys" ? "page" : undefined}
            onClick={(e) => (e.preventDefault(), p.onNavigate("/keys"))}
          >
            <IconKey width={16} height={16} />
            <span>{t("کلیدهای API", "API keys")}</span>
          </a>
          <a
            href="/memory"
            className={`chat-link ${p.page === "memory" ? "active" : ""}`}
            aria-current={p.page === "memory" ? "page" : undefined}
            onClick={(e) => (e.preventDefault(), p.onNavigate("/memory"))}
          >
            <IconMemory width={16} height={16} />
            <span>{t("حافظه", "Memory")}</span>
          </a>
          {!isDesktop && (
            <a href="https://www.salamlang.ir/agent/" className="chat-link" target="_blank" rel="noreferrer">
              <IconMonitor width={16} height={16} />
              <span>{t("دریافت ایجنت سلام", "Get Salam Agent")}</span>
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
              <span>{t("مدیریت", "Admin")}</span>
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
          <button type="button" className="icon-button" onClick={p.onToggleTheme} aria-label={t("تغییر پوسته", "Toggle theme")} title={p.theme === "dark" ? t("پوسته‌ی روشن", "Light theme") : t("پوسته‌ی تیره", "Dark theme")}>
            {p.theme === "dark" ? <IconSun /> : <IconMoon />}
          </button>
          <button
            type="button"
            className="icon-button"
            onClick={() => setLang(isFa ? "en" : "fa")}
            aria-label={t("تغییر زبان به انگلیسی", "Switch to Persian")}
            title={isFa ? "English" : "فارسی"}
          >
            <IconGlobe />
          </button>
          <button type="button" className="icon-button" onClick={p.onLogout} aria-label={t("خروج", "Sign out")} title={t("خروج", "Sign out")}>
            <IconLogout />
          </button>
        </div>
      </aside>
    </>
  );
}
