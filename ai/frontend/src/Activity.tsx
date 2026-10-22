import { useEffect, useState, type ReactNode } from "react";
import type { ThinkStep, ToolStep } from "./api";
import { BidiText } from "./Bidi";
import { isFa, t } from "./i18n";
import {
  IconAlert,
  IconBook,
  IconCheck,
  IconChevronDown,
  IconCode,
  IconFolder,
  IconLink,
  IconPackage,
  IconPlay,
  IconSearch,
  IconSpark,
  IconTag,
} from "./icons";
import { toPersianDigits } from "./theme";

const ICONS: Record<string, (p: { width?: number; height?: number }) => ReactNode> = {
  search: IconSearch,
  read: IconBook,
  files: IconFolder,
  check: IconCheck,
  run: IconPlay,
  package: IconPackage,
  example: IconCode,
  keyword: IconTag,
  code: IconCode,
  tool: IconSpark,
};

function seconds(ms: number): string {
  const s = Math.max(1, Math.round(ms / 1000));
  if (s < 60) return t(`${toPersianDigits(s)} ثانیه`, `${s}s`);
  const m = Math.floor(s / 60);
  const r = s % 60;
  return r ? t(`${toPersianDigits(m)} دقیقه و ${toPersianDigits(r)} ثانیه`, `${m}m ${r}s`) : t(`${toPersianDigits(m)} دقیقه`, `${m}m`);
}

function lastLine(text: string): string {
  const lines = text.trim().split("\n").filter((l) => l.trim());
  const last = lines[lines.length - 1] || "";
  return last.length > 160 ? `…${last.slice(-160)}` : last;
}

function useElapsed(active: boolean, createdSec: number): number {
  const [now, setNow] = useState(() => Date.now());
  useEffect(() => {
    if (!active) return;
    const t = window.setInterval(() => setNow(Date.now()), 1000);
    return () => window.clearInterval(t);
  }, [active]);
  return Math.max(0, now - createdSec * 1000);
}

function StepStatus({ s }: { s: ToolStep }) {
  if (s.status === "running") return <span className="spinner tiny" aria-label={t("در حال انجام", "In progress")} />;
  if (s.status === "error")
    return (
      <span className="step-status error" title={t("ناموفق", "Failed")}>
        <IconAlert width={14} height={14} />
      </span>
    );
  return (
    <span className="step-status ok" title={t("انجام شد", "Done")}>
      <IconCheck width={14} height={14} />
    </span>
  );
}

function Thought({ text, live }: { text: string; live: boolean }) {
  const [more, setMore] = useState(false);
  const long = text.length > 320;
  return (
    <div className={`thought ${live ? "live" : ""}`}>
      <BidiText text={text} className={long && !more && !live ? "clamped" : undefined} />
      {long && !live && (
        <button type="button" className="link small-link" onClick={() => setMore((v) => !v)}>
          {more ? t("کمتر", "Less") : t("ادامه", "More")}
        </button>
      )}
    </div>
  );
}

export function ActivityPanel({
  steps,
  pending,
  ms,
  created,
}: {
  steps: (ThinkStep | ToolStep)[];
  pending: boolean;
  ms?: number;
  created: number;
}) {
  const [open, setOpen] = useState(false);
  const elapsed = useElapsed(pending, created);
  if (!pending && steps.length === 0) return null;
  const tools = steps.filter((s): s is ToolStep => s.kind === "tool");
  const sources = tools.filter((t) => t.status !== "error").length;
  const last = steps[steps.length - 1];
  let headline: string;
  if (pending) {
    if (last?.kind === "tool" && last.status === "running") headline = isFa ? last.title : last.title_en || last.title;
    else if (last?.kind === "tool") headline = t("در حال جمع‌بندی", "Wrapping up");
    else headline = t("در حال فکر کردن", "Thinking");
  } else {
    const took = ms && ms > 0 ? seconds(ms) : "";
    headline = took ? t(`${took} فکر کرد`, `Thought for ${took}`) : t("فکر کرد", "Thought");
    if (sources > 0) headline += t(`، ${toPersianDigits(sources)} مرحله‌ی بررسی`, `, ${sources} ${sources === 1 ? "lookup" : "lookups"}`);
  }
  const liveThought = pending && last?.kind === "think" ? lastLine(last.text) : "";
  return (
    <div className={`activity ${pending ? "pending" : ""} ${open ? "open" : ""}`}>
      <button type="button" className="activity-head" aria-expanded={open} onClick={() => setOpen((v) => !v)}>
        <span className="activity-icon">{pending ? <span className="spinner tiny" /> : <IconSpark width={16} height={16} />}</span>
        <span className="activity-title">{headline}</span>
        {pending && <span className="activity-time">{seconds(elapsed)}</span>}
        {steps.length > 0 && <IconChevronDown width={16} height={16} className="activity-chevron" />}
      </button>
      {!open && liveThought && (
        <p className="activity-live" dir="auto">
          {liveThought}
        </p>
      )}
      {open && steps.length > 0 && (
        <ol className="activity-steps">
          {steps.map((s, i) => {
            const isLast = i === steps.length - 1;
            if (s.kind === "think")
              return (
                <li key={i} className="step think">
                  <span className="step-icon">
                    <IconSpark width={15} height={15} />
                  </span>
                  <div className="step-body">
                    <span className="step-title">{t("فکر کردن", "Thinking")}</span>
                    <Thought text={s.text} live={pending && isLast} />
                  </div>
                </li>
              );
            const Icon = ICONS[s.icon] || IconSpark;
            return (
              <li key={i} className={`step tool ${s.status}`}>
                <span className="step-icon">
                  <Icon width={15} height={15} />
                </span>
                <div className="step-body">
                  <span className="step-title">
                    {isFa ? s.title : s.title_en || s.title}
                    {s.status === "error" && <span className="step-failed">{t(" (ناموفق)", " (failed)")}</span>}
                  </span>
                  {s.detail && (
                    <span className="step-detail" dir="auto">
                      {isFa ? s.detail : s.detail_en || s.detail}
                    </span>
                  )}
                  {s.link && (
                    <a className="step-link" href={s.link} target="_blank" rel="noreferrer noopener">
                      <IconLink width={13} height={13} />
                      {t("مشاهده‌ی منبع", "View source")}
                    </a>
                  )}
                </div>
                <StepStatus s={s} />
              </li>
            );
          })}
        </ol>
      )}
    </div>
  );
}
