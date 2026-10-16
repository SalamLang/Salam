const EN_KEYWORDS = new Set(
  "func ret if else until on mut const type struct enum end import as true false null this break continue layout package printerr printerrln input defer extern interface pub inline noinline pure noret deprecated component repeat impl to by each in match switch include export and or not eq neq lt gt lte gte main spawn join case default dyn link".split(
    " ",
  ),
);
const FA_KEYWORDS = new Set(
  "برگشت اگر وگرنه تا بر ناپایا پایا گونه ساختار جداشمار پایان واردسازی برگردان درست نادرست پوچ این بشکن گذر چیدمان بسته نادرست‌چاپ نادرست‌سرچاپ ورودی دیرکن درون‌داد میانجی همگانی درخط نادرخط ناب نابرگشت بی‌کاره بخش تکرار کاربست هر در همخوان و یا برابر نابرابر ترابرد فراخوانی برون‌داد وارونه کوچکتر بزرگتر کوچکتربرابر بزرگتربرابر تابع ریشه نه ثابت تهی".split(
    " ",
  ),
);
const KEYWORDS = new Set([...EN_KEYWORDS, ...FA_KEYWORDS]);
const TYPES = new Set(
  "int str bool f64 f32 i64 i32 i16 i8 u64 u32 u16 u8 char void Vector HashMap صحیح رشته منطقی اعشار۶۴ اعشار۳۲ نویسه بردار نگاشت".split(
    /\s+/,
  ),
);
const BUILTINS_EN = new Set(
  "println print printerr printerrln input len push get set".split(" "),
);
const BUILTINS_FA = new Set("سرچاپ چاپ نادرست‌چاپ نادرست‌سرچاپ ورودی".split(" "));
const BUILTINS = new Set([...BUILTINS_EN, ...BUILTINS_FA]);

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

const LTR_LANGS = new Set(
  "sh bash shell console zsh powershell ps1 cmd bat js javascript mjs ts typescript tsx jsx json jsonc py python c cpp h hpp go rust rs java kotlin swift php ruby rb html xml css scss yaml yml toml ini sql diff dockerfile makefile http curl llvm ir asm".split(
    " ",
  ),
);
const RTL_WORD = /[\u0600-\u06FF]/;

export function codeDirection(code: string, lang: string): "rtl" | "ltr" {
  const l = lang.toLowerCase();
  if (LTR_LANGS.has(l)) return "ltr";
  const salamLike = l === "" || l === "salam" || l === "سلام";
  if (!salamLike) {
    const fa = (code.match(/[\u0600-\u06FF]/g) || []).length;
    const latin = (code.match(/[A-Za-z]/g) || []).length;
    return fa > latin ? "rtl" : "ltr";
  }
  let fa = 0;
  let en = 0;
  for (const m of code.matchAll(TOKEN)) {
    const word = m[4];
    if (word === undefined) continue;
    if (FA_KEYWORDS.has(word) || BUILTINS_FA.has(word)) fa += 3;
    else if (EN_KEYWORDS.has(word) || BUILTINS_EN.has(word)) en += 3;
    else if (RTL_WORD.test(word)) fa += 1;
    else en += 1;
  }
  return fa > en ? "rtl" : "ltr";
}
