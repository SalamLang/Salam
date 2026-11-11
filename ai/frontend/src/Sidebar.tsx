import { useEffect, useRef, useState } from "react";
import type { Project, Subscription, User } from "./api";
import { isDesktop, useAgentUpdate } from "./desktop";
import { IconChart, IconChat, IconClose, IconDownload, IconFolder, IconKey, IconMemory, IconMonitor, IconPlug, IconPlus, IconSettings, IconShield, IconSpark } from "./icons";
import { isFa, t } from "./i18n";
import { fmtNum } from "./format";

type Props = {
  user: User;
  plan: Subscription | null;
  projects: Project[];
  activeProject: string | null;
  open: boolean;
  onClose: () => void;
  onNew: () => void;
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
  const [menu, setMenu] = useState(false);
  const agentUpdate = useAgentUpdate();
  const wrap = useRef<HTMLDivElement>(null);
  const gear = useRef<HTMLButtonElement>(null);
  useEffect(() => {
    if (!menu) return;
    const down = (e: Event) => {
      if (!wrap.current?.contains(e.target as Node)) setMenu(false);
    };
    const key = (e: KeyboardEvent) => {
      if (e.key !== "Escape") return;
      setMenu(false);
      gear.current?.focus();
    };
    document.addEventListener("pointerdown", down);
    document.addEventListener("keydown", key);
    return () => {
      document.removeEventListener("pointerdown", down);
      document.removeEventListener("keydown", key);
    };
  }, [menu]);
  useEffect(() => {
    if (!p.open) setMenu(false);
  }, [p.open]);
  const pick = (go: () => void) => {
    setMenu(false);
    go();
    p.onClose();
  };
  const items = [
    { key: "connections", path: "/connections", icon: <IconPlug width={16} height={16} />, label: t("اتصال‌ها و برنامه‌ها", "Connections and apps") },
    { key: "plans", path: "/plans", icon: <IconSpark width={16} height={16} />, label: t("اشتراک‌ها", "Plans") },
    { key: "usage", path: "/usage", icon: <IconChart width={16} height={16} />, label: t("مصرف و توکن‌ها", "Usage and tokens") },
    { key: "keys", path: "/keys", icon: <IconKey width={16} height={16} />, label: t("کلیدهای API", "API keys") },
    { key: "memory", path: "/memory", icon: <IconMemory width={16} height={16} />, label: t("حافظه", "Memory") },
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
        </div>
        <div className="sidebar-foot">
          {!isDesktop ? (
            <a
              href="https://www.salamlang.ir/agent/"
              className="foot-banner"
              target="_blank"
              rel="noreferrer"
            >
              <IconMonitor width={16} height={16} />
              <span>{t("دریافت دستیار زبان سلام", "Get Salam Agent")}</span>
            </a>
          ) : (
            agentUpdate && (
              <a
                href={agentUpdate.url}
                className="foot-banner foot-banner-update"
                target="_blank"
                rel="noreferrer"
              >
                <IconDownload width={16} height={16} />
                <span>{t(`به‌روزرسانی به نسخه‌ی ${agentUpdate.version}`, `Update to version ${agentUpdate.version}`)}</span>
              </a>
            )
          )}
          <div className="sidebar-foot-row">
            <div className="avatar" aria-hidden="true">
              {p.user.name.trim().charAt(0)}
            </div>
            <div className="who">
              <strong dir="auto">{p.user.name}</strong>
              <span dir="ltr">{p.user.email}</span>
              {p.plan && (
                <button
                  type="button"
                  className={`plan-chip ${p.plan.plan.id === "free" ? "free" : "ok"}`}
                  onClick={(e) => (e.preventDefault(), pick(() => p.onNavigate("/plans")))}
                  title={t("دیدن پلن‌ها", "See plans")}
                >
                  <IconSpark width={12} height={12} />
                  {isFa ? p.plan.plan.name : p.plan.plan.name_en || p.plan.plan.name}
                  {p.plan.plan.id === "free" && <span className="plan-chip-cta">{t("ارتقا", "Upgrade")}</span>}
                </button>
              )}
            </div>
            <div className="settings-wrap" ref={wrap}>
              <button
                type="button"
                className={`icon-button ${menu ? "active" : ""}`}
                onClick={() => setMenu(!menu)}
                aria-label={t("تنظیمات", "Settings")}
                title={t("تنظیمات", "Settings")}
                aria-haspopup="true"
                aria-expanded={menu}
                aria-controls="settings-menu"
                ref={gear}
              >
                <IconSettings />
              </button>
              {menu && (
                <nav className="settings-menu" id="settings-menu" aria-label={t("تنظیمات", "Settings")}>
                  {items.map((item) => (
                    <a
                      key={item.path}
                      href={item.path}
                      className={`chat-link ${p.page === item.key ? "active" : ""}`}
                      aria-current={p.page === item.key ? "page" : undefined}
                      onClick={(e) => (e.preventDefault(), pick(() => p.onNavigate(item.path)))}
                    >
                      {item.icon}
                      <span>{item.label}</span>
                    </a>
                  ))}
                  {p.user.admin && (
                    <a
                      href="/admin"
                      className={`chat-link ${p.page === "admin" ? "active" : ""}`}
                      aria-current={p.page === "admin" ? "page" : undefined}
                      onClick={(e) => (e.preventDefault(), pick(() => p.onNavigate("/admin")))}
                    >
                      <IconShield width={16} height={16} />
                      <span>{t("مدیریت", "Admin")}</span>
                    </a>
                  )}
                </nav>
              )}
            </div>
          </div>
        </div>
      </aside>
    </>
  );
}
