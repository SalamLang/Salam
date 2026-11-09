import type { FormEvent } from "react";
import { useEffect, useRef, useState } from "react";
import type { Chat, Project, ProjectInput } from "./api";
import { confirmDialog } from "./Confirm";
import { fmtDate, fmtNum } from "./format";
import { t } from "./i18n";
import { agent, isDesktop } from "./desktop";
import { autoDir } from "./dir";
import { MasterField, PROJECT_MASTER_MAX } from "./MasterPrompt";
import {
  IconChat,
  IconClose,
  IconEdit,
  IconFolder,
  IconPlus,
  IconTrash,
} from "./icons";

export const PROJECT_COLORS = [
  "#ff5c00",
  "#2f7cf6",
  "#12a594",
  "#e0468b",
  "#8b5cf6",
  "#d97706",
  "#0ea5e9",
  "#64748b",
];

const PROJECT_EMOJIS = [
  "",
  "🚀",
  "🧪",
  "🌐",
  "🤖",
  "📱",
  "🎨",
  "📚",
  "⚙️",
  "🧠",
  "🛠️",
  "💡",
];

function tint(color: string): string {
  return color ? `${color}1f` : "var(--brand-soft)";
}

function when(ts: number): string {
  if (!ts) return t("تازه", "New");
  const now = Date.now() / 1000;
  const diff = Math.max(0, now - ts);
  if (diff < 60) return t("همین حالا", "Just now");
  if (diff < 3600)
    return t(
      `${fmtNum(Math.floor(diff / 60))} دقیقه پیش`,
      `${fmtNum(Math.floor(diff / 60))}m ago`,
    );
  if (diff < 86400)
    return t(
      `${fmtNum(Math.floor(diff / 3600))} ساعت پیش`,
      `${fmtNum(Math.floor(diff / 3600))}h ago`,
    );
  const days = Math.floor(diff / 86400);
  if (days < 30) return t(`${fmtNum(days)} روز پیش`, `${fmtNum(days)}d ago`);
  return fmtDate(ts);
}

function stateText(c: Chat): string {
  if (c.state === "working") {
    if (c.phase === "writing") return t("در حال نوشتن", "Writing");
    if (c.phase === "tool") return t("در حال اجرای ابزار", "Running tools");
    return t("در حال فکر کردن", "Thinking");
  }
  if (c.state === "waiting") return t("منتظر تأیید شما", "Waiting for you");
  if (c.state === "queued") return t("در صف", "Queued");
  if (c.state === "offline") return t("دستگاه آفلاین است", "Device offline");
  if (c.last_error) return t("آخرین پاسخ ناموفق بود", "Last reply failed");
  return when(c.updated);
}

type DialogProps = {
  project: Project | null;
  busy?: boolean;
  running?: boolean;
  onSave: (input: ProjectInput) => void;
  onClose: () => void;
};

export function ProjectDialog({ project, busy, running, onSave, onClose }: DialogProps) {
  const [name, setName] = useState(project?.name ?? "");
  const [description, setDescription] = useState(project?.description ?? "");
  const [master, setMaster] = useState(project?.master_prompt ?? "");
  const [color, setColor] = useState(project?.color || PROJECT_COLORS[0]);
  const [emoji, setEmoji] = useState(project?.emoji ?? "");
  const [path, setPath] = useState(project?.path ?? "");
  const [pathError, setPathError] = useState("");
  const nameRef = useRef<HTMLInputElement>(null);
  // Keep the latest onClose without re-running the mount effect below: the
  // parent re-renders on polling and passes a fresh inline callback, which
  // used to re-focus the name field and yank the caret out of the description.
  const closeRef = useRef(onClose);
  useEffect(() => {
    closeRef.current = onClose;
  }, [onClose]);
  useEffect(() => {
    const previous = document.activeElement as HTMLElement | null;
    nameRef.current?.focus();
    const onKey = (e: KeyboardEvent) => {
      if (e.key === "Escape") {
        e.preventDefault();
        closeRef.current();
      }
    };
    window.addEventListener("keydown", onKey);
    return () => {
      window.removeEventListener("keydown", onKey);
      if (previous && document.contains(previous)) previous.focus();
    };
  }, []);
  const needsPath = isDesktop && path === "";
  const submit = (e: FormEvent) => {
    e.preventDefault();
    const trimmed = name.trim();
    if (!trimmed || busy || needsPath) return;
    const input: ProjectInput = { name: trimmed, description: description.trim(), master_prompt: master.trim(), color, emoji };
    if (path !== (project?.path ?? "")) input.path = path;
    onSave(input);
  };
  const pickPath = async () => {
    try {
      const picked = await agent.pickFolder();
      if (picked) {
        setPath(picked);
        setPathError("");
      }
    } catch {
      setPathError(t("انتخاب پوشه ناموفق بود.", "Could not choose the folder."));
    }
  };
  const pathLocked = project !== null && running === true && (project.path ?? "") !== "";
  return (
    <div className="project-dialog-scrim">
      <button
        type="button"
        className="project-dialog-backdrop"
        onClick={onClose}
        aria-label={t("بستن", "Close")}
        tabIndex={-1}
      />
      <form
        className="project-dialog"
        onSubmit={submit}
        role="dialog"
        aria-modal="true"
        aria-label={
          project
            ? t("ویرایش پروژه", "Edit project")
            : t("پروژه‌ی تازه", "New project")
        }
      >
        <div className="project-dialog-head">
          <div>
            <h2>
              {project
                ? t("ویرایش پروژه", "Edit project")
                : t("پروژه‌ی تازه", "New project")}
            </h2>
            <p className="muted">
              {t(
                "گفت‌وگوهای یک کار را یک‌جا نگه دارید و بعداً راحت پیدایشان کنید.",
                "Keep one job's chats together and find them again easily.",
              )}
            </p>
          </div>
          <button
            type="button"
            className="icon-button"
            onClick={onClose}
            aria-label={t("بستن", "Close")}
          >
            <IconClose />
          </button>
        </div>
        <label className="field">
          <span>{t("نام پروژه", "Project name")}</span>
          <input
            ref={nameRef}
            value={name}
            dir={autoDir(name, t("مثلاً بازطراحی سایت", "For example: website redesign"))}
            onChange={(e) => setName(e.target.value)}
            maxLength={120}
            placeholder={t(
              "مثلاً بازطراحی سایت",
              "For example: website redesign",
            )}
          />
        </label>
        <label className="field">
          <span>{t("توضیح کوتاه", "Short description")}</span>
          <textarea
            className="project-textarea"
            value={description}
            dir={autoDir(description, t("این پروژه چه کاری انجام می‌دهد؟", "What is this project about?"))}
            onChange={(e) => setDescription(e.target.value)}
            maxLength={1000}
            rows={3}
            placeholder={t(
              "این پروژه چه کاری انجام می‌دهد؟",
              "What is this project about?",
            )}
          />
        </label>
        <div className="field project-path-field">
          <span>{t("پوشه‌ی پروژه", "Project folder")}</span>
          <div className="project-path-row">
            <span className={`project-path${path ? "" : " empty"}`} title={path}>
              <IconFolder width={15} height={15} />
              {path ? <bdi dir="ltr">{path}</bdi> : t("هنوز انتخاب نشده", "Not chosen yet")}
            </span>
            {isDesktop && (
              <button type="button" className="button ghost" onClick={pickPath} disabled={busy || pathLocked}>
                {path ? t("تغییر پوشه", "Change folder") : t("انتخاب پوشه", "Choose folder")}
              </button>
            )}
          </div>
          {pathLocked && (
            <small className="muted">
              {t(
                "تا وقتی گفت‌وگویی در این پروژه در حال اجراست، پوشه را نمی‌شود عوض کرد.",
                "The folder cannot be changed while a chat in this project is running.",
              )}
            </small>
          )}
          {!isDesktop && (
            <small className="muted">
              {t("پوشه‌ی پروژه را فقط از برنامه‌ی ایجنت سلام می‌شود تعیین کرد.", "The project folder can only be set from the Salam Agent app.")}
            </small>
          )}
          {isDesktop && !pathLocked && (
            <small className="muted">
              {t(
                "همه‌ی گفت‌وگوهای این پروژه در این پوشه کار می‌کنند. فقط از همین‌جا عوض می‌شود.",
                "Every chat in this project works in this folder. It can only be changed here.",
              )}
            </small>
          )}
          {pathError && <small className="error">{pathError}</small>}
        </div>
        <label className="field">
          <span>{t("پرامپت اصلی پروژه", "Project master prompt")}</span>
          <MasterField
            value={master}
            max={PROJECT_MASTER_MAX}
            onChange={setMaster}
            rows={5}
            placeholder={t(
              "چگونه این پروژه را پیش ببرد؟ مثلاً: بعد از هر کار تست کن و مرحله به مرحله کامیت و پوش کن.",
              "How should this project be run? For example: test after each task, commit and push step by step.",
            )}
          />
          <small className="muted">
            {t(
              "برای همه‌ی گفت‌وگوهای این پروژه. از پرامپت اصلی حساب بالاتر و از قوانین سرویس پایین‌تر است.",
              "Applies to every chat in this project. Ranks above your account master prompt and below the service rules.",
            )}
          </small>
        </label>
        <fieldset className="project-color-row">
          <legend className="project-dialog-label">{t("رنگ", "Color")}</legend>
          {PROJECT_COLORS.map((c) => (
            <button
              key={c}
              type="button"
              className={`project-swatch ${color === c ? "on" : ""}`}
              style={{ background: c }}
              aria-label={c}
              aria-pressed={color === c}
              onClick={() => setColor(c)}
            />
          ))}
        </fieldset>
        <fieldset className="project-emoji-row">
          <legend className="project-dialog-label">{t("نماد", "Icon")}</legend>
          {PROJECT_EMOJIS.map((e) => (
            <button
              key={e || "none"}
              type="button"
              className={`project-emoji ${emoji === e ? "on" : ""}`}
              aria-pressed={emoji === e}
              onClick={() => setEmoji(e)}
            >
              {e || <IconFolder width={17} height={17} />}
            </button>
          ))}
        </fieldset>
        <div className="project-dialog-foot">
          <button
            type="button"
            className="button ghost"
            onClick={onClose}
            disabled={busy}
          >
            {t("انصراف", "Cancel")}
          </button>
          <button
            type="submit"
            className="button primary"
            disabled={!name.trim() || busy || needsPath}
          >
            {project
              ? t("ذخیره‌ی تغییرها", "Save changes")
              : t("ساخت پروژه", "Create project")}
          </button>
        </div>
      </form>
    </div>
  );
}

type HomeProps = {
  projects: Project[];
  chats: Chat[];
  onOpenProject: (id: string) => void;
  onOpenChat: (id: string, projectId: string) => void;
  onOpenOthers: () => void;
  onNewProject: () => void;
  onEditProject: (project: Project) => void;
  onDeleteProject: (project: Project) => void;
};

export function ProjectsHome({
  projects,
  chats,
  onOpenProject,
  onOpenChat,
  onOpenOthers,
  onNewProject,
  onEditProject,
  onDeleteProject,
}: HomeProps) {
  const others = chats.filter((c) => !c.project_id);
  const recent = chats.slice(0, 6);
  return (
    <div className="page projects-page">
      <header className="projects-hero">
        <div>
          <p className="projects-eyebrow">
            {t("فضای کار شما", "Your workspace")}
          </p>
          <h2>{t("هر کار، یک پروژه", "One project per job")}</h2>
          <p className="muted">
            {t(
              "پروژه بسازید، گفت‌وگوهایش را یک‌جا ببینید و روی هر کدام جداگانه کار کنید. هر چیزی هم که به پروژه‌ای تعلق ندارد در «سایر گفت‌وگوها» می‌ماند.",
              "Create a project, keep its chats together and work on each one. Anything that belongs to no project stays in Others.",
            )}
          </p>
        </div>
        <button type="button" className="button primary" onClick={onNewProject}>
          <IconPlus width={18} height={18} />
          {t("پروژه‌ی تازه", "New project")}
        </button>
      </header>
      <div className="project-grid">
        {projects.map((p) => (
          <article
            key={p.id}
            className="project-card"
            style={{ borderTopColor: p.color || "var(--brand)" }}
          >
            <button
              type="button"
              className="project-card-open"
              onClick={() => onOpenProject(p.id)}
            >
              <span
                className="project-avatar"
                style={{
                  background: tint(p.color),
                  color: p.color || "var(--brand)",
                }}
              >
                {p.emoji || <IconFolder width={22} height={22} />}
              </span>
              <span className="project-card-main">
                <strong dir="auto">{p.name}</strong>
                <span className="project-desc">
                  {p.description || t("بدون توضیح", "No description")}
                </span>
                <span className="project-meta">
                  <span>
                    {t(`${fmtNum(p.chats)} گفت‌وگو`, `${fmtNum(p.chats)} chats`)}
                  </span>
                  <span aria-hidden="true">·</span>
                  <span>{when(p.last || p.updated)}</span>
                </span>
              </span>
              {p.live > 0 && (
                <span
                  className="project-live"
                  title={t("در حال کار", "Working")}
                >
                  <span className="live-dot" aria-hidden="true" />
                  {fmtNum(p.live)}
                </span>
              )}
            </button>
            <div className="project-card-actions">
              <button
                type="button"
                className="icon-button tiny"
                onClick={() => onEditProject(p)}
                aria-label={t("ویرایش پروژه", "Edit project")}
              >
                <IconEdit width={15} height={15} />
              </button>
              <button
                type="button"
                className="icon-button tiny danger"
                onClick={() => onDeleteProject(p)}
                aria-label={t("حذف پروژه", "Delete project")}
              >
                <IconTrash width={15} height={15} />
              </button>
            </div>
          </article>
        ))}
        <button
          type="button"
          className="project-card project-add"
          onClick={onNewProject}
        >
          <span className="project-avatar project-add-icon">
            <IconPlus width={22} height={22} />
          </span>
          <span className="project-card-main">
            <strong>{t("پروژه‌ی تازه", "New project")}</strong>
            <span className="project-desc">
              {t(
                "یک کار تازه را از همین‌جا شروع کنید.",
                "Start a new piece of work here.",
              )}
            </span>
          </span>
        </button>
        <button
          type="button"
          className="project-card project-others"
          onClick={onOpenOthers}
        >
          <span className="project-avatar others">
            <IconChat width={22} height={22} />
          </span>
          <span className="project-card-main">
            <strong>{t("سایر گفت‌وگوها", "Others")}</strong>
            <span className="project-desc">
              {t(
                "گفت‌وگوهای موقت و نامرتبط با پروژه‌ها.",
                "Temporary chats that belong to no project.",
              )}
            </span>
            <span className="project-meta">
              <span>
                {t(
                  `${fmtNum(others.length)} گفت‌وگو`,
                  `${fmtNum(others.length)} chats`,
                )}
              </span>
            </span>
          </span>
        </button>
      </div>
      {recent.length > 0 && (
        <section className="projects-recent">
          <div className="section-head">
            <h3>{t("گفت‌وگوهای اخیر", "Recent chats")}</h3>
            <span className="muted">
              {t("از هر پروژه", "From every project")}
            </span>
          </div>
          <ul className="project-chats">
            {recent.map((c) => (
              <li key={c.id}>
                <button
                  type="button"
                  className="project-chat-row"
                  onClick={() => onOpenChat(c.id, c.project_id || "")}
                >
                  <span
                    className={`chat-state ${c.state ?? "idle"}`}
                    aria-hidden="true"
                  />
                  <span className="project-chat-main">
                    <strong dir="auto">{c.title}</strong>
                    <small>{stateText(c)}</small>
                  </span>
                  <span className="project-chat-where">
                    {c.project_id
                      ? (projects.find((p) => p.id === c.project_id)?.name ??
                        t("پروژه", "Project"))
                      : t("سایر", "Others")}
                  </span>
                </button>
              </li>
            ))}
          </ul>
        </section>
      )}
    </div>
  );
}

type PageProps = {
  project: Project | null;
  chats: Chat[];
  onOpenChat: (id: string, projectId: string) => void;
  onNewChat: () => void;
  onEdit?: () => void;
  onDelete?: () => void;
  onRenameChat?: (id: string, title: string) => void;
  onDeleteChat?: (id: string) => void;
};

export function ProjectPage({
  project,
  chats,
  onOpenChat,
  onNewChat,
  onEdit,
  onDelete,
  onRenameChat,
  onDeleteChat,
}: PageProps) {
  const [renaming, setRenaming] = useState<string | null>(null);
  const [draft, setDraft] = useState("");
  const commit = (id: string) => {
    const title = draft.trim();
    if (title && onRenameChat) onRenameChat(id, title);
    setRenaming(null);
  };
  const live = chats.filter(
    (c) =>
      c.state === "working" || c.state === "waiting" || c.state === "queued",
  ).length;
  return (
    <div className="page project-page">
      <header className="project-head">
        <span
          className="project-avatar big"
          style={{
            background: tint(project?.color ?? ""),
            color: project?.color || "var(--brand)",
          }}
        >
          {project ? (
            project.emoji || <IconFolder width={28} height={28} />
          ) : (
            <IconChat width={28} height={28} />
          )}
        </span>
        <div className="project-head-main">
          <h2 dir="auto">
            {project ? project.name : t("سایر گفت‌وگوها", "Others")}
          </h2>
          <p className="muted">
            {project
              ? project.description ||
                t(
                  "برای این پروژه هنوز توضیحی ننوشته‌اید.",
                  "No description for this project yet.",
                )
              : t(
                  "گفت‌وگوهای موقت و نامرتبط با پروژه‌ها. هر وقت خواستید می‌توانید یکی را به پروژه‌ای منتقل کنید.",
                  "Temporary chats that belong to no project. You can move any of them into a project.",
                )}
          </p>
          <div className="project-head-meta">
            <span>
              {t(
                `${fmtNum(chats.length)} گفت‌وگو`,
                `${fmtNum(chats.length)} chats`,
              )}
            </span>
            {live > 0 && (
              <span className="project-live-text">
                <span className="live-dot" aria-hidden="true" />
                {t(`${fmtNum(live)} در حال کار`, `${fmtNum(live)} working`)}
              </span>
            )}
          </div>
        </div>
        <div className="project-head-actions">
          <button type="button" className="button primary" onClick={onNewChat}>
            <IconPlus width={18} height={18} />
            {t("گفت‌وگوی تازه", "New chat")}
          </button>
          {project && onEdit && (
            <button
              type="button"
              className="icon-button"
              onClick={onEdit}
              aria-label={t("ویرایش پروژه", "Edit project")}
              title={t("ویرایش پروژه", "Edit project")}
            >
              <IconEdit />
            </button>
          )}
          {project && onDelete && (
            <button
              type="button"
              className="icon-button danger"
              onClick={onDelete}
              aria-label={t("حذف پروژه", "Delete project")}
              title={t("حذف پروژه", "Delete project")}
            >
              <IconTrash />
            </button>
          )}
        </div>
      </header>
      {chats.length === 0 ? (
        <div className="card empty-card project-empty">
          <h3>{t("هنوز گفت‌وگویی این‌جا نیست", "No chats here yet")}</h3>
          <p className="muted">
            {t(
              "اولین پرسش را بپرسید تا گفت‌وگو در همین پروژه ساخته شود.",
              "Ask your first question and the chat will be created in this project.",
            )}
          </p>
          <button type="button" className="button primary" onClick={onNewChat}>
            <IconPlus width={18} height={18} />
            {t("شروع گفت‌وگو", "Start a chat")}
          </button>
        </div>
      ) : (
        <ul className="project-chats project-chats-large">
          {chats.map((c) => (
            <li key={c.id} className="project-chat-item">
              {renaming === c.id ? (
                <input
                  className="rename"
                  value={draft}
                  dir="auto"
                  autoFocus
                  maxLength={80}
                  onChange={(e) => setDraft(e.target.value)}
                  onBlur={() => commit(c.id)}
                  onKeyDown={(e) => {
                    if (e.key === "Enter") commit(c.id);
                    if (e.key === "Escape") setRenaming(null);
                  }}
                />
              ) : (
                <button
                  type="button"
                  className="project-chat-row"
                  onClick={() => onOpenChat(c.id, project?.id ?? "")}
                >
                  <span
                    className={`chat-state ${c.state ?? "idle"}`}
                    aria-hidden="true"
                  />
                  <span className="project-chat-main">
                    <strong dir="auto">{c.title}</strong>
                    <small>{stateText(c)}</small>
                  </span>
                  {c.shared && c.shared.length > 0 && (
                    <span className="project-chat-badge">
                      {t("اشتراکی", "Shared")}
                    </span>
                  )}
                  <span className="project-chat-open" aria-hidden="true">
                    ›
                  </span>
                </button>
              )}
              {(onRenameChat || onDeleteChat) && renaming !== c.id && (
                <div className="project-chat-actions">
                  {onRenameChat && (
                    <button
                      type="button"
                      className="icon-button tiny"
                      aria-label={t("تغییر نام", "Rename")}
                      onClick={() => {
                        setDraft(c.title);
                        setRenaming(c.id);
                      }}
                    >
                      <IconEdit width={15} height={15} />
                    </button>
                  )}
                  {onDeleteChat && (
                    <button
                      type="button"
                      className="icon-button tiny danger"
                      aria-label={t("حذف گفت‌وگو", "Delete chat")}
                      onClick={async () => {
                        const ok = await confirmDialog({
                          title: t("حذف گفت‌وگو", "Delete chat"),
                          subject: c.title,
                          message: t(
                            "این گفت‌وگو و همه‌ی پیام‌هایش برای همیشه حذف می‌شود و برگرداندنش ممکن نیست.",
                            "This chat and all its messages will be deleted for good. This cannot be undone.",
                          ),
                          confirmLabel: t("حذف", "Delete"),
                        });
                        if (ok) onDeleteChat(c.id);
                      }}
                    >
                      <IconTrash width={15} height={15} />
                    </button>
                  )}
                </div>
              )}
            </li>
          ))}
        </ul>
      )}
      {project && (
        <p className="project-tip muted">
          {t(
            "گفت‌وگوهای این پروژه با هم یک فضا دارند؛ می‌توانید هر گفت‌وگو را از نوار بالای صفحه به پروژه‌ی دیگری هم منتقل کنید.",
            "The chats of this project share one space; you can move any chat to another project from the bar above it.",
          )}
        </p>
      )}
    </div>
  );
}

export async function confirmDeleteProject(project: Project): Promise<boolean> {
  return confirmDialog({
    title: t("حذف پروژه", "Delete project"),
    subject: project.name,
    message: t(
      "خود پروژه حذف می‌شود، اما گفت‌وگوهایش پاک نمی‌شوند و به «سایر گفت‌وگوها» منتقل می‌شوند.",
      "The project is removed, but its chats are kept and moved to Others.",
    ),
    confirmLabel: t("حذف پروژه", "Delete project"),
  });
}
