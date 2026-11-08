import { useEffect, useRef } from "react";
import type { Project, User } from "./api";
import { isDesktop } from "./desktop";
import { IconChart, IconChat, IconClose, IconFolder, IconGlobe, IconKey, IconLogout, IconMemory, IconMonitor, IconMoon, IconPlug, IconPlus, IconShield, IconSpark, IconSun } from "./icons";
import type { Theme } from "./theme";
import { isFa, setLang, t } from "./i18n";
import { fmtNum } from "./format";

type Props = {
  user: User;
  projects: Project[];
  activeProject: string | null;
  open: boolean;
  theme: Theme;
  onClose: () => void;
  onNew: () => void;
  onLogout: () => void;
  onToggleTheme: () => void;
  onOpenProjects: () => void;
  onOpenProject: (id: string) => void;
  onOpenOthers: () => void;
  onNewProject: () => void;
  page: string;
  onNavigate: (path: string) => void;
};

export function Sidebar(p: Props) {
  const scroller = useRef<HTMLDivElement>(null);
  const scope = p.activeProject ?? p.page;
  useEffect(() => {
    scroller.current?.scrollTo({ top: 0 });
  }, [scope]);
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
        <div className="sidebar-scroll" ref={scroller}>
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
                <span className="project-link-name">{t("سایر", "Others")}</span>
              </button>
            </div>
          </div>
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
        </div>
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
