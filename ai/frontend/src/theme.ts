import { useEffect, useState } from "react";
import { isFa } from "./i18n";

export type Theme = "light" | "dark";
const KEY = "salam-ai-theme";

function initial(): Theme {
  const t = document.documentElement.dataset.theme;
  return t === "dark" ? "dark" : "light";
}

export function useTheme(): [Theme, () => void] {
  const [theme, setTheme] = useState<Theme>(initial);
  useEffect(() => {
    document.documentElement.dataset.theme = theme;
    document
      .querySelector('meta[name="theme-color"]')
      ?.setAttribute("content", "#ff5c00");
  }, [theme]);
  const toggle = () => {
    setTheme((t) => {
      const next = t === "dark" ? "light" : "dark";
      try {
        localStorage.setItem(KEY, next);
      } catch {}
      return next;
    });
  };
  return [theme, toggle];
}

export function toPersianDigits(s: string | number): string {
  if (!isFa) return String(s);
  return String(s).replace(/[0-9]/g, (d) => "۰۱۲۳۴۵۶۷۸۹"[Number(d)]);
}
