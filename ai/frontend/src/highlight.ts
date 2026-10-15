const KEYWORDS = new Set(
  (
    "func main end if else until repeat each in break continue ret mut const struct pub enum match and or not true false this as by to import include package defer spawn join extern interface impl switch case default null dyn link " +
    "تابع ریشه پایان اگر وگرنه تا تکرار هر در بشکن گذر برگشت ناپایا پایا ساختار همگانی جداشمار همخوان و یا نه درست نادرست این برگردان واردسازی بسته ثابت تهی"
  ).split(/\s+/),
);
const TYPES = new Set(
  "int str bool f64 f32 i64 i32 i16 i8 u64 u32 u16 u8 char void Vector HashMap صحیح رشته منطقی اعشار۶۴ اعشار۳۲ نویسه بردار نگاشت".split(
    /\s+/,
  ),
);
const BUILTINS = new Set(
  "println print سرچاپ چاپ len push get set".split(/\s+/),
);

function esc(s: string): string {
  return s.replace(/&/g, "&amp;").replace(/</g, "&lt;").replace(/>/g, "&gt;");
}

const TOKEN =
  /(\/\/[^\n]*|\/\*[\s\S]*?\*\/|#[^\n]*)|("(?:\\.|[^"\\])*"|`[^`]*`|«[^»]*»|'(?:\\.|[^'\\])')|([0-9۰-۹]+(?:\.[0-9۰-۹]+)?)|([A-Za-z_\u0600-\u06FF\u200c][A-Za-z0-9_\u0600-\u06FF\u200c]*)/g;

export function highlight(code: string, lang: string): string {
  const l = lang.toLowerCase();
  const salamLike = l === "" || l === "salam" || l === "سلام";
  let out = "";
  let last = 0;
  for (const m of code.matchAll(TOKEN)) {
    const i = m.index ?? 0;
    out += esc(code.slice(last, i));
    const [tok, comment, string, number, word] = m;
    if (comment !== undefined) {
      if (comment.startsWith("#") && salamLike) out += esc(tok);
      else out += `<span class="tk-c">${esc(tok)}</span>`;
    } else if (string !== undefined)
      out += `<span class="tk-s">${esc(tok)}</span>`;
    else if (number !== undefined)
      out += `<span class="tk-n">${esc(tok)}</span>`;
    else if (word !== undefined) {
      if (KEYWORDS.has(word)) out += `<span class="tk-k">${esc(tok)}</span>`;
      else if (TYPES.has(word)) out += `<span class="tk-t">${esc(tok)}</span>`;
      else if (BUILTINS.has(word))
        out += `<span class="tk-b">${esc(tok)}</span>`;
      else if (code[i + tok.length] === "(")
        out += `<span class="tk-f">${esc(tok)}</span>`;
      else out += esc(tok);
    }
    last = i + tok.length;
  }
  out += esc(code.slice(last));
  return out;
}

export function hasRtl(s: string): boolean {
  return /[\u0600-\u06FF]/.test(s);
}
