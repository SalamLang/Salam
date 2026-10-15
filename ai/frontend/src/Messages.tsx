import { useEffect, useRef, useState } from "react";
import type { Message } from "./api";
import { CopyButton, Markdown } from "./Markdown";

const PHASE_TEXT: Record<string, string> = {
  thinking: "در حال فکر کردن",
  searching: "در حال بررسی مستندات سلام",
  writing: "در حال نوشتن",
};

const FRAME_MS = 33;
const CATCH_UP_MS = 260;

function useSmoothText(target: string, animate: boolean): string {
  const [shown, setShown] = useState(animate ? "" : target);
  const shownRef = useRef(shown);

  useEffect(() => {
    if (!animate) {
      shownRef.current = target;
      setShown(target);
      return;
    }
    let raf = 0;
    let last = 0;
    const tick = (ts: number) => {
      const cur = shownRef.current;
      if (!target.startsWith(cur)) {
        shownRef.current = target;
        setShown(target);
        return;
      }
      const backlog = target.length - cur.length;
      if (backlog <= 0) return;
      if (last && ts - last < FRAME_MS) {
        raf = requestAnimationFrame(tick);
        return;
      }
      const dt = last ? ts - last : FRAME_MS;
      last = ts;
      const step = Math.max(2, Math.ceil(backlog * Math.min(1, dt / CATCH_UP_MS)));
      let end = Math.min(target.length, cur.length + step);
      const code = target.charCodeAt(end - 1);
      if (end < target.length && code >= 0xd800 && code <= 0xdbff) end++;
      const next = target.slice(0, end);
      shownRef.current = next;
      setShown(next);
      if (end < target.length) raf = requestAnimationFrame(tick);
    };
    raf = requestAnimationFrame(tick);
    return () => cancelAnimationFrame(raf);
  }, [target, animate]);

  return shown;
}

function AssistantMessage({ m }: { m: Message }) {
  const [animate] = useState(m.status === "pending");
  const text = useSmoothText(m.body, animate);
  const pending = m.status === "pending";
  const typing = pending || text.length < m.body.length;
  const phase = m.phase || "thinking";
  const showPhase = pending && (text.length === 0 || phase === "searching");
  const visible = typing ? text.replace(/\n?`{1,3}[^\n`]*$/, "") : text;
  return (
    <div className={`msg assistant ${m.status}`}>
      <img src="/logo.svg" alt="" className="msg-avatar" width={28} height={28} />
      <div className="msg-body">
        {visible ? (
          <div className={typing ? "streaming" : undefined}>
            <Markdown text={visible} />
          </div>
        ) : null}
        {showPhase && (
          <div className="phase" aria-live="polite">
            <span className="dots">
              <i />
              <i />
              <i />
            </span>
            {PHASE_TEXT[phase] || PHASE_TEXT.thinking}
          </div>
        )}
        {!typing && m.body && (
          <div className="msg-tools">
            <CopyButton text={m.body} label="رونوشت پاسخ" />
          </div>
        )}
      </div>
    </div>
  );
}

export function MessageView({ m, name }: { m: Message; name: string }) {
  if (m.role === "user") {
    return (
      <div className="msg user">
        <div className="bubble" dir="auto">
          {m.body}
        </div>
        <span className="sr-only">{name}</span>
      </div>
    );
  }
  return <AssistantMessage m={m} />;
}
