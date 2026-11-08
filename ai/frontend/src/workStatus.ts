import { t } from "./i18n";

export type WorkInput = {
  phase?: string;
  tool?: string;
  kind?: string;
  summary?: string;
  detail?: string;
  cloud?: boolean;
};

export type WorkLabel = { text: string; subject: string; specific: boolean };

const MAX_SUBJECT = 28;
const SECRET = /(^|[._-])(env|secret|secrets|token|tokens|key|keys|pem|id_rsa|credentials|password|passwd)([._-]|$)/i;

const FRONT_EXT = /\.(tsx|jsx|vue|svelte|html|htm|astro)$/i;
const STYLE_EXT = /\.(css|scss|sass|less|styl)$/i;
const BACK_EXT = /\.(salam|py|go|rs|java|php|rb|kt|cs|c|cpp|ex|exs)$/i;
const DB_EXT = /\.(sql|prisma|db|sqlite)$/i;
const DOC_EXT = /\.(md|mdx|txt|rst|adoc)$/i;
const CONF_EXT = /\.(ya?ml|json|toml|ini|env|conf|cfg|lock)$/i;
const IMG_EXT = /\.(svg|png|ico|icns|webp|jpe?g|gif)$/i;
const JS_EXT = /\.(ts|js|mjs|cjs)$/i;

export function baseName(path: string): string {
  const clean = path.trim().replace(/[\\/]+$/, "");
  const part = clean.split(/[\\/]/).pop() || "";
  if (!part || part.length > 120 || /[\s"'`$|;&<>*]/.test(part) || SECRET.test(part)) return "";
  return part.length > MAX_SUBJECT ? part.slice(0, MAX_SUBJECT - 1) + "…" : part;
}

export function pathArea(path: string): string {
  const p = path.replace(/\\/g, "/").toLowerCase();
  const name = p.split("/").pop() || "";
  if (/(^|\/)(tests?|__tests__|spec|specs|e2e)\//.test(p) || /(\.|_|-)(test|spec)\./.test(name) || /^test_/.test(name)) return "test";
  if (/(^|\/)(migrations?|db|database|schema|schemas)\//.test(p) || DB_EXT.test(name)) return "db";
  if (/dockerfile|docker-compose|makefile/.test(name) || /(^|\/)\.github\//.test(p)) return "config";
  if (IMG_EXT.test(name) || /(logo|icon|favicon)/.test(name)) return "design";
  if (STYLE_EXT.test(name)) return "style";
  if (FRONT_EXT.test(name) || /(^|\/)(frontend|components|pages|ui|views|web|public)\//.test(p)) return "front";
  if (BACK_EXT.test(name) || /(^|\/)(backend|server|api|routes|handlers|controllers|services)\//.test(p)) return "back";
  if (DOC_EXT.test(name) || /(^|\/)(docs?|documentation)\//.test(p)) return "docs";
  if (CONF_EXT.test(name)) return "config";
  if (JS_EXT.test(name)) return "front";
  return "";
}

function writeText(a: string): string {
  switch (a) {
    case "front":
      return t("دارم بخش فرانت‌اند را می‌نویسم", "Writing the frontend");
    case "style":
      return t("دارم ظاهر و استایل را می‌سازم", "Building the look and styles");
    case "back":
      return t("دارم بک‌اند را می‌نویسم", "Writing the backend");
    case "db":
      return t("دارم ساختار دیتابیس را می‌نویسم", "Writing the database structure");
    case "test":
      return t("دارم تست می‌نویسم", "Writing tests");
    case "docs":
      return t("دارم مستندات را می‌نویسم", "Writing the docs");
    case "config":
      return t("دارم تنظیمات را می‌نویسم", "Writing the settings");
    case "design":
      return t("دارم روی طراحی لوگو و آیکون کار می‌کنم", "Working on the logo and icon design");
  }
  return t("دارم کد را می‌نویسم", "Writing code");
}

export function commandText(cmd: string): string {
  const c = cmd.toLowerCase();
  if (/\b(test|tests|pytest|jest|vitest|mocha|rspec)\b/.test(c)) return t("دارم تست‌ها را اجرا می‌کنم", "Running the tests");
  if (/\b(npm|yarn|pnpm|bun|pip|pip3|apt|apt-get|brew|choco|winget|gem|composer)\b.*\b(install|add|i|ci)\b|cargo (add|fetch)|go (get|mod)/.test(c))
    return t("دارم وابستگی‌ها را نصب می‌کنم", "Installing dependencies");
  if (/\bgit\b|\bgh\b/.test(c)) return t("دارم با گیت کار می‌کنم", "Working with git");
  if (/screenshot|chrome|chromium|firefox|playwright|puppeteer|xdg-open|localhost/.test(c))
    return t("دارم نتیجه را در مرورگر می‌بینم", "Checking the result in the browser");
  if (/\b(build|compile|make|cmake|gcc|clang|tsc|rustc|cargo|salam|gradle|mvn)\b/.test(c)) return t("دارم پروژه را می‌سازم", "Building the project");
  if (/\bdocker\b|\bpodman\b/.test(c)) return t("دارم با داکر کار می‌کنم", "Working with Docker");
  return t("دارم یک دستور را اجرا می‌کنم", "Running a command");
}

function toolText(tool: string, kind: string): string {
  if (/figma|design/.test(tool)) return t("دارم طراحی‌ها را از فیگما برمی‌دارم", "Pulling the designs from Figma");
  if (/github|git_hub|^gh_|pull_request|issue/.test(tool)) return t("دارم با گیت‌هاب کار می‌کنم", "Working with GitHub");
  if (/browser|screenshot|playwright|puppeteer/.test(tool)) return t("دارم نتیجه را در مرورگر می‌بینم", "Checking the result in the browser");
  if (/web_search|web_fetch|fetch_url|^web|search_web/.test(tool)) return t("دارم در وب جست‌وجو می‌کنم", "Searching the web");
  if (/docs|learn|keyword|example|std|package|source/.test(tool)) return t("دارم مستندات سلام را می‌خوانم", "Reading the Salam docs");
  if (/check|compile|verify|lint/.test(tool)) return t("دارم کد را با کامپایلر سلام بررسی می‌کنم", "Checking the code with the Salam compiler");
  if (/run_salam|run_code|^run$/.test(tool)) return t("دارم کد را اجرا می‌کنم", "Running the code");
  if (/format|fmt|ast|llvm|js_|transpile|version/.test(tool)) return t("دارم کد را بررسی می‌کنم", "Looking over the code");
  if (kind === "write") return t("دارم کد را می‌نویسم", "Writing code");
  if (kind === "read") return t("دارم کد را بررسی می‌کنم", "Looking over the code");
  return "";
}

function phaseText(phase: string, cloud: boolean): WorkLabel {
  const generic = (text: string): WorkLabel => ({ text, subject: "", specific: false });
  switch (phase) {
    case "writing":
      return generic(t("دارم پاسخ را می‌نویسم", "Writing the answer"));
    case "searching":
      return generic(t("دارم مستندات سلام را می‌خوانم", "Reading the Salam docs"));
    case "checking":
      return generic(t("دارم کد را با کامپایلر سلام بررسی می‌کنم", "Checking the code with the Salam compiler"));
    case "tool":
    case "working":
      return generic(cloud ? t("دارم در محیط ابری کار می‌کنم", "Working in the cloud sandbox") : t("دارم روی رایانه‌ی شما کار می‌کنم", "Working on your computer"));
  }
  return generic(t("دارم فکر می‌کنم و نقشه می‌چینم", "Thinking and planning"));
}

export function workLabel(i: WorkInput): WorkLabel {
  const tool = (i.tool || "").toLowerCase();
  const kind = i.kind || "";
  if (!tool) return phaseText(i.phase || "thinking", Boolean(i.cloud));
  const path = i.summary || i.detail || "";
  const labeled = (text: string, subject = ""): WorkLabel => ({ text, subject, specific: true });
  switch (tool) {
    case "write_file":
    case "edit_file":
      return labeled(writeText(pathArea(path)), baseName(path));
    case "read_file":
      return labeled(t("دارم فایل‌های پروژه را می‌خوانم", "Reading the project files"), baseName(path));
    case "list_dir":
    case "glob":
      return labeled(t("دارم فایل‌های پروژه را می‌خوانم", "Reading the project files"));
    case "grep":
      return labeled(t("دارم کد را بررسی می‌کنم", "Looking over the code"));
    case "run_tests":
      return labeled(t("دارم تست‌ها را اجرا می‌کنم", "Running the tests"));
    case "docker":
    case "wsl":
    case "shell":
      return labeled(commandText(`${tool === "shell" ? "" : tool + " "}${i.summary || ""} ${i.detail || ""}`));
  }
  const text = toolText(tool, kind);
  if (text) return labeled(text);
  return phaseText(i.phase || "thinking", Boolean(i.cloud));
}
