export type Lang = "fa" | "en";

const KEY = "salam-ai-lang";

function detect(): Lang {
  try {
    const asked = new URLSearchParams(location.search).get("lang");
    if (asked === "fa" || asked === "en") {
      localStorage.setItem(KEY, asked);
      return asked;
    }
    const saved = localStorage.getItem(KEY);
    if (saved === "fa" || saved === "en") return saved;
  } catch {}
  const langs = navigator.languages?.length
    ? navigator.languages
    : [navigator.language];
  if (langs.some((l) => l?.toLowerCase().startsWith("fa"))) return "fa";
  try {
    if (Intl.DateTimeFormat().resolvedOptions().timeZone === "Asia/Tehran")
      return "fa";
  } catch {}
  return "en";
}

export const lang: Lang = detect();
export const isFa = lang === "fa";
export const locale = isFa ? "fa-IR" : "en-US";
export const dir = isFa ? "rtl" : "ltr";

export function t(fa: string, en: string): string {
  return isFa ? fa : en;
}

export function setLang(next: Lang) {
  try {
    localStorage.setItem(KEY, next);
  } catch {}
  const url = new URL(location.href);
  url.searchParams.delete("lang");
  location.replace(url.toString());
}

export function applyDocumentLang() {
  document.documentElement.lang = lang;
  document.documentElement.dir = dir;
  document.title = t("دستیار هوشمند سلام", "Salam AI");
}
