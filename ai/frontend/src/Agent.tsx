import { useCallback, useEffect, useLayoutEffect, useRef, useState } from "react";
import { api, getToken } from "./api";
import { agent, folderName, onAgent, type AgentAnswer, type AgentCall, type AgentInfo, type AgentMode } from "./desktop";
import { IconAlert, IconChevronDown, IconClose, IconFile, IconFolder, IconSearch, IconTerminal } from "./icons";
import { useHighlighted } from "./Markdown";

export type CallView = AgentCall & { mid?: number };

const MAX_OUTPUT = 40000;

const TOOL_LABEL: Record<string, string> = {
  shell: "اجرای فرمان",
  run_tests: "اجرای تست‌ها",
  docker: "داکر",
  wsl: "WSL",
  read_file: "خواندن فایل",
  write_file: "نوشتن فایل",
  edit_file: "ویرایش فایل",
  list_dir: "فهرست پوشه",
  glob: "یافتن فایل‌ها",
  grep: "جست‌وجو در فایل‌ها",
};

const STATE_LABEL: Record<AgentCall["state"], string> = {
  pending: "منتظر اجازه‌ی شما",
  running: "در حال اجرا",
  done: "انجام شد",
  error: "خطا",
  denied: "رد شد",
};

export const MODE_LABEL: Record<AgentMode, string> = {
  ask: "پیش از هر تغییر بپرس",
  auto: "خودکار در پوشه‌ی کاری",
  readonly: "فقط خواندنی",
};

function clipOutput(s: string): string {
  return s.length > MAX_OUTPUT ? `…${s.slice(s.length - MAX_OUTPUT)}` : s;
}

function upsert(list: CallView[], c: CallView): CallView[] {
  const i = list.findIndex((x) => x.call_id === c.call_id);
  if (i < 0) return [...list, c];
  const old = list[i];
  const output = c.output || (c.state === "running" ? old.output : c.output);
  const next = { ...old, ...c, mid: old.mid ?? c.mid, output };
  return [...list.slice(0, i), next, ...list.slice(i + 1)];
}

export function useAgent(enabled: boolean, midFor: (chat: string) => number | undefined, onError: (msg: string) => void) {
  const [info, setInfo] = useState<AgentInfo | null>(null);
  const [calls, setCalls] = useState<CallView[]>([]);
  const midRef = useRef(midFor);
  midRef.current = midFor;
  const errRef = useRef(onError);
  errRef.current = onError;

  useEffect(() => {
    if (!enabled) return;
    let alive = true;
    const ensure = async () => {
      try {
        let current = await agent.info();
        const mine = await api.devices().catch(() => null);
        const known = current.device && (!mine || mine.devices.some((d) => String(d.id) === current.device?.id));
        if (!known) current = await agent.link(getToken());
        if (alive) setInfo(current);
      } catch {
        if (alive) errRef.current("وصل کردن این رایانه به دستیار ناموفق بود. برنامه را دوباره باز کنید.");
      }
    };
    ensure();
    agent
      .pending()
      .then((ps) => {
        if (!alive) return;
        setCalls((cs) => ps.reduce((acc, p) => upsert(acc, { ...p, state: "pending", output: "", mid: midRef.current(p.chat) }), cs));
      })
      .catch(() => {});
    const off = onAgent({
      call: (c) => setCalls((cs) => upsert(cs, { ...c, mid: midRef.current(c.chat) })),
      output: (id, chunk) => setCalls((cs) => cs.map((c) => (c.call_id === id ? { ...c, output: clipOutput(c.output + chunk) } : c))),
      connection: (connected) => setInfo((i) => (i ? { ...i, connected } : i)),
      device: () => {
        ensure();
      },
    });
    return () => {
      alive = false;
      off();
    };
  }, [enabled]);

  const decide = useCallback((id: string, answer: AgentAnswer) => {
    agent.decide(id, answer).catch(() => {});
  }, []);

  const stop = useCallback((id: string) => {
    agent.stop(id).catch(() => {});
  }, []);

  return { info, calls, decide, stop };
}

function DiffView({ text }: { text: string }) {
  return (
    <pre className="tool-code diff" dir="ltr">
      {text.split("\n").map((line, i) => {
        const cls = line.startsWith("+++") || line.startsWith("---") ? "d-file" : line.startsWith("@@") ? "d-hunk" : line.startsWith("+") ? "d-add" : line.startsWith("-") ? "d-del" : undefined;
        return (
          <span key={i} className={cls}>
            {line}
            {"\n"}
          </span>
        );
      })}
    </pre>
  );
}

function CodeView({ text, lang }: { text: string; lang: string }) {
  const html = useHighlighted(text, lang);
  return (
    <pre className="tool-code" dir="ltr">
      <code dangerouslySetInnerHTML={{ __html: html }} />
    </pre>
  );
}

function Output({ text, live }: { text: string; live: boolean }) {
  const ref = useRef<HTMLPreElement>(null);
  useLayoutEffect(() => {
    const el = ref.current;
    if (el && live) el.scrollTop = el.scrollHeight;
  }, [text, live]);
  return (
    <pre ref={ref} className="tool-output" dir="ltr">
      {text}
    </pre>
  );
}

function ToolIcon({ c }: { c: AgentCall }) {
  if (c.kind === "exec") return <IconTerminal width={16} height={16} />;
  if (c.tool === "glob" || c.tool === "grep") return <IconSearch width={16} height={16} />;
  if (c.tool === "list_dir") return <IconFolder width={16} height={16} />;
  return <IconFile width={16} height={16} />;
}

export function ToolCard({ c, onDecide, onStop }: { c: CallView; onDecide: (id: string, a: AgentAnswer) => void; onStop: (id: string) => void }) {
  const [open, setOpen] = useState(c.state === "pending" || c.state === "running");
  useEffect(() => {
    if (c.state === "pending" || c.state === "running") setOpen(true);
  }, [c.state]);
  const live = c.state === "running";
  const showDetail = c.detail !== "" && c.kind !== "read";
  return (
    <div className={`tool-card ${c.state}`}>
      <button type="button" className="tool-head" onClick={() => setOpen((o) => !o)} aria-expanded={open}>
        <span className="tool-icon">
          <ToolIcon c={c} />
        </span>
        <span className="tool-label">{TOOL_LABEL[c.tool] || c.tool}</span>
        <code className="tool-summary" dir="ltr">
          {c.summary}
        </code>
        <span className={`tool-state ${c.state}`}>
          {live && <span className="spinner tiny" />}
          {STATE_LABEL[c.state]}
        </span>
        <IconChevronDown className={`tool-chevron${open ? " open" : ""}`} width={16} height={16} />
      </button>
      {open && (showDetail || c.output || c.outside || c.kind === "exec") && (
        <div className="tool-body">
          {c.outside && (
            <p className="tool-warn">
              <IconAlert width={16} height={16} />
              این کار بیرون از پوشه‌ی کاری انجام می‌شود.
            </p>
          )}
          {c.kind === "exec" && c.cwd && (
            <p className="tool-cwd">
              پوشه: <bdi dir="ltr">{c.cwd}</bdi>
            </p>
          )}
          {showDetail && (c.lang === "diff" ? <DiffView text={c.detail} /> : <CodeView text={c.detail} lang={c.lang} />)}
          {c.output && c.state !== "pending" && <Output text={c.output} live={live} />}
        </div>
      )}
      {c.state === "pending" && (
        <div className="tool-actions">
          <button type="button" className="button small primary" onClick={() => onDecide(c.call_id, "allow")}>
            اجازه بده
          </button>
          {!c.outside && (
            <button type="button" className="button small ghost" onClick={() => onDecide(c.call_id, "always")}>
              همیشه در این گفت‌وگو
            </button>
          )}
          <button type="button" className="button small danger" onClick={() => onDecide(c.call_id, "deny")}>
            رد کن
          </button>
        </div>
      )}
      {live && c.kind === "exec" && (
        <div className="tool-actions">
          <button type="button" className="button small ghost" onClick={() => onStop(c.call_id)}>
            توقف
          </button>
        </div>
      )}
    </div>
  );
}

type BarProps = {
  info: AgentInfo | null;
  workspace: string;
  locked: boolean;
  elsewhere: boolean;
  mode: AgentMode;
  onPick: () => void;
  onClear: () => void;
  onMode: (m: AgentMode) => void;
};

export function AgentBar({ info, workspace, locked, elsewhere, mode, onPick, onClear, onMode }: BarProps) {
  const online = info?.connected ?? false;
  return (
    <div className="agent-bar">
      <span className={`agent-dot${online ? " on" : ""}`} title={online ? `وصل به ${info?.device?.name ?? "این رایانه"}` : "در حال اتصال"} />
      {workspace ? (
        <span className="agent-ws" title={workspace}>
          <IconFolder width={15} height={15} />
          <bdi dir="ltr">{folderName(workspace)}</bdi>
          {!locked && (
            <button type="button" className="agent-ws-clear" onClick={onClear} aria-label="برداشتن پوشه">
              <IconClose width={14} height={14} />
            </button>
          )}
        </span>
      ) : (
        <button type="button" className="agent-ws pick" onClick={onPick}>
          <IconFolder width={15} height={15} />
          انتخاب پوشه‌ی کاری
        </button>
      )}
      {workspace && (
        <select className="agent-mode" value={mode} onChange={(e) => onMode(e.target.value as AgentMode)} aria-label="سطح دسترسی">
          {(Object.keys(MODE_LABEL) as AgentMode[]).map((m) => (
            <option key={m} value={m}>
              {MODE_LABEL[m]}
            </option>
          ))}
        </select>
      )}
      {elsewhere && <span className="agent-note">این گفت‌وگو به رایانه‌ی دیگری وصل است.</span>}
    </div>
  );
}
