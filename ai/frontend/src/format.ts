import { locale, t } from "./i18n";

const numberFmt = new Intl.NumberFormat(locale);
const dateFmt = new Intl.DateTimeFormat(locale, {
  year: "numeric",
  month: "long",
  day: "numeric",
});
const dateTimeFmt = new Intl.DateTimeFormat(locale, {
  year: "numeric",
  month: "long",
  day: "numeric",
  hour: "2-digit",
  minute: "2-digit",
});
const dayFmt = new Intl.DateTimeFormat(locale, {
  month: "short",
  day: "numeric",
});
const monthFmt = new Intl.DateTimeFormat(locale, {
  month: "long",
  year: "numeric",
  timeZone: "Asia/Tehran",
});
const compactFmt = new Intl.NumberFormat(locale, {
  notation: "compact",
  maximumFractionDigits: 1,
});
const costFmt = new Intl.NumberFormat(locale, {
  style: "currency",
  currency: "USD",
  maximumFractionDigits: 4,
});
const bigCostFmt = new Intl.NumberFormat(locale, {
  style: "currency",
  currency: "USD",
  maximumFractionDigits: 2,
});
const relFmt = new Intl.RelativeTimeFormat(locale, { numeric: "auto" });

export function fmtNum(n: number): string {
  return numberFmt.format(n);
}

export function fmtCompact(n: number): string {
  return Math.abs(n) < 10000 ? numberFmt.format(n) : compactFmt.format(n);
}

export function fmtCost(n: number): string {
  return n >= 1 ? bigCostFmt.format(n) : costFmt.format(n);
}

export function fmtMonth(ts: number): string {
  return ts ? monthFmt.format(ts * 1000) : "";
}

export type TokenCounts = {
  input: number;
  output: number;
  reasoning: number;
  cache_read: number;
  cache_write: number;
};

export function tokenSplit(u: TokenCounts) {
  const cache = u.cache_read + u.cache_write;
  const input = u.input + cache;
  const output = u.output + u.reasoning;
  return { input, output, cache, total: input + output };
}

export function fmtDate(ts: number): string {
  return ts ? dateFmt.format(ts * 1000) : "";
}

export function fmtDateTime(ts: number): string {
  return ts ? dateTimeFmt.format(ts * 1000) : "";
}

export function fmtDay(day: string): string {
  const d = new Date(`${day}T12:00:00`);
  return Number.isNaN(d.getTime()) ? day : dayFmt.format(d);
}

const UNITS: [Intl.RelativeTimeFormatUnit, number][] = [
  ["year", 365 * 86400],
  ["month", 30 * 86400],
  ["week", 7 * 86400],
  ["day", 86400],
  ["hour", 3600],
  ["minute", 60],
];

export function fmtRelative(ts: number): string {
  if (!ts) return t("هرگز", "never");
  const diff = ts - Date.now() / 1000;
  const abs = Math.abs(diff);
  if (abs < 45) return t("همین الان", "just now");
  for (const [unit, size] of UNITS) {
    if (abs >= size) return relFmt.format(Math.round(diff / size), unit);
  }
  return relFmt.format(Math.round(diff / 60), "minute");
}

export function fmtDuration(ms: number): string {
  if (ms < 1000)
    return t(
      `${fmtNum(Math.round(ms))} میلی‌ثانیه`,
      `${fmtNum(Math.round(ms))} ms`,
    );
  return t(
    `${fmtNum(Math.round(ms / 100) / 10)} ثانیه`,
    `${fmtNum(Math.round(ms / 100) / 10)} s`,
  );
}

export function prettyArgs(args: string): string {
  if (!args) return "";
  try {
    return JSON.stringify(JSON.parse(args), null, 2);
  } catch {
    return args;
  }
}

export function initial(name: string): string {
  return name.trim().charAt(0).toUpperCase() || "?";
}
