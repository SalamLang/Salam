import { useEffect, useRef, useState } from "react";
import { ToolCard, type CallView } from "./Agent";
import type { ActivityStep, Message, ThinkStep, ToolStep } from "./api";
import type { AgentAnswer } from "./desktop";
import { ActivityPanel } from "./Activity";
import { CopyButton, Markdown } from "./Markdown";

const PHASE_TEXT: Record<string, string> = {
  thinking: "در حال فکر کردن",
  searching: "در حال بررسی مستندات سلام",
  working: "در حال کار روی رایانه‌ی شما",
  writing: "در حال نوشتن",
  checking: "در حال اصلاح کد با کامپایلر سلام",
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

function groupSteps(steps: ActivityStep[], body: string): (BoxGroup | TextGroup)[] {
  const groups: (BoxGroup | TextGroup)[] = [];
  let sawText = false;
  for (const s of steps) {
    if (s.kind === "text") {
      sawText = true;
      groups.push({ kind: "text", text: s.text });
      continue;
    }
    const last = groups[groups.length - 1];
    if (last && last.kind === "boxes") last.items.push(s);
    else groups.push({ kind: "boxes", items: [s] });
  }
  if (!sawText && body) groups.push({ kind: "text", text: body });
  return groups;
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
  const waiting = calls.some((c) => c.state === "pending" || c.state === "running");
  const steps = m.activity ?? [];
  const groups = groupSteps(steps, m.body);
  const showPhase = pending && calls.length > 0 && !waiting && m.body.length === 0;
  return (
    <div className={`msg assistant ${m.status}`}>
      <img src="/logo.svg" alt="" className="msg-avatar" width={28} height={28} />
      <div className="msg-body">
        {groups.map((g, i) => {
          const isLast = i === groups.length - 1;
          if (g.kind === "text") return <TextGroupView key={i} text={g.text} live={pending && isLast} />;
          return <ActivityPanel key={i} steps={g.items} pending={pending && isLast} ms={isLast ? m.ms : undefined} created={m.created} />;
        })}
        {calls.length > 0 && (
          <div className="tool-list">
            {calls.map((c) => (
              <ToolCard key={c.call_id} c={c} onDecide={onDecide} onStop={onStop} />
            ))}
          </div>
        )}
        {showPhase && (
          <div className="phase" aria-live="polite">
            <span className="dots">
              <i />
              <i />
              <i />
            </span>
            {calls.length > 0 ? PHASE_TEXT.working : PHASE_TEXT[phase] || PHASE_TEXT.thinking}
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

export function MessageView({ m, name, ...tools }: { m: Message; name: string } & Tools) {
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
  return <AssistantMessage m={m} {...tools} />;
}
