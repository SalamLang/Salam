import { useState, type ChangeEvent } from "react";
import type { AppMode, Language, AppTheme } from "../App";

interface HeaderProps {
  mode: AppMode;
  setMode: (mode: AppMode) => void;
  lang: Language;
  setLang: (lang: Language) => void;
  theme: AppTheme;
  setTheme: (theme: AppTheme) => void;
  autoRun: boolean;
  setAutoRun: (autoRun: boolean) => void;
  onRun: () => void;
}

interface LocaleStrings {
  title: string;
  app: string;
  layout: string;
  examples: string;
  autoRun: string;
  run: string;
  // Added theme variant translations to the interface schema
  themeAuto: string;
  themeLight: string;
  themeDark: string;
}

const TRANSLATION_MAP: Record<Language, LocaleStrings> = {
  en: {
    title: "Salam Playground",
    app: "App",
    layout: "Layout",
    examples: "Examples",
    autoRun: "Auto-run",
    run: "Run",
    themeAuto: "◐ Auto",
    themeLight: "☼ Light",
    themeDark: "🌙 Dark",
  },
  fa: {
    title: "محیط آزمایشی سلام",
    app: "برنامه",
    layout: "چیدمان",
    examples: "نمونه‌ها",
    autoRun: "اجرای خودکار",
    run: "اجرا",
    themeAuto: "◐ خودکار",
    themeLight: "☼ روشن",
    themeDark: "🌙 تاریک",
  },
};

export default function Header({
  mode,
  setMode,
  lang,
  setLang,
  theme,
  setTheme,
  autoRun,
  setAutoRun,
  onRun,
}: HeaderProps) {
  const [menuOpen, setMenuOpen] = useState<boolean>(false);
  const t = TRANSLATION_MAP[lang];

  const cycleTheme = (): void => {
    const modes: AppTheme[] = ["auto", "light", "dark"];
    const nextIndex = (modes.indexOf(theme) + 1) % modes.length;
    setTheme(modes[nextIndex]);
  };

  return (
    <header className="flex flex-col md:flex-row items-center justify-between px-4 py-3 bg-white dark:bg-slate-900 border-b border-slate-200 dark:border-slate-800 shadow-xs relative z-50 select-none transition-colors duration-200">
      {/* Brand Header */}
      <div className="w-full md:w-auto flex items-center justify-between">
        <a href="?" className="flex items-center gap-2 group">
          <span className="w-5 h-5 rounded-full bg-brand shadow-sm group-hover:scale-105 transition-transform" aria-hidden="true" />
          <span className="font-bold text-lg tracking-tight text-slate-800 dark:text-white">
            {t.title}
          </span>
        </a>

        {/* Mobile Hamburger Button */}
        <button
          type="button"
          onClick={() => setMenuOpen(!menuOpen)}
          className="flex flex-col gap-1 md:hidden p-2 text-slate-500 hover:text-slate-800 dark:text-slate-400 dark:hover:text-white"
          aria-label="Menu"
          aria-expanded={menuOpen}
        >
          <span className={`h-0.5 w-5 bg-current transform transition-transform ${menuOpen ? "rotate-45 translate-y-1.5" : ""}`} />
          <span className={`h-0.5 w-5 bg-current transition-opacity ${menuOpen ? "opacity-0" : ""}`} />
          <span className={`h-0.5 w-5 bg-current transform transition-transform ${menuOpen ? "-rotate-45 -translate-y-1.5" : ""}`} />
        </button>
      </div>

      {/* Control Actions Panel */}
      <div className={`${menuOpen ? "flex" : "hidden"} md:flex flex-col md:flex-row w-full md:w-auto items-stretch md:items-center gap-3 mt-4 md:mt-0`}>

        {/* Mode Segments */}
        <div className="inline-flex bg-slate-100 dark:bg-slate-800 p-0.5 rounded-md border border-slate-200 dark:border-slate-700">
          <button
            type="button"
            onClick={() => setMode("app")}
            className={`px-3 py-1 text-sm font-medium rounded-sm transition-all ${mode === "app" ? "bg-white dark:bg-slate-700 text-brand shadow-xs" : "text-slate-600 dark:text-slate-400"}`}
          >
            {t.app}
          </button>
          <button
            type="button"
            onClick={() => setMode("layout")}
            className={`px-3 py-1 text-sm font-medium rounded-sm transition-all ${mode === "layout" ? "bg-white dark:bg-slate-700 text-brand shadow-xs" : "text-slate-600 dark:text-slate-400"}`}
          >
            {t.layout}
          </button>
        </div>

        {/* Language Segments */}
        <div className="inline-flex bg-slate-100 dark:bg-slate-800 p-0.5 rounded-md border border-slate-200 dark:border-slate-700">
          <button
            type="button"
            onClick={() => setLang("en")}
            className={`px-3 py-1 text-xs font-semibold rounded-sm transition-all ${lang === "en" ? "bg-white dark:bg-slate-700 text-slate-900 dark:text-white shadow-xs" : "text-slate-500"}`}
          >
            English
          </button>
          <button
            type="button"
            onClick={() => setLang("fa")}
            className={`px-3 py-1 text-xs font-semibold rounded-sm transition-all ${lang === "fa" ? "bg-white dark:bg-slate-700 text-slate-900 dark:text-white shadow-xs" : "text-slate-500"}`}
          >
            فارسی
          </button>
        </div>

        {/* Custom Examples Dropdown Wrapper */}
        <div className="relative">
          <select
            onChange={(e: ChangeEvent<HTMLSelectElement>) => console.log("Selected example:", e.target.value)}
            className="w-full md:w-auto appearance-none bg-slate-50 dark:bg-slate-800 border border-slate-200 dark:border-slate-700 text-slate-700 dark:text-slate-300 text-sm px-3 py-1.5 pe-8 rounded-md focus:outline-hidden focus:border-brand"
            defaultValue=""
          >
            <option value="" disabled hidden>{t.examples}</option>
            <option value="hello">Hello World</option>
            <option value="fibonacci">Fibonacci</option>
          </select>
          <span className={`absolute top-1/2 -translate-y-1/2 pointer-events-none text-xs text-slate-400 ${lang === 'fa' ? 'left-2.5' : 'right-2.5'}`}>▼</span>
        </div>

        {/* Theme Toggle Button — FIXED: Maps strings safely via active t translation map */}
        <button
          type="button"
          onClick={cycleTheme}
          className="flex items-center justify-center gap-1.5 px-3 py-1.5 border border-slate-200 dark:border-slate-700 hover:bg-slate-50 dark:hover:bg-slate-800 rounded-md text-sm text-slate-600 dark:text-slate-300 cursor-pointer select-none transition-colors"
        >
          <span>
            {theme === "auto" ? t.themeAuto : theme === "light" ? t.themeLight : t.themeDark}
          </span>
        </button>

        {/* Auto-run Switch Button */}
        <button
          type="button"
          onClick={() => setAutoRun(!autoRun)}
          className={`flex items-center justify-center gap-2 px-3 py-1.5 border rounded-md text-sm transition-colors ${
            autoRun
              ? "bg-emerald-50 dark:bg-emerald-950/30 border-emerald-200 dark:border-emerald-800 text-emerald-700 dark:text-emerald-400"
              : "border-slate-200 dark:border-slate-700 text-slate-500"
          }`}
        >
          <span className={`w-2 h-2 rounded-full ${autoRun ? "bg-emerald-500 animate-pulse" : "bg-slate-400"}`} />
          <span>{t.autoRun}</span>
        </button>

        {/* Main Run Trigger */}
        <button
          type="button"
          onClick={onRun}
          disabled={autoRun}
          className="flex items-center justify-center gap-2 bg-brand hover:brightness-110 disabled:bg-slate-200 dark:disabled:bg-slate-800 disabled:text-slate-400 text-white font-medium text-sm px-4 py-1.5 rounded-md transition-colors shadow-xs"
        >
          <span>▶</span>
          <span>{t.run}</span>
        </button>
      </div>
    </header>
  );
}