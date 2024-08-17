/**
 * 
 * @var valid_layout_attributes
 * @brief Valid layout attributes
 * @type {ast_layout_attribute_type_t[]}
 */
ast_layout_attribute_type_t valid_layout_attributes[] = {
	AST_LAYOUT_ATTRIBUTE_TYPE_TITLE,
	AST_LAYOUT_ATTRIBUTE_TYPE_DESCRIPTION,
	AST_LAYOUT_ATTRIBUTE_TYPE_AUTHOR,
	AST_LAYOUT_ATTRIBUTE_TYPE_KEYWORDS,
	AST_LAYOUT_ATTRIBUTE_TYPE_ICON,
	AST_LAYOUT_ATTRIBUTE_TYPE_CHARSET,
	AST_LAYOUT_ATTRIBUTE_TYPE_DIR,
	AST_LAYOUT_ATTRIBUTE_TYPE_LANG,
	AST_LAYOUT_ATTRIBUTE_TYPE_VIEWPORT,
	AST_LAYOUT_ATTRIBUTE_TYPE_REFRESH,
	AST_LAYOUT_ATTRIBUTE_TYPE_CHARSET,
};

const ast_layout_attribute_style_pair_t ast_layout_allowed_style_color[] = {
    {"قرمز", "red"},
    {"سفید", "white"},
    {"سیاه", "black"},
    {"نارنجی", "orange"},
    {"زرد", "yellow"},
    {"سبز", "green"},
    {"آبی", "blue"},
    {"ابی", "blue"},
    {"صورتی", "pink"},
};

const ast_layout_attribute_style_pair_t ast_layout_allowed_style_display[] = {
    {"شناور", "float"},
    {"نمایش", "display"},
    {"نهان", "none"},
    {"نمایش-بلوک", "block"},
    {"نمایش-بلوک-داخلی", "inline-block"},
    {"نمایش-فلکس", "flex"},
    {"نمایش-فلکس-داخلی", "inline-flex"},
    {"نمایش-گرید", "grid"},
    {"نمایش-گرید-داخلی", "inline-grid"},
    {"نمایش-جدول", "table"},
    {"نمایش-سطر-جدول", "table-row"},
    {"نمایش-ستون-جدول", "table-column"},
    {"نمایش-سلول-جدول", "table-cell"},
};
