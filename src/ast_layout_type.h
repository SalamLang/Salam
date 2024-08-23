// SIGNATURE: ADD_LAYOUT_TYPE(TYPE, NAME, NAME_LOWER, GENERATED_NAME, ENDUSER_NAME, IS_SINGLE_TAG)

// Paragraph Types
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_PARAGRAPH, "PARAGRAPH", "paragraph", "p", "پراراگراف", false)
ADD_LAYOUT_TYPE_HIDE(AST_LAYOUT_TYPE_PARAGRAPH_RAW, "PARAGRAPH_RAW", "paragraph_raw", "content", "محتوا", false)

// Form Elements
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_BUTTON, "BUTTON", "button", "button", "دکمه", false)
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_TEXTAREA, "TEXTAREA", "textarea", "textarea", "ویرایشگرمتن", false)
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_LABEL, "LABEL", "label", "label", "برچسب", false)
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_SELECT, "SELECT", "select", "select", "سلکت باکس", false)
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_OPTION, "OPTION", "option", "option", "سلکت ایتم", false)
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_FIELDSET, "FIELDSET", "fieldset", "fieldset", "گروهبندی", false)
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_LEGEND, "LEGEND", "legend", "legend", "عنوان گروه", false)
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_FORM, "FORM", "form", "form", "فرم", false)
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_OPTGROUP, "OPTGROUP", "optgroup", "optgroup", "گروه انتخاب", false)
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_DATALIST, "DATALIST", "datalist", "datalist", "داده لیست", false)
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_OUTPUT, "OUTPUT", "output", "output", "خروجی", false)
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_KEYGEN, "KEYGEN", "keygen", "keygen", "تولید کلید", false)

// Text Elements
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_SPAN, "SPAN", "span", "span", "قطعه", false)
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_STRONG, "STRONG", "strong", "strong", "درشت مهم", false)
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_EM, "EM", "em", "em", "اتیالیک پیشرفته", false)
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_ITALIC, "ITALIC", "italic", "i", "ایتالیک", false)
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_BOLD, "BOLD", "bold", "b", "درشت", false)
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_UNDERLINE, "UNDERLINE", "underline", "u", "زیرخط", false)
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_SMALL, "SMALL", "small", "small", "کوچک", false)
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_BIG, "BIG", "big", "big", "بزرگ", false)
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_SUB, "SUB", "sub", "sub", "زیرین", false)
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_SUP, "SUP", "sup", "sup", "رویین", false)
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_S, "S", "s", "s", "s", false)
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_MARK, "MARK", "mark", "mark", "علامتگذاری", false)
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_Q, "Q", "q", "q", "نقل قول", false)
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_CITE, "CITE", "cite", "cite", "اثر", false)
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_DFN, "DFN", "dfn", "dfn", "اصطلاح", false)
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_ABBR, "ABBR", "abbr", "abbr", "اختصار", false)
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_KBD, "KBD", "kbd", "kbd", "ورودی کیبورد", false)
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_SAMP, "SAMP", "samp", "samp", "نمونه", false)
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_VAR, "VAR", "var", "var", "متغیر", false)
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_RUBY, "RUBY", "ruby", "ruby", "روبین", false)
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_RT, "RT", "rt", "rt", "متن روبین", false)
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_RP, "RP", "rp", "rp", "جایگزین روبین", false)
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_BDI, "BDI", "bdi", "bdi", "جداگانه", false)
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_BDO, "BDO", "bdo", "bdo", "جهت دهی", false)
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_WBR, "WBR", "wbr", "wbr", "شکستن کلمه", false)

// Sectioning Elements
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_HEADER, "HEADER", "header", "header", "سربخش", false)
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_FOOTER, "FOOTER", "footer", "footer", "پاصفحه", false)
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_NAV, "NAV", "nav", "nav", "ناوبری", false)
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_SECTION, "SECTION", "section", "section", "بخش", false)
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_ARTICLE, "ARTICLE", "article", "article", "مقاله", false)
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_ASIDE, "ASIDE", "aside", "aside", "جداگانه", false)
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_MAIN, "MAIN", "main", "main", "اصلی", false)
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_HGROUP, "HGROUP", "hgroup", "hgroup", "گروه هدر", false)
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_FIGURE, "FIGURE", "figure", "figure", "شکل", false)
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_FIGCAPTION, "FIGCAPTION", "figcaption", "figcaption", "زیرنویس شکل", false)

// Multimedia Elements
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_VIDEO, "VIDEO", "video", "video", "فیلم", false)
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_AUDIO, "AUDIO", "audio", "audio", "اهنگ", false)
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_IFRAME, "IFRAME", "iframe", "iframe", "سایت", false)
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_CANVAS, "CANVAS", "canvas", "canvas", "گرافیکی", false)

// Interactive Elements
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_DETAILS, "DETAILS", "details", "details", "جزئیات", false)
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_SUMMARY, "SUMMARY", "summary", "summary", "خلاصه", false)
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_DIALOG, "DIALOG", "dialog", "dialog", "دیالوگ", false)
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_MENU, "MENU", "menu", "menu", "منو", false)
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_MENUITEM, "MENUITEM", "menuitem", "menuitem", "آیتم منو", false)
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_COMMAND, "COMMAND", "command", "command", "فرمان", false)

// List Elements
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_UL, "UL", "ul", "ul", "لیست غیرمرتب", false)
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_OL, "OL", "ol", "ol", "لیست مرتب", false)
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_LI, "LI", "li", "li", "آیتم لیست", false)

// Table Elements
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_TABLE, "TABLE", "table", "table", "جدول", false)
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_TABLE_HEADER, "TABLE_HEADER", "table_header", "thead", "سر جدول", false)
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_TABLE_BODY, "TABLE_BODY", "table_body", "tbody", "بدنه جدول", false)
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_TABLE_FOOTER, "TABLE_FOOTER", "table_footer", "tfoot", "پاصفحه جدول", false)
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_TABLE_TR, "TABLE_TR", "table_tr", "tr", "ردیف", false)
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_TABLE_TD, "TABLE_TD", "table_td", "td", "سلول", false)
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_TABLE_TH, "TABLE_TH", "table_th", "th", "سرسلول", false)
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_CAPTION, "CAPTION", "caption", "caption", "عنوان جدول", false)
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_COL, "COL", "col", "col", "ستون", false)
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_COLGROUP, "COLGROUP", "colgroup", "colgroup", "گروه ستون‌ها", false)

// Miscellaneous Elements
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_BLOCKQUOTE, "BLOCKQUOTE", "blockquote", "blockquote", "نقل قول بلوکی", false)
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_PRE, "PRE", "pre", "pre", "متن پیش‌قالب‌بندی‌شده", false)
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_CODE, "CODE", "code", "code", "کد", false)
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_ADDRESS, "ADDRESS", "address", "address", "آدرس", false)
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_TIME, "TIME", "time", "time", "زمان", false)
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_PROGRESS, "PROGRESS", "progress", "progress", "پیشرفت", false)
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_METER, "METER", "meter", "meter", "سنجش", false)

// Script and Style Elements
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_SCRIPT, "SCRIPT", "script", "script", "اسکریپت", false)
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_STYLE, "STYLE", "style", "style", "ظاهر", false)

// Special Elements
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_DIV, "DIV", "div", "div", "جعبه", false)
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_LINK, "LINK", "a", "link", "لینک", false)

// Utility Elements
ADD_LAYOUT_TYPE_HIDE(AST_LAYOUT_TYPE_NONE, "NONE", "none", "layout", "صفحه", false)
ADD_LAYOUT_TYPE_HIDE(AST_LAYOUT_TYPE_ERROR, "ERROR", "error", "error", "خطا", false)

// Single Elements
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_BR, "BR", "br", "br", "خط بعدی", true)
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_HR, "HR", "hr", "hr", "خط", true)
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_INPUT, "INPUT", "input", "input", "ورودی", true)
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_IMG, "IMG", "img", "img", "تصویر", true)
ADD_LAYOUT_TYPE_REPEAT(AST_LAYOUT_TYPE_IMG, "IMG", "img", "img", "عکس", true)

// Own/Custom Elements
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_FONT, "FONT", "font", "font", "فونت", true)

ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_MEDIA_MAX_WIDTH, "RESPONSIVE", "responsive", "@media only screen and (max-width:", "پذیرا حداکثر عرض", true)
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_MEDIA_MIN_WIDTH, "RESPONSIVE", "responsive", "@media only screen and (min-width:", "پذیرا حداقل عرض", true)
