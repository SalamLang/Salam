// ADD_LAYOUT_TYPE(TYPE, NAME, NAME_LOWER, GENERATED_NAME, ENDUSER_NAME)

// Paragraph Types
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_PARAGRAPH, "PARAGRAPH", "paragraph", "p", "پراراگراف")
ADD_LAYOUT_TYPE_HIDE(AST_LAYOUT_TYPE_PARAGRAPH_RAW, "PARAGRAPH_RAW", "paragraph_raw", "content", "محتوا")

// Form Elements
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_BUTTON, "BUTTON", "button", "button", "دکمه")
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_INPUT, "INPUT", "input", "input", "ورودی")
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_TEXTAREA, "TEXTAREA", "textarea", "textarea", "ویرایشگرمتن")
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_LABEL, "LABEL", "label", "label", "برچسب")
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_SELECT, "SELECT", "select", "select", "سلکت باکس")
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_OPTION, "OPTION", "option", "option", "سلکت ایتم")
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_FIELDSET, "FIELDSET", "fieldset", "fieldset", "گروهبندی")
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_LEGEND, "LEGEND", "legend", "legend", "عنوان گروه")
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_FORM, "FORM", "form", "form", "فرم")

// Text Elements
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_SPAN, "SPAN", "span", "span", "قطعه")
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_STRONG, "STRONG", "strong", "strong", "پررنگ پیشرفته")
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_EM, "EM", "em", "em", "اتیالیک پیشرفته")
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_ITALIC, "ITALIC", "italic", "i", "ایتالیک")
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_BOLD, "BOLD", "bold", "b", "پررنگ")
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_UNDERLINE, "UNDERLINE", "underline", "u", "زیرخط")
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_SMALL, "SMALL", "small", "small", "کوچک")
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_BIG, "BIG", "big", "big", "بزرگ")
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_SUB, "SUB", "sub", "sub", "زیرین")
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_SUP, "SUP", "sup", "sup", "رویین")
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_S, "S", "s", "s", "s")
// ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_CENTER, "CENTER", "center", "center", "مرکز")
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_MARK, "MARK", "mark", "mark", "علامتگذاری")
ADD_LAYOUT_TYPE_REPEAT(AST_LAYOUT_TYPE_MARK, "MARK", "mark", "mark", "علامت گذاری")
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_Q, "Q", "q", "q", "نقل قول")
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_CITE, "CITE", "cite", "cite", "اثر")
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_DFN, "DFN", "dfn", "dfn", "اصطلاح")

// Sectioning Elements
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_HEADER, "HEADER", "header", "header", "سربخش")
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_FOOTER, "FOOTER", "footer", "footer", "پاصفحه")
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_NAV, "NAV", "nav", "nav", "ناوبری")
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_SECTION, "SECTION", "section", "section", "بخش")
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_ARTICLE, "ARTICLE", "article", "article", "مقاله")
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_ASIDE, "ASIDE", "aside", "aside", "جداگانه")
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_MAIN, "MAIN", "main", "main", "اصلی")

// Multimedia Elements
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_VIDEO, "VIDEO", "video", "video", "فیلم")
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_AUDIO, "AUDIO", "audio", "audio", "اهنگ")
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_IMG, "IMG", "img", "img", "عکس")
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_IFRAME, "IFRAME", "iframe", "iframe", "سایت")
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_CANVAS, "CANVAS", "canvas", "canvas", "گرافیکی")

// Interactive Elements
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_DETAILS, "DETAILS", "details", "details", "جزئیات")
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_SUMMARY, "SUMMARY", "summary", "summary", "خلاصه")
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_DIALOG, "DIALOG", "dialog", "dialog", "دیالوگ")
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_MENU, "MENU", "menu", "menu", "منو")
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_MENUITEM, "MENUITEM", "menuitem", "menuitem", "آیتم منو")
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_COMMAND, "COMMAND", "command", "command", "فرمان")

// List Elements
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_UL, "UL", "ul", "ul", "لیست غیرمرتب")
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_OL, "OL", "ol", "ol", "لیست مرتب")
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_LI, "LI", "li", "li", "آیتم لیست")

// Table Elements
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_TABLE, "TABLE", "table", "table", "جدول")
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_TABLE_HEADER, "TABLE_HEADER", "table_header", "thead", "سر جدول")
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_TABLE_BODY, "TABLE_BODY", "table_body", "tbody", "بدنه جدول")
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_TABLE_FOOTER, "TABLE_FOOTER", "table_footer", "tfoot", "پاصفحه جدول")
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_TABLE_TR, "TABLE_TR", "table_tr", "tr", "ردیف")
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_TABLE_TD, "TABLE_TD", "table_td", "td", "سلول")
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_TABLE_TH, "TABLE_TH", "table_th", "th", "سرسلول")
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_CAPTION, "CAPTION", "caption", "caption", "عنوان جدول")
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_COL, "COL", "col", "col", "ستون")
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_COLGROUP, "COLGROUP", "colgroup", "colgroup", "گروه ستون‌ها")

// Miscellaneous Elements
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_BLOCKQUOTE, "BLOCKQUOTE", "blockquote", "blockquote", "نقل قول بلوکی")
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_PRE, "PRE", "pre", "pre", "متن پیش‌قالب‌بندی‌شده")
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_CODE, "CODE", "code", "code", "کد")
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_BR, "BR", "br", "br", "خط بعدی")
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_HR, "HR", "hr", "hr", "خط")
// ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_DEL, "DEL", "del", "del", "حذف شده")
// ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_INS, "INS", "ins", "ins", "اضافه شده")
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_ADDRESS, "ADDRESS", "address", "address", "آدرس")
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_TIME, "TIME", "time", "time", "زمان")
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_PROGRESS, "PROGRESS", "progress", "progress", "پیشرفت")
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_METER, "METER", "meter", "meter", "سنجش")

// Script and Style Elements
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_SCRIPT, "SCRIPT", "script", "script", "اسکریپت")
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_STYLE, "STYLE", "style", "style", "استایل")

// Special Elements
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_DIV, "DIV", "div", "div", "جعبه")
ADD_LAYOUT_TYPE(AST_LAYOUT_TYPE_LINK, "LINK", "link", "a", "لینک")

// Utility Elements
ADD_LAYOUT_TYPE_HIDE(AST_LAYOUT_TYPE_NONE, "NONE", "none", "layout", "صفحه")
ADD_LAYOUT_TYPE_HIDE(AST_LAYOUT_TYPE_ERROR, "ERROR", "error", "error", "خطا")
