import { useState, useEffect } from "react";
import Header from "./components/Header";

export type AppMode = "app" | "layout";
export type Language = "en" | "fa";
export type AppTheme = "auto" | "light" | "dark" | "random";

interface RandomPalette {
  bg: string;
  bgDarker: string;
  text: string;
  textMuted: string;
  border: string;
  headerBg: string;
}

const generateRandomPalette = (): RandomPalette => {
  const randomHue = Math.floor(Math.random() * 360);

  // High contrast neon/retro-futuristic dark mode parameters
  return {
    bg: `hsl(${randomHue}, 40%, 8%)`,
    bgDarker: `hsl(${randomHue}, 45%, 4%)`,
    headerBg: `hsl(${randomHue}, 35%, 12%)`,
    text: `hsl(${(randomHue + 150) % 360}, 95%, 80%)`,
    textMuted: `hsl(${(randomHue + 150) % 360}, 60%, 65%)`,
    border: `hsl(${randomHue}, 50%, 22%)`,
  };
};

export default function App() {
  const [mode, setMode] = useState<AppMode>("app");
  const [lang, setLang] = useState<Language>("en");
  const [theme, setTheme] = useState<AppTheme>("auto");
  const [autoRun, setAutoRun] = useState<boolean>(true);
  const [randomPalette, setRandomPalette] = useState<RandomPalette>(generateRandomPalette);

  // Re-generate palette whenever switching back to the random theme
  useEffect(() => {
    if (theme === "random") {
      setRandomPalette(generateRandomPalette());
    }
  }, [theme]);

  useEffect(() => {
    document.documentElement.dir = lang === "fa" ? "rtl" : "ltr";
    document.documentElement.lang = lang;
  }, [lang]);

  useEffect(() => {
    const root = document.documentElement;
    const mediaQuery = window.matchMedia("(prefers-color-scheme: dark)");

    // Remove only the custom properties set by the random theme when leaving it
    if (theme !== "random") {
      root.style.removeProperty("--custom-bg");
      root.style.removeProperty("--custom-bg-darker");
      root.style.removeProperty("--custom-text");
      root.style.removeProperty("--custom-text-muted");
      root.style.removeProperty("--custom-border");
      root.style.removeProperty("--custom-header-bg");
    }

    const syncTheme = () => {
      if (theme === "random") {
        root.classList.add("dark");
        root.setAttribute("data-theme", "random");

        // Push full theme palette array straight to global CSS window namespace
        root.style.setProperty("--custom-bg", randomPalette.bg);
        root.style.setProperty("--custom-bg-darker", randomPalette.bgDarker);
        root.style.setProperty("--custom-text", randomPalette.text);
        root.style.setProperty("--custom-text-muted", randomPalette.textMuted);
        root.style.setProperty("--custom-border", randomPalette.border);
        root.style.setProperty("--custom-header-bg", randomPalette.headerBg);
        return;
      }

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
  }, [theme, randomPalette]);

  return (
    <div className="min-h-screen flex flex-col font-sans transition-colors duration-200 bg-slate-50 dark:bg-slate-950 text-slate-900 dark:text-slate-100">
      <Header
        mode={mode}
        setMode={setMode}
        lang={lang}
        setLang={setLang}
        theme={theme}
        setTheme={setTheme}
        autoRun={autoRun}
        setAutoRun={setAutoRun}
        onRun={() => {}}
      />

      <main className="flex-1 flex items-center justify-center p-8 bg-slate-50 dark:bg-slate-950">
        <div className="text-sm border border-dashed border-slate-300 dark:border-slate-800 p-12 rounded-lg text-slate-500 dark:text-slate-400">
          {lang === "en"
            ? "Workspace Panes (Editor & Results) go here."
            : "صفحه ویرایشگر و خروجی اینجا قرار می‌گیرند."}
        </div>
      </main>
    </div>
  );
}