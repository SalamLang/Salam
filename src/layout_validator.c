#include "layout_validator.h"

typedef struct {
    char* tag;
    ast_layout_node_type_t type;
} validator_layout_t;

typedef enum {
    VALIDATOR_LAYOUT_CSS_FILTER_GROUP,
    VALIDATOR_LAYOUT_CSS_FILTER_SIZE,
    VALIDATOR_LAYOUT_CSS_FILTER_SIZES,
    VALIDATOR_LAYOUT_CSS_FILTER_COLOR,
    VALIDATOR_LAYOUT_CSS_FILTER_COLORS
} validator_layout_css_filter_t;

typedef struct {
    char* alias;
    char* standard_value;
} alias_mapping_t;

typedef struct {
    char* key;
    ast_layout_attribute_type_t type;
    size_t min_value;
    size_t max_value;
    validator_layout_css_filter_t filter;
    char allowed_values[10][20];
    alias_mapping_t alias_mappings[10];
} validator_layout_css_t;


validator_layout_css_t css_attributes[] = {
    {"color", AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_COLOR, 1, -1, VALIDATOR_LAYOUT_CSS_FILTER_COLOR, {"red", "green", "blue"}, {[0] = ""}},
    {"رنگ", AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_COLOR, 1, -1, VALIDATOR_LAYOUT_CSS_FILTER_COLOR, {"قرمز", "سبز", "آبی", "ابی"}, {[0] = ""}},

    {"display", AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FLEX, 1, 1, VALIDATOR_LAYOUT_CSS_FILTER_SIZE, {"flex", "block", "inline"}, {[0] = ""}},
    {"نمایش", AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FLEX, 1, 1, VALIDATOR_LAYOUT_CSS_FILTER_SIZE, {"فلکس", "بلاک", "اینلاین"}, {
        [0] = "flex",
        [1] = "block",
        [2] = "inline",
    }},
};

