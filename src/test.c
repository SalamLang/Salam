typedef enum {
    AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_COLOR,
    AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FLEX
} ast_layout_attribute_type_t;

typedef enum {
    AST_LAYOUT_NODE_TYPE_TAG,
    AST_LAYOUT_NODE_TYPE_ATTRIBUTE
} ast_layout_node_type_t;

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
