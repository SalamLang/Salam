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

const ast_layout_attribute_style_pair_t ast_layout_allowed_style_width[] = {
    {"خودکار", "auto"},
    {"حداکثر محتوا", "max-content"},
    {"حداقل محتوا", "min-content"},
    {"فیت", "fit-content"},
};

const ast_layout_attribute_style_pair_t ast_layout_allowed_style_height[] = {
    {"خودکار", "auto"},
    {"حداکثر محتوا", "max-content"},
    {"حداقل محتوا", "min-content"},
    {"فیت", "fit-content"},
};

const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_accent_color[] = {
    {"سیاه", "black"},
    {"سفید", "white"},
    {"قرمز", "red"},
    {"سبز", "green"},
    {"آبی", "blue"},
    {"زرد", "yellow"},
    {"آبی تیره", "darkblue"},
    {"سبز تیره", "darkgreen"},
    {"خاکستری", "gray"},
    {"بدون رنگ", "transparent"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_appearance[] = {
    {"مدیر کنترل پیش‌فرض", "auto"},
    {"دکمه", "button"},
    {"پیش‌فرض", "default"},
    {"متن", "text"},
    {"منوی کشویی", "dropdown"},
    {"فیلد متنی", "textfield"},
    {"خروجی", "output"},
    {"مخفی", "none"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_aspect_ratio[] = {
    {"خودکار", "auto"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_backface_visibility[] = {
    {"مشاهده‌پذیر", "visible"},
    {"پنهان", "hidden"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_block_size[] = {
    {"خودکار", "auto"},
    {"حداقل با محتوا", "min-content"},
    {"ماکزیمم محتوا", "max-content"},
    {"فیت", "fit-content"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_background_image[] = {
    {"هیچ", "none"},
};

const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_background_attachment[] = {
    {"پیمایش", "scroll"},
    {"ثابت", "fixed"},
    {"محلی", "local"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_background_blend_mode[] = {
    {"نرمال", "normal"},
    {"سخت", "multiply"},
    {"افزودن", "screen"},
    {"حذف", "overlay"},
    {"تیره کردن", "darken"},
    {"روشن کردن", "lighten"},
    {"سایه", "color-dodge"},
    {"فراکتال", "color-burn"},
    {"حالت ترکیب", "hue"},
    {"مات", "saturation"},
    {"رنگ", "color"},
    {"تن سایه", "luminosity"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_background_clip[] = {
    {"محتوا", "content-box"},
    {"حاشیه", "padding-box"},
    {"کل عنصر", "border-box"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_background_color[] = {
    {"سفید", "white"},
    {"سیاه", "black"},
    {"قرمز", "red"},
    {"سبز", "green"},
    {"آبی", "blue"},
    {"زرد", "yellow"},
    {"خاکستری", "gray"},
    {"بی‌رنگ", "transparent"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_background_origin[] = {
    {"محتوا", "content-box"},
    {"حاشیه", "padding-box"},
    {"کل عنصر", "border-box"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_background_position_x[] = {
    {"چپ", "left"},
    {"مرکز", "center"},
    {"راست", "right"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_background_position_y[] = {
    {"بالا", "top"},
    {"مرکز", "center"},
    {"پایین", "bottom"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_background_repeat[] = {
    {"تکرار", "repeat"},
    {"تکرار در محور افقی", "repeat-x"},
    {"تکرار در محور عمودی", "repeat-y"},
    {"هیچ", "no-repeat"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_background_size[] = {
    {"خودکار", "auto"},
    {"پوشش", "cover"},
    {"مناسب با محتوا", "contain"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_border_block_end_color[] = {
    {"سفید", "white"},
    {"سیاه", "black"},
    {"قرمز", "red"},
    {"سبز", "green"},
    {"آبی", "blue"},
    {"خاکستری", "gray"},
    {"بی‌رنگ", "transparent"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_border_block_end_style[] = {
    {"پیوسته", "solid"},
    {"خط‌چین", "dashed"},
    {"نقطه‌چین", "dotted"},
    {"دوگانه", "double"},
    {"پنهان", "none"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_border_block_end_width[] = {
    {"خودکار", "auto"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_border_block_start_color[] = {
    {"سفید", "white"},
    {"سیاه", "black"},
    {"قرمز", "red"},
    {"سبز", "green"},
    {"آبی", "blue"},
    {"خاکستری", "gray"},
    {"بی‌رنگ", "transparent"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_border_block_start_style[] = {
    {"پیوسته", "solid"},
    {"خط‌چین", "dashed"},
    {"نقطه‌چین", "dotted"},
    {"دوگانه", "double"},
    {"پنهان", "none"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_border_block_start_width[] = {
    {"خودکار", "auto"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_border_bottom_color[] = {
    {"سفید", "white"},
    {"سیاه", "black"},
    {"قرمز", "red"},
    {"سبز", "green"},
    {"آبی", "blue"},
    {"خاکستری", "gray"},
    {"بی‌رنگ", "transparent"},
};

const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_border_bottom_left_radius[] = {

};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_border_bottom_right_radius[] = {

};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_border_bottom_style[] = {
    {"پیوسته", "solid"},
    {"خط‌چین", "dashed"},
    {"نقطه‌چین", "dotted"},
    {"دوگانه", "double"},
    {"پنهان", "none"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_border_bottom_width[] = {

};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_border_collapse[] = {
    {"بله", "collapse"},
    {"خیر", "separate"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_border_end_end_radius[] = {

};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_border_end_start_radius[] = {

};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_border_image_outset[] = {

};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_border_image_repeat[] = {
    {"تکرار", "repeat"},
    {"تکرار افقی", "repeat-x"},
    {"تکرار عمودی", "repeat-y"},
    {"بدون تکرار", "no-repeat"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_border_image_slice[] = {

};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_border_image_source[] = {
    {"هیچ", "none"},
    {"تصویر پایه", "url('base_image.jpg')"},
    {"تصویر آبی", "url('blue_image.png')"},
    {"تصویر سبز", "url('green_image.png')"},
    {"تصویر خاکستری", "url('gray_image.jpg')"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_border_image_width[] = {
    {"خودکار", "auto"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_border_inline_end_color[] = {
    {"سفید", "white"},
    {"سیاه", "black"},
    {"قرمز", "red"},
    {"سبز", "green"},
    {"آبی", "blue"},
    {"خاکستری", "gray"},
    {"بی‌رنگ", "transparent"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_border_inline_end_style[] = {
    {"پیوسته", "solid"},
    {"خط‌چین", "dashed"},
    {"نقطه‌چین", "dotted"},
    {"دوگانه", "double"},
    {"پنهان", "none"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_border_inline_end_width[] = {

};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_border_inline_start_color[] = {
    {"سفید", "white"},
    {"سیاه", "black"},
    {"قرمز", "red"},
    {"سبز", "green"},
    {"آبی", "blue"},
    {"خاکستری", "gray"},
    {"بی‌رنگ", "transparent"},
};

const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_border_inline_start_style[] = {
    {"پیوسته", "solid"},
    {"خط‌چین", "dashed"},
    {"نقطه‌چین", "dotted"},
    {"دوگانه", "double"},
    {"پنهان", "none"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_border_inline_start_width[] = {

};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_border_left_color[] = {
    {"سفید", "white"},
    {"سیاه", "black"},
    {"قرمز", "red"},
    {"سبز", "green"},
    {"آبی", "blue"},
    {"خاکستری", "gray"},
    {"بی‌رنگ", "transparent"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_border_left_style[] = {
    {"پیوسته", "solid"},
    {"خط‌چین", "dashed"},
    {"نقطه‌چین", "dotted"},
    {"دوگانه", "double"},
    {"پنهان", "none"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_border_left_width[] = {

};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_border_right_color[] = {
    {"سفید", "white"},
    {"سیاه", "black"},
    {"قرمز", "red"},
    {"سبز", "green"},
    {"آبی", "blue"},
    {"خاکستری", "gray"},
    {"بی‌رنگ", "transparent"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_border_right_style[] = {
    {"پیوسته", "solid"},
    {"خط‌چین", "dashed"},
    {"نقطه‌چین", "dotted"},
    {"دوگانه", "double"},
    {"پنهان", "none"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_border_right_width[] = {

};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_border_spacing[] = {

};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_border_start_end_radius[] = {

};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_border_start_start_radius[] = {

};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_border_top_color[] = {
    {"سفید", "white"},
    {"سیاه", "black"},
    {"قرمز", "red"},
    {"سبز", "green"},
    {"آبی", "blue"},
    {"خاکستری", "gray"},
    {"بی‌رنگ", "transparent"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_border_top_left_radius[] = {

};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_border_top_right_radius[] = {

};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_border_top_style[] = {
    {"پیوسته", "solid"},
    {"خط‌چین", "dashed"},
    {"نقطه‌چین", "dotted"},
    {"دوگانه", "double"},
    {"پنهان", "none"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_border_top_width[] = {

};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_bottom[] = {
    {"خودکار", "auto"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_box_sizing[] = {
    {"محتویات", "content-box"},
    {"مرزی", "border-box"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_break_inside[] = {
    {"اجتناب", "avoid"},
    {"اجتناب پرچم", "avoid-page"},
    {"اجتناب صفحه", "avoid-column"},
    {"شکست", "auto"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_break_before[] = {
    {"شکست", "auto"},
    {"صفحه", "page"},
    {"ستون", "column"},
    {"خلاصه", "region"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_break_after[] = {
    {"شکست", "auto"},
    {"صفحه", "page"},
    {"ستون", "column"},
    {"خلاصه", "region"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_caption_side[] = {
    {"بالا", "top"},
    {"پایین", "bottom"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_caret_color[] = {
    {"سیاه", "black"},
    {"سفید", "white"},
    {"قرمز", "red"},
    {"سبز", "green"},
    {"آبی", "blue"},
    {"خاکستری", "gray"},
    {"بی‌رنگ", "transparent"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_clear[] = {
    {"هیچ", "none"},
    {"چپ", "left"},
    {"راست", "right"},
    {"هر دو", "both"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_clip_path[] = {
    {"هیچ", "none"},
    {"دایره", "circle()"},
    {"مستطیل", "rect()"},
    {"پنج‌ضلعی", "polygon()"},
    {"مسیر", "path()"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_clip_rule[] = {
    {"تازه", "nonzero"},
    {"یکسان", "evenodd"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_color[] = {
    {"سیاه", "black"},
    {"سفید", "white"},
    {"قرمز", "red"},
    {"سبز", "green"},
    {"آبی", "blue"},
    {"خاکستری", "gray"},
    {"بی‌رنگ", "transparent"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_color_interpolation[] = {
    {"سرویس", "auto"},
    {"RGB", "srgb"},
    {"خطی", "linearRGB"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_color_interpolation_filters[] = {
    {"سرویس", "auto"},
    {"RGB", "srgb"},
    {"خطی", "linearRGB"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_color_scheme[] = {
    {"خودکار", "auto"},
    {"روشن", "light"},
    {"تاریک", "dark"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_column_count[] = {
    {"خودکار", "auto"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_column_fill[] = {
    {"نیمه", "auto"},
    {"بلافاصله", "balance"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_column_gap[] = {

};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_column_rule_color[] = {
    {"سیاه", "black"},
    {"سفید", "white"},
    {"قرمز", "red"},
    {"سبز", "green"},
    {"آبی", "blue"},
    {"خاکستری", "gray"},
    {"بی‌رنگ", "transparent"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_column_rule_style[] = {
    {"پیوسته", "solid"},
    {"خط‌چین", "dashed"},
    {"نقطه‌چین", "dotted"},
    {"دوگانه", "double"},
    {"پنهان", "none"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_column_rule_width[] = {

};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_column_span[] = {
    {"یک ستون", "none"},
    {"بیش از یک ستون", "all"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_columns[] = {
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_contain[] = {
    {"خودکار", "auto"},
    {"موارد", "content"},
    {"حساب", "layout"},
    {"سایز", "size"},
    {"حساب و سایز", "layout size"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_contain_intrinsic_block_size[] = {
    {"خودکار", "auto"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_contain_intrinsic_height[] = {
    {"خودکار", "auto"},
};

const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_contain_intrinsic_inline_size[] = {
    {"خودکار", "auto"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_contain_intrinsic_size[] = {
    {"خودکار", "auto"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_contain_intrinsic_width[] = {
    {"خودکار", "auto"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_container[] = {
    {"خودکار", "auto"},
    {"کامل", "inline-size"},
    {"بیشتر", "block-size"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_container_name[] = {
    {"نام", "name"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_container_type[] = {
    {"خودکار", "auto"},
    {"بزرگ", "inline-size"},
    {"بیشتر", "block-size"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_content[] = {
    {"خالی", "none"},
    {"محتوا", "content"},
    {"نام", "var()"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_content_visibility[] = {
    {"پنهان", "hidden"},
    {"معلوم", "visible"},
    {"مشاهده", "auto"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_counter_increment[] = {
    {"عدد", "number"},
    {"حساب", "counter()"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_counter_reset[] = {
    {"صفر", "0"},
    {"تعداد", "counter()"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_counter_set[] = {
    {"عدد", "number"},
    {"شمارنده", "counter()"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_cursor[] = {
    {"دست", "pointer"},
    {"نشانگر", "default"},
    {"مشکل", "not-allowed"},
    {"متن", "text"},
    {"بازی", "progress"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_direction[] = {
    {"چپ به راست", "ltr"},
    {"راست به چپ", "rtl"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_display[] = {
    {"بلوک", "block"},
    {"خطی", "inline"},
    {"بلوک خطی", "inline-block"},
    {"پنهان", "none"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_empty_cells[] = {
    {"پنهان", "hide"},
    {"نمایش", "show"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_fill[] = {
    {"رنگ", "color"},
    {"تصویر", "image"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_fill_opacity[] = {

};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_fill_rule[] = {
    {"تازه", "nonzero"},
    {"یکسان", "evenodd"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_filter[] = {
    {"مات", "blur()"},
    {"گرم", "brightness()"},
    {"کنتراست", "contrast()"},
    {"گرادیان", "drop-shadow()"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_flex[] = {
    {"خودکار", "auto"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_flex_basis[] = {
    {"خودکار", "auto"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_flex_direction[] = {
    {"افقی", "row"},
    {"عمودی", "column"},
    {"افقی معکوس", "row-reverse"},
    {"عمودی معکوس", "column-reverse"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_flex_flow[] = {
    {"جهت فلکس و چینش", "row wrap"},
    {"پخش", "row nowrap"},
    {"جهت عمودی و چینش", "column wrap"},
    {"عمودی و پخش", "column nowrap"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_flex_grow[] = {

};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_flex_shrink[] = {

};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_flex_wrap[] = {
    {"پهن", "nowrap"},
    {"چینش", "wrap"},
    {"چینش معکوس", "wrap-reverse"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_float[] = {
    {"چپ", "left"},
    {"راست", "right"},
    {"خالی", "none"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_font[] = {
    {"خودکار", "auto"},
    {"خانواده", "family"},
    {"اندازه", "size"},
    {"سبک", "style"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_font_family[] = {
    {"سنس", "sans-serif"},
    {"سریف", "serif"},
    {"مونو", "monospace"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_font_feature_settings[] = {
    {"خودکار", "auto"},
    {"فعال", "normal"},
    {"غیرفعال", "none"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_font_kerning[] = {
    {"خودکار", "auto"},
    {"نه", "none"},
    {"بله", "normal"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_font_language_override[] = {
    {"خودکار", "auto"},
    {"غیر فعال", "normal"},
    {"فعال", "override"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_font_optical_sizing[] = {
    {"خودکار", "auto"},
    {"سازگار", "optical"},
    {"غیر سازگار", "none"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_font_size[] = {

};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_font_stretch[] = {
    {"پهن", "ultra-condensed"},
    {"متوسط", "normal"},
    {"کشیده", "ultra-expanded"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_font_style[] = {
    {"نرمال", "normal"},
    {"خوب", "italic"},
    {"خمیده", "oblique"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_font_variant[] = {
    {"نرمال", "normal"},
    {"حرف‌های بزرگ", "small-caps"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_font_weight[] = {
    {"نرمال", "normal"},
    {"سنگین", "bold"},
    {"خیلی سنگین", "bolder"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_grid[] = {
    {"خودکار", "auto"},
    {"ستون", "column"},
    {"ردیف", "row"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_grid_area[] = {
    {"خودکار", "auto"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_grid_auto_columns[] = {
    {"خودکار", "auto"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_grid_auto_flow[] = {
    {"خودکار", "auto"},
    {"ستون", "column"},
    {"ردیف", "row"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_grid_auto_rows[] = {
    {"خودکار", "auto"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_grid_column[] = {
    {"خودکار", "auto"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_grid_column_end[] = {
    {"خودکار", "auto"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_grid_column_start[] = {
    {"خودکار", "auto"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_grid_row[] = {
    {"خودکار", "auto"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_grid_row_end[] = {
    {"خودکار", "auto"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_grid_row_start[] = {
    {"خودکار", "auto"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_grid_template[] = {
    {"خودکار", "auto"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_grid_template_areas[] = {
    {"خودکار", "auto"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_grid_template_columns[] = {
    {"خودکار", "auto"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_grid_template_rows[] = {
    {"خودکار", "auto"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_height[] = {
    {"خودکار", "auto"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_justify_content[] = {
    {"چپ", "flex-start"},
    {"مرکز", "center"},
    {"راست", "flex-end"},
    {"پخش", "space-between"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_left[] = {
    {"خودکار", "auto"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_letter_spacing[] = {

};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_line_height[] = {

};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_list_style[] = {
    {"نشان‌دهنده", "disc"},
    {"دایره‌ای", "circle"},
    {"مستطیل", "square"},
    {"غیر فعال", "none"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_margin[] = {
    {"خودکار", "auto"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_margin_bottom[] = {
    {"خودکار", "auto"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_margin_left[] = {
    {"خودکار", "auto"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_margin_right[] = {
    {"خودکار", "auto"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_margin_top[] = {
    {"خودکار", "auto"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_max_height[] = {

};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_max_width[] = {

};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_min_height[] = {

};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_min_width[] = {

};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_object_fit[] = {
    {"پوشش", "cover"},
    {"پوشش کامل", "contain"},
    {"خودکار", "auto"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_opacity[] = {

};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_overflow[] = {
    {"پنهان", "hidden"},
    {"غیر فعال", "visible"},
    {"چرخش", "scroll"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_overflow_x[] = {
    {"پنهان", "hidden"},
    {"غیر فعال", "visible"},
    {"چرخش", "scroll"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_overflow_y[] = {
    {"پنهان", "hidden"},
    {"غیر فعال", "visible"},
    {"چرخش", "scroll"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_padding[] = {
    {"خودکار", "auto"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_padding_bottom[] = {
    {"خودکار", "auto"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_padding_left[] = {
    {"خودکار", "auto"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_padding_right[] = {
    {"خودکار", "auto"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_padding_top[] = {
    {"خودکار", "auto"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_position[] = {
    {"ثابت", "fixed"},
    {"مطلق", "absolute"},
    {"نسبی", "relative"},
    {"چسبیده", "sticky"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_right[] = {
    {"خودکار", "auto"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_text_align[] = {
    {"چپ", "left"},
    {"مرکز", "center"},
    {"راست", "right"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_text_decoration[] = {
    {"زیرخط", "underline"},
    {"خط خورده", "line-through"},
    {"بی‌خط", "none"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_text_transform[] = {
    {"نرمال", "none"},
    {"بزرگ", "uppercase"},
    {"کوچک", "lowercase"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_top[] = {
    {"خودکار", "auto"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_visibility[] = {
    {"مشهود", "visible"},
    {"نامرئی", "hidden"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_width[] = {
    {"خودکار", "auto"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_z_index[] = {
    {"پایین", "0"},
    {"میانه", "1"},
    {"بالا", "9999"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_outline_color[] = {
    {"هیچ", "none"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_outline_offset[] = {

};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_outline_style[] = {
    {"خط‌چین", "dashed"},
    {"خط‌دار", "dotted"},
    {"مستقیم", "solid"},
    {"دوگانه", "double"},
    {"موجی", "groove"},
    {"فرورفته", "inset"},
    {"برآمده", "outset"},
    {"هیچ", "none"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_outline_width[] = {
    {"خودکار", "auto"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_overflow[] = {
    {"پنهان", "hidden"},
    {"غیر فعال", "visible"},
    {"چرخش", "scroll"},
    {"خودکار", "auto"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_overflow_anchor[] = {
    {"بله", "auto"},
    {"خیر", "none"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_overflow_block[] = {
    {"پنهان", "hidden"},
    {"غیر فعال", "visible"},
    {"چرخش", "scroll"},
    {"خودکار", "auto"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_overflow_clip_margin[] = {

};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_overflow_inline[] = {
    {"پنهان", "hidden"},
    {"غیر فعال", "visible"},
    {"چرخش", "scroll"},
    {"خودکار", "auto"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_overflow_wrap[] = {
    {"بله", "break-word"},
    {"خیر", "normal"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_overflow_x[] = {
    {"پنهان", "hidden"},
    {"غیر فعال", "visible"},
    {"چرخش", "scroll"},
    {"خودکار", "auto"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_overflow_y[] = {
    {"پنهان", "hidden"},
    {"غیر فعال", "visible"},
    {"چرخش", "scroll"},
    {"خودکار", "auto"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_padding[] = {
    {"خودکار", "auto"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_padding[] = {
    {"خودکار", "auto"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_padding_block_end[] = {
    {"خودکار", "auto"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_padding_block_start[] = {
    {"خودکار", "auto"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_padding_bottom[] = {
    {"خودکار", "auto"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_padding_inline[] = {
    {"خودکار", "auto"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_padding_inline_end[] = {
    {"خودکار", "auto"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_padding_inline_start[] = {
    {"خودکار", "auto"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_padding_left[] = {
    {"خودکار", "auto"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_padding_right[] = {
    {"خودکار", "auto"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_padding_top[] = {
    {"خودکار", "auto"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_page[] = {
    {"بله", "auto"},
    {"خیر", "none"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_page_break_after[] = {
    {"بله", "always"},
    {"خیر", "auto"},
};

const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_page_break_before[] = {
    {"بله", "always"},
    {"خیر", "auto"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_page_break_inside[] = {
    {"بله", "auto"},
    {"خیر", "avoid"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_paint_order[] = {
    {"محتوا", "content"},
    {"حاشیه", "border"},
    {"پدینگ", "padding"},
    {"همه", "all"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_perspective[] = {
    {"", ""},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_perspective_origin[] = {

};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_place_content[] = {
    {"فضا", "space-between"},
    {"مرکز", "center"},
    {"چپ", "start"},
    {"راست", "end"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_place_items[] = {
    {"فضا", "space-between"},
    {"مرکز", "center"},
    {"چپ", "start"},
    {"راست", "end"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_place_self[] = {
    {"فضا", "space-between"},
    {"مرکز", "center"},
    {"چپ", "start"},
    {"راست", "end"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_pointer_events[] = {
    {"فعال", "auto"},
    {"غیرفعال", "none"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_position[] = {
    {"مطلق", "absolute"},
    {"ثابت", "fixed"},
    {"چسبان", "sticky"},
    {"نسبی", "relative"},
    {"خودکار", "static"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_print_color_adjust[] = {
    {"خودکار", "auto"},
    {"بهینه", "exact"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_quotes[] = {
    {"مطمئن", "\"\""},
    {"غیرمطمئن", "''"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_r[] = {

};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_resize[] = {
    {"افقی", "horizontal"},
    {"عمودی", "vertical"},
    {"خودکار", "both"},
    {"غیرفعال", "none"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_right[] = {
    {"خودکار", "auto"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_rotate[] = {
    {"درجه", "deg"},
    {"رادیان", "rad"},
    {"مدور", "turn"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_row_gap[] = {

};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_ruby_align[] = {
    {"راست", "right"},
    {"چپ", "left"},
    {"مرکز", "center"},
    {"بنیاد", "base"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_ruby_position[] = {
    {"بالا", "over"},
    {"پایین", "under"},
    {"پیش", "before"},
    {"پس", "after"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_rx[] = {

};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_ry[] = {

};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_scale[] = {
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_scroll_behavior[] = {
    {"هموار", "smooth"},
    {"خودکار", "auto"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_scroll_margin[] = {

};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_scroll_margin_block[] = {

};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_scroll_margin_block_end[] = {

};

const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_scroll_margin_block_start[] = {

};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_scroll_margin_bottom[] = {

};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_scroll_margin_inline[] = {

};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_scroll_margin_inline_end[] = {

};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_scroll_margin_inline_start[] = {

};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_scroll_margin_left[] = {

};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_scroll_margin_right[] = {

};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_scroll_margin_top[] = {

};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_scroll_padding[] = {

};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_scroll_padding_block[] = {

};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_scroll_padding_block_end[] = {

};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_scroll_padding_block_start[] = {

};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_scroll_padding_bottom[] = {

};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_scroll_padding_inline[] = {

};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_scroll_padding_inline_end[] = {

};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_scroll_padding_inline_start[] = {

};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_scroll_padding_left[] = {

};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_scroll_padding_right[] = {

};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_scroll_padding_top[] = {

};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_scroll_snap_align[] = {
    {"چپ", "start"},
    {"مرکز", "center"},
    {"راست", "end"},
    {"ترازبندی", "none"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_scroll_snap_type[] = {
    {"تک محور", "x"},
    {"دو محور", "y"},
    {"غیرفعال", "none"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_shape_outside[] = {
    {"دایره", "circle"},
    {"مستطیل", "rect"},
    {"بیضی", "ellipse"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_text_align[] = {
    {"چپ", "left"},
    {"راست", "right"},
    {"مرکز", "center"},
    {"ترازبندی", "justify"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_text_align_last[] = {
    {"چپ", "left"},
    {"راست", "right"},
    {"مرکز", "center"},
    {"ترازبندی", "justify"},
    {"خودکار", "auto"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_text_indent[] = {
    {"خودکار", "auto"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_text_transform[] = {
    {"بزرگ", "uppercase"},
    {"کوچک", "lowercase"},
    {"بزرگ و کوچک", "capitalize"},
    {"خودکار", "none"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_top[] = {
    {"خودکار", "auto"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_transform[] = {
    {"چرخش", "rotate"},
    {"مقیاس", "scale"},
    {"ترجمه", "translate"},
    {"شیب", "skew"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_transform_origin[] = {
    {"مرکز", "center"},
    {"چپ", "left"},
    {"راست", "right"},
    {"بالا", "top"},
    {"پایین", "bottom"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_translate[] = {

};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_unicode_bidi[] = {
    {"خودکار", "normal"},
    {"معکوس", "bidi-override"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_user_select[] = {
    {"اجازه", "auto"},
    {"غیرفعال", "none"},
    {"جلوگیری", "text"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_width[] = {
    {"خودکار", "auto"},
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_will_change[] = {
    {"خودکار", "auto"},
    {"شفاف", "transform"},
    {"کاهش", "opacity"},
};

const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_transform[] = {
    {"چرخش", "rotate"},
    {"مقیاس", "scale"},
    {"ترجمه", "translate"},
    {"شیب", "skew"}
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_transform_box[] = {
    {"مرز", "border-box"},
    {"محتوا", "content-box"},
    {"پوشش", "fill-box"},
    {"نمایش", "view-box"}
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_transform_origin[] = {
    {"مرکز", "center"},
    {"چپ", "left"},
    {"راست", "right"},
    {"بالا", "top"},
    {"پایین", "bottom"}
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_transform_style[] = {
    {"مسطح", "flat"},
    {"سه‌بعدی", "preserve-3d"}
};

const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_transition[] = {
    {"مدت", "duration"},
    {"تاخیر", "delay"},
    {"ویژگی", "property"},
    {"تابع زمان‌بندی", "timing-function"}
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_transition_behavior[] = {
    {"مسطح", "flat"},
    {"مناسب", "smooth"}
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_transition_delay[] = {
    {"ثانیه", "s"},
    {"میلی‌ثانیه", "ms"}
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_transition_duration[] = {
    {"ثانیه", "s"},
    {"میلی‌ثانیه", "ms"}
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_transition_property[] = {
    {"همه", "all"},
    {"عرض", "width"},
    {"ارتفاع", "height"},
    {"شفافیت", "opacity"}
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_transition_timing_function[] = {
    {"آسان", "ease"},
    {"آسان وارد شدن", "ease-in"},
    {"آسان خارج شدن", "ease-out"},
    {"آسان وارد و خارج شدن", "ease-in-out"},
    {"خطی", "linear"}
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_translate[] = {
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_user_select[] = {
    {"اجازه", "auto"},
    {"غیرفعال", "none"},
    {"جلوگیری", "text"}
};

const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_vector_effect[] = {
    {"هیچ", "none"},
    {"مقیاس", "non-scaling-stroke"}
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_vertical_align[] = {
    {"بالا", "top"},
    {"پایین", "bottom"},
    {"میانه", "middle"},
    {"متن بالا", "text-top"},
    {"متن پایین", "text-bottom"},
    {"زیرنویس", "sub"},
    {"بالانویس", "super"},
    {"مقدار بیس لاین", "baseline"}
};

const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_visibility[] = {
    {"قابل مشاهده", "visible"},
    {"پنهان", "hidden"},
    {"خاموش", "collapse"}
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_white_space[] = {
    {"معمولی", "normal"},
    {"بدون شکستن", "nowrap"},
    {"پیش قالب‌بندی شده", "pre"},
    {"پیش قالب‌بندی شده و عادی", "pre-wrap"},
    {"پیش قالب‌بندی شده و شکستن", "pre-line"}
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_widows[] = {
};

const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_width[] = {
    {"خودکار", "auto"}
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_width[] = {
    {"خودکار", "auto"}
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_word_break[] = {
    {"معمولی", "normal"},
    {"بشکن", "break-all"},
    {"شکست کلمه", "keep-all"}
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_word_spacing[] = {
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_writing_mode[] = {
    {"افقی", "horizontal-tb"},
    {"عمودی", "vertical-rl"},
    {"عمودی، از چپ به راست", "vertical-lr"}
};
const ast_layout_attribute_style_pair_t ast_layout_allowed_style_list_zoom[] = {
    {"خودکار", "auto"},
    {"مقیاس", "scale"}
};