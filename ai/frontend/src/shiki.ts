import type {
  HighlighterCore,
  LanguageRegistration,
  ThemedToken,
  ThemeRegistration,
} from "shiki/core";

type Palette = {
  fg: string;
  bg: string;
  keyword: string;
  type: string;
  builtin: string;
  string: string;
  number: string;
  comment: string;
  func: string;
  operator: string;
  escape: string;
  invalid: string;
};

const LIGHT: Palette = {
  fg: "#1c1b1a",
  bg: "#faf8f6",
  keyword: "#d84a00",
  type: "#8a3ffc",
  builtin: "#0b7a5f",
  string: "#1a7f37",
  number: "#b45309",
  comment: "#8a8580",
  func: "#1f6feb",
  operator: "#9a4a1a",
  escape: "#c2410c",
  invalid: "#d93025",
};

const DARK: Palette = {
  fg: "#ecebe9",
  bg: "#121317",
  keyword: "#ff8a4c",
  type: "#c4a2ff",
  builtin: "#4fd1a8",
  string: "#8bd49c",
  number: "#f2b56b",
  comment: "#7d7a76",
  func: "#79b8ff",
  operator: "#f0a37a",
  escape: "#ffb37a",
  invalid: "#ff6b5f",
};

function theme(
  name: string,
  type: "light" | "dark",
  p: Palette,
): ThemeRegistration {
  return {
    name,
    type,
    colors: { "editor.foreground": p.fg, "editor.background": p.bg },
    tokenColors: [
      { settings: { foreground: p.fg } },
      {
        scope: ["comment", "punctuation.definition.comment"],
        settings: { foreground: p.comment, fontStyle: "italic" },
      },
      {
        scope: [
          "keyword",
          "keyword.control",
          "keyword.declaration",
          "storage.type",
          "storage.modifier",
          "keyword.other",
        ],
        settings: { foreground: p.keyword, fontStyle: "bold" },
      },
      {
        scope: ["keyword.operator", "punctuation.separator.directive"],
        settings: { foreground: p.operator, fontStyle: "" },
      },
      {
        scope: [
          "keyword.operator.logical",
          "keyword.operator.word",
          "keyword.operator.new",
        ],
        settings: { foreground: p.keyword, fontStyle: "bold" },
      },
      {
        scope: [
          "support.type",
          "entity.name.type",
          "support.class",
          "entity.name.class",
          "storage.type.primitive",
          "entity.other.inherited-class",
        ],
        settings: { foreground: p.type },
      },
      {
        scope: [
          "support.function.builtin",
          "support.function",
          "variable.language",
          "storage.type.auto",
        ],
        settings: { foreground: p.builtin },
      },
      {
        scope: [
          "entity.name.function",
          "meta.function-call",
          "entity.name.function.call",
        ],
        settings: { foreground: p.func },
      },
      {
        scope: ["string", "punctuation.definition.string", "markup.inline.raw"],
        settings: { foreground: p.string },
      },
      {
        scope: [
          "constant.character.escape",
          "meta.interpolation",
          "punctuation.section.embedded",
        ],
        settings: { foreground: p.escape },
      },
      {
        scope: [
          "constant.numeric",
          "constant.language",
          "constant.other",
          "support.constant",
        ],
        settings: { foreground: p.number },
      },
      {
        scope: [
          "storage.modifier.annotation",
          "entity.name.tag",
          "meta.preprocessor",
          "entity.other.attribute-name",
        ],
        settings: { foreground: p.func },
      },
      {
        scope: [
          "variable.other.member",
          "variable.other.property",
          "support.variable.property",
        ],
        settings: { foreground: p.fg },
      },
      {
        scope: ["invalid"],
        settings: { foreground: p.invalid, fontStyle: "underline" },
      },
      { scope: ["markup.inserted"], settings: { foreground: p.string } },
      { scope: ["markup.deleted"], settings: { foreground: p.invalid } },
      {
        scope: ["markup.heading", "markup.bold"],
        settings: { fontStyle: "bold" },
      },
    ],
  };
}

const EXTRA: Record<
  string,
  () => Promise<{ default: LanguageRegistration[] }>
> = {
  bash: () => import("@shikijs/langs/bash"),
  shellscript: () => import("@shikijs/langs/shellscript"),
  powershell: () => import("@shikijs/langs/powershell"),
  json: () => import("@shikijs/langs/json"),
  yaml: () => import("@shikijs/langs/yaml"),
  toml: () => import("@shikijs/langs/toml"),
  ini: () => import("@shikijs/langs/ini"),
  python: () => import("@shikijs/langs/python"),
  javascript: () => import("@shikijs/langs/javascript"),
  typescript: () => import("@shikijs/langs/typescript"),
  c: () => import("@shikijs/langs/c"),
  cpp: () => import("@shikijs/langs/cpp"),
  go: () => import("@shikijs/langs/go"),
  rust: () => import("@shikijs/langs/rust"),
  java: () => import("@shikijs/langs/java"),
  kotlin: () => import("@shikijs/langs/kotlin"),
  swift: () => import("@shikijs/langs/swift"),
  php: () => import("@shikijs/langs/php"),
  ruby: () => import("@shikijs/langs/ruby"),
  html: () => import("@shikijs/langs/html"),
  css: () => import("@shikijs/langs/css"),
  xml: () => import("@shikijs/langs/xml"),
  sql: () => import("@shikijs/langs/sql"),
  diff: () => import("@shikijs/langs/diff"),
  docker: () => import("@shikijs/langs/docker"),
  makefile: () => import("@shikijs/langs/makefile"),
  nginx: () => import("@shikijs/langs/nginx"),
  llvm: () => import("@shikijs/langs/llvm"),
  asm: () => import("@shikijs/langs/asm"),
  markdown: () => import("@shikijs/langs/markdown"),
};

const ALIASES: Record<string, string> = {
  "": "salam",
  salam: "salam",
  سلام: "salam",
  sl: "salam",
  sh: "bash",
  shell: "bash",
  zsh: "bash",
  console: "bash",
  terminal: "bash",
  ps1: "powershell",
  pwsh: "powershell",
  jsonc: "json",
  yml: "yaml",
  py: "python",
  js: "javascript",
  mjs: "javascript",
  jsx: "javascript",
  ts: "typescript",
  tsx: "typescript",
  h: "c",
  hpp: "cpp",
  "c++": "cpp",
  rs: "rust",
  kt: "kotlin",
  rb: "ruby",
  htm: "html",
  svg: "xml",
  dockerfile: "docker",
  make: "makefile",
  ll: "llvm",
  ir: "llvm",
  md: "markdown",
};

let core: Promise<HighlighterCore> | null = null;
const loading = new Map<string, Promise<boolean>>();

function load(): Promise<HighlighterCore> {
  if (!core) {
    core = (async () => {
      const [
        { createHighlighterCore },
        { createJavaScriptRegexEngine },
        grammar,
      ] = await Promise.all([
        import("shiki/core"),
        import("shiki/engine/javascript"),
        import("@salam/grammar"),
      ]);
      const salam = {
        ...(grammar.default as unknown as LanguageRegistration),
        name: "salam",
      };
      return createHighlighterCore({
        themes: [
          theme("salam-light", "light", LIGHT),
          theme("salam-dark", "dark", DARK),
        ],
        langs: [salam],
        engine: createJavaScriptRegexEngine({ forgiving: true }),
      });
    })();
    core.catch(() => {
      core = null;
    });
  }
  return core;
}

export function resolveLang(lang: string): string | null {
  const l = lang.trim().toLowerCase();
  const id = ALIASES[l] ?? l;
  if (id === "salam" || id in EXTRA) return id;
  return null;
}

async function ensureLang(h: HighlighterCore, id: string): Promise<boolean> {
  if (h.getLoadedLanguages().includes(id)) return true;
  let p = loading.get(id);
  if (!p) {
    const importer = EXTRA[id];
    if (!importer) return false;
    p = importer()
      .then((m) => h.loadLanguage(...m.default))
      .then(() => true)
      .catch(() => false);
    loading.set(id, p);
  }
  return p;
}

function esc(s: string): string {
  return s
    .replace(/&/g, "&amp;")
    .replace(/</g, "&lt;")
    .replace(/>/g, "&gt;")
    .replace(/"/g, "&quot;");
}

function styleOf(t: ThemedToken): string {
  const s = t.htmlStyle;
  if (!s) return "";
  return Object.entries(s)
    .map(([k, v]) => `${k}:${v}`)
    .join(";");
}

const cache = new Map<string, string>();
const CACHE_MAX = 300;

export async function highlightRich(
  code: string,
  lang: string,
): Promise<string | null> {
  const id = resolveLang(lang);
  if (!id) return null;
  const key = `${id}\u0000${code}`;
  const hit = cache.get(key);
  if (hit !== undefined) return hit;
  const h = await load();
  if (id !== "salam" && !(await ensureLang(h, id))) return null;
  const lines = h.codeToTokens(code, {
    lang: id,
    themes: { light: "salam-light", dark: "salam-dark" },
    defaultColor: false,
  }).tokens;
  const html = lines
    .map((line) =>
      line
        .map((t) => `<span style="${esc(styleOf(t))}">${esc(t.content)}</span>`)
        .join(""),
    )
    .join("\n");
  if (cache.size >= CACHE_MAX)
    cache.delete(cache.keys().next().value as string);
  cache.set(key, html);
  return html;
}

export function preloadHighlighter() {
  load().catch(() => {});
}
