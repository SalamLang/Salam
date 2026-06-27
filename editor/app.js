(function () {
  "use strict";
  var EXAMPLES = window.SALAM_EXAMPLES || [];
  var STORE_KEY = "salam:playground";
  var I18N = {
    en: {
      title: "Salam Playground",
      mode_app: "App",
      mode_layout: "Layout",
      run: "Run",
      autorun: "Auto-run",
      editor: "Editor",
      output: "Output",
      preview: "Preview",
      ready: "ready",
      running: "running…",
      done: "done",
      error: "error",
      loading: "loading compiler…",
      custom: "Custom",
      examples: "Examples",
      g_console: "Console programs",
      g_web: "Web pages",
      badge_web: "WEB",
      theme_auto: "Auto",
      theme_light: "Light",
      theme_dark: "Dark",
      view_run: "Run",
      view_tokens: "Tokens",
      view_ast: "AST",
      view_symbols: "Symbols",
      view_html: "HTML",
      view_css: "CSS",
      view_js: "JS",
      view_c: "C",
      view_llvm: "LLVM IR",
      copy: "Copy",
      copied: "Copied",
    },
    fa: {
      title: "زمین بازی سلام",
      mode_app: "برنامه",
      mode_layout: "چیدمان",
      run: "اجرا",
      autorun: "اجرای خودکار",
      editor: "ویرایشگر",
      output: "خروجی",
      preview: "پیش‌نمایش",
      ready: "آماده",
      running: "در حال اجرا…",
      done: "انجام شد",
      error: "خطا",
      loading: "بارگذاری کامپایلر…",
      custom: "سفارشی",
      examples: "نمونه‌ها",
      g_console: "برنامه‌های کنسولی",
      g_web: "صفحه‌های وب",
      badge_web: "وب",
      theme_auto: "خودکار",
      theme_light: "روشن",
      theme_dark: "تاریک",
      view_run: "اجرا",
      view_tokens: "توکن‌ها",
      view_ast: "درخت نحو",
      view_symbols: "نمادها",
      view_html: "HTML",
      view_css: "CSS",
      view_js: "JS",
      view_c: "کد C",
      view_llvm: "LLVM IR",
      copy: "کپی",
      copied: "کپی شد",
    },
  };
  var THEME_ICON = { auto: "◐", light: "☀", dark: "☾" };
  var LEX = window.SALAM_LEX || { kw: [], ty: [], el: [] };
  var KW = new Set(LEX.kw || []);
  var TY = new Set(LEX.ty || []);
  var EL = new Set(LEX.el || []);
  function esc(s) {
    return s.replace(/&/g, "&amp;").replace(/</g, "&lt;").replace(/>/g, "&gt;");
  }
  var TOK =
    /(\/\/[^\n]*|\/\*[\s\S]*?\*\/)|("(?:[^"\\]|\\.)*"|`[^`]*`|'(?:[^'\\]|\\.)*')|(\d+\.\d+|\d+)|([A-Za-z_؀-ۿ‌][\w؀-ۿ‌]*)|([=!<>+\-*\/%&|:^~?]+)/g;
  function highlight(src) {
    var out = "",
      last = 0,
      m;
    TOK.lastIndex = 0;
    while ((m = TOK.exec(src)) !== null) {
      if (m.index > last) out += esc(src.slice(last, m.index));
      var tok = m[0],
        cls = null;
      if (m[1]) cls = "c";
      else if (m[2]) cls = "s";
      else if (m[3]) cls = "n";
      else if (m[4])
        cls = KW.has(tok) || EL.has(tok) ? "k" : TY.has(tok) ? "t" : null;
      else if (m[5]) cls = "o";
      out += cls
        ? '<span class="' + cls + '">' + esc(tok) + "</span>"
        : esc(tok);
      last = TOK.lastIndex;
    }
    out += esc(src.slice(last));
    return out + "\n";
  }
  function highlightAttrs(s) {
    return s.replace(
      /([\w:.\-]+)(\s*=\s*)("[^"]*"|'[^']*')/g,
      function (_, name, eq, val) {
        return (
          '<span class="xa">' +
          esc(name) +
          "</span>" +
          esc(eq) +
          '<span class="xv">' +
          esc(val) +
          "</span>"
        );
      },
    );
  }
  var XML_TOK =
    /<!--[\s\S]*?-->|<\?[\s\S]*?\?>|<\/?[\w:.\-]+(?:"[^"]*"|'[^']*'|[^"'>])*>/g;
  function highlightXML(src) {
    var out = "",
      last = 0,
      m;
    XML_TOK.lastIndex = 0;
    while ((m = XML_TOK.exec(src)) !== null) {
      if (m.index > last) out += esc(src.slice(last, m.index));
      var t = m[0];
      if (t.slice(0, 4) === "<!--") {
        out += '<span class="xc">' + esc(t) + "</span>";
      } else if (t.slice(0, 2) === "<?") {
        out += '<span class="xp">' + esc(t) + "</span>";
      } else {
        var tm = /^(<\/?)([\w:.\-]+)([\s\S]*?)(\/?>)$/.exec(t);
        if (tm) {
          out +=
            '<span class="xb">' +
            esc(tm[1]) +
            "</span>" +
            '<span class="xt">' +
            esc(tm[2]) +
            "</span>" +
            highlightAttrs(tm[3]) +
            '<span class="xb">' +
            esc(tm[4]) +
            "</span>";
        } else {
          out += esc(t);
        }
      }
      last = XML_TOK.lastIndex;
    }
    out += esc(src.slice(last));
    return out;
  }
  var IR_KW = new Set([
    "define",
    "declare",
    "ret",
    "br",
    "switch",
    "indirectbr",
    "invoke",
    "resume",
    "unreachable",
    "call",
    "tail",
    "musttail",
    "notail",
    "callbr",
    "load",
    "store",
    "alloca",
    "getelementptr",
    "inbounds",
    "fence",
    "cmpxchg",
    "atomicrmw",
    "extractvalue",
    "insertvalue",
    "extractelement",
    "insertelement",
    "shufflevector",
    "add",
    "fadd",
    "sub",
    "fsub",
    "mul",
    "fmul",
    "sdiv",
    "udiv",
    "fdiv",
    "srem",
    "urem",
    "frem",
    "and",
    "or",
    "xor",
    "shl",
    "lshr",
    "ashr",
    "neg",
    "fneg",
    "icmp",
    "fcmp",
    "eq",
    "ne",
    "ugt",
    "uge",
    "ult",
    "ule",
    "sgt",
    "sge",
    "slt",
    "sle",
    "oeq",
    "ogt",
    "oge",
    "olt",
    "ole",
    "one",
    "ord",
    "uno",
    "ueq",
    "une",
    "trunc",
    "zext",
    "sext",
    "fptrunc",
    "fpext",
    "fptoui",
    "fptosi",
    "uitofp",
    "sitofp",
    "ptrtoint",
    "inttoptr",
    "bitcast",
    "addrspacecast",
    "select",
    "phi",
    "freeze",
    "va_arg",
    "global",
    "constant",
    "private",
    "internal",
    "external",
    "common",
    "linkonce",
    "weak",
    "appending",
    "available_externally",
    "extern_weak",
    "dllimport",
    "dllexport",
    "unnamed_addr",
    "local_unnamed_addr",
    "dso_local",
    "dso_preemptable",
    "thread_local",
    "align",
    "nsw",
    "nuw",
    "exact",
    "volatile",
    "atomic",
    "syncscope",
    "seq_cst",
    "acquire",
    "release",
    "acq_rel",
    "monotonic",
    "nounwind",
    "noinline",
    "alwaysinline",
    "readonly",
    "readnone",
    "willreturn",
    "nofree",
    "noalias",
    "nocapture",
    "dereferenceable",
    "byval",
    "sret",
    "inreg",
    "signext",
    "zeroext",
    "noundef",
    "nonnull",
    "nest",
    "returned",
    "speculatable",
    "mustprogress",
    "norecurse",
    "target",
    "datalayout",
    "triple",
    "source_filename",
    "attributes",
    "module",
    "asm",
    "to",
    "from",
    "x",
    "null",
    "none",
    "undef",
    "poison",
    "zeroinitializer",
    "blockaddress",
    "true",
    "false",
    "type",
    "opaque",
    "label",
    "metadata",
    "distinct",
    "section",
    "comdat",
  ]);
  function isIRType(w) {
    return /^(i\d+|void|float|double|half|bfloat|fp128|x86_fp80|x86_mmx|ppc_fp128|ptr|token)$/.test(
      w,
    );
  }
  var IR_TOK =
    /(;[^\n]*)|(c?"(?:[^"\\]|\\.)*")|(@[\w.$\-]+|@"(?:[^"\\]|\\.)*")|(%[\w.$\-]+|%"(?:[^"\\]|\\.)*")|([!#][\w.$\-]*)|(-?0x[0-9A-Fa-f]+|-?\d+\.\d+(?:[eE][+\-]?\d+)?|-?\d+)|([A-Za-z_.$][\w.$\-]*)|([=,*{}()\[\]<>])/g;
  function highlightIR(src) {
    var out = "",
      last = 0,
      m;
    IR_TOK.lastIndex = 0;
    while ((m = IR_TOK.exec(src)) !== null) {
      if (m.index > last) out += esc(src.slice(last, m.index));
      var tok = m[0],
        cls = null;
      if (m[1]) cls = "ic";
      else if (m[2]) cls = "is";
      else if (m[3]) cls = "ig";
      else if (m[4]) cls = "il";
      else if (m[5]) cls = "im";
      else if (m[6]) cls = "in";
      else if (m[7]) {
        if (
          src.charCodeAt(IR_TOK.lastIndex) === 58 &&
          src.charCodeAt(IR_TOK.lastIndex + 1) !== 58
        ) {
          tok += ":";
          IR_TOK.lastIndex++;
          cls = "ilbl";
        } else {
          cls = IR_KW.has(tok) ? "ik" : isIRType(tok) ? "it" : null;
        }
      } else if (m[8]) cls = "io";
      out += cls
        ? '<span class="' + cls + '">' + esc(tok) + "</span>"
        : esc(tok);
      last = IR_TOK.lastIndex;
    }
    out += esc(src.slice(last));
    return out;
  }
  var C_KW = new Set([
    "auto",
    "break",
    "case",
    "char",
    "const",
    "continue",
    "default",
    "do",
    "double",
    "else",
    "enum",
    "extern",
    "float",
    "for",
    "goto",
    "if",
    "inline",
    "int",
    "long",
    "register",
    "restrict",
    "return",
    "short",
    "signed",
    "sizeof",
    "static",
    "struct",
    "switch",
    "typedef",
    "union",
    "unsigned",
    "void",
    "volatile",
    "while",
    "_Bool",
    "_Complex",
    "_Imaginary",
    "_Atomic",
    "_Noreturn",
    "_Static_assert",
    "_Thread_local",
    "_Alignas",
    "_Alignof",
    "_Generic",
    "NULL",
    "true",
    "false",
    "nullptr",
  ]);
  var C_TY = new Set([
    "int8_t",
    "int16_t",
    "int32_t",
    "int64_t",
    "uint8_t",
    "uint16_t",
    "uint32_t",
    "uint64_t",
    "intptr_t",
    "uintptr_t",
    "intmax_t",
    "uintmax_t",
    "ptrdiff_t",
    "size_t",
    "ssize_t",
    "wchar_t",
    "char16_t",
    "char32_t",
    "bool",
    "FILE",
    "va_list",
    "int_least8_t",
    "int_least16_t",
    "int_least32_t",
    "int_least64_t",
    "uint_least8_t",
    "uint_least16_t",
    "uint_least32_t",
    "uint_least64_t",
    "int_fast8_t",
    "int_fast16_t",
    "int_fast32_t",
    "int_fast64_t",
    "uint_fast8_t",
    "uint_fast16_t",
    "uint_fast32_t",
    "uint_fast64_t",
  ]);
  function highlightPre(s) {
    var re = /("(?:[^"\\]|\\.)*"|<[^>\n]*>)/g,
      out = "",
      last = 0,
      m;
    while ((m = re.exec(s)) !== null) {
      if (m.index > last) out += esc(s.slice(last, m.index));
      out += '<span class="cs">' + esc(m[0]) + "</span>";
      last = re.lastIndex;
    }
    out += esc(s.slice(last));
    return '<span class="cpp">' + out + "</span>";
  }
  var C_TOK =
    /(\/\/[^\n]*|\/\*[\s\S]*?\*\/)|(^[ \t]*#[^\n]*)|("(?:[^"\\]|\\.)*")|('(?:[^'\\]|\\.)*')|(\.?\d[\w.]*(?:[eEpP][+\-]?\w+)?)|([A-Za-z_]\w*)|([-+*\/%=!<>&|^~?:.,;(){}\[\]])/gm;
  function highlightC(src) {
    var out = "",
      last = 0,
      m;
    C_TOK.lastIndex = 0;
    while ((m = C_TOK.exec(src)) !== null) {
      if (m.index > last) out += esc(src.slice(last, m.index));
      var tok = m[0],
        cls = null;
      if (m[1]) cls = "cm";
      else if (m[2]) {
        out += highlightPre(tok);
        last = C_TOK.lastIndex;
        continue;
      } else if (m[3]) cls = "cs";
      else if (m[4]) cls = "cl";
      else if (m[5]) cls = "cn";
      else if (m[6]) {
        if (C_KW.has(tok)) cls = "ck";
        else if (C_TY.has(tok)) cls = "ct";
        else if (src.charCodeAt(C_TOK.lastIndex) === 40) cls = "cf";
      } else if (m[7]) cls = "co";
      out += cls
        ? '<span class="' + cls + '">' + esc(tok) + "</span>"
        : esc(tok);
      last = C_TOK.lastIndex;
    }
    out += esc(src.slice(last));
    return out;
  }
  var CSS_TOK =
    /(\/\*[\s\S]*?\*\/)|("(?:[^"\\]|\\.)*"|'(?:[^'\\]|\\.)*')|(#[0-9A-Fa-f]{3,8}\b)|(-?(?:\d*\.\d+|\d+)(?:px|em|rem|%|vh|vw|vmin|vmax|pt|pc|cm|mm|in|ex|ch|deg|rad|turn|s|ms|fr|dpi)?)|(@[A-Za-z-]+)|(!important)|([A-Za-z_][\w-]*)|([{}();:,>~+*.#\[\]=])/g;
  function highlightCSS(src) {
    var out = "",
      last = 0,
      m;
    CSS_TOK.lastIndex = 0;
    while ((m = CSS_TOK.exec(src)) !== null) {
      if (m.index > last) out += esc(src.slice(last, m.index));
      var tok = m[0],
        cls = null;
      if (m[1]) cls = "cm";
      else if (m[2]) cls = "cs";
      else if (m[3]) cls = "cn";
      else if (m[4]) cls = "cn";
      else if (m[5]) cls = "ck";
      else if (m[6]) cls = "ck";
      else if (m[7]) {
        var j = CSS_TOK.lastIndex;
        while (src.charCodeAt(j) === 32 || src.charCodeAt(j) === 9) j++;
        cls = src.charCodeAt(j) === 58 ? "cf" : "ct";
      } else if (m[8]) cls = "co";
      out += cls
        ? '<span class="' + cls + '">' + esc(tok) + "</span>"
        : esc(tok);
      last = CSS_TOK.lastIndex;
    }
    out += esc(src.slice(last));
    return out;
  }
  var JS_KW = new Set([
    "break",
    "case",
    "catch",
    "class",
    "const",
    "continue",
    "debugger",
    "default",
    "delete",
    "do",
    "else",
    "export",
    "extends",
    "finally",
    "for",
    "function",
    "if",
    "import",
    "in",
    "instanceof",
    "new",
    "return",
    "super",
    "switch",
    "this",
    "throw",
    "try",
    "typeof",
    "var",
    "void",
    "while",
    "with",
    "yield",
    "let",
    "static",
    "async",
    "await",
    "of",
    "get",
    "set",
    "null",
    "true",
    "false",
    "undefined",
    "NaN",
    "Infinity",
  ]);
  var JS_TOK =
    /(\/\/[^\n]*|\/\*[\s\S]*?\*\/)|("(?:[^"\\]|\\.)*"|'(?:[^'\\]|\\.)*'|`(?:[^`\\]|\\.)*`)|(\.?\d[\w.]*)|([A-Za-z_$][\w$]*)|([-+*\/%=!<>&|^~?:.,;(){}\[\]])/g;
  function highlightJS(src) {
    var out = "",
      last = 0,
      m;
    JS_TOK.lastIndex = 0;
    while ((m = JS_TOK.exec(src)) !== null) {
      if (m.index > last) out += esc(src.slice(last, m.index));
      var tok = m[0],
        cls = null;
      if (m[1]) cls = "cm";
      else if (m[2]) cls = "cs";
      else if (m[3]) cls = "cn";
      else if (m[4]) {
        if (JS_KW.has(tok)) cls = "ck";
        else if (src.charCodeAt(JS_TOK.lastIndex) === 40) cls = "cf";
      } else if (m[5]) cls = "co";
      out += cls
        ? '<span class="' + cls + '">' + esc(tok) + "</span>"
        : esc(tok);
      last = JS_TOK.lastIndex;
    }
    out += esc(src.slice(last));
    return out;
  }
  var editor = null;
  var lang = "en",
    mode = "app",
    autorun = true,
    themePref = "auto";
  var view = "run"; // run | tokens | ast | symbols | html | css | js | c | llvm
  var exampleId = "hello";
  var runTimer = null,
    saveTimer = null;
  var wasm = { ready: false, runApp: null, buildLayout: null, emit: null };
  var $ = function (id) {
    return document.getElementById(id);
  };
  function findExample(id) {
    for (var i = 0; i < EXAMPLES.length; i++)
      if (EXAMPLES[i].id === id) return EXAMPLES[i];
    return null;
  }
  function renderHL() {
    var hl = $("hl"),
      ta = $("code");
    hl.innerHTML = highlight(ta.value);
    hl.dir = ta.dir;
    hl.scrollTop = ta.scrollTop;
    hl.scrollLeft = ta.scrollLeft;
  }
  function saveState() {
    clearTimeout(saveTimer);
    saveTimer = setTimeout(function () {
      try {
        localStorage.setItem(
          STORE_KEY,
          JSON.stringify({
            code: editor ? editor.getValue() : "",
            lang: lang,
            mode: mode,
            autorun: autorun,
            theme: themePref,
            example: exampleId,
            view: view,
          }),
        );
      } catch (e) {}
    }, 200);
  }
  function loadState() {
    try {
      return JSON.parse(localStorage.getItem(STORE_KEY) || "null");
    } catch (e) {
      return null;
    }
  }
  function effectiveTheme() {
    if (themePref !== "auto") return themePref;
    var h = new Date().getHours();
    return h >= 7 && h < 19 ? "light" : "dark";
  }
  function applyTheme() {
    document.documentElement.setAttribute("data-theme", effectiveTheme());
    var b = $("theme");
    b.querySelector(".theme-ico").textContent = THEME_ICON[themePref];
    b.querySelector(".theme-lbl").textContent =
      I18N[lang]["theme_" + themePref];
    if (editor) renderHL();
  }
  function cycleTheme() {
    themePref =
      themePref === "auto" ? "light" : themePref === "light" ? "dark" : "auto";
    applyTheme();
    saveState();
  }
  var ICON = {
    app: '<svg class="ic" viewBox="0 0 16 16" width="14" height="14" fill="none" stroke="currentColor" stroke-width="1.5"><rect x="1.5" y="2.7" width="13" height="10.6" rx="1.6"/><path d="M4.3 6.2l2.3 1.8-2.3 1.8" stroke-linecap="round" stroke-linejoin="round"/><path d="M8.4 10H12" stroke-linecap="round"/></svg>',
    layout:
      '<svg class="ic web" viewBox="0 0 16 16" width="14" height="14" fill="none" stroke="currentColor" stroke-width="1.5"><rect x="1.5" y="2.7" width="13" height="10.6" rx="1.6"/><path d="M1.5 5.7h13"/><circle cx="3.7" cy="4.2" r=".55" fill="currentColor" stroke="none"/><circle cx="5.5" cy="4.2" r=".55" fill="currentColor" stroke="none"/></svg>',
    custom:
      '<svg class="ic" viewBox="0 0 16 16" width="14" height="14" fill="none" stroke="currentColor" stroke-width="1.5"><path d="M10.5 2.6l2.9 2.9-8 8H2.6v-2.9z" stroke-linejoin="round"/></svg>',
  };
  function modeIcon(m) {
    return ICON[m] || ICON.app;
  }
  function dropdownLabel() {
    if (exampleId === "custom") return I18N[lang].custom;
    var ex = findExample(exampleId);
    return ex ? ex.title[lang] : I18N[lang].examples;
  }
  function rebuildDropdown() {
    var list = $("dd-list");
    list.innerHTML = "";
    [
      ["app", I18N[lang].g_console],
      ["layout", I18N[lang].g_web],
    ].forEach(function (g) {
      var items = EXAMPLES.filter(function (e) {
        return e.mode === g[0];
      });
      if (!items.length) return;
      var head = document.createElement("li");
      head.className = "dd-head" + (g[0] === "layout" ? " web" : "");
      head.textContent = g[1];
      list.appendChild(head);
      items.forEach(function (ex) {
        var li = document.createElement("li");
        li.className = "dd-item" + (ex.mode === "layout" ? " is-web" : "");
        li.dataset.id = ex.id;
        li.innerHTML =
          modeIcon(ex.mode) +
          '<span class="dd-text">' +
          esc(ex.title[lang]) +
          "</span>" +
          (ex.mode === "layout"
            ? '<span class="dd-badge">' + esc(I18N[lang].badge_web) + "</span>"
            : "");
        if (ex.id === exampleId) li.classList.add("sel");
        list.appendChild(li);
      });
    });
    if (exampleId === "custom") {
      var li = document.createElement("li");
      li.className = "dd-item sel custom";
      li.dataset.id = "custom";
      li.innerHTML =
        modeIcon("custom") +
        '<span class="dd-text">' +
        esc(I18N[lang].custom) +
        "</span>";
      list.appendChild(li);
    }
    var curIcon = exampleId === "custom" ? modeIcon("custom") : modeIcon(mode);
    $("dd-label").innerHTML =
      curIcon + '<span class="dd-text">' + esc(dropdownLabel()) + "</span>";
  }
  function openDropdown(open) {
    $("dd-list").hidden = !open;
    $("examples").classList.toggle("open", open);
  }
  function defaultExampleFor(m) {
    for (var i = 0; i < EXAMPLES.length; i++)
      if (EXAMPLES[i].mode === m) return EXAMPLES[i].id;
    return EXAMPLES.length ? EXAMPLES[0].id : "hello";
  }
  function selectExample(id) {
    var ex = findExample(id);
    if (!ex) return;
    exampleId = id;
    if (ex.mode !== mode) applyMode(ex.mode);
    editor.setValue(ex.code[lang] || "");
    rebuildDropdown();
    saveState();
    scheduleRun();
  }
  function applyLang(next) {
    var keepExample = exampleId;
    lang = next;
    var rtl = lang === "fa";
    var dict = I18N[lang];
    document.title = dict.title;
    document.documentElement.lang = lang;
    document.documentElement.dir = rtl ? "rtl" : "ltr";
    document.querySelectorAll("[data-i18n]").forEach(function (el) {
      var k = el.getAttribute("data-i18n");
      if (dict[k]) el.textContent = dict[k];
    });
    document.querySelectorAll("#lang-seg .seg-btn").forEach(function (b) {
      b.classList.toggle("active", b.getAttribute("data-lang") === lang);
    });
    if (editor) {
      $("code").dir = rtl ? "rtl" : "ltr";
      if (keepExample !== "custom") {
        var ex = findExample(keepExample);
        if (ex) editor.setValue(ex.code[lang] || editor.getValue());
      } else {
        exampleId = defaultExampleFor(mode);
        var dex = findExample(exampleId);
        editor.setValue(dex ? dex.code[lang] || "" : "");
      }
      renderHL();
    }
    $("output").setAttribute("dir", rtl ? "rtl" : "ltr");
    applyTheme();
    rebuildDropdown();
    syncCopyChrome();
    setStatus("ready", "");
    saveState();
    scheduleRun();
  }
  function setStatus(key, cls) {
    var s = $("status");
    s.textContent = I18N[lang][key] || "";
    s.className = "status" + (cls ? " " + cls : "");
  }
  function applyMode(next) {
    mode = next;
    document.querySelectorAll("#mode-seg .seg-btn").forEach(function (b) {
      b.classList.toggle("active", b.getAttribute("data-mode") === mode);
    });
    applyView();
    saveState();
  }
  function viewAllowed(v) {
    var b = document.querySelector('#view-seg .seg-btn[data-view="' + v + '"]');
    if (!b) return false;
    var only = b.getAttribute("data-only");
    return !only || only === mode;
  }
  function applyView() {
    if (!viewAllowed(view)) view = "run";
    document.querySelectorAll("#view-seg .seg-btn").forEach(function (b) {
      var only = b.getAttribute("data-only");
      b.hidden = !!(only && only !== mode);
      b.classList.toggle("active", b.getAttribute("data-view") === view);
    });
    var showPreview = view === "run" && mode === "layout";
    $("preview").hidden = !showPreview;
    $("output").hidden = showPreview;
  }
  function applyViewChange(next) {
    if (next === view) return;
    view = next;
    applyView();
    saveState();
    run();
  }
  function scheduleRun() {
    if (!autorun) return;
    clearTimeout(runTimer);
    runTimer = setTimeout(function () {
      if (autorun && wasm.ready) run();
    }, 500);
  }
  function showPhase(src) {
    var pre = $("output");
    var txt =
      (wasm.emit ? wasm.emit(src, lang, view) : "(inspector unavailable)") ||
      "(empty)";
    var isErr =
      /error\[|runtime error|خطا|compilation failed|layout build failed|no layout block/i.test(
        txt,
      );
    var isXml =
      !isErr &&
      (view === "tokens" ||
        view === "ast" ||
        view === "symbols" ||
        view === "html");
    var isIR = !isErr && view === "llvm";
    var isC = !isErr && view === "c";
    var isCss = !isErr && view === "css";
    var isJs = !isErr && view === "js";
    if (isXml) pre.innerHTML = highlightXML(txt);
    else if (isIR) pre.innerHTML = highlightIR(txt);
    else if (isC) pre.innerHTML = highlightC(txt);
    else if (isCss) pre.innerHTML = highlightCSS(txt);
    else if (isJs) pre.innerHTML = highlightJS(txt);
    else pre.textContent = txt;
    pre.className =
      "output codeview" +
      (isXml ? " xml" : "") +
      (isIR ? " ir" : "") +
      (isC || isCss || isJs ? " clang" : "");
    pre.setAttribute("dir", "ltr");
    setStatus(isErr ? "error" : "done", isErr ? "err" : "ok");
  }
  function run() {
    if (!wasm.ready || !editor) return;
    var src = editor.getValue();
    setStatus("running", "busy");
    setTimeout(function () {
      try {
        if (view !== "run") {
          showPhase(src);
          return;
        }
        if (mode === "layout") {
          var res = wasm.buildLayout(src, lang);
          if (/^\s*</.test(res)) {
            $("preview").srcdoc = res;
            setStatus("done", "ok");
          } else {
            var dir = lang === "fa" ? "rtl" : "ltr";
            $("preview").srcdoc =
              '<!doctype html><meta charset="utf-8">' +
              '<div dir="' +
              dir +
              '" style="font-family:Vazirmatn,system-ui,sans-serif;' +
              'color:#c0392b;padding:16px 20px;white-space:pre-wrap;line-height:1.8">' +
              esc(res) +
              "</div>";
            setStatus("error", "err");
          }
        } else {
          var out = wasm.runApp(src, lang);
          var pre = $("output");
          var isErr = /error\[|runtime error|timed out|aborted|خطا/i.test(out);
          pre.textContent = out || "(no output)";
          pre.className = "output" + (isErr ? " error" : "");
          pre.setAttribute("dir", lang === "fa" ? "rtl" : "ltr");
          setStatus(isErr ? "error" : "done", isErr ? "err" : "ok");
        }
      } catch (e) {
        $("output").textContent = "Internal error: " + ((e && e.message) || e);
        $("output").className = "output error";
        setStatus("error", "err");
      }
    }, 10);
  }
  function showBanner(html) {
    var b = document.createElement("div");
    b.className = "banner";
    b.innerHTML = html;
    document.body.insertBefore(b, document.body.firstChild);
  }
  function bootWasm() {
    setStatus("loading", "busy");
    window.Module = {
      locateFile: function (p) {
        return p;
      },
      printErr: function (t) {
        console.warn("[salam]", t);
      },
      onRuntimeInitialized: function () {
        wasm.runApp = Module.cwrap("salam_web_run_app", "string", [
          "string",
          "string",
        ]);
        wasm.buildLayout = Module.cwrap("salam_web_build_layout", "string", [
          "string",
          "string",
        ]);
        wasm.emit = Module.cwrap("salam_web_emit", "string", [
          "string",
          "string",
          "string",
        ]);
        wasm.ready = true;
        $("run").disabled = false;
        setStatus("ready", "ok");
        if (autorun) run();
      },
    };
    var s = document.createElement("script");
    s.src = "salam-wa.js";
    s.onerror = function () {
      setStatus("error", "err");
      showBanner(
        "The WebAssembly build was not found. Build it with " +
          "<code>sh compiler/tools/bash/build-wasm.sh</code>, then reload. You can still edit code meanwhile.",
      );
    };
    document.body.appendChild(s);
  }
  function setMenu(open) {
    document.body.classList.toggle("menu-open", open);
    var b = $("menu-toggle");
    if (b) b.setAttribute("aria-expanded", open ? "true" : "false");
    if (!open) openDropdown(false);
  }
  function menuOpen() {
    return document.body.classList.contains("menu-open");
  }
  function legacyCopy(text) {
    var ta = document.createElement("textarea");
    ta.value = text;
    ta.style.position = "fixed";
    ta.style.top = "-1000px";
    ta.style.opacity = "0";
    document.body.appendChild(ta);
    ta.focus();
    ta.select();
    try {
      document.execCommand("copy");
    } catch (e) {}
    document.body.removeChild(ta);
  }
  function flashCopied(btn) {
    btn.classList.add("copied");
    var lbl = btn.querySelector(".icon-btn-lbl");
    if (lbl) lbl.textContent = I18N[lang].copied;
    btn.setAttribute("title", I18N[lang].copied);
    clearTimeout(btn._copyT);
    btn._copyT = setTimeout(function () {
      btn.classList.remove("copied");
      if (lbl) lbl.textContent = I18N[lang].copy;
      btn.setAttribute("title", I18N[lang].copy);
    }, 1300);
  }
  function doCopy(text, btn) {
    if (!text) return;
    var ok = function () {
      flashCopied(btn);
    };
    if (navigator.clipboard && navigator.clipboard.writeText) {
      navigator.clipboard.writeText(text).then(ok, function () {
        legacyCopy(text);
        ok();
      });
    } else {
      legacyCopy(text);
      ok();
    }
  }
  function resultText() {
    if (!$("output").hidden) return $("output").textContent;
    if (!$("preview").hidden) return $("preview").getAttribute("srcdoc") || "";
    return "";
  }
  function syncCopyChrome() {
    ["copy-code", "copy-out"].forEach(function (id) {
      var b = $(id);
      if (!b || b.classList.contains("copied")) return;
      b.setAttribute("title", I18N[lang].copy);
      var lbl = b.querySelector(".icon-btn-lbl");
      if (lbl) lbl.textContent = I18N[lang].copy;
    });
  }
  function setAutorun(on) {
    autorun = on;
    var btn = $("autorun");
    btn.classList.toggle("on", on);
    btn.setAttribute("aria-pressed", on ? "true" : "false");
    saveState();
    if (on && wasm.ready) run();
  }
  function onEdit() {
    renderHL();
    if (exampleId !== "custom") {
      var ex = findExample(exampleId);
      if (!ex || editor.getValue() !== ex.code[lang]) {
        exampleId = "custom";
        rebuildDropdown();
      }
    }
    saveState();
    scheduleRun();
  }
  function setupEditor() {
    var ta = $("code");
    editor = {
      el: ta,
      getValue: function () {
        return ta.value;
      },
      setValue: function (v) {
        ta.value = v;
        ta.scrollTop = 0;
        renderHL();
      },
    };
    ta.addEventListener("input", onEdit);
    ta.addEventListener("scroll", function () {
      var hl = $("hl");
      hl.scrollTop = ta.scrollTop;
      hl.scrollLeft = ta.scrollLeft;
    });
    var INDENT = "    ";
    function setRange(start, end, text, selS, selE) {
      ta.focus();
      ta.selectionStart = start;
      ta.selectionEnd = end;
      var ok = false;
      try {
        ok = document.execCommand("insertText", false, text);
      } catch (_) {
        ok = false;
      }
      if (!ok) {
        ta.value = ta.value.slice(0, start) + text + ta.value.slice(end);
        onEdit();
      }
      ta.selectionStart = selS;
      ta.selectionEnd = selE;
    }
    function blockIndent(outdent) {
      var val = ta.value,
        s = ta.selectionStart,
        e = ta.selectionEnd;
      var ls = val.lastIndexOf("\n", s - 1) + 1;
      var eAdj = e > s && val.charAt(e - 1) === "\n" ? e - 1 : e;
      var le = val.indexOf("\n", eAdj);
      if (le < 0) le = val.length;
      var firstDelta = 0,
        totalDelta = 0;
      var out = val
        .slice(ls, le)
        .split("\n")
        .map(function (line, i) {
          var d = 0;
          if (outdent) {
            var m = line.match(/^( {1,4}|\t)/);
            if (m) {
              d = -m[0].length;
              line = line.slice(m[0].length);
            }
          } else if (line.length) {
            d = INDENT.length;
            line = INDENT + line;
          }
          if (i === 0) firstDelta = d;
          totalDelta += d;
          return line;
        })
        .join("\n");
      setRange(ls, le, out, Math.max(ls, s + firstDelta), e + totalDelta);
    }
    function autoIndentEnter() {
      var val = ta.value,
        s = ta.selectionStart,
        e = ta.selectionEnd;
      var ls = val.lastIndexOf("\n", s - 1) + 1;
      var indent = (val.slice(ls, s).match(/^[ \t]*/) || [""])[0];
      var extra = /:$/.test(val.slice(ls, s).replace(/\s+$/, "")) ? INDENT : "";
      var ins = "\n" + indent + extra;
      setRange(s, e, ins, s + ins.length, s + ins.length);
    }
    ta.addEventListener("keydown", function (e) {
      if (e.isComposing || e.keyCode === 229) return;
      var ctrl = e.ctrlKey || e.metaKey;
      if (e.key === "Tab") {
        e.preventDefault();
        var s = ta.selectionStart,
          en = ta.selectionEnd;
        if (e.shiftKey) blockIndent(true);
        else if (val_has_nl(ta.value, s, en)) blockIndent(false);
        else setRange(s, en, INDENT, s + INDENT.length, s + INDENT.length);
      } else if (ctrl && (e.key === "]" || e.code === "BracketRight")) {
        e.preventDefault();
        blockIndent(false);
      } else if (ctrl && (e.key === "[" || e.code === "BracketLeft")) {
        e.preventDefault();
        blockIndent(true);
      } else if (e.key === "Enter" && ctrl) {
        e.preventDefault();
        run();
      } else if (e.key === "Enter" && !e.shiftKey && !e.altKey) {
        e.preventDefault();
        autoIndentEnter();
      }
    });
    function val_has_nl(v, s, e) {
      return v.slice(s, e).indexOf("\n") >= 0;
    }
    var saved = loadState() || {};
    var params = new URLSearchParams(location.search);
    themePref = params.get("theme") || saved.theme || "auto";
    autorun =
      params.get("autorun") === "0"
        ? false
        : params.has("autorun")
          ? true
          : saved.autorun !== false;
    lang = params.get("lang") || saved.lang || "en";
    exampleId = params.get("example") || saved.example || "hello";
    var VIEWS = [
      "run",
      "tokens",
      "ast",
      "symbols",
      "html",
      "css",
      "js",
      "c",
      "llvm",
    ];
    view = params.get("view") || saved.view || "run";
    if (VIEWS.indexOf(view) < 0) view = "run";
    var ex = findExample(exampleId);
    if (ex) {
      mode = ex.mode;
    } else {
      exampleId = "custom";
      mode = params.get("mode") || saved.mode || "app";
    }
    applyTheme();
    applyMode(mode);
    setAutorun(autorun);
    if (exampleId === "custom") ta.value = saved.code != null ? saved.code : "";
    else ta.value = ex.code[lang];
    applyLangChrome();
    rebuildDropdown();
    renderHL();
    bootWasm();
  }
  function applyLangChrome() {
    if (!I18N[lang]) lang = "en";
    var rtl = lang === "fa",
      dict = I18N[lang];
    document.title = dict.title;
    document.documentElement.lang = lang;
    document.documentElement.dir = rtl ? "rtl" : "ltr";
    document.querySelectorAll("[data-i18n]").forEach(function (el) {
      var k = el.getAttribute("data-i18n");
      if (dict[k]) el.textContent = dict[k];
    });
    document.querySelectorAll("#lang-seg .seg-btn").forEach(function (b) {
      b.classList.toggle("active", b.getAttribute("data-lang") === lang);
    });
    $("code").dir = rtl ? "rtl" : "ltr";
    $("output").setAttribute("dir", rtl ? "rtl" : "ltr");
    applyTheme();
    syncCopyChrome();
  }
  document.addEventListener("click", function (e) {
    var t = e.target;
    if (t.closest && t.closest("#menu-toggle")) {
      setMenu(!menuOpen());
      return;
    }
    if (t.closest && t.closest("#scrim")) {
      setMenu(false);
      return;
    }
    var cpc = t.closest && t.closest("#copy-code");
    if (cpc) {
      doCopy(editor ? editor.getValue() : "", cpc);
      return;
    }
    var cpo = t.closest && t.closest("#copy-out");
    if (cpo) {
      doCopy(resultText(), cpo);
      return;
    }
    var seg = t.closest && t.closest(".seg-btn, #autorun, #run, #theme");
    if (seg) {
      if (seg.id === "run") {
        if (view !== "run") {
          view = "run";
          applyView();
          saveState();
        }
        run();
      } else if (seg.id === "autorun") setAutorun(!autorun);
      else if (seg.id === "theme") cycleTheme();
      else if (seg.dataset.lang) applyLang(seg.dataset.lang);
      else if (seg.dataset.view) applyViewChange(seg.dataset.view);
      else if (seg.dataset.mode) {
        if (seg.dataset.mode !== mode)
          selectExample(defaultExampleFor(seg.dataset.mode));
      }
      if (!seg.dataset.view) setMenu(false);
      return;
    }
    var ddBtn = t.closest && t.closest("#dd-button");
    if (ddBtn) {
      openDropdown($("dd-list").hidden);
      return;
    }
    var item = t.closest && t.closest(".dd-item");
    if (item) {
      openDropdown(false);
      selectExample(item.dataset.id);
      setMenu(false);
      return;
    }
    openDropdown(false);
  });
  document.addEventListener("keydown", function (e) {
    if (e.key === "Escape" && menuOpen()) setMenu(false);
  });
  window.addEventListener("resize", function () {
    if (window.innerWidth > 1024 && menuOpen()) setMenu(false);
  });
  setInterval(function () {
    if (themePref === "auto") applyTheme();
  }, 60000);
  function setupSplit() {
    var split = document.querySelector(".split"),
      gutter = $("gutter");
    if (!split || !gutter) return;
    var SPLIT_KEY = "salam.split",
      MIN = 140,
      GUT = 6;
    var rowsMQ = window.matchMedia("(max-width: 900px)");
    var isRows = function () {
      return rowsMQ.matches;
    };
    var saved2;
    try {
      saved2 = JSON.parse(localStorage.getItem(SPLIT_KEY) || "null");
    } catch (e) {
      saved2 = null;
    }
    function applySaved() {
      if (!saved2) return;
      if (isRows()) {
        if (saved2.rows != null)
          split.style.setProperty("--row-a", saved2.rows + "px");
      } else {
        if (saved2.cols != null)
          split.style.setProperty("--col-a", saved2.cols + "px");
      }
    }
    applySaved();
    var dragging = false;
    function onMove(e) {
      if (!dragging) return;
      var rect = split.getBoundingClientRect();
      if (isRows()) {
        var h = Math.max(
          MIN,
          Math.min(e.clientY - rect.top, rect.height - MIN - GUT),
        );
        split.style.setProperty("--row-a", h + "px");
      } else {
        var rtl = document.documentElement.dir === "rtl";
        var w = rtl ? rect.right - e.clientX : e.clientX - rect.left;
        w = Math.max(MIN, Math.min(w, rect.width - MIN - GUT));
        split.style.setProperty("--col-a", w + "px");
      }
      e.preventDefault();
    }
    function onUp() {
      if (!dragging) return;
      dragging = false;
      gutter.classList.remove("dragging");
      document.body.style.cursor = "";
      document.body.style.userSelect = "";
      window.removeEventListener("pointermove", onMove);
      window.removeEventListener("pointerup", onUp);
      if (!saved2) saved2 = {};
      if (isRows())
        saved2.rows = parseFloat(split.style.getPropertyValue("--row-a"));
      else saved2.cols = parseFloat(split.style.getPropertyValue("--col-a"));
      try {
        localStorage.setItem(SPLIT_KEY, JSON.stringify(saved2));
      } catch (e) {}
    }
    gutter.addEventListener("pointerdown", function (e) {
      dragging = true;
      gutter.classList.add("dragging");
      document.body.style.cursor = isRows() ? "row-resize" : "col-resize";
      document.body.style.userSelect = "none";
      window.addEventListener("pointermove", onMove);
      window.addEventListener("pointerup", onUp);
      e.preventDefault();
    });
    gutter.addEventListener("dblclick", function () {
      split.style.removeProperty("--col-a");
      split.style.removeProperty("--row-a");
      saved2 = null;
      try {
        localStorage.removeItem(SPLIT_KEY);
      } catch (e) {}
    });
    (rowsMQ.addEventListener
      ? rowsMQ.addEventListener.bind(rowsMQ, "change")
      : rowsMQ.addListener.bind(rowsMQ))(applySaved);
  }
  setupEditor();
  setupSplit();
})();
