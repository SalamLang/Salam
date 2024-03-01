import { useEffect, useId, useRef, useState, useSyncExternalStore, type ReactNode } from "react";
import { createPortal } from "react-dom";
import { IconAlert, IconTrash } from "./icons";
import { t } from "./i18n";

export type ConfirmOptions = {
  title: string;
  subject?: string;
  message?: ReactNode;
  confirmLabel?: string;
  cancelLabel?: string;
  tone?: "danger" | "default";
  icon?: "trash" | "alert";
};

type Request = ConfirmOptions & { id: number; resolve: (ok: boolean) => void };

let queue: Request[] = [];
let nextId = 1;
const listeners = new Set<() => void>();

function emit() {
  for (const l of listeners) l();
}

function subscribe(l: () => void) {
  listeners.add(l);
  return () => {
    listeners.delete(l);
  };
}

function current(): Request | null {
  return queue[0] ?? null;
}

export function confirmDialog(opts: ConfirmOptions): Promise<boolean> {
  return new Promise((resolve) => {
    queue = [...queue, { ...opts, id: nextId++, resolve }];
    emit();
  });
}

function settle(id: number, ok: boolean) {
  const req = queue.find((r) => r.id === id);
  if (!req) return;
  queue = queue.filter((r) => r.id !== id);
  req.resolve(ok);
  emit();
}

const EXIT_MS = 140;

const FONT_FACES = ["400 15px Shabnam", "500 15px Shabnam", "700 17px Shabnam"];

function warmFonts() {
  if (!document.fonts?.load) return;
  for (const f of FONT_FACES) document.fonts.load(f, "حذف گفت‌وگو").catch(() => {});
}

const FOCUSABLE = 'button:not([disabled]), [href], input:not([disabled]), textarea:not([disabled]), [tabindex]:not([tabindex="-1"])';

export function ConfirmHost() {
  const req = useSyncExternalStore(subscribe, current, current);
  const [shown, setShown] = useState<Request | null>(null);
  const [closing, setClosing] = useState(false);

  useEffect(() => {
    warmFonts();
  }, []);

  useEffect(() => {
    if (req && req.id !== shown?.id && !closing) {
      setShown(req);
      return;
    }
    if (!req && shown && !closing) setShown(null);
  }, [req, shown, closing]);

  if (!shown) return null;

  const close = (ok: boolean) => {
    if (closing) return;
    setClosing(true);
    window.setTimeout(() => {
      setClosing(false);
      setShown(null);
      settle(shown.id, ok);
    }, EXIT_MS);
  };

  return createPortal(<Dialog key={shown.id} req={shown} closing={closing} onClose={close} />, document.body);
}

function Dialog({ req, closing, onClose }: { req: Request; closing: boolean; onClose: (ok: boolean) => void }) {
  const titleId = useId();
  const quoteId = useId();
  const bodyId = useId();
  const panel = useRef<HTMLDivElement>(null);
  const cancelBtn = useRef<HTMLButtonElement>(null);
  const confirmBtn = useRef<HTMLButtonElement>(null);
  const pressedOnBackdrop = useRef(false);
  const danger = (req.tone ?? "danger") === "danger";
  const icon = req.icon ?? (danger ? "trash" : "alert");

  useEffect(() => {
    const previous = document.activeElement as HTMLElement | null;
    const root = document.getElementById("root");
    const overflow = document.body.style.overflow;
    document.body.style.overflow = "hidden";
    root?.setAttribute("inert", "");
    root?.setAttribute("aria-hidden", "true");
    (danger ? cancelBtn : confirmBtn).current?.focus();
    return () => {
      document.body.style.overflow = overflow;
      root?.removeAttribute("inert");
      root?.removeAttribute("aria-hidden");
      if (previous && document.contains(previous)) previous.focus();
    };
  }, [danger]);

  useEffect(() => {
    const onKey = (e: KeyboardEvent) => {
      if (e.key === "Escape") {
        e.preventDefault();
        e.stopPropagation();
        onClose(false);
        return;
      }
      if (e.key !== "Tab" || !panel.current) return;
      const items = Array.from(panel.current.querySelectorAll<HTMLElement>(FOCUSABLE));
      if (items.length === 0) return;
      const first = items[0];
      const last = items[items.length - 1];
      const active = document.activeElement;
      if (e.shiftKey && (active === first || !panel.current.contains(active))) {
        e.preventDefault();
        last.focus();
      } else if (!e.shiftKey && (active === last || !panel.current.contains(active))) {
        e.preventDefault();
        first.focus();
      }
    };
    document.addEventListener("keydown", onKey, true);
    return () => document.removeEventListener("keydown", onKey, true);
  }, [onClose]);

  return (
    <div
      className={`confirm-backdrop${closing ? " closing" : ""}`}
      onMouseDown={(e) => {
        pressedOnBackdrop.current = e.target === e.currentTarget;
      }}
      onMouseUp={(e) => {
        if (pressedOnBackdrop.current && e.target === e.currentTarget) onClose(false);
        pressedOnBackdrop.current = false;
      }}
    >
      <div
        ref={panel}
        className={`confirm-panel${danger ? " danger" : ""}`}
        role="alertdialog"
        aria-modal="true"
        aria-labelledby={titleId}
        aria-describedby={[req.subject && quoteId, req.message && bodyId].filter(Boolean).join(" ") || undefined}
      >
        <div className="confirm-icon" aria-hidden="true">
          {icon === "trash" ? <IconTrash width={22} height={22} /> : <IconAlert width={22} height={22} />}
        </div>
        <div className="confirm-text">
          <h2 id={titleId} className="confirm-title">
            {req.title}
          </h2>
          {req.subject && (
            <p id={quoteId} className="confirm-quote" dir="auto" title={req.subject}>
              <span>{req.subject}</span>
            </p>
          )}
          {req.message && (
            <div id={bodyId} className="confirm-message">
              {req.message}
            </div>
          )}
        </div>
        <div className="confirm-actions">
          <button ref={cancelBtn} type="button" className="button ghost" onClick={() => onClose(false)}>
            {req.cancelLabel ?? t("انصراف", "Cancel")}
          </button>
          <button
            ref={confirmBtn}
            type="button"
            className={`button ${danger ? "danger-solid" : "primary"}`}
            onClick={() => onClose(true)}
          >
            {req.confirmLabel ?? (danger ? t("حذف", "Delete") : t("تأیید", "Confirm"))}
          </button>
        </div>
        <p className="confirm-hint" aria-hidden="true">
          <kbd>Esc</kbd> {t("برای انصراف", "to cancel")}
        </p>
      </div>
    </div>
  );
}

export function Subject({ children }: { children: ReactNode }) {
  return (
    <bdi className="confirm-subject" dir="auto">
      {children}
    </bdi>
  );
}
