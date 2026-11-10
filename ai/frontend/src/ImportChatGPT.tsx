import { useCallback, useEffect, useRef, useState } from "react";
import { ApiError, api, type ChatgptImport, getToken } from "./api";
import { confirmDialog, Subject } from "./Confirm";
import {
  buildParts,
  type ExportPart,
  fingerprint,
  partBody,
} from "./chatgptImport";
import {
  type ChatgptProgress,
  chatgptExport,
  nativeChatgptImport,
} from "./desktop";
import { fmtDateTime, fmtNum } from "./format";
import { t } from "./i18n";

const ACTIVE = new Set(["queued", "running"]);
const UPLOAD_LANES = 3;
const TRIES = 5;

function errText(e: unknown, fallback: string): string {
  if (e instanceof ApiError) return e.message;
  if (e instanceof Error && e.message) return e.message;
  return fallback;
}

function mb(bytes: number): string {
  return (bytes / 1048576).toFixed(bytes < 10485760 ? 1 : 0);
}

function wait(ms: number): Promise<void> {
  return new Promise((resolve) => window.setTimeout(resolve, ms));
}

async function sendPart(
  id: number,
  n: number,
  part: ExportPart,
  have: Map<number, string>,
): Promise<void> {
  const body = await partBody(part);
  if (have.get(n) === body.sha256) return;
  let pause = 1000;
  for (let attempt = 1; ; attempt += 1) {
    try {
      await api.chatgptImportPart(id, n, body);
      return;
    } catch (e) {
      const retry =
        e instanceof ApiError && (e.code === "network" || e.status >= 500);
      if (!retry || attempt >= TRIES) throw e;
      await wait(pause);
      pause = Math.min(pause * 2, 10000);
    }
  }
}

async function uploadAll(
  file: File,
  parts: ExportPart[],
  onSent: (done: number, bytes: number) => void,
): Promise<ChatgptImport> {
  const made = await api.chatgptImportCreate({
    fingerprint: await fingerprint(file),
    filename: file.name.slice(0, 200),
    parts: parts.length,
    raw_bytes: parts.reduce((n, p) => n + p.raw, 0),
  });
  const id = made.import.id;
  const have = new Map((made.import.have ?? []).map((h) => [h.n, h.sha256]));
  let next = 0;
  let done = 0;
  let bytes = 0;
  const lane = async () => {
    for (;;) {
      const n = next;
      next += 1;
      if (n >= parts.length) return;
      await sendPart(id, n, parts[n], have);
      done += 1;
      bytes += parts[n].gz.length;
      onSent(done, bytes);
    }
  };
  await Promise.all(
    Array.from({ length: Math.min(UPLOAD_LANES, parts.length) }, lane),
  );
  return (await api.chatgptImportRun(id)).import;
}

function statusText(i: ChatgptImport): string {
  switch (i.status) {
    case "uploading":
      return t(
        `ناتمام: ${fmtNum(i.received)} از ${fmtNum(i.parts)} بخش بارگذاری شده`,
        `Unfinished: ${fmtNum(i.received)} of ${fmtNum(i.parts)} parts uploaded`,
      );
    case "queued":
      return t("در صف پردازش", "Waiting to be processed");
    case "running":
      return t(
        `در حال پردازش: بخش ${fmtNum(i.parts_done)} از ${fmtNum(i.parts)}`,
        `Processing: part ${fmtNum(i.parts_done)} of ${fmtNum(i.parts)}`,
      );
    case "done":
      return t(
        `${fmtNum(i.chats)} گفت‌وگو، ${fmtNum(i.messages)} پیام و ${fmtNum(i.memories)} خاطره وارد شد`,
        `${fmtNum(i.chats)} chats, ${fmtNum(i.messages)} messages and ${fmtNum(i.memories)} memories imported`,
      );
    case "canceled":
      return t("لغو شد", "Canceled");
    default:
      return t("ناموفق: ", "Failed: ") + i.error;
  }
}

function details(i: ChatgptImport): string {
  const bits: string[] = [];
  if (i.existing > 0)
    bits.push(
      t(
        `${fmtNum(i.existing)} گفت‌وگوی تکراری رد شد`,
        `${fmtNum(i.existing)} duplicates skipped`,
      ),
    );
  if (i.skipped > 0)
    bits.push(
      t(
        `${fmtNum(i.skipped)} گفت‌وگوی بدون متن رد شد`,
        `${fmtNum(i.skipped)} empty chats skipped`,
      ),
    );
  if (i.projects > 0)
    bits.push(
      t(
        `${fmtNum(i.projects)} پروژه از GPTها`,
        `${fmtNum(i.projects)} projects from GPTs`,
      ),
    );
  if (i.instructions === 1)
    bits.push(
      t(
        "دستور سفارشی در پرامپت حساب گذاشته شد",
        "custom instructions saved as your account prompt",
      ),
    );
  if (i.instructions === -1)
    bits.push(
      t(
        "پرامپت حساب شما از قبل پر بود و دست نخورد",
        "your account prompt was already set and was left alone",
      ),
    );
  return bits.join(" - ");
}

export function ImportChatGPT({
  notify,
  onDone,
}: {
  notify: (msg: string, ok?: boolean) => void;
  onDone: () => void;
}) {
  const [imports, setImports] = useState<ChatgptImport[]>([]);
  const [progress, setProgress] = useState<ChatgptProgress | null>(null);
  const [busy, setBusy] = useState(false);
  const picker = useRef<HTMLInputElement>(null);
  const known = useRef<Map<number, string>>(new Map());

  const refresh = useCallback(async () => {
    try {
      const list = (await api.chatgptImports()).imports;
      setImports(list);
      for (const i of list) {
        const was = known.current.get(i.id);
        if (was && was !== i.status) {
          if (i.status === "done") {
            notify(statusText(i), true);
            onDone();
          } else if (i.status === "failed") {
            notify(statusText(i));
          }
        }
        known.current.set(i.id, i.status);
      }
    } catch {
      return;
    }
  }, [notify, onDone]);

  useEffect(() => {
    void refresh();
  }, [refresh]);

  const active = imports.some((i) => ACTIVE.has(i.status));
  useEffect(() => {
    if (!active) return;
    const timer = window.setInterval(() => void refresh(), 1500);
    return () => window.clearInterval(timer);
  }, [active, refresh]);

  const fail = (e: unknown) => {
    notify(errText(e, t("درون‌ریزی ناموفق بود.", "The import failed.")));
  };

  const importFile = async (file: File) => {
    setBusy(true);
    try {
      setProgress({ phase: "reading", done: 0, total: 0, bytes: 0 });
      const parts = await buildParts(file, (done, total) =>
        setProgress({ phase: "reading", done, total, bytes: 0 }),
      );
      const total = parts.length;
      await uploadAll(file, parts, (done, bytes) =>
        setProgress({ phase: "uploading", done, total, bytes }),
      );
      setProgress(null);
      await refresh();
    } catch (e) {
      setProgress(null);
      fail(e);
    } finally {
      setBusy(false);
    }
  };

  const importNative = async () => {
    let off: (() => void) | null = null;
    setBusy(true);
    try {
      const path = await chatgptExport.pick();
      if (!path) return;
      off = await chatgptExport.onProgress(setProgress);
      setProgress({ phase: "reading", done: 0, total: 0, bytes: 0 });
      await chatgptExport.run(path, getToken());
      setProgress(null);
      await refresh();
    } catch (e) {
      setProgress(null);
      fail(typeof e === "string" ? new Error(e) : e);
    } finally {
      off?.();
      setBusy(false);
    }
  };

  const choose = () => {
    if (nativeChatgptImport) void importNative();
    else picker.current?.click();
  };

  const onFile = (file: File | undefined) => {
    if (picker.current) picker.current.value = "";
    if (file) void importFile(file);
  };

  const undo = async (i: ChatgptImport) => {
    const running = ACTIVE.has(i.status);
    const ok = await confirmDialog({
      title: running
        ? t("لغو درون‌ریزی", "Cancel import")
        : t("برگرداندن درون‌ریزی", "Undo import"),
      subject: i.filename || `#${i.id}`,
      message: running ? (
        t(
          "پردازش متوقف می‌شود. گفت‌وگوهایی که تا اینجا وارد شده‌اند می‌مانند.",
          "Processing stops. Chats imported so far stay.",
        )
      ) : (
        <>
          <Subject>{i.filename || `#${i.id}`}</Subject>{" "}
          {t(
            "همراه با همه‌ی گفت‌وگوها و خاطره‌هایی که از آن وارد شده و نسخه‌ی ذخیره‌شده‌ی روی سرور پاک می‌شود.",
            "is removed together with every chat and memory imported from it, and the stored copy on the server.",
          )}
        </>
      ),
      confirmLabel: running
        ? t("لغو کن", "Cancel it")
        : t("پاک کن", "Remove it"),
      icon: "alert",
    });
    if (!ok) return;
    try {
      const res = await api.chatgptImportDelete(i.id, true);
      if (!running)
        notify(
          t(
            `${fmtNum(res.chats_removed ?? 0)} گفت‌وگو برداشته شد.`,
            `${fmtNum(res.chats_removed ?? 0)} chats removed.`,
          ),
          true,
        );
      await refresh();
      if (!running) onDone();
    } catch (e) {
      fail(e);
    }
  };

  const percent =
    progress && progress.total > 0
      ? Math.min(100, Math.round((progress.done / progress.total) * 100))
      : 0;
  const label = progress
    ? progress.phase === "reading"
      ? t(
          `در حال خواندن و فشرده‌سازی: ${fmtNum(progress.done)} از ${fmtNum(progress.total)}`,
          `Reading and compressing: ${fmtNum(progress.done)} of ${fmtNum(progress.total)}`,
        )
      : progress.phase === "uploading"
        ? t(
            `در حال بارگذاری: بخش ${fmtNum(progress.done)} از ${fmtNum(progress.total)} (${mb(progress.bytes)} مگابایت)`,
            `Uploading: part ${fmtNum(progress.done)} of ${fmtNum(progress.total)} (${mb(progress.bytes)} MB)`,
          )
        : t("بارگذاری تمام شد", "Upload finished")
    : "";

  return (
    <section className="card panel import-card">
      <div className="panel-head">
        <h3>{t("وارد کردن از ChatGPT", "Import from ChatGPT")}</h3>
      </div>
      <p className="muted">
        {t(
          "در ChatGPT به تنظیمات، کنترل داده‌ها و خروجی گرفتن از داده‌ها بروید. ایمیلی با یک فایل zip می‌آید؛ آن را اینجا انتخاب کنید. فایل روی دستگاه خودتان خوانده می‌شود و فقط متن گفت‌وگوها (و حافظه و دستور سفارشی، اگر در خروجی بود) فشرده و بارگذاری می‌شود. عکس‌ها و فایل‌های داخل zip فرستاده نمی‌شوند.",
          "In ChatGPT open Settings, Data controls, Export data. You get an email with a zip file; choose it here. It is read on your device and only the chat text (and memories and custom instructions, if the export has them) is compressed and uploaded. Images and files inside the zip are not sent.",
        )}
      </p>
      <p className="muted">
        {t(
          "هر گفت‌وگو به یک گفت‌وگوی عادی، با یک خاطره‌ی خلاصه، تبدیل می‌شود. نسخه‌ی فشرده‌ی خروجی با نام تصادفی در پوشه‌ای خصوصی روی سرور می‌ماند تا بشود دوباره پردازشش کرد، و با برگرداندن درون‌ریزی پاک می‌شود.",
          "Each chat becomes a normal chat with a summary memory. The compressed copy of your export stays in a private folder on the server under a random name so it can be processed again, and is deleted when you undo the import.",
        )}
      </p>
      <div className="memory-actions">
        <button
          type="button"
          className="button small"
          onClick={choose}
          disabled={busy || active}
        >
          {busy && <span className="spinner" />}
          {t("انتخاب فایل zip خروجی", "Choose the export zip")}
        </button>
        <input
          ref={picker}
          type="file"
          accept=".zip,application/zip"
          hidden
          onChange={(e) => onFile(e.target.files?.[0])}
        />
      </div>
      {progress && (
        <div className="import-progress" role="status">
          <span>{label}</span>
          <span className="meter">
            <i style={{ width: `${percent}%` }} />
          </span>
        </div>
      )}
      {imports.length > 0 && (
        <ul className="import-list">
          {imports.map((i) => (
            <li key={i.id} className="import-row" data-status={i.status}>
              <div className="import-main">
                <strong dir="auto">{i.filename || `#${i.id}`}</strong>
                <span className="muted">{fmtDateTime(i.created)}</span>
                <span className="import-status">{statusText(i)}</span>
                {i.status === "running" && (
                  <span className="meter">
                    <i
                      style={{
                        width: `${i.parts > 0 ? Math.round((i.parts_done / i.parts) * 100) : 0}%`,
                      }}
                    />
                  </span>
                )}
                {i.status === "done" && details(i) && (
                  <span className="muted">{details(i)}</span>
                )}
                {i.status === "uploading" && (
                  <span className="muted">
                    {t(
                      "همان فایل را دوباره انتخاب کنید تا بارگذاری از همین‌جا ادامه یابد.",
                      "Choose the same file again to continue the upload from here.",
                    )}
                  </span>
                )}
              </div>
              <button
                type="button"
                className="button ghost small"
                onClick={() => void undo(i)}
              >
                {ACTIVE.has(i.status)
                  ? t("لغو", "Cancel")
                  : t("برگرداندن", "Undo")}
              </button>
            </li>
          ))}
        </ul>
      )}
    </section>
  );
}
