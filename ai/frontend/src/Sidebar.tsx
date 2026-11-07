import { useState } from "react";
import type { Chat, Project, User } from "./api";
import { isDesktop } from "./desktop";
import { IconChart, IconChat, IconClose, IconEdit, IconFolder, IconGlobe, IconKey, IconLogout, IconMemory, IconMonitor, IconMoon, IconPlug, IconPlus, IconShield, IconShare, IconSpark, IconSun, IconTrash } from "./icons";
import type { Theme } from "./theme";
import { confirmDialog } from "./Confirm";
import { isFa, setLang, t } from "./i18n";
import { fmtNum } from "./format";

type Filter = "all" | "active" | "idle";

function groupOf(updated: number): string {
  const now = new Date();
  const start = new Date(now.getFullYear(), now.getMonth(), now.getDate()).getTime() / 1000;
  if (updated >= start) return t("امروز", "Today");
  if (updated >= start - 86400) return t("دیروز", "Yesterday");
  if (updated >= start - 7 * 86400) return t("۷ روز گذشته", "Previous 7 days");
  if (updated >= start - 30 * 86400) return t("۳۰ روز گذشته", "Previous 30 days");
  return t("قدیمی‌تر", "Older");
}

function isLive(c: Chat): boolean {
  return c.state === "working" || c.state === "waiting" || c.state === "queued";
}

function stateOf(c: Chat): string {
  return c.state ?? "idle";
}

function statusText(c: Chat): string {
  switch (stateOf(c)) {
    case "working":
      if (c.phase === "writing") return t("در حال نوشتن", "Writing");
      if (c.phase === "tool") return t("در حال اجرای ابزار", "Running tools");
      return t("در حال فکر کردن", "Thinking");
    case "waiting":
      return t("منتظر تأیید شما", "Waiting for you");
    case "queued": {
      const n = c.queued ?? 1;
      return n > 1 ? t(`${fmtNum(n)} پیام در صف`, `${fmtNum(n)} queued`) : t("در صف", "Queued");
    }
    case "offline":
      return t("دستگاه آفلاین است", "Device offline");
    default:
      if (c.last_error) return t("آخرین پاسخ ناموفق بود", "Last reply failed");
      if (c.agent && c.online) return t("دستگاه متصل است", "Device connected");
      return "";
  }
}

type Props = {
  user: User;
  chats: Chat[];
  projects: Project[];
  activeProject: string | null;
  othersCount: number;
  chatHeading: string;
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
  onOpenProjects: () => void;
  onOpenProject: (id: string) => void;
  onOpenOthers: () => void;
  onNewProject: () => void;
  page: string;
  onNavigate: (path: string) => void;
  activeBusy?: boolean;
  activeWaiting?: boolean;
  activePhase?: Chat["phase"];
};

export function Sidebar(p: Props) {
  const [editing, setEditing] = useState<string | null>(null);
  const [draft, setDraft] = useState("");
  const [filter, setFilter] = useState<Filter>("all");
  const items = p.chats.map((c) => {
    if (c.id !== p.activeId) return c;
    if (p.activeWaiting) return { ...c, state: "waiting" as const };
    if (p.activeBusy) return { ...c, state: "working" as const, phase: p.activePhase ?? c.phase ?? "thinking" };
    return c;
  });
  const liveCount = items.filter(isLive).length;
  const seen = filter === "all" ? items : filter === "active" ? items.filter(isLive) : items.filter((c) => !isLive(c));
  const groups: { name: string; items: Chat[] }[] = [];
  for (const c of seen) {
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
  const filters: { id: Filter; label: string; count: number }[] = [
    { id: "all", label: t("همه", "All"), count: items.length },
    { id: "active", label: t("فعال", "Active"), count: liveCount },
    { id: "idle", label: t("بی‌کار", "Idle"), count: items.length - liveCount },
  ];
  return (
    <>
      <div className={`scrim ${p.open ? "show" : ""}`} onClick={p.onClose} aria-hidden="true" />
      <aside className={`sidebar ${p.open ? "open" : ""}`} aria-label={t("فهرست گفت‌وگوها", "Chat list")}>
        <div className="sidebar-head">
          <a className="brand" href="/" onClick={(e) => (e.preventDefault(), p.onOpenProjects())}>
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
        <div className="sidebar-projects">
          <div className="sidebar-section-head">
            <button type="button" className={`sidebar-section-title ${p.page === "projects" ? "active" : ""}`} onClick={p.onOpenProjects}>
              <IconFolder width={14} height={14} />
              <span>{t("پروژه‌ها", "Projects")}</span>
            </button>
            <button type="button" className="icon-button tiny" onClick={p.onNewProject} aria-label={t("پروژه‌ی تازه", "New project")} title={t("پروژه‌ی تازه", "New project")}>
              <IconPlus width={15} height={15} />
            </button>
          </div>
          <div className="project-nav">
            {p.projects.map((project) => (
              <button
                key={project.id}
                type="button"
                className={`project-link ${p.activeProject === project.id ? "active" : ""}`}
                onClick={() => p.onOpenProject(project.id)}
                title={project.name}
              >
                <span className="project-link-icon" style={{ background: project.color ? `${project.color}22` : "var(--brand-soft)", color: project.color || "var(--brand)" }}>
                  {project.emoji || <IconFolder width={15} height={15} />}
                </span>
                <span className="project-link-name" dir="auto">{project.name}</span>
                {project.live > 0 ? (
                  <span className="project-link-live" title={t("در حال کار", "Working")}>
                    <span className="live-dot" aria-hidden="true" />
                  </span>
                ) : project.chats > 0 ? (
                  <b>{fmtNum(project.chats)}</b>
                ) : null}
              </button>
            ))}
            <button type="button" className={`project-link others ${p.page === "others" ? "active" : ""}`} onClick={p.onOpenOthers}>
              <span className="project-link-icon others">
                <IconChat width={15} height={15} />
              </span>
              <span className="project-link-name">{t("سایر گفت‌وگوها", "Others")}</span>
              <b>{fmtNum(p.othersCount)}</b>
            </button>
          </div>
        </div>
        <h3 className="sidebar-chats-title">{p.chatHeading}</h3>
        <div className="chat-filters" role="group" aria-label={t("صافی گفت‌وگوها", "Filter chats")}>
          {filters.map((f) => (
            <button
              key={f.id}
              type="button"
              className={`chat-filter ${filter === f.id ? "on" : ""}`}
              aria-pressed={filter === f.id}
              onClick={() => setFilter(f.id)}
            >
              {f.id === "active" && f.count > 0 && <span className="live-dot" aria-hidden="true" />}
              <span>{f.label}</span>
              <b>{fmtNum(f.count)}</b>
            </button>
          ))}
        </div>
        <nav className="chat-list">
          {p.chats.length === 0 && <p className="empty-list">{t("هنوز گفت‌وگویی ندارید. اولین پرسش خود را بنویسید.", "No chats yet. Ask your first question.")}</p>}
          {p.chats.length > 0 && seen.length === 0 && (
            <p className="empty-list">
              {filter === "active"
                ? t("هیچ گفت‌وگویی در حال کار نیست. همه بی‌کارند.", "No chat is working right now. All are idle.")
                : t("همه‌ی گفت‌وگوها در حال کارند.", "Every chat is working right now.")}
            </p>
          )}
          {groups.map((g) => (
            <div key={g.name} className="chat-group">
              <h3>{g.name}</h3>
              {g.items.map((c) => {
                const state = stateOf(c);
                const status = statusText(c);
                const tip = `${c.title}${status ? ` — ${status}` : ` — ${t("بی‌کار", "Idle")}`}`;
                return (
                  <div key={c.id} className={`chat-item ${c.id === p.activeId ? "active" : ""} state-${state}${c.last_error ? " last-error" : ""}`}>
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
                      <button type="button" className="chat-link" onClick={() => p.onSelect(c.id)} title={tip} aria-label={tip}>
                        <span className={`chat-state ${state}`} aria-hidden="true" />
                        <span className="chat-main">
                          <span className="chat-title" dir="auto">{c.title}</span>
                          {status && <span className={`chat-status ${state}${c.last_error ? " failed" : ""}`}>{status}</span>}
                        </span>
                        {c.shared && c.shared.length > 0 && (
                          <IconShare className="chat-shared" width={13} height={13} aria-label={t("اشتراکی", "Shared")} role="img" />
                        )}
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
                );
              })}
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
            href="/usage"
            className={`chat-link ${p.page === "usage" ? "active" : ""}`}
            aria-current={p.page === "usage" ? "page" : undefined}
            onClick={(e) => (e.preventDefault(), p.onNavigate("/usage"))}
          >
            <IconChart width={16} height={16} />
            <span>{t("مصرف و توکن‌ها", "Usage and tokens")}</span>
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
              <span>{t("دریافت دستیار زبان سلام", "Get Salam Agent")}</span>
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
