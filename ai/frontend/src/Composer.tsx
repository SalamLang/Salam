import { useEffect, useRef, useState, type ClipboardEvent, type DragEvent, type KeyboardEvent, type ReactNode } from "react";
import type { OutgoingFile } from "./api";
import { acceptList, kindOf, PendingChips, splitText, textBytes, textFile, toOutgoing, useCapabilities, type Pending } from "./Attach";
import { EffortPicker } from "./effort";
import { IconPaperclip, IconQueueAdd, IconSend, IconStop } from "./icons";
import { t } from "./i18n";
import { autoDir } from "./dir";
import { toPersianDigits } from "./theme";
import { pushToast } from "./Toasts";
import { MentionPopup, useMention, type Trigger } from "./Mention";
import { useVoiceInput, VoiceBar, VoiceButton } from "./Voice";

type Props = {
  busy: boolean;
  disabled?: boolean;
  onSend: (text: string, files: OutgoingFile[], previews: Pending[]) => Promise<boolean> | boolean | void;
  onStop: () => void;
  onQueue?: (text: string, files: OutgoingFile[], previews: Pending[]) => Promise<boolean> | boolean | void;
  queueing?: boolean;
  autoFocus?: boolean;
  bar?: ReactNode;
  placeholder?: string;
  note?: string;
  draft?: string;
  onDraftUsed?: () => void;
  workspace?: string;
};

// The server caps a message at 12000 *bytes* (Salam strings are UTF-8), so a
// Persian message reaches the limit after about 6000 characters. Anything
// longer is moved into an attached text file instead of being cut off.
const MAX_MESSAGE_BYTES = 12000;
// Keep each part under the server's 256 KB per-file text limit.
const PART_BYTES = 240 * 1024;

let seq = 0;
let pasteSeq = 0;

function pasteStamp(): string {
  const d = new Date();
  const two = (n: number) => String(n).padStart(2, "0");
  return `${d.getFullYear()}${two(d.getMonth() + 1)}${two(d.getDate())}-${two(d.getHours())}${two(d.getMinutes())}${two(d.getSeconds())}`;
}

function clipboardFiles(data: DataTransfer): File[] {
  const out: File[] = [];
  const seen = new Set<string>();
  const push = (file: File | null) => {
    if (!file) return;
    const key = `${file.name}|${file.size}|${file.type}|${file.lastModified}`;
    if (seen.has(key)) return;
    seen.add(key);
    out.push(file);
  };
  for (const file of Array.from(data.files ?? [])) push(file);
  for (const item of Array.from(data.items ?? [])) {
    if (item.kind !== "file") continue;
    push(item.getAsFile());
  }
  return out;
}

function namedClipboardFile(file: File, i: number): File {
  if (file.name) return file;
  const mime = (file.type || "image/png").toLowerCase();
  const ext = mime === "image/jpeg" ? "jpg" : mime === "image/gif" ? "gif" : mime === "image/webp" ? "webp" : "png";
  return new File([file], `pasted-image-${pasteStamp()}-${i}.${ext}`, { type: file.type || "image/png", lastModified: Date.now() });
}

export function Composer({ busy, disabled, onSend, onStop, onQueue, queueing, autoFocus, bar, placeholder, note, draft, onDraftUsed, workspace }: Props) {
  const [text, setText] = useState(draft ?? "");
  const [files, setFiles] = useState<Pending[]>([]);
  const [problem, setProblem] = useState("");
  const [dragging, setDragging] = useState(false);
  const [sending, setSending] = useState(false);
  const ref = useRef<HTMLTextAreaElement>(null);
  const picker = useRef<HTMLInputElement>(null);
  const [caret, setCaret] = useState(0);
  const caps = useCapabilities();
  const applyMention = (trigger: Trigger, insert: string) => {
    const next = text.slice(0, trigger.start) + insert + text.slice(trigger.end);
    const at = trigger.start + insert.length;
    setText(next);
    setCaret(at);
    requestAnimationFrame(() => {
      const el = ref.current;
      if (!el) return;
      el.focus();
      el.setSelectionRange(at, at);
    });
  };
  const mention = useMention(workspace ?? "", text, caret, applyMention);
  const hint = placeholder || t("پرسش خود را درباره‌ی زبان سلام بنویسید…", "Ask anything about the Salam language…");

  useEffect(() => {
    const el = ref.current;
    if (!el) return;
    el.style.height = "auto";
    el.style.height = `${Math.min(el.scrollHeight, 240)}px`;
  }, [text]);

  useEffect(() => {
    if (autoFocus && matchMedia("(pointer: fine)").matches) ref.current?.focus();
  }, [autoFocus]);

  useEffect(() => {
    if (!draft) return;
    setText(draft);
    onDraftUsed?.();
    const el = ref.current;
    if (el) {
      el.focus();
      el.setSelectionRange(draft.length, draft.length);
    }
  }, [draft, onDraftUsed]);

  const add = (list: FileList | File[]) => {
    if (!caps) return;
    const next = [...files];
    const problems: string[] = [];
    for (const file of Array.from(list)) {
      if (next.length >= caps.max_files) {
        problems.push(t(`حداکثر ${caps.max_files.toLocaleString("fa-IR")} فایل در هر پیام می‌شود فرستاد.`, `You can send at most ${caps.max_files} files per message.`));
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
      problems.push(t("حجم همه‌ی فایل‌ها با هم بیش از اندازه است.", "The files are too large together."));
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

  // Safety net for text that got into the box without a paste (voice input, a
  // draft, typing): it is attached as a file instead of being refused by the API.
  const textAsPending = (raw: string, prefix: string): Pending[] | null => {
    if (!caps || raw.length === 0) return null;
    const slots = caps.max_files - files.length;
    const room = caps.max_total - files.reduce((n, p) => n + p.file.size, 0);
    if (slots <= 0 || room <= 0) return null;
    const per = Math.max(1024, Math.min(caps.max_text || PART_BYTES, PART_BYTES));
    const parts = splitText(raw, per);
    if (parts.length > slots) return null;
    const stamp = pasteStamp();
    const n = ++pasteSeq;
    const made: Pending[] = [];
    let total = 0;
    parts.forEach((part, i) => {
      const name = parts.length === 1 ? `${prefix}-${stamp}-${n}.txt` : `${prefix}-${stamp}-${n}-${i + 1}.txt`;
      const file = textFile(name, part);
      total += file.size;
      made.push({ key: `f${++seq}`, file, kind: "text" });
    });
    return total > room ? null : made;
  };

  const attachText = (raw: string) => {
    const made = textAsPending(raw, "pasted-text");
    if (!made) {
      setProblem(
        t(
          "این متن در این پیام جا نمی‌شود. فایل‌های پیوست را کم کنید یا متن را به چند بخش تقسیم کنید.",
          "This text does not fit in this message. Remove some attachments or split it into parts.",
        ),
      );
      pushToast({
        tone: "failed",
        title: t("پیوست کردن متن ممکن نشد", "Could not attach the text"),
        body: t("سقف تعداد یا حجم فایل‌های این پیام پر شده است.", "This message has reached its file count or size limit."),
      });
      return;
    }
    add(made.map((p) => p.file));
    pushToast({
      tone: "done",
      title: t("متن بلند به فایل تبدیل شد", "The long text became a file"),
      body:
        made.length > 1
          ? t(`متن به ${toPersianDigits(made.length)} فایل تقسیم و پیوست شد.`, `The text was split into ${made.length} files and attached.`)
          : t(`«${made[0].file.name}» پیوست شد و متن داخل کادر دست‌نخورده ماند.`, `"${made[0].file.name}" was attached; the text already in the box was left as it is.`),
      seconds: 8,
    });
  };

  const addSpoken = (spoken: string) => {
    const current = ref.current?.value ?? "";
    const base = current.trimEnd();
    const joined = base ? `${base}${current.endsWith("\n") ? "\n" : " "}${spoken}` : spoken;
    setText(joined);
    if (textBytes(joined) > MAX_MESSAGE_BYTES) {
      pushToast({
        tone: "info",
        title: t("متن گفتار از سقف یک پیام بیشتر شد", "The speech is longer than one message"),
        body: t("هنگام فرستادن، خودکار به فایل تبدیل می‌شود.", "It becomes a file automatically when you send."),
      });
    }
    requestAnimationFrame(() => {
      const el = ref.current;
      if (!el) return;
      el.scrollTop = el.scrollHeight;
      if (matchMedia("(pointer: fine)").matches) {
        el.focus();
        el.setSelectionRange(joined.length, joined.length);
      }
    });
  };

  const voice = useVoiceInput(caps, addSpoken);
  const voiceActive = voice.stage !== "idle";

  const send = async () => {
    const typed = text.trim();
    const later = !!onQueue && (busy || !!queueing);
    if ((!typed && files.length === 0) || (busy && !later) || disabled || sending || voiceActive) return;
    let body = typed;
    let previews = files;
    if (textBytes(typed) > MAX_MESSAGE_BYTES) {
      const made = textAsPending(typed, "message");
      if (!made) {
        setProblem(
          t(
            "این متن برای یک پیام بلند است و به‌صورت فایل هم در آن جا نمی‌شود. فایل‌های پیوست را کم کنید یا متن را به چند بخش تقسیم کنید.",
            "This text is too long for one message and does not fit as a file either. Remove some attachments or split it into parts.",
          ),
        );
        return;
      }
      body = "";
      previews = [...files, ...made];
      pushToast({
        tone: "info",
        title: t("متن به فایل تبدیل شد و فرستاده می‌شود", "The text became a file and is being sent"),
        body: made.length > 1 ? t(`متن به ${toPersianDigits(made.length)} فایل تقسیم شد.`, `It was split into ${made.length} files.`) : undefined,
      });
    }
    const keep = { text, files };
    setSending(true);
    try {
      const payload = previews.length > 0 ? await toOutgoing(previews) : [];
      setText("");
      setFiles([]);
      setProblem("");
      const ok = later && onQueue ? await onQueue(body, payload, previews) : await onSend(body, payload, previews);
      if (ok === false) {
        setText(keep.text);
        setFiles(keep.files);
      }
    } catch {
      setProblem(t("خواندن فایل ناموفق بود.", "Could not read the file."));
    } finally {
      setSending(false);
    }
  };

  const onKey = (e: KeyboardEvent<HTMLTextAreaElement>) => {
    if (mention.onKey(e)) return;
    if (e.key === "Enter" && !e.shiftKey && !e.nativeEvent.isComposing && matchMedia("(pointer: fine)").matches) {
      e.preventDefault();
      send();
    }
  };

  const onPaste = (e: ClipboardEvent<HTMLTextAreaElement>) => {
    if (!caps) return;
    const onClipboard = clipboardFiles(e.clipboardData);
    if (onClipboard.length > 0) {
      e.preventDefault();
      add(onClipboard.map((f, i) => namedClipboardFile(f, i + 1)));
      return;
    }
    // Fallback for webviews where pasted images aren't exposed on the paste event:
    // read image items from the async Clipboard API on the same user gesture.
    if (navigator.clipboard?.read) {
      void navigator.clipboard.read().then(async (items) => {
        const files: File[] = [];
        let idx = 0;
        for (const item of items) {
          const type = item.types.find((t) => t.startsWith("image/"));
          if (!type) continue;
          try {
            const blob = await item.getType(type);
            idx += 1;
            files.push(namedClipboardFile(new File([blob], "", { type }), idx));
          } catch {}
        }
        if (files.length > 0) add(files);
      }).catch(() => {});
    }
    const pasted = e.clipboardData.getData("text/plain");
    if (!pasted) return;
    const el = e.currentTarget;
    const before = text.slice(0, el.selectionStart ?? text.length);
    const after = text.slice(el.selectionEnd ?? text.length);
    if (textBytes(before + pasted + after) <= MAX_MESSAGE_BYTES) return;
    e.preventDefault();
    attachText(pasted);
  };

  const onDrop = (e: DragEvent<HTMLDivElement>) => {
    e.preventDefault();
    setDragging(false);
    if (e.dataTransfer.files.length > 0) add(e.dataTransfer.files);
  };

  const canAttach = !!caps && !disabled;
  const ready = (text.trim().length > 0 || files.length > 0) && !disabled && !sending && !voiceActive;

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
      <MentionPopup state={mention} id="mention-list" />
      <form
        className="composer"
        onSubmit={(e) => {
          e.preventDefault();
          send();
        }}
      >
        <PendingChips items={files} onRemove={remove} />
        <VoiceBar voice={voice} />
        <div className="composer-row">
          {canAttach && caps && (
            <>
              <button
                type="button"
                className="attach-button"
                onClick={() => picker.current?.click()}
                aria-label={t("پیوست فایل", "Attach file")}
                title={caps.image ? t("پیوست تصویر، فایل متنی یا کد، یا ZIP", "Attach an image, a text or code file, or a ZIP") : t("پیوست فایل متنی یا کد، یا ZIP", "Attach a text or code file, or a ZIP")}
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
          <VoiceButton voice={voice} disabled={disabled} />
          <textarea
            ref={ref}
            dir={autoDir(text, hint)}
            rows={1}
            value={text}
            placeholder={hint}
            onChange={(e) => {
              setText(e.target.value);
              setCaret(e.target.selectionStart ?? e.target.value.length);
            }}
            onSelect={(e) => setCaret(e.currentTarget.selectionStart ?? 0)}
            onKeyDown={onKey}
            {...(mention.active ? { role: "combobox", "aria-expanded": true, "aria-controls": "mention-list", "aria-autocomplete": "list" as const, "aria-activedescendant": `mention-list-${mention.index}` } : {})}
            onPaste={canAttach ? onPaste : undefined}
            aria-label={t("پیام", "Message")}
          />
          <EffortPicker disabled={disabled} />
          {busy && (
            <button type="button" className="send stop" onClick={onStop} aria-label={t("توقف پاسخ", "Stop answer")} title={t("توقف", "Stop")}>
              <IconStop />
            </button>
          )}
          {busy || queueing ? (
            onQueue && (
              <button type="submit" className="send queue-add" disabled={!ready} aria-label={t("افزودن به صف", "Add to the queue")} title={t("افزودن به صف: بعد از پاسخ فعلی اجرا می‌شود (Enter)", "Add to the queue: runs after the current answer (Enter)")}>
                {sending ? <span className="spinner tiny" /> : <IconQueueAdd />}
              </button>
            )
          ) : (
            <button type="submit" className="send" disabled={!ready} aria-label={t("فرستادن", "Send")} title={t("فرستادن", "Send")}>
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
      {dragging && <div className="drop-hint">{t("فایل را اینجا رها کنید", "Drop the file here")}</div>}
      <p className="disclaimer">{note || t("دستیار سلام ممکن است اشتباه کند. کد مهم را با کامپایلر سلام بررسی کنید.", "Salam AI can make mistakes. Check important code with the Salam compiler.")}</p>
    </div>
  );
}
