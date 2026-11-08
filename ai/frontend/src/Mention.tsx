import { useCallback, useEffect, useRef, useState, type KeyboardEvent, type ReactNode } from "react";
import { isDesktop, mention, type MentionHit } from "./desktop";
import { IconFile, IconFolder } from "./icons";
import { t } from "./i18n";

export type Trigger = { start: number; end: number; query: string };

export function findTrigger(text: string, caret: number): Trigger | null {
  let i = caret;
  while (i > 0) {
    const ch = text[i - 1];
    if (/\s/.test(ch)) return null;
    if (ch === "@") {
      if (i - 1 > 0 && !/\s/.test(text[i - 2])) return null;
      const query = text.slice(i, caret);
      return query.length > 200 ? null : { start: i - 1, end: caret, query };
    }
    i--;
  }
  return null;
}

export function mentionText(hit: MentionHit): string {
  const path = hit.is_dir ? `${hit.path}/` : hit.path;
  return /\s/.test(path) ? `@"${path}"` : `@${path}`;
}

function marks(text: string, offset: number, ranges: [number, number][]): ReactNode[] {
  const out: ReactNode[] = [];
  let at = 0;
  for (const [a, b] of ranges) {
    const from = Math.max(a - offset, 0);
    const to = Math.min(b - offset, text.length);
    if (to <= at || from >= text.length) continue;
    const start = Math.max(from, at);
    if (start > at) out.push(text.slice(at, start));
    out.push(<mark key={start}>{text.slice(start, to)}</mark>);
    at = to;
  }
  if (at < text.length) out.push(text.slice(at));
  return out;
}

export type MentionState = {
  active: boolean;
  open: boolean;
  hits: MentionHit[];
  indexing: boolean;
  index: number;
  trigger: Trigger | null;
  pick: (hit: MentionHit) => void;
  onKey: (e: KeyboardEvent<HTMLTextAreaElement>) => boolean;
  setIndex: (i: number) => void;
};

export function useMention(root: string, text: string, caret: number, apply: (trigger: Trigger, insert: string) => void): MentionState {
  const enabled = isDesktop && root !== "";
  const trigger = enabled ? findTrigger(text, Math.min(caret, text.length)) : null;
  const [hits, setHits] = useState<MentionHit[]>([]);
  const [indexing, setIndexing] = useState(false);
  const [index, setIndex] = useState(0);
  const [dismissed, setDismissed] = useState(-1);
  const [broken, setBroken] = useState(false);
  const order = useRef(0);
  const query = trigger ? trigger.query : null;
  const dead = broken || !enabled;

  const run = useCallback(
    (q: string) => {
      const mine = ++order.current;
      mention
        .search(root, q, 50)
        .then((r) => {
          if (mine !== order.current || r.stale) return;
          setHits(r.hits);
          setIndexing(r.indexing);
          setIndex(0);
        })
        .catch(() => {
          if (mine === order.current) setBroken(true);
        });
    },
    [root],
  );

  useEffect(() => {
    setBroken(false);
    setHits([]);
    if (enabled) mention.open(root).catch(() => setBroken(true));
  }, [root, enabled]);

  useEffect(() => {
    if (dead || query === null) {
      order.current++;
      return;
    }
    const timer = window.setTimeout(() => run(query), 40);
    return () => window.clearTimeout(timer);
  }, [dead, query, run]);

  const shown = !dead && trigger !== null && dismissed !== trigger.start;
  useEffect(() => {
    if (!shown || query === null) return;
    let timer = 0;
    const off = mention.onUpdated(() => {
      window.clearTimeout(timer);
      timer = window.setTimeout(() => run(query), 150);
    });
    return () => {
      window.clearTimeout(timer);
      off();
    };
  }, [shown, query, run]);

  useEffect(() => {
    if (trigger === null) setDismissed(-1);
  }, [trigger]);

  const open = shown && (hits.length > 0 || indexing);
  const active = shown && hits.length > 0;

  const pick = (hit: MentionHit) => {
    if (!trigger) return;
    mention.picked(root, hit.path).catch(() => {});
    apply(trigger, mentionText(hit) + (hit.is_dir ? "" : " "));
  };

  const onKey = (e: KeyboardEvent<HTMLTextAreaElement>): boolean => {
    if (!active || e.nativeEvent.isComposing || e.keyCode === 229) return false;
    if (e.key === "ArrowDown") {
      e.preventDefault();
      setIndex((i) => (i + 1) % hits.length);
      return true;
    }
    if (e.key === "ArrowUp") {
      e.preventDefault();
      setIndex((i) => (i - 1 + hits.length) % hits.length);
      return true;
    }
    if (e.key === "Enter" || e.key === "Tab") {
      if (e.shiftKey && e.key === "Enter") return false;
      e.preventDefault();
      pick(hits[Math.min(index, hits.length - 1)]);
      return true;
    }
    if (e.key === "Escape" && trigger) {
      e.preventDefault();
      setDismissed(trigger.start);
      return true;
    }
    return false;
  };

  return { active, open, hits, indexing, index, trigger, pick, onKey, setIndex };
}

export function MentionPopup({ state, id }: { state: MentionState; id: string }) {
  const list = useRef<HTMLUListElement>(null);
  useEffect(() => {
    const el = list.current?.children[state.index] as HTMLElement | undefined;
    el?.scrollIntoView({ block: "nearest" });
  }, [state.index, state.hits]);
  if (!state.open) return null;
  return (
    <div className="mention-pop">
      {state.hits.length > 0 && (
        <ul ref={list} id={id} role="listbox" aria-label={t("فایل‌ها و پوشه‌های پروژه", "Project files and folders")}>
          {state.hits.map((hit, i) => {
            const name = hit.path.slice(hit.name_at);
            const dir = hit.path.slice(0, hit.name_at);
            return (
              <li
                key={hit.path}
                id={`${id}-${i}`}
                role="option"
                aria-selected={i === state.index}
                className={i === state.index ? "on" : undefined}
                onMouseDown={(e) => {
                  e.preventDefault();
                  state.pick(hit);
                }}
                onMouseMove={() => i !== state.index && state.setIndex(i)}
              >
                <span className="mention-ico" aria-hidden="true">
                  {hit.is_dir ? <IconFolder /> : <IconFile />}
                </span>
                <bdi dir="auto" className="mention-name">
                  {marks(name, hit.name_at, hit.ranges)}
                  {hit.is_dir ? "/" : ""}
                </bdi>
                {dir && (
                  <bdi dir="auto" className="mention-dir">
                    {marks(dir, 0, hit.ranges)}
                  </bdi>
                )}
              </li>
            );
          })}
        </ul>
      )}
      {state.indexing && (
        <p className="mention-hint" role="status">
          {t("در حال فهرست‌برداری از پوشه…", "Indexing the folder…")}
        </p>
      )}
    </div>
  );
}
