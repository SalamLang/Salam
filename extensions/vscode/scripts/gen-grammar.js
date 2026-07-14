// Generates the Salam TextMate grammar (source.salam).

const UNI = [
  "\\x{0600}-\\x{06FF}", // Arabic
  "\\x{0750}-\\x{077F}", // Arabic Supplement
  "\\x{08A0}-\\x{08FF}", // Arabic Extended-A
  "\\x{FB50}-\\x{FDFF}", // Arabic Presentation Forms-A
  "\\x{FE70}-\\x{FEFF}", // Arabic Presentation Forms-B
  "\\x{200C}\\x{200D}", // ZWNJ / ZWJ (used inside words like پیاده‌سازی)
].join("");

const IDC = `A-Za-z0-9_${UNI}`; // identifier continue chars
const IDS = `A-Za-z_${UNI}`; // identifier start chars (no digits)
const IDENT = `[${IDS}][${IDC}]*`; // a full identifier
const NBB = `(?<![${IDC}])`; // not preceded by an identifier char
const NBA = `(?![${IDC}])`; // not followed by an identifier char

function kw(words) {
  const sorted = [...words].sort((a, b) => b.length - a.length);
  return `${NBB}(?:${sorted.join("|")})${NBA}`;
}

const CONTROL = [
  "if",
  "else",
  "while",
  "for",
  "ret",
  "break",
  "continue",
  "repeat",
  "to",
  "every",
  "defer",
  "end",
  "اگر",
  "وگرنه",
  "تاوقتی",
  "برای",
  "بازگشت",
  "بشکن",
  "ادامه",
  "تکرار",
  "تا",
  "هر",
  "تعلیق",
  "پایان",
  // Arabic
  "إذا",
  "وإلا",
  "بينما",
  "لكل",
  "أرجع",
  "اكسر",
  "واصل",
  "كرر",
  "إلى",
  "كل",
  "أجل",
  "نهاية",
];

// Logical operators expressed as words: و / یا (Persian) and أو (Arabic),
// the localized forms of && / ||. Highlighted as operators, not control words.
const LOGICAL = ["و", "یا", "أو"];

const STORAGE = [
  "func",
  "mut",
  "const",
  "type",
  "struct",
  "enum",
  "interface",
  "impl",
  "package",
  "import",
  "as",
  "pub",
  "inline",
  "noinline",
  "pure",
  "noret",
  "deprecated",
  "component",
  "layout",
  "operator",
  "extern",
  "تابع",
  "متغیر",
  "ثابت",
  "نوع",
  "ساختار",
  "شمارش",
  "واسط",
  "پیاده‌سازی",
  "بسته",
  "واردکردن",
  "بعنوان",
  "عمومی",
  "درون‌خطی",
  "غیردرون‌خطی",
  "خالص",
  "بی‌بازگشت",
  "منسوخ",
  "مولفه",
  "صفحه",
  "عملگر",
  "خارجی",
  // Arabic (نوع/ثابت shared with Persian above)
  "دالة",
  "متغير",
  "بنية",
  "تعداد",
  "واجهة",
  "تنفيذ",
  "حزمة",
  "استيراد",
  "بوصفه",
  "عام",
  "مضمن",
  "غيرمضمن",
  "نقي",
  "لايرجع",
  "مهجور",
  "مكون",
  "صفحة",
  "عامل",
  "خارجي",
];

const IO = [
  "printerrln",
  "printerr",
  "println",
  "print",
  "input",
  "خطاچاپ",
  "خطابنویس",
  "چاپ",
  "بنویس",
  "بخوان",
  // Arabic
  "اطبع_خطأ",
  "اكتب_خطأ",
  "اطبع",
  "اكتب",
  "اقرأ",
];

const CONSTS = [
  "true",
  "false",
  "null",
  "درست",
  "نادرست",
  "پوچ",
  "صواب",
  "خطأ",
  "عدم",
];
const THIS = ["this", "این", "هذا"];
const AUTO = ["auto", "خودکار", "تلقائي"];
const CONTEXTUAL = [
  "link",
  "static",
  "dynamic",
  "framework",
  "kind",
  "dyn",
  "پیوند",
  "ایستا",
  "پویا",
  "چارچوب",
  "گونه",
  // Arabic
  "رابط",
  "ساكن",
  "ديناميكي",
  "إطار",
  "صنف",
];

const TYPES = [
  "i8",
  "i16",
  "i32",
  "i64",
  "u8",
  "u16",
  "u32",
  "u64",
  "f32",
  "f64",
  "isize",
  "usize",
  "bool",
  "str",
  "char",
  "void",
  "صحیح۸",
  "صحیح۱۶",
  "صحیح۳۲",
  "صحیح۶۴",
  "اعشار۳۲",
  "اعشار۶۴",
  "رشته",
  "منطقی",
  "بایت",
  "کاراکتر",
  // Arabic (Arabic-Indic digit suffixes ٨/١٦/٣٢/٦٤)
  "صحيح٨",
  "صحيح١٦",
  "صحيح٣٢",
  "صحيح٦٤",
  "صحيح",
  "طبيعي٨",
  "طبيعي١٦",
  "طبيعي٣٢",
  "طبيعي٦٤",
  "طبيعي",
  "عشري٣٢",
  "عشري٦٤",
  "عشري",
  "نص",
  "منطقي",
  "حرف",
  "فراغ",
  "يونيكود",
];

const FUNC_DEF_KW = ["func", "تابع", "دالة"];
const TYPE_DEF_KW = [
  "type",
  "struct",
  "enum",
  "interface",
  "component",
  "package",
  "نوع",
  "ساختار",
  "شمارش",
  "واسط",
  "مولفه",
  "بسته",
  // Arabic (نوع shared with Persian above)
  "بنية",
  "تعداد",
  "واجهة",
  "مكون",
  "حزمة",
];

const grammar = {
  $schema:
    "https://raw.githubusercontent.com/martinring/tmlanguage/master/tmlanguage.json",
  name: "Salam",
  scopeName: "source.salam",
  patterns: [
    { include: "#directives" },
    { include: "#comments" },
    { include: "#meta-annotations" },
    { include: "#strings" },
    { include: "#numbers" },
    { include: "#definitions" },
    { include: "#keywords" },
    { include: "#layout-element" },
    { include: "#function-call" },
    { include: "#member" },
    { include: "#operators" },
    { include: "#punctuation" },
  ],
  repository: {
    directives: {
      patterns: [
        {
          name: "comment.line.directive.salam",
          begin: "//!",
          beginCaptures: {
            0: { name: "punctuation.definition.comment.salam" },
          },
          end: "$",
          patterns: [
            {
              match: "\\b(mode|title|lang|dir|name|version)\\b",
              name: "keyword.other.directive.salam",
            },
            { match: "\\|", name: "punctuation.separator.directive.salam" },
          ],
        },
      ],
    },
    comments: {
      patterns: [
        {
          name: "comment.block.salam",
          begin: "/\\*",
          beginCaptures: {
            0: { name: "punctuation.definition.comment.salam" },
          },
          end: "\\*/",
          endCaptures: { 0: { name: "punctuation.definition.comment.salam" } },
        },
        {
          name: "comment.line.double-slash.salam",
          begin: "//",
          beginCaptures: {
            0: { name: "punctuation.definition.comment.salam" },
          },
          end: "$",
        },
      ],
    },
    "meta-annotations": {
      patterns: [
        {
          match: `(@)([A-Za-z_][${IDC}]*)`,
          captures: {
            1: { name: "punctuation.definition.annotation.salam" },
            2: { name: "storage.modifier.annotation.salam" },
          },
        },
      ],
    },
    strings: {
      patterns: [
        {
          name: "string.quoted.triple.salam",
          begin: '"""',
          end: '"""',
          beginCaptures: {
            0: { name: "punctuation.definition.string.begin.salam" },
          },
          endCaptures: {
            0: { name: "punctuation.definition.string.end.salam" },
          },
          patterns: [{ include: "#string-escapes" }],
        },
        {
          name: "string.quoted.double.salam",
          begin: '"',
          end: '"',
          beginCaptures: {
            0: { name: "punctuation.definition.string.begin.salam" },
          },
          endCaptures: {
            0: { name: "punctuation.definition.string.end.salam" },
          },
          patterns: [
            { include: "#string-escapes" },
            { name: "invalid.illegal.newline.salam", match: "\\n" },
          ],
        },
        {
          name: "string.quoted.raw.salam",
          begin: "`",
          end: "`",
          beginCaptures: {
            0: { name: "punctuation.definition.string.begin.salam" },
          },
          endCaptures: {
            0: { name: "punctuation.definition.string.end.salam" },
          },
        },
        {
          name: "string.quoted.single.salam",
          match: `${NBB}u?'(\\\\(?:x[0-9A-Fa-f]{2}|u\\{[0-9A-Fa-f]+\\}|u[0-9A-Fa-f]{4}|.)|[^'\\\\])'`,
        },
      ],
    },
    "string-escapes": {
      patterns: [
        {
          name: "constant.character.escape.salam",
          match:
            "\\\\(?:x[0-9A-Fa-f]{2}|u\\{[0-9A-Fa-f]+\\}|u[0-9A-Fa-f]{4}|[0abfnrtv\"'\\\\])",
        },
        { name: "invalid.illegal.unknown-escape.salam", match: "\\\\." },
      ],
    },
    numbers: {
      patterns: [
        {
          name: "constant.numeric.float.salam",
          match: `${NBB}(?:[0-9][0-9_]*\\.[0-9][0-9_]*(?:[eE][+-]?[0-9][0-9_]*)?|[0-9][0-9_]*[eE][+-]?[0-9][0-9_]*)f?${NBA}`,
        },
        {
          name: "constant.numeric.hex.salam",
          match: `${NBB}0[xX][0-9A-Fa-f][0-9A-Fa-f_]*${NBA}`,
        },
        {
          name: "constant.numeric.binary.salam",
          match: `${NBB}0[bB][01][01_]*${NBA}`,
        },
        {
          name: "constant.numeric.octal.salam",
          match: `${NBB}0[oO][0-7][0-7_]*${NBA}`,
        },
        {
          name: "constant.numeric.integer.salam",
          match: `${NBB}[0-9][0-9_]*f?${NBA}`,
        },
      ],
    },
    definitions: {
      patterns: [
        {
          match: `(${kw(FUNC_DEF_KW)})\\s+(${IDENT})`,
          captures: {
            1: {
              name: "storage.type.function.salam keyword.declaration.function.salam",
            },
            2: { name: "entity.name.function.salam" },
          },
        },
        {
          match: `(${kw(TYPE_DEF_KW)})\\s+(${IDENT})`,
          captures: {
            1: { name: "storage.type.salam keyword.declaration.type.salam" },
            2: { name: "entity.name.type.salam" },
          },
        },
      ],
    },
    keywords: {
      patterns: [
        { name: "keyword.control.salam", match: kw(CONTROL) },
        { name: "keyword.operator.logical.salam", match: kw(LOGICAL) },
        { name: "storage.type.salam", match: kw(STORAGE) },
        { name: "support.function.builtin.salam", match: kw(IO) },
        {
          name: "support.function.builtin.print.salam",
          match: `${NBB}_{1,2}${NBA}`,
        },
        { name: "constant.language.salam", match: kw(CONSTS) },
        { name: "variable.language.this.salam", match: kw(THIS) },
        { name: "storage.type.auto.salam", match: kw(AUTO) },
        { name: "support.type.primitive.salam", match: kw(TYPES) },
        { name: "keyword.other.salam", match: kw(CONTEXTUAL) },
      ],
    },
    "layout-element": {
      patterns: [
        {
          match: `^\\s*${NBB}(?!(?:else|وگرنه)${NBA})(${IDENT})\\s*(:)\\s*$`,
          captures: {
            1: { name: "entity.name.tag.layout.salam" },
            2: { name: "punctuation.section.layout.salam" },
          },
        },
      ],
    },
    "function-call": {
      patterns: [
        {
          match: `(${IDENT})\\s*(?=\\()`,
          name: "entity.name.function.call.salam",
        },
      ],
    },
    member: {
      patterns: [
        {
          match: `(?<=\\.)\\s*(${IDENT})`,
          name: "variable.other.member.salam",
        },
      ],
    },
    operators: {
      patterns: [
        { name: "keyword.operator.arrow.salam", match: "=>" },
        { name: "keyword.operator.spread.salam", match: "\\.\\.\\." },
        { name: "keyword.operator.comparison.salam", match: "==|!=|<=|>=|<|>" },
        {
          name: "keyword.operator.assignment.compound.salam",
          match: "\\+=|-=|\\*=|/=|%=",
        },
        { name: "keyword.operator.logical.salam", match: "&&|\\|\\||!" },
        {
          name: "keyword.operator.arithmetic.salam",
          match: "\\*\\*|[+\\-*/%]",
        },
        { name: "keyword.operator.reference.salam", match: "&" },
        { name: "keyword.operator.assignment.salam", match: "=" },
      ],
    },
    punctuation: {
      patterns: [
        { name: "punctuation.separator.salam", match: "[,:]" },
        { name: "punctuation.accessor.salam", match: "\\." },
        { name: "meta.brace.round.salam", match: "[()]" },
        { name: "meta.brace.square.salam", match: "[\\[\\]]" },
        { name: "meta.brace.curly.salam", match: "[{}]" },
      ],
    },
  },
};

process.stdout.write(`${JSON.stringify(grammar, null, 2)}\n`);
