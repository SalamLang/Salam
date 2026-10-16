import { useEffect, useRef, useState, type KeyboardEvent, type ReactNode } from "react";
import { IconSend, IconStop } from "./icons";

type Props = {
  busy: boolean;
  disabled?: boolean;
  onSend: (text: string) => void;
  onStop: () => void;
  autoFocus?: boolean;
  bar?: ReactNode;
  placeholder?: string;
  note?: string;
};

export function Composer({ busy, disabled, onSend, onStop, autoFocus, bar, placeholder, note }: Props) {
  const [text, setText] = useState("");
  const ref = useRef<HTMLTextAreaElement>(null);

  useEffect(() => {
    const el = ref.current;
    if (!el) return;
    el.style.height = "auto";
    el.style.height = `${Math.min(el.scrollHeight, 240)}px`;
  }, [text]);

  useEffect(() => {
    if (autoFocus && matchMedia("(pointer: fine)").matches) ref.current?.focus();
  }, [autoFocus]);

  const send = () => {
    const t = text.trim();
    if (!t || busy || disabled) return;
    onSend(t);
    setText("");
  };

  const onKey = (e: KeyboardEvent<HTMLTextAreaElement>) => {
    if (e.key === "Enter" && !e.shiftKey && !e.nativeEvent.isComposing && matchMedia("(pointer: fine)").matches) {
      e.preventDefault();
      send();
    }
  };

  return (
    <div className="composer-wrap">
      {bar}
      <form
        className="composer"
        onSubmit={(e) => {
          e.preventDefault();
          send();
        }}
      >
        <textarea
          ref={ref}
          dir="auto"
          rows={1}
          value={text}
          maxLength={12000}
          placeholder={placeholder || "پرسش خود را درباره‌ی زبان سلام بنویسید…"}
          onChange={(e) => setText(e.target.value)}
          onKeyDown={onKey}
          aria-label="پیام"
        />
        {busy ? (
          <button type="button" className="send stop" onClick={onStop} aria-label="توقف پاسخ" title="توقف">
            <IconStop />
          </button>
        ) : (
          <button type="submit" className="send" disabled={!text.trim() || disabled} aria-label="فرستادن" title="فرستادن">
            <IconSend />
          </button>
        )}
      </form>
      <p className="disclaimer">{note || "دستیار سلام ممکن است اشتباه کند. کد مهم را با کامپایلر سلام بررسی کنید."}</p>
    </div>
  );
}
