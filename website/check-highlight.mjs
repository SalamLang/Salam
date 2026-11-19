import { readFileSync } from "node:fs";
import { dirname, join } from "node:path";
import { fileURLToPath } from "node:url";

const here = dirname(fileURLToPath(import.meta.url));
const source = readFileSync(join(here, "assets/js/site.js"), "utf8");
const from = source.indexOf("const DIGIT");
const to = source.indexOf("const SALAM =");
if (from < 0 || to < from) {
  console.error("check-highlight: the highlighter moved in site.js");
  process.exit(2);
}
const body =
  'const words = new Map([["repeat", "k"]]);' +
  "const decls = new Map();" +
  "const escapeHtml = (s) => s;" +
  source.slice(from, to) +
  "return highlightLine;";
const highlightLine = new Function(body)();

const numbers = (line) =>
  [...highlightLine(line, {}).matchAll(/<span class="tok-n">([^<]*)<\/span>/g)].map(
    (m) => m[1],
  );

const cases = [
  ["columns = repeat(2, 1fr)", ["2", "1fr"]],
  ["padding = 8, 16", ["8", "16"]],
  ["border = 1, solid, red", ["1"]],
  ["transition = 150ms, ease out", ["150ms"]],
  ["delay = 2s  size = 10px  gap = 1.5rem  turn = 45deg", ["2s", "10px", "1.5rem", "45deg"]],
  ["width = 50%", ["50%"]],
  ["x := 7 % 3", ["7", "3"]],
  ["ستون‌ها = ۱fr ، ۱۵۰ms ، ۰.۳s", ["۱fr", "۱۵۰ms", "۰.۳s"]],
  ["عرض = ۵۰٪", ["۵۰٪"]],
  ["a = 3sx", ["3"]],
];

let failed = 0;
for (const [line, want] of cases) {
  const got = numbers(line);
  if (JSON.stringify(got) !== JSON.stringify(want)) {
    console.error(
      `check-highlight: ${line}\n  want ${JSON.stringify(want)}\n  got  ${JSON.stringify(got)}`,
    );
    failed += 1;
  }
}
process.exit(failed ? 1 : 0);
