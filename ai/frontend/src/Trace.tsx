import { useEffect, useState } from "react";
import { api, type TraceItem } from "./api";
import { BidiText } from "./Bidi";
import { Markdown } from "./Markdown";
import { toPersianDigits } from "./theme";
import { t } from "./i18n";

const CODE_FIELDS = ["source", "code", "content", "command", "commands", "patch", "newString", "oldString", "new_string", "old_string", "query", "pattern"];

function duration(item: TraceItem): string {
  if (item.kind !== "tool" || !item.at || !item.end || item.end < item.at) return "";
  const ms = item.end - item.at;
  return ms < 1000 ? t(`${toPersianDigits(ms)} میلی‌ثانیه`, `${ms} ms`) : t(`${toPersianDigits((ms / 1000).toFixed(1))} ثانیه`, `${(ms / 1000).toFixed(1)} s`);
}

function clock(ms: number): string {
  if (!ms) return "";
  return new Date(ms).toLocaleTimeString("fa-IR", { hour: "2-digit", minute: "2-digit", second: "2-digit" });
}

function parseInput(input: string): { fields: [string, string][]; rest: string } {
  try {
    const obj = JSON.parse(input) as Record<string, unknown>;
    const fields: [string, string][] = [];
    const rest: Record<string, unknown> = {};
    for (const [k, v] of Object.entries(obj)) {
      if (typeof v === "string" && (CODE_FIELDS.includes(k) || v.includes("\n"))) fields.push([k, v]);
      else rest[k] = v;
    }
    return { fields, rest: Object.keys(rest).length ? JSON.stringify(rest, null, 2) : "" };
  } catch {
    return { fields: [], rest: input };
  }
}

function ToolItem({ item }: { item: Extract<TraceItem, { kind: "tool" }> }) {
  const { fields, rest } = parseInput(item.input);
  const failed = item.status === "error";
  return (
    <li className={`trace-item tool ${failed ? "failed" : ""}`}>
      <div className="trace-head">
        <span className="trace-kind">{t("ابزار", "Tool")}</span>
        <code dir="ltr">{item.tool}</code>
        {item.title && (
          <span className="trace-title" dir="auto">
            {item.title}
          </span>
        )}
        <span className={`trace-status ${item.status}`}>{failed ? t("ناموفق", "Failed") : item.status === "completed" ? t("انجام شد", "Done") : t("در حال اجرا", "Running")}</span>
        <span className="trace-time">{[clock(item.at), duration(item)].filter(Boolean).join(t("، ", ", "))}</span>
      </div>
      {fields.map(([k, v]) => (
        <div key={k} className="trace-block">
          <span className="trace-label" dir="ltr">
            {k}
          </span>
          <pre dir="ltr">{v}</pre>
        </div>
      ))}
      {rest && (
        <div className="trace-block">
          <span className="trace-label">{t("ورودی", "Input")}</span>
          <pre dir="ltr">{rest}</pre>
        </div>
      )}
      {item.output && (
        <div className="trace-block">
          <span className="trace-label">{t("خروجی", "Output")}</span>
          <pre dir="auto">{item.output}</pre>
        </div>
      )}
      {item.error && (
        <div className="trace-block">
          <span className="trace-label">{t("خطا", "Error")}</span>
          <pre dir="auto" className="trace-error">
            {item.error}
          </pre>
        </div>
      )}
    </li>
  );
}

function Item({ item, first }: { item: TraceItem; first: boolean }) {
  switch (item.kind) {
    case "prompt":
      return (
        <li className="trace-item prompt">
          <div className="trace-head">
            <span className="trace-kind">{first ? t("پیام به دستیار", "Message to the assistant") : t("پیام بعدی به دستیار", "Next message to the assistant")}</span>
            <span className="trace-time">{clock(item.at)}</span>
          </div>
          <details open={first}>
            <summary>{first ? t("متن کامل فرستاده‌شده", "Full text sent") : t("بازخورد خودکار یا تلاش دوباره", "Automatic feedback or retry")}</summary>
            <pre dir="auto">{item.text}</pre>
          </details>
        </li>
      );
    case "think":
      return (
        <li className="trace-item think">
          <div className="trace-head">
            <span className="trace-kind">{t("فکر کردن", "Thinking")}</span>
            <span className="trace-time">{clock(item.at)}</span>
          </div>
          <BidiText text={item.text} />
        </li>
      );
    case "text":
      return (
        <li className="trace-item text">
          <div className="trace-head">
            <span className="trace-kind">{item.synthetic ? t("متن خودکار", "Automatic text") : t("متن پاسخ", "Answer text")}</span>
            <span className="trace-time">{clock(item.at)}</span>
          </div>
          <div className="trace-md" dir="auto">
            <Markdown text={item.text} />
          </div>
        </li>
      );
    case "tool":
      return <ToolItem item={item} />;
    case "file":
      return (
        <li className="trace-item file">
          <div className="trace-head">
            <span className="trace-kind">{t("فایل", "File")}</span>
            <code dir="ltr">{item.text}</code>
          </div>
        </li>
      );
    case "error":
      return (
        <li className="trace-item failed">
          <div className="trace-head">
            <span className="trace-kind">{t("خطای موتور", "Engine error")}</span>
            <span className="trace-time">{clock(item.at)}</span>
          </div>
          <pre dir="auto" className="trace-error">
            {item.text}
          </pre>
        </li>
      );
  }
}

export function Trace({ messageId }: { messageId: number }) {
  const [items, setItems] = useState<TraceItem[] | null>(null);
  const [error, setError] = useState("");

  useEffect(() => {
    let live = true;
    setItems(null);
    setError("");
    api
      .adminTrace(messageId)
      .then((r) => live && setItems(r.trace))
      .catch((e) => live && setError(e instanceof Error && e.message ? e.message : t("بارگذاری جزئیات ناموفق بود.", "Could not load the details.")));
    return () => {
      live = false;
    };
  }, [messageId]);

  if (error)
    return (
      <div className="form-error" role="alert">
        {error}
      </div>
    );
  if (items === null)
    return (
      <div className="list-loading" role="status">
        <span className="spinner" />
        <span className="sr-only">{t("در حال بارگذاری", "Loading")}</span>
      </div>
    );
  if (items.length === 0) return <p className="muted">{t("جزئیاتی برای این پاسخ در موتور دستیار نمانده است.", "The assistant engine has no details left for this answer.")}</p>;
  const tools = items.filter((i) => i.kind === "tool").length;
  return (
    <div className="trace">
      <p className="muted trace-summary">
        {t(
          `${toPersianDigits(items.length)} مرحله، ${toPersianDigits(tools)} فراخوانی ابزار، به همان ترتیبی که انجام شد.`,
          `${items.length} steps, ${tools} tool calls, in the order they ran.`,
        )}
      </p>
      <ol className="trace-list">
        {items.map((item, i) => (
          <Item key={i} item={item} first={i === 0} />
        ))}
      </ol>
    </div>
  );
}
