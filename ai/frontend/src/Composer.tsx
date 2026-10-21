import { useEffect, useRef, useState, type ClipboardEvent, type DragEvent, type KeyboardEvent, type ReactNode } from "react";
import type { OutgoingFile } from "./api";
import { acceptList, kindOf, PendingChips, toOutgoing, useCapabilities, type Pending } from "./Attach";
import { IconPaperclip, IconSend, IconStop } from "./icons";

type Props = {
  busy: boolean;
  disabled?: boolean;
  onSend: (text: string, files: OutgoingFile[], previews: Pending[]) => Promise<boolean> | boolean | void;
  onStop: () => void;
  autoFocus?: boolean;
  bar?: ReactNode;
  placeholder?: string;
  note?: string;
};

const RTL_FIRST = /^[^A-Za-z֐-ࣿיִ-﷿ﹰ-﻿]*[֐-ࣿיִ-﷿ﹰ-﻿]/;

let seq = 0;

export function Composer({ busy, disabled, onSend, onStop, autoFocus, bar, placeholder, note }: Props) {
  const [text, setText] = useState("");
  const [files, setFiles] = useState<Pending[]>([]);
  const [problem, setProblem] = useState("");
  const [dragging, setDragging] = useState(false);
  const [sending, setSending] = useState(false);
  const ref = useRef<HTMLTextAreaElement>(null);
  const picker = useRef<HTMLInputElement>(null);
  const caps = useCapabilities();
  const hint = placeholder || "پرسش خود را درباره‌ی زبان سلام بنویسید…";

  useEffect(() => {
    const el = ref.current;
    if (!el) return;
    el.style.height = "auto";
    el.style.height = `${Math.min(el.scrollHeight, 240)}px`;
  }, [text]);

  useEffect(() => {
    if (autoFocus && matchMedia("(pointer: fine)").matches) ref.current?.focus();
  }, [autoFocus]);

  const add = (list: FileList | File[]) => {
    if (!caps) return;
    const next = [...files];
    const problems: string[] = [];
    for (const file of Array.from(list)) {
      if (next.length >= caps.max_files) {
        problems.push(`حداکثر ${caps.max_files.toLocaleString("fa-IR")} فایل در هر پیام می‌شود فرستاد.`);
        break;
      }
      const { kind, problem: why } = kindOf(file, caps);
      if (!kind) {
        problems.push(why ?? "");
        continue;
      }
      next.push({ key: `f${++seq}`, file, kind, preview: kind === "image" ? URL.createObjectURL(file) : undefined });
    }
    if (next.reduce((n, f) => n + f.file.size, 0) > caps.max_total) {
      problems.push("حجم همه‌ی فایل‌ها با هم بیش از اندازه است.");
      setProblem(problems.join(" "));
      return;
    }
    setProblem(problems.join(" "));
    setFiles(next);
  };

  const remove = (key: string) => {
    const gone = files.find((f) => f.key === key);
    if (gone?.preview) URL.revokeObjectURL(gone.preview);
    setFiles((cur) => cur.filter((f) => f.key !== key));
    setProblem("");
  };

  const send = async () => {
    const t = text.trim();
    if ((!t && files.length === 0) || busy || disabled || sending) return;
    const keep = { text, files };
    setSending(true);
    try {
      const payload = files.length > 0 ? await toOutgoing(files) : [];
      setText("");
      setFiles([]);
      setProblem("");
      const ok = await onSend(t, payload, keep.files);
      if (ok === false) {
        setText(keep.text);
        setFiles(keep.files);
      }
    } catch {
      setProblem("خواندن فایل ناموفق بود.");
    } finally {
      setSending(false);
    }
  };

  const onKey = (e: KeyboardEvent<HTMLTextAreaElement>) => {
    if (e.key === "Enter" && !e.shiftKey && !e.nativeEvent.isComposing && matchMedia("(pointer: fine)").matches) {
      e.preventDefault();
      send();
    }
  };

  const onPaste = (e: ClipboardEvent<HTMLTextAreaElement>) => {
    if (e.clipboardData.files.length > 0) {
      e.preventDefault();
      add(e.clipboardData.files);
    }
  };

  const onDrop = (e: DragEvent<HTMLDivElement>) => {
    e.preventDefault();
    setDragging(false);
    if (e.dataTransfer.files.length > 0) add(e.dataTransfer.files);
  };

  const canAttach = !!caps && !disabled;
  const ready = (text.trim().length > 0 || files.length > 0) && !disabled && !sending;

  return (
    <div
      className={`composer-wrap ${dragging ? "dragging" : ""}`}
      onDragOver={(e) => {
        if (!canAttach || !e.dataTransfer.types.includes("Files")) return;
        e.preventDefault();
        setDragging(true);
      }}
      onDragLeave={(e) => {
        if (!e.currentTarget.contains(e.relatedTarget as Node | null)) setDragging(false);
      }}
      onDrop={canAttach ? onDrop : undefined}
    >
      {bar}
      <form
        className="composer"
        onSubmit={(e) => {
          e.preventDefault();
          send();
        }}
      >
        <PendingChips items={files} onRemove={remove} />
        <div className="composer-row">
          {canAttach && caps && (
            <>
              <button
                type="button"
                className="attach-button"
                onClick={() => picker.current?.click()}
                aria-label="پیوست فایل"
                title={caps.image ? "پیوست تصویر، فایل متنی یا کد، یا ZIP" : "پیوست فایل متنی یا کد، یا ZIP"}
              >
                <IconPaperclip />
              </button>
              <input
                ref={picker}
                type="file"
                multiple
                hidden
                accept={acceptList(caps)}
                onChange={(e) => {
                  if (e.target.files) add(e.target.files);
                  e.target.value = "";
                }}
              />
            </>
          )}
          <textarea
            ref={ref}
            dir={text ? "auto" : RTL_FIRST.test(hint) ? "rtl" : "ltr"}
            rows={1}
            value={text}
            maxLength={12000}
            placeholder={hint}
            onChange={(e) => setText(e.target.value)}
            onKeyDown={onKey}
            onPaste={canAttach ? onPaste : undefined}
            aria-label="پیام"
          />
          {busy ? (
            <button type="button" className="send stop" onClick={onStop} aria-label="توقف پاسخ" title="توقف">
              <IconStop />
            </button>
          ) : (
            <button type="submit" className="send" disabled={!ready} aria-label="فرستادن" title="فرستادن">
              {sending ? <span className="spinner tiny" /> : <IconSend />}
            </button>
          )}
        </div>
        {problem && (
          <p className="attach-problem" role="alert">
            {problem}
          </p>
        )}
      </form>
      {dragging && <div className="drop-hint">فایل را اینجا رها کنید</div>}
      <p className="disclaimer">{note || "دستیار سلام ممکن است اشتباه کند. کد مهم را با کامپایلر سلام بررسی کنید."}</p>
    </div>
  );
}
