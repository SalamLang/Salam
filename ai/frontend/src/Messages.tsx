import { useEffect, useRef, useState } from "react";
import { ToolCard, type CallView } from "./Agent";
import { uiText, type ActivityStep, type Message, type ThinkStep, type ToolStep } from "./api";
import type { AgentAnswer } from "./desktop";
import { ActivityPanel } from "./Activity";
import { MessageFiles } from "./Attach";
import { CopyButton, Markdown } from "./Markdown";
import { t } from "./i18n";

const PHASE_TEXT: Record<string, string> = {
  thinking: t("در حال فکر کردن", "Thinking"),
  searching: t("در حال بررسی مستندات سلام", "Checking the Salam docs"),
  working: t("در حال کار روی رایانه‌ی شما", "Working on your computer"),
  writing: t("در حال نوشتن", "Writing"),
  checking: t("در حال اصلاح کد با کامپایلر سلام", "Fixing the code with the Salam compiler"),
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

type Tools = {
  calls?: CallView[];
  onDecide?: (id: string, a: AgentAnswer) => void;
  onStop?: (id: string) => void;
};

type BoxGroup = { kind: "boxes"; items: (ThinkStep | ToolStep)[] };
type TextGroup = { kind: "text"; text: string };
type CallItem = { kind: "call"; call: CallView };
type CallsGroup = { kind: "calls"; calls: CallView[] };
type TimelineItem = BoxGroup | TextGroup | CallItem | CallsGroup;

const DESKTOP_TOOLS = new Set([
  "shell",
  "read_file",
  "write_file",
  "edit_file",
  "list_dir",
  "glob",
  "grep",
  "docker",
  "wsl",
  "run_tests",
]);

function withCalls(m: Message, live: CallView[]): CallView[] {
  const all = new Map<string, CallView>();
  for (const c of m.calls ?? []) all.set(c.call_id, c as CallView);
  for (const c of live) all.set(c.call_id, c);
  return [...all.values()];
}

function timeline(steps: ActivityStep[], body: string, calls: CallView[]): TimelineItem[] {
  const items: TimelineItem[] = [];
  const used = new Set<string>();
  let boxes: (ThinkStep | ToolStep)[] = [];
  let sawText = false;
  const flush = () => {
    if (boxes.length > 0) {
      items.push({ kind: "boxes", items: boxes });
      boxes = [];
    }
  };
  for (const s of steps) {
    if (s.kind === "text") {
      flush();
      sawText = true;
      items.push({ kind: "text", text: s.text });
      continue;
    }
    if (s.kind === "tool" && s.tool && DESKTOP_TOOLS.has(s.tool)) {
      const call = calls.find((c) => !used.has(c.call_id) && c.tool === s.tool);
      if (call) {
        used.add(call.call_id);
        flush();
        items.push({ kind: "call", call });
        continue;
      }
    }
    boxes.push(s);
  }
  flush();
  if (!sawText && body) items.push({ kind: "text", text: body });
  const rest = calls.filter((c) => !used.has(c.call_id));
  if (rest.length > 0) items.push({ kind: "calls", calls: rest });
  return items;
}

function TextGroupView({ text, live }: { text: string; live: boolean }) {
  const shown = useSmoothText(text, live);
  const typing = live && shown.length < text.length;
  const visible = typing ? shown.replace(/(?:^|\n)[ \t]*`{1,3}[^\n`]*$/, "") : shown;
  if (!visible) return null;
  return (
    <div className={typing ? "streaming" : undefined}>
      <Markdown text={visible} />
    </div>
  );
}

function AssistantMessage({ m, calls = [], onDecide = () => {}, onStop = () => {} }: { m: Message } & Tools) {
  const pending = m.status === "pending";
  const phase = m.phase || "thinking";
  const all = withCalls(m, calls);
  const waiting = all.some((c) => c.state === "pending" || c.state === "running");
  const steps = m.activity ?? [];
  const items = timeline(steps, uiText(m.body), all);
  const showPhase = pending && all.length > 0 && !waiting && m.body.length === 0;
  return (
    <div className={`msg assistant ${m.status}`}>
      <img src="/logo.svg" alt="" className="msg-avatar" width={28} height={28} />
      <div className="msg-body">
        {items.map((g, i) => {
          const isLast = i === items.length - 1;
          if (g.kind === "text") return <TextGroupView key={i} text={g.text} live={pending && isLast} />;
          if (g.kind === "call")
            return (
              <div key={i} className="tool-list">
                <ToolCard c={g.call} onDecide={onDecide} onStop={onStop} />
              </div>
            );
          if (g.kind === "calls")
            return (
              <div key={i} className="tool-list">
                {g.calls.map((c) => (
                  <ToolCard key={c.call_id} c={c} onDecide={onDecide} onStop={onStop} />
                ))}
              </div>
            );
          return <ActivityPanel key={i} steps={g.items} pending={pending && isLast} ms={isLast ? m.ms : undefined} created={m.created} />;
        })}
        {showPhase && (
          <div className="phase" aria-live="polite">
            <span className="dots">
              <i />
              <i />
              <i />
            </span>
            {all.length > 0 ? PHASE_TEXT.working : PHASE_TEXT[phase] || PHASE_TEXT.thinking}
          </div>
        )}
        {!pending && m.body && (
          <div className="msg-tools">
            <CopyButton text={uiText(m.body)} label={t("رونوشت پاسخ", "Copy answer")} />
          </div>
        )}
      </div>
    </div>
  );
}

export function MessageView({ m, name, loadFile, ...tools }: { m: Message; name: string; loadFile?: (id: number) => Promise<string> } & Tools) {
  if (m.role === "user") {
    return (
      <div className="msg user">
        <div className="msg-user-stack">
          {m.files && m.files.length > 0 && <MessageFiles files={m.files} load={loadFile ?? (() => Promise.reject(new Error("no loader")))} />}
          {m.body && (
            <div className="bubble" dir="auto">
              {m.body}
            </div>
          )}
        </div>
        <span className="sr-only">{name}</span>
      </div>
    );
  }
  return <AssistantMessage m={m} {...tools} />;
}
