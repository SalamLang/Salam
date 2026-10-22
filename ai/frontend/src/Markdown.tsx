import { memo, useEffect, useRef, useState, type ReactNode } from "react";
import ReactMarkdown, { type Components } from "react-markdown";
import remarkGfm from "remark-gfm";
import { codeDirection, highlight } from "./highlight";
import { IconCheck, IconCopy } from "./icons";
import { highlightRich, preloadHighlighter } from "./shiki";
import { t } from "./i18n";

const LANG_NAMES: Record<string, string> = {
  salam: t("سلام", "Salam"),
  text: t("متن", "Text"),
  plain: t("متن", "Text"),
  txt: t("متن", "Text"),
  output: t("خروجی", "Output"),
  sh: t("ترمینال", "Terminal"),
  bash: t("ترمینال", "Terminal"),
  shell: t("ترمینال", "Terminal"),
  console: t("ترمینال", "Terminal"),
  zsh: t("ترمینال", "Terminal"),
  powershell: "PowerShell",
  json: "JSON",
  yaml: "YAML",
  yml: "YAML",
  toml: "TOML",
  python: "Python",
  py: "Python",
  javascript: "JavaScript",
  js: "JavaScript",
  typescript: "TypeScript",
  ts: "TypeScript",
  c: "C",
  cpp: "C++",
  go: "Go",
  rust: "Rust",
  rs: "Rust",
  java: "Java",
  php: "PHP",
  html: "HTML",
  css: "CSS",
  sql: "SQL",
  diff: "Diff",
  dockerfile: "Dockerfile",
  docker: "Dockerfile",
  llvm: "LLVM IR",
};

const RICH_EVERY_MS = 250;

export function useHighlighted(code: string, lang: string): string {
  const quick = highlight(code, lang);
  const [rich, setRich] = useState<{ key: string; html: string } | null>(null);
  const last = useRef(0);
  const key = `${lang}\u0000${code}`;

  useEffect(() => {
    preloadHighlighter();
  }, []);

  useEffect(() => {
    let alive = true;
    const run = () => {
      last.current = Date.now();
      highlightRich(code, lang)
        .then((html) => {
          if (alive && html !== null) setRich({ key, html });
        })
        .catch(() => {});
    };
    const wait = Math.max(0, RICH_EVERY_MS - (Date.now() - last.current));
    const timer = window.setTimeout(run, wait);
    return () => {
      alive = false;
      window.clearTimeout(timer);
    };
  }, [code, lang, key]);

  if (rich && rich.key === key) return rich.html;
  if (rich && code.startsWith(rich.key.slice(lang.length + 1))) return rich.html + highlight(code.slice(rich.key.length - lang.length - 1), lang);
  return quick;
}

function textOf(node: ReactNode): string {
  if (typeof node === "string" || typeof node === "number") return String(node);
  if (Array.isArray(node)) return node.map(textOf).join("");
  if (node && typeof node === "object" && "props" in node) return textOf((node as { props: { children?: ReactNode } }).props.children);
  return "";
}

export function CopyButton({ text, label = t("رونوشت", "Copy") }: { text: string; label?: string }) {
  const [done, setDone] = useState(false);
  return (
    <button
      type="button"
      className="icon-button small"
      aria-label={label}
      title={label}
      onClick={() => {
        navigator.clipboard?.writeText(text).then(() => {
          setDone(true);
          setTimeout(() => setDone(false), 1600);
        });
      }}
    >
      {done ? <IconCheck width={16} height={16} /> : <IconCopy width={16} height={16} />}
      <span>{done ? t("رونوشت شد", "Copied") : label}</span>
    </button>
  );
}

function CodeBlock({ lang, code }: { lang: string; code: string }) {
  const dir = codeDirection(code, lang);
  const html = useHighlighted(code, lang);
  const name = LANG_NAMES[lang] || lang || t("کد", "Code");
  return (
    <div className="code-block">
      <div className="code-head">
        <span className="code-lang">{name}</span>
        <CopyButton text={code} />
      </div>
      <pre dir={dir}>
        <code dangerouslySetInnerHTML={{ __html: html }} />
      </pre>
    </div>
  );
}

const components: Components = {
  pre({ children }) {
    const child = Array.isArray(children) ? children[0] : children;
    const props = (child as { props?: { className?: string; children?: ReactNode } })?.props || {};
    const lang = /language-([\w-]+)/.exec(props.className || "")?.[1] || "";
    const code = textOf(props.children).replace(/\n$/, "");
    return <CodeBlock lang={lang} code={code} />;
  },
  code({ children, className }) {
    return (
      <code className={className ? className : "inline-code"} dir="auto">
        {children}
      </code>
    );
  },
  p: ({ children }) => <p dir="auto">{children}</p>,
  li: ({ children }) => <li dir="auto">{children}</li>,
  h1: ({ children }) => <h3 dir="auto">{children}</h3>,
  h2: ({ children }) => <h3 dir="auto">{children}</h3>,
  h3: ({ children }) => <h4 dir="auto">{children}</h4>,
  a: ({ children, href }) => (
    <a href={href} target="_blank" rel="noreferrer noopener">
      {children}
    </a>
  ),
  table: ({ children }) => (
    <div className="table-wrap">
      <table>{children}</table>
    </div>
  ),
};

export const Markdown = memo(function Markdown({ text }: { text: string }) {
  return (
    <div className="markdown">
      <ReactMarkdown remarkPlugins={[remarkGfm]} components={components}>
        {text}
      </ReactMarkdown>
    </div>
  );
});
