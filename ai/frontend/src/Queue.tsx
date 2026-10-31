import type { QueueItem } from "./api";
import { IconClose, IconPlay } from "./icons";
import { fmtNum } from "./format";
import { t } from "./i18n";

type Props = {
  items: QueueItem[];
  working: boolean;
  onRun: (id: number) => void;
  onRemove: (id: number) => void;
  onClear: () => void;
};

export function QueuePanel({ items, working, onRun, onRemove, onClear }: Props) {
  if (items.length === 0) return null;
  const count = fmtNum(items.length);
  return (
    <section className="queue" aria-label={t("صف پیام‌ها", "Message queue")}>
      <header className="queue-head">
        <strong>{t(`صف پیام‌ها (${count})`, `Queued messages (${count})`)}</strong>
        <span className="queue-note">{working ? t("بعد از پایان پاسخ فعلی، یکی‌یکی روی سرور اجرا می‌شوند.", "They run one by one on the server after the current answer.") : t("به‌زودی اجرا می‌شود…", "Starting soon…")}</span>
        <button type="button" className="queue-clear" onClick={onClear}>
          {t("پاک کردن همه", "Clear all")}
        </button>
      </header>
      <ol className="queue-list">
        {items.map((q, i) => (
          <li key={q.id} className="queue-item">
            <span className="queue-index">{fmtNum(i + 1)}</span>
            <span className="queue-text" dir="auto">
              {q.text || q.files.map((f) => f.name).join("، ")}
              {q.text && q.files.length > 0 && <em className="queue-files"> +{fmtNum(q.files.length)}</em>}
            </span>
            <button type="button" className="queue-btn run" onClick={() => onRun(q.id)} title={t("اجرای فوری: پاسخ فعلی متوقف می‌شود و این پیام همین الان اجرا می‌شود", "Run now: stops the current answer and runs this message immediately")} aria-label={t("اجرای فوری", "Run now")}>
              <IconPlay width={15} height={15} />
            </button>
            <button type="button" className="queue-btn" onClick={() => onRemove(q.id)} title={t("حذف از صف", "Remove from the queue")} aria-label={t("حذف از صف", "Remove from the queue")}>
              <IconClose width={15} height={15} />
            </button>
          </li>
        ))}
      </ol>
    </section>
  );
}
