import { useSyncExternalStore } from "react";
import { createPortal } from "react-dom";
import { t } from "./i18n";
import { IconAlert, IconBell, IconCheck, IconClose, IconSpark } from "./icons";

export type ToastTone = "done" | "failed" | "network" | "approval" | "info";

type Item = {
  id: number;
  tone: ToastTone;
  title: string;
  body?: string;
};

let items: Item[] = [];
let next = 1;
let sound = true;
const listeners = new Set<() => void>();
const timers = new Map<number, number>();

function emit() {
  for (const listener of listeners) listener();
}

function subscribe(listener: () => void) {
  listeners.add(listener);
  return () => {
    listeners.delete(listener);
  };
}

function snapshot() {
  return items;
}

export function pushToast(input: {
  tone?: ToastTone;
  title: string;
  body?: string;
  seconds?: number;
}): number {
  const id = next++;
  const tone = input.tone ?? "info";
  items = [...items, { id, tone, title: input.title, body: input.body }].slice(
    -4,
  );
  emit();
  const seconds = input.seconds ?? (tone === "approval" ? 12 : 6);
  if (seconds > 0) {
    timers.set(
      id,
      window.setTimeout(() => dismissToast(id), seconds * 1000),
    );
  }
  return id;
}

export function dismissToast(id: number) {
  const timer = timers.get(id);
  if (timer !== undefined) {
    window.clearTimeout(timer);
    timers.delete(id);
  }
  if (!items.some((item) => item.id === id)) return;
  items = items.filter((item) => item.id !== id);
  emit();
}

export function setToastSound(on: boolean) {
  sound = on;
}

let audio: AudioContext | null = null;

export function chime(tone: ToastTone = "info") {
  if (!sound) return;
  try {
    const Ctor =
      window.AudioContext ??
      (window as unknown as { webkitAudioContext?: typeof AudioContext })
        .webkitAudioContext;
    if (!Ctor) return;
    audio ??= new Ctor();
    const ctx = audio;
    if (ctx.state === "suspended") void ctx.resume();
    const notes =
      tone === "failed"
        ? [392, 294]
        : tone === "approval"
          ? [523.25, 659.25]
          : tone === "done"
            ? [659.25, 987.77]
            : [783.99];
    const start = ctx.currentTime + 0.01;
    for (const [i, hz] of notes.entries()) {
      const at = start + i * 0.12;
      const osc = ctx.createOscillator();
      const gain = ctx.createGain();
      osc.type = "sine";
      osc.frequency.setValueAtTime(hz, at);
      gain.gain.setValueAtTime(0.0001, at);
      gain.gain.exponentialRampToValueAtTime(
        tone === "failed" ? 0.1 : 0.13,
        at + 0.02,
      );
      gain.gain.exponentialRampToValueAtTime(0.0001, at + 0.34);
      osc.connect(gain);
      gain.connect(ctx.destination);
      osc.start(at);
      osc.stop(at + 0.38);
    }
  } catch {}
}

function Note({ item }: { item: Item }) {
  const icon =
    item.tone === "failed" || item.tone === "network" ? (
      <IconAlert width={17} height={17} />
    ) : item.tone === "approval" ? (
      <IconBell width={17} height={17} />
    ) : item.tone === "done" ? (
      <IconCheck width={17} height={17} />
    ) : (
      <IconSpark width={17} height={17} />
    );
  return (
    <div className={`note ${item.tone}`} role="status">
      <span className="note-icon" aria-hidden="true">
        {icon}
      </span>
      <div className="note-text">
        <strong className="note-title" dir="auto">
          {item.title}
        </strong>
        {item.body && (
          <span className="note-body" dir="auto">
            {item.body}
          </span>
        )}
      </div>
      <button
        type="button"
        className="note-close"
        onClick={() => dismissToast(item.id)}
        aria-label={t("بستن اعلان", "Dismiss notification")}
      >
        <IconClose width={14} height={14} />
      </button>
    </div>
  );
}

export function ToastHost() {
  const list = useSyncExternalStore(subscribe, snapshot, snapshot);
  if (list.length === 0) return null;
  return createPortal(
    <section
      className="notes"
      aria-label={t("اعلان‌ها", "Notifications")}
      aria-live="polite"
    >
      {list.map((item) => (
        <Note key={item.id} item={item} />
      ))}
    </section>,
    document.body,
  );
}
