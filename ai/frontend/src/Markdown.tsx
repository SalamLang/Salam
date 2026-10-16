import { memo, useState, type ReactNode } from "react";
import ReactMarkdown, { type Components } from "react-markdown";
import remarkGfm from "remark-gfm";
import { codeDirection, highlight } from "./highlight";
import { IconCheck, IconCopy } from "./icons";

const LANG_NAMES: Record<string, string> = {
  salam: "سلام",
  text: "متن",
  sh: "ترمینال",
  bash: "ترمینال",
  shell: "ترمینال",
  console: "ترمینال",
};

function textOf(node: ReactNode): string {
  if (typeof node === "string" || typeof node === "number") return String(node);
  if (Array.isArray(node)) return node.map(textOf).join("");
  if (node && typeof node === "object" && "props" in node) return textOf((node as { props: { children?: ReactNode } }).props.children);
  return "";
}

export function CopyButton({ text, label = "رونوشت" }: { text: string; label?: string }) {
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
      <span>{done ? "رونوشت شد" : label}</span>
    </button>
  );
}

function CodeBlock({ lang, code }: { lang: string; code: string }) {
  const dir = codeDirection(code, lang);
  const name = LANG_NAMES[lang] || lang || "کد";
  return (
    <div className="code-block">
      <div className="code-head">
        <span className="code-lang">{name}</span>
        <CopyButton text={code} />
      </div>
      <pre dir={dir}>
        <code dangerouslySetInnerHTML={{ __html: highlight(code, lang) }} />
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
