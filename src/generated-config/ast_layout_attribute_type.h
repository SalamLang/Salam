// ----------- BEGIN AUTO GENERATED ----------- //
ADD_LAYOUT_ATTRIBUTE_TYPE(AST_LAYOUT_ATTRIBUTE_TYPE_ERROR, "ERROR", "error", "",
                          "", AST_LAYOUT_ATTRIBUTE_FILTER_STRING, NULL, NULL,
                          AST_LAYOUT_ATTRIBUTE_VALUE_HANDLER_SIMPLE)

ADD_LAYOUT_ATTRIBUTE_TYPE(AST_LAYOUT_ATTRIBUTE_TYPE_GROUP, "GROUP", "group",
                          "گروه", "group",
                          AST_LAYOUT_ATTRIBUTE_FILTER_STRING_ANY, NULL, NULL,
                          AST_LAYOUT_ATTRIBUTE_VALUE_HANDLER_SIMPLE)

ADD_LAYOUT_ATTRIBUTE_TYPE(AST_LAYOUT_ATTRIBUTE_TYPE_FONT_WEIGHT, "FONT_WEIGHT",
                          "font_weight", "وزن", "font-weight",
                          AST_LAYOUT_ATTRIBUTE_FILTER_STRING_ANY, NULL, NULL,
                          AST_LAYOUT_ATTRIBUTE_VALUE_HANDLER_SIMPLE)

ADD_LAYOUT_ATTRIBUTE_TYPE(AST_LAYOUT_ATTRIBUTE_TYPE_FONT_UNICODE_RANGE,
                          "FONT_UNICODE_RANGE", "font_unicode_range",
                          "محدود یونیکد", "unicode-range",
                          AST_LAYOUT_ATTRIBUTE_FILTER_STRING_ANY, NULL, NULL,
                          AST_LAYOUT_ATTRIBUTE_VALUE_HANDLER_SIMPLE)

ADD_LAYOUT_ATTRIBUTE_TYPE(AST_LAYOUT_ATTRIBUTE_TYPE_FONT_STYLE, "FONT_STYLE",
                          "font_style", "سبک", "font-style",
                          AST_LAYOUT_ATTRIBUTE_FILTER_STRING_ANY, NULL, NULL,
                          AST_LAYOUT_ATTRIBUTE_VALUE_HANDLER_SIMPLE)

ADD_LAYOUT_ATTRIBUTE_TYPE(AST_LAYOUT_ATTRIBUTE_TYPE_CLASS, "CLASS", "class",
                          "کلاس", "class",
                          AST_LAYOUT_ATTRIBUTE_FILTER_STRING_ANY, NULL, NULL,
                          AST_LAYOUT_ATTRIBUTE_VALUE_HANDLER_SIMPLE)

ADD_LAYOUT_ATTRIBUTE_TYPE(AST_LAYOUT_ATTRIBUTE_TYPE_ID, "ID", "id", "شناسه",
                          "id", AST_LAYOUT_ATTRIBUTE_FILTER_STRING_ANY, NULL,
                          NULL, AST_LAYOUT_ATTRIBUTE_VALUE_HANDLER_SIMPLE)

ADD_LAYOUT_ATTRIBUTE_TYPE(AST_LAYOUT_ATTRIBUTE_TYPE_CONTENT, "CONTENT",
                          "content", "محتوا", "content",
                          AST_LAYOUT_ATTRIBUTE_FILTER_STRING_ANY, NULL, NULL,
                          AST_LAYOUT_ATTRIBUTE_VALUE_HANDLER_SIMPLE)

ADD_LAYOUT_ATTRIBUTE_TYPE(AST_LAYOUT_ATTRIBUTE_TYPE_FONT_SRC, "FONT_SRC",
                          "font_src", "منبع", "src",
                          AST_LAYOUT_ATTRIBUTE_FILTER_STRING_ANY, NULL, NULL,
                          AST_LAYOUT_ATTRIBUTE_VALUE_HANDLER_FONT_URL)

ADD_LAYOUT_ATTRIBUTE_TYPE(AST_LAYOUT_ATTRIBUTE_TYPE_HREF, "HREF", "href",
                          "منبع", "href",
                          AST_LAYOUT_ATTRIBUTE_FILTER_STRING_ANY, NULL, NULL,
                          AST_LAYOUT_ATTRIBUTE_VALUE_HANDLER_SIMPLE)

ADD_LAYOUT_ATTRIBUTE_TYPE(AST_LAYOUT_ATTRIBUTE_TYPE_SRC, "SRC", "src", "منبع",
                          "src", AST_LAYOUT_ATTRIBUTE_FILTER_STRING_ANY, NULL,
                          NULL, AST_LAYOUT_ATTRIBUTE_VALUE_HANDLER_SIMPLE)

ADD_LAYOUT_ATTRIBUTE_TYPE(AST_LAYOUT_ATTRIBUTE_TYPE_LABEL, "LABEL", "label",
                          "برچسب", "label",
                          AST_LAYOUT_ATTRIBUTE_FILTER_STRING_ANY, NULL, NULL,
                          AST_LAYOUT_ATTRIBUTE_VALUE_HANDLER_SIMPLE)

ADD_LAYOUT_ATTRIBUTE_TYPE(AST_LAYOUT_ATTRIBUTE_TYPE_REPEAT, "REPEAT", "repeat",
                          "تکرار", "repeat", AST_LAYOUT_ATTRIBUTE_FILTER_NUMBER,
                          NULL, NULL, AST_LAYOUT_ATTRIBUTE_VALUE_HANDLER_SIMPLE)

ADD_LAYOUT_ATTRIBUTE_TYPE(AST_LAYOUT_ATTRIBUTE_TYPE_DIR, "DIR", "dir", "جهت",
                          "dir", AST_LAYOUT_ATTRIBUTE_FILTER_STRING,
                          ast_layout_allowed_dir, NULL,
                          AST_LAYOUT_ATTRIBUTE_VALUE_HANDLER_SIMPLE)

ADD_LAYOUT_ATTRIBUTE_TYPE(AST_LAYOUT_ATTRIBUTE_TYPE_LANG, "LANG", "lang",
                          "زبان", "lang", AST_LAYOUT_ATTRIBUTE_FILTER_STRING,
                          ast_layout_allowed_lang, NULL,
                          AST_LAYOUT_ATTRIBUTE_VALUE_HANDLER_SIMPLE)

ADD_LAYOUT_ATTRIBUTE_TYPE(AST_LAYOUT_ATTRIBUTE_TYPE_TITLE, "TITLE", "title",
                          "عنوان", "title",
                          AST_LAYOUT_ATTRIBUTE_FILTER_STRING_ANY, NULL, NULL,
                          AST_LAYOUT_ATTRIBUTE_VALUE_HANDLER_SIMPLE)

ADD_LAYOUT_ATTRIBUTE_TYPE(AST_LAYOUT_ATTRIBUTE_TYPE_TYPE, "TYPE", "type", "نوع",
                          "type", AST_LAYOUT_ATTRIBUTE_FILTER_STRING,
                          ast_layout_allowed_form_type, NULL,
                          AST_LAYOUT_ATTRIBUTE_VALUE_HANDLER_SIMPLE)

ADD_LAYOUT_ATTRIBUTE_TYPE(AST_LAYOUT_ATTRIBUTE_TYPE_KIND, "KIND", "kind", "روش",
                          "target", AST_LAYOUT_ATTRIBUTE_FILTER_STRING,
                          ast_layout_allowed_form_kind, NULL,
                          AST_LAYOUT_ATTRIBUTE_VALUE_HANDLER_SIMPLE)

ADD_LAYOUT_ATTRIBUTE_TYPE(AST_LAYOUT_ATTRIBUTE_TYPE_AUTHOR, "AUTHOR", "author",
                          "نویسنده", "author",
                          AST_LAYOUT_ATTRIBUTE_FILTER_STRING_ANY, NULL, NULL,
                          AST_LAYOUT_ATTRIBUTE_VALUE_HANDLER_SIMPLE)

ADD_LAYOUT_ATTRIBUTE_TYPE(AST_LAYOUT_ATTRIBUTE_TYPE_DESCRIPTION, "DESCRIPTION",
                          "description", "توضیح", "description",
                          AST_LAYOUT_ATTRIBUTE_FILTER_STRING_ANY, NULL, NULL,
                          AST_LAYOUT_ATTRIBUTE_VALUE_HANDLER_SIMPLE)

ADD_LAYOUT_ATTRIBUTE_TYPE(AST_LAYOUT_ATTRIBUTE_TYPE_KEYWORDS, "KEYWORDS",
                          "keywords", "کلمات", "keywords",
                          AST_LAYOUT_ATTRIBUTE_FILTER_STRING_ANY, NULL, NULL,
                          AST_LAYOUT_ATTRIBUTE_VALUE_HANDLER_SIMPLE)

ADD_LAYOUT_ATTRIBUTE_TYPE(AST_LAYOUT_ATTRIBUTE_TYPE_ICON, "ICON", "icon",
                          "نمایه", "icon",
                          AST_LAYOUT_ATTRIBUTE_FILTER_STRING_ANY, NULL, NULL,
                          AST_LAYOUT_ATTRIBUTE_VALUE_HANDLER_SIMPLE)

ADD_LAYOUT_ATTRIBUTE_TYPE(AST_LAYOUT_ATTRIBUTE_TYPE_FONT_NAME, "FONT_NAME",
                          "font_name", "نام", "font-family",
                          AST_LAYOUT_ATTRIBUTE_FILTER_STRING_ANY, NULL, NULL,
                          AST_LAYOUT_ATTRIBUTE_VALUE_HANDLER_SIMPLE)

ADD_LAYOUT_ATTRIBUTE_TYPE(AST_LAYOUT_ATTRIBUTE_TYPE_NAME, "NAME", "name", "نام",
                          "name", AST_LAYOUT_ATTRIBUTE_FILTER_STRING_ANY, NULL,
                          NULL, AST_LAYOUT_ATTRIBUTE_VALUE_HANDLER_SIMPLE)

ADD_LAYOUT_ATTRIBUTE_TYPE(AST_LAYOUT_ATTRIBUTE_TYPE_CHARSET, "CHARSET",
                          "charset", "فرمت متنی", "charset",
                          AST_LAYOUT_ATTRIBUTE_FILTER_STRING,
                          ast_layout_allowed_charset, NULL,
                          AST_LAYOUT_ATTRIBUTE_VALUE_HANDLER_SIMPLE)

ADD_LAYOUT_ATTRIBUTE_TYPE(AST_LAYOUT_ATTRIBUTE_TYPE_REFRESH, "REFRESH",
                          "refresh", "بروزرسانی", "refresh",
                          AST_LAYOUT_ATTRIBUTE_FILTER_STRING_ANY, NULL, NULL,
                          AST_LAYOUT_ATTRIBUTE_VALUE_HANDLER_SIMPLE)

ADD_LAYOUT_ATTRIBUTE_TYPE(AST_LAYOUT_ATTRIBUTE_TYPE_VIEWPORT, "VIEWPORT",
                          "viewport", "محدوده نمایش", "viewport",
                          AST_LAYOUT_ATTRIBUTE_FILTER_STRING_ANY, NULL, NULL,
                          AST_LAYOUT_ATTRIBUTE_VALUE_HANDLER_SIMPLE)

ADD_LAYOUT_ATTRIBUTE_TYPE(AST_LAYOUT_ATTRIBUTE_TYPE_VALUE, "VALUE", "value",
                          "مقدار", "value",
                          AST_LAYOUT_ATTRIBUTE_FILTER_STRING_ANY, NULL, NULL,
                          AST_LAYOUT_ATTRIBUTE_VALUE_HANDLER_SIMPLE)

ADD_LAYOUT_ATTRIBUTE_TYPE(AST_LAYOUT_ATTRIBUTE_TYPE_FOR, "FOR", "for", "برای",
                          "for", AST_LAYOUT_ATTRIBUTE_FILTER_STRING_ANY, NULL,
                          NULL, AST_LAYOUT_ATTRIBUTE_VALUE_HANDLER_SIMPLE)

ADD_LAYOUT_ATTRIBUTE_TYPE(AST_LAYOUT_ATTRIBUTE_TYPE_PLACEHOLDER, "PLACEHOLDER",
                          "placeholder", "راهنما", "placeholder",
                          AST_LAYOUT_ATTRIBUTE_FILTER_STRING_ANY, NULL, NULL,
                          AST_LAYOUT_ATTRIBUTE_VALUE_HANDLER_SIMPLE)

ADD_LAYOUT_ATTRIBUTE_TYPE(AST_LAYOUT_ATTRIBUTE_TYPE_REQUIRED, "REQUIRED",
                          "required", "اجباری", "required",
                          AST_LAYOUT_ATTRIBUTE_FILTER_BOOL, NULL, NULL,
                          AST_LAYOUT_ATTRIBUTE_VALUE_HANDLER_SIMPLE)

ADD_LAYOUT_ATTRIBUTE_TYPE(AST_LAYOUT_ATTRIBUTE_TYPE_RESPONSIVE_MAX_WIDTH,
                          "RESPONSIVE_MAX_WIDTH", "responsive_max_width",
                          "شرط حداکثر عرض", "media-max-width",
                          AST_LAYOUT_ATTRIBUTE_FILTER_NUMBER, NULL, NULL,
                          AST_LAYOUT_ATTRIBUTE_VALUE_HANDLER_SIMPLE)

ADD_LAYOUT_ATTRIBUTE_TYPE(AST_LAYOUT_ATTRIBUTE_TYPE_RESPONSIVE_MAX_HEIGHT,
                          "RESPONSIVE_MAX_HEIGHT", "responsive_max_height",
                          "شرط حداکثر ارتفاع", "media-max-height",
                          AST_LAYOUT_ATTRIBUTE_FILTER_NUMBER, NULL, NULL,
                          AST_LAYOUT_ATTRIBUTE_VALUE_HANDLER_SIMPLE)

ADD_LAYOUT_ATTRIBUTE_TYPE(AST_LAYOUT_ATTRIBUTE_TYPE_RESPONSIVE_MIN_WIDTH,
                          "RESPONSIVE_MIN_WIDTH", "responsive_min_width",
                          "شرط حداقل عرض", "media-min-width",
                          AST_LAYOUT_ATTRIBUTE_FILTER_STRING, NULL, NULL,
                          AST_LAYOUT_ATTRIBUTE_VALUE_HANDLER_SIMPLE)

ADD_LAYOUT_ATTRIBUTE_TYPE(AST_LAYOUT_ATTRIBUTE_TYPE_RESPONSIVE_MIN_HEIGHT,
                          "RESPONSIVE_MIN_HEIGHT", "responsive_min_height",
                          "شرط حداقل ارتفاع", "media-min-height",
                          AST_LAYOUT_ATTRIBUTE_FILTER_STRING, NULL, NULL,
                          AST_LAYOUT_ATTRIBUTE_VALUE_HANDLER_SIMPLE)

// ----------- END AUTO GENERATED ----------- //
