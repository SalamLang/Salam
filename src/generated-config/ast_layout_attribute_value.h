#ifndef _AST_LAYOUT_ATTRIBUTE_VALUE_
#define _AST_LAYOUT_ATTRIBUTE_VALUE_

// ----------- BEGIN AUTO GENERATED ----------- //
const ast_layout_attribute_pair_t ast_layout_allowed_rel[] = {
    {"جایگزین", "alternate"},
    {"جانشین", "alternate"},
    {"نویسنده", "author"},
    {"نشانک", "bookmark"},
    {"خارجی", "external"},
    {"راهنما", "help"},
    {"مجوز", "license"},
    {"بعدی", "next"},
    {"بدون دنبال کردن", "nofollow"},
    {"بدون ارجاع", "noreferrer"},
    {"برچسب", "tag"},
    {"جستجو", "search"},
    {"قبلی", "prev"},
    {NULL, NULL},
};

const ast_layout_attribute_pair_t ast_layout_allowed_referrerpolicy[] = {
    {"بدون ارجاع", "no-referrer"},
    {"بدون ارجاع هنگام کاهش", "no-referrer-when-downgrade"},
    {"منشا", "origin"},
    {"منشا هنگام عبور از منشا", "origin-when-cross-origin"},
    {"همان منشا", "same-origin"},
    {"محدودیت منشا هنگام عبور از منشا", "strict-origin-when-cross-origin"},
    {"آدرس ناامن", "unsafe-url"},
    {NULL, NULL},
};

const ast_layout_attribute_pair_t ast_layout_allowed_dir[] = {
    {"چپ به راست", "ltr"}, {"چپ راست", "ltr"}, {"راست به چپ", "rtl"},
    {"راست چپ", "rtl"},    {NULL, NULL},
};

const ast_layout_attribute_pair_t ast_layout_allowed_form_type[] = {
    {"گت", "get"},
    {"پست", "post"},
    {NULL, NULL},
};

const ast_layout_attribute_pair_t ast_layout_allowed_form_kind[] = {
    {"خالی", "_blank"}, {"خود", "_self"}, {"والد", "_parent"},
    {"بالا", "_top"},   {NULL, NULL},
};

const ast_layout_attribute_pair_t ast_layout_allowed_charset[] = {
    {"یوتی‌اف", "UTF-8"},
    {"یوتی‌اف8", "UTF-8"},
    {"یوتی‌اف 8", "UTF-8"},
    {NULL, NULL},
};

const ast_layout_attribute_pair_t ast_layout_allowed_lang[] = {
    {"انگلیسی", "fa"},
    {"فارسی", "fa"},
    {NULL, NULL},
};

// ----------- END AUTO GENERATED ----------- //

#endif
