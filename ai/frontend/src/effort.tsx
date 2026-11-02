import { useSyncExternalStore } from "react";
import { t } from "./i18n";
import { IconBolt, IconBrain, IconGauge } from "./icons";
import { Select, type SelectOption } from "./Select";

export type Effort = "low" | "medium" | "high";

const KEY = "salam-ai-effort";
const listeners = new Set<() => void>();

function read(): Effort {
  try {
    const v = localStorage.getItem(KEY);
    if (v === "low" || v === "high") return v;
  } catch {}
  return "medium";
}

let current: Effort = read();

export function getEffort(): Effort {
  return current;
}

export function setEffort(e: Effort) {
  current = e;
  try {
    localStorage.setItem(KEY, e);
  } catch {}
  for (const fn of listeners) fn();
}

function subscribe(fn: () => void) {
  listeners.add(fn);
  return () => listeners.delete(fn);
}

export function useEffort(): Effort {
  return useSyncExternalStore(subscribe, getEffort, getEffort);
}

export function effortLabel(e: Effort): string {
  if (e === "low") return t("سریع", "Fast");
  if (e === "high") return t("عمیق", "Deep");
  return t("متعادل", "Balanced");
}

export function EffortIcon({ effort, size = 16 }: { effort: Effort; size?: number }) {
  if (effort === "low") return <IconBolt width={size} height={size} />;
  if (effort === "high") return <IconBrain width={size} height={size} />;
  return <IconGauge width={size} height={size} />;
}

export function EffortPicker({ disabled }: { disabled?: boolean }) {
  const effort = useEffort();
  const options: SelectOption<Effort>[] = [
    {
      value: "low",
      label: effortLabel("low"),
      hint: t("پاسخ کوتاه و فوری، برای کارهای ساده.", "Short, fast answers for simple tasks."),
      icon: <EffortIcon effort="low" />,
    },
    {
      value: "medium",
      label: effortLabel("medium"),
      hint: t("حالت پیش‌فرض برای بیشتر کارها.", "The default for most work."),
      icon: <EffortIcon effort="medium" />,
    },
    {
      value: "high",
      label: effortLabel("high"),
      hint: t("فکر بیشتر و بررسی دقیق، برای ریاضی، استدلال و کارهای سخت.", "Thinks longer and checks its work, for math, reasoning and hard tasks."),
      icon: <EffortIcon effort="high" />,
    },
  ];
  return (
    <Select
      className={`effort-pick effort-${effort}`}
      value={effort}
      onChange={setEffort}
      options={options}
      disabled={disabled}
      ariaLabel={t("سطح فکر کردن", "Thinking level")}
      title={t("سطح فکر کردن برای پیام بعدی", "Thinking level for the next message")}
    />
  );
}

export function EffortBadge({ effort }: { effort?: string }) {
  if (effort !== "low" && effort !== "high") return null;
  return (
    <span className={`effort-badge effort-${effort}`} title={t("سطح فکر کردن", "Thinking level")}>
      <EffortIcon effort={effort} size={12} />
      {effortLabel(effort)}
    </span>
  );
}
