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
    const isDarkSystem = window.matchMedia("(prefers-color-scheme: dark)").matches;

    if (theme === "dark" || (theme === "auto" && isDarkSystem)) {
      root.classList.add("dark");
    } else {
      root.classList.remove("dark");
    }
  }, [theme]);

  const handleRun = (): void => {
    console.log("Compiling & Running Salam code...");
  };

  return (
    <div className="min-h-screen flex flex-col bg-slate-50 dark:bg-slate-900 text-slate-900 dark:text-slate-100 font-sans transition-colors duration-200">
      <Header
        mode={mode}
        setMode={setMode}
        lang={lang}
        setLang={setLang}
        theme={theme}
        setTheme={setTheme}
        autoRun={autoRun}
        setAutoRun={setAutoRun}
        onRun={handleRun}
      />

      <main className="flex-1 flex items-center justify-center p-8">
        <p className="text-sm border border-dashed border-slate-300 dark:border-slate-700 p-12 rounded-lg text-slate-400 dark:text-slate-500">
          {lang === "en"
            ? "Workspace Panes (Editor & Results) go here."
            : "صفحه ویرایشگر و خروجی اینجا قرار می‌گیرند."}
        </p>
      </main>
    </div>
  );
}