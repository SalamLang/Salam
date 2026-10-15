import type { Message } from "./api";
import { CopyButton, Markdown } from "./Markdown";

const PHASE_TEXT: Record<string, string> = {
  thinking: "در حال فکر کردن",
  searching: "در حال بررسی مستندات سلام",
  writing: "در حال نوشتن",
};

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
  const pending = m.status === "pending";
  return (
    <div className={`msg assistant ${m.status}`}>
      <img src="/logo.svg" alt="" className="msg-avatar" width={28} height={28} />
      <div className="msg-body">
        {m.body ? <Markdown text={m.body} /> : null}
        {pending && (
          <div className="phase" aria-live="polite">
            <span className="dots">
              <i />
              <i />
              <i />
            </span>
            {PHASE_TEXT[m.phase || "thinking"] || PHASE_TEXT.thinking}
          </div>
        )}
        {!pending && m.body && (
          <div className="msg-tools">
            <CopyButton text={m.body} label="رونوشت پاسخ" />
          </div>
        )}
      </div>
    </div>
  );
}
