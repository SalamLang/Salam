import { useState, useEffect } from "react";
import Header from "./components/Header";

export type AppMode = "app" | "layout";
export type Language = "en" | "fa";
export type AppTheme = "auto" | "light" | "dark";

export default function App() {
  const [mode, setMode] = useState<AppMode>("app");
  const [lang, setLang] = useState<Language>("en");
  const [theme, setTheme] = useState<AppTheme>("auto");
  const [autoRun, setAutoRun] = useState<boolean>(true);

  useEffect(() => {
    document.documentElement.dir = lang === "fa" ? "rtl" : "ltr";
    document.documentElement.lang = lang;
  }, [lang]);

  useEffect(() => {
    const root = document.documentElement;
    const mediaQuery = window.matchMedia("(prefers-color-scheme: dark)");

    const syncTheme = () => {
      const isDarkSystem = mediaQuery.matches;
      const shouldBeDark = theme === "dark" || (theme === "auto" && isDarkSystem);

      if (shouldBeDark) {
        root.classList.add("dark");
        root.setAttribute("data-theme", "dark");
      } else {
        root.classList.remove("dark");
        root.setAttribute("data-theme", "light");
      }
    };

    syncTheme();

    if (theme === "auto") {
      mediaQuery.addEventListener("change", syncTheme);
      return () => mediaQuery.removeEventListener("change", syncTheme);
    }
  }, [theme]);

  return (
    <div className="min-h-screen flex flex-col bg-slate-50 dark:bg-slate-950 text-slate-900 dark:text-slate-100 font-sans transition-colors duration-200">
      <Header
        mode={mode}
        setMode={setMode}
        lang={lang}
        setLang={setLang}
        theme={theme}
        setTheme={setTheme}
        autoRun={autoRun}
        setAutoRun={setAutoRun}
        onRun={() => console.log("Code run triggered...")}
      />

      <main className="flex-1 flex items-center justify-center p-8">
        <div className="text-sm border border-dashed border-slate-300 dark:border-slate-800 p-12 rounded-lg text-slate-400 dark:text-slate-500">
          {lang === "en"
            ? "Workspace Panes (Editor & Results) go here."
            : "صفحه ویرایشگر و خروجی اینجا قرار می‌گیرند."}
        </div>
      </main>
    </div>
  );
}