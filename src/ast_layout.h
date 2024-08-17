#ifndef _AST_LAYOUT_H_
#define _AST_LAYOUT_H_

#include "base.h"
#include "memory.h"
#include "ast.h"

typedef enum {
    AST_LAYOUT_TYPE_PARAGRAPH,
    AST_LAYOUT_TYPE_PARAGRAPH_RAW,
    AST_LAYOUT_TYPE_BUTTON,
    AST_LAYOUT_TYPE_INPUT,
    AST_LAYOUT_TYPE_TEXTAREA,
    AST_LAYOUT_TYPE_SPAN,
    AST_LAYOUT_TYPE_LABEL,
    AST_LAYOUT_TYPE_HEADER,
    AST_LAYOUT_TYPE_FOOTER,
    AST_LAYOUT_TYPE_NAV,
    AST_LAYOUT_TYPE_MAIN,
    AST_LAYOUT_TYPE_SECTION,
    AST_LAYOUT_TYPE_ARTICLE,
    AST_LAYOUT_TYPE_ASIDE,
    AST_LAYOUT_TYPE_SELECT,
    AST_LAYOUT_TYPE_OPTION,
    AST_LAYOUT_TYPE_IFRAME,
    AST_LAYOUT_TYPE_CANVAS,
    AST_LAYOUT_TYPE_BLOCKQUOTE,
    AST_LAYOUT_TYPE_PRE,
    AST_LAYOUT_TYPE_CODE,
    AST_LAYOUT_TYPE_BR,
    AST_LAYOUT_TYPE_HR,
    AST_LAYOUT_TYPE_STRONG,
    AST_LAYOUT_TYPE_EM,
    AST_LAYOUT_TYPE_ITALIC,
    AST_LAYOUT_TYPE_BOLD,
    AST_LAYOUT_TYPE_UNDERLINE,
    AST_LAYOUT_TYPE_S,
    AST_LAYOUT_TYPE_SMALL,
    AST_LAYOUT_TYPE_BIG,
    AST_LAYOUT_TYPE_SUB,
    AST_LAYOUT_TYPE_SUP,
    AST_LAYOUT_TYPE_CENTER,
    AST_LAYOUT_TYPE_DEL,
    AST_LAYOUT_TYPE_INS,
    AST_LAYOUT_TYPE_MARK,
    AST_LAYOUT_TYPE_Q,
    AST_LAYOUT_TYPE_CITE,
    AST_LAYOUT_TYPE_DFN,
    AST_LAYOUT_TYPE_ADDRESS,
    AST_LAYOUT_TYPE_TIME,
    AST_LAYOUT_TYPE_PROGRESS,
    AST_LAYOUT_TYPE_METER,
    AST_LAYOUT_TYPE_DETAILS,
    AST_LAYOUT_TYPE_SUMMARY,
    AST_LAYOUT_TYPE_DIALOG,
    AST_LAYOUT_TYPE_MENU,
    AST_LAYOUT_TYPE_MENUITEM,
    AST_LAYOUT_TYPE_COMMAND,
    AST_LAYOUT_TYPE_LEGEND,
    AST_LAYOUT_TYPE_FIELDSET,
    AST_LAYOUT_TYPE_CAPTION,
    AST_LAYOUT_TYPE_COL,
    AST_LAYOUT_TYPE_COLGROUP,
    AST_LAYOUT_TYPE_TABLE_HEADER,
    AST_LAYOUT_TYPE_TABLE_BODY,
    AST_LAYOUT_TYPE_TABLE_FOOTER,
    AST_LAYOUT_TYPE_UL,
    AST_LAYOUT_TYPE_OL,
    AST_LAYOUT_TYPE_LI,
    AST_LAYOUT_TYPE_LINK,
    AST_LAYOUT_TYPE_IMG,
    AST_LAYOUT_TYPE_TABLE,
    AST_LAYOUT_TYPE_TABLE_TR,
    AST_LAYOUT_TYPE_TABLE_TD,
    AST_LAYOUT_TYPE_TABLE_TH,
    AST_LAYOUT_TYPE_VIDEO,
    AST_LAYOUT_TYPE_AUDIO,
    AST_LAYOUT_TYPE_FORM,
    AST_LAYOUT_TYPE_DIV,
    AST_LAYOUT_TYPE_SCRIPT,
    AST_LAYOUT_TYPE_STYLE,

    AST_LAYOUT_TYPE_NONE,
    AST_LAYOUT_TYPE_ERROR,
} ast_layout_node_type_t;

typedef enum {
    AST_LAYOUT_ATTRIBUTE_TYPE_CLASS,
    AST_LAYOUT_ATTRIBUTE_TYPE_ID,
    AST_LAYOUT_ATTRIBUTE_TYPE_CONTENT,
    AST_LAYOUT_ATTRIBUTE_TYPE_SRC,
    AST_LAYOUT_ATTRIBUTE_TYPE_DIR,
    AST_LAYOUT_ATTRIBUTE_TYPE_LANG,
    AST_LAYOUT_ATTRIBUTE_TYPE_TITLE,
    AST_LAYOUT_ATTRIBUTE_TYPE_TYPE,
    AST_LAYOUT_ATTRIBUTE_TYPE_ALT,
    AST_LAYOUT_ATTRIBUTE_TYPE_AUTHOR,
    AST_LAYOUT_ATTRIBUTE_TYPE_DESCRIPTION,
    AST_LAYOUT_ATTRIBUTE_TYPE_KEYWORDS,
    AST_LAYOUT_ATTRIBUTE_TYPE_ICON,
    AST_LAYOUT_ATTRIBUTE_TYPE_NAME,
    AST_LAYOUT_ATTRIBUTE_TYPE_CHARSET,
    AST_LAYOUT_ATTRIBUTE_TYPE_REFRESH,
    AST_LAYOUT_ATTRIBUTE_TYPE_VIEWPORT,

    AST_LAYOUT_ATTRIBUTE_TYPE_VALUE,
    
    AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BACKGROUND,
    AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_COLOR,
    AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FONT,
    AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FONT_FAMILY,
    AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FONT_SIZE,
    AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FONT_STYLE,
    AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FONT_WEIGHT,
    AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_HEIGHT,
    AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_WIDTH,
    AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_MARGIN,
    AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_MARGIN_LEFT,
    AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_MARGIN_RIGHT,
    AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_MARGIN_TOP,
    AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_MARGIN_BOTTOM,
    AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_PADDING,
    AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_PADDING_LEFT,
    AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_PADDING_RIGHT,
    AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_PADDING_TOP,
    AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_PADDING_BOTTOM,
    AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_TEXT_EMPHASIS_POSITION,
    AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_TEXT_EMPHASIS_STYLE,
    AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_TEXT_ALIGN,
    AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_TEXT_ALIGN_LAST,
    AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_TEXT_DECORATION,
    AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_TEXT_DECORATION_LINE,
    AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_TEXT_DECORATION_STYLE,
    AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_TEXT_EMPHASIS_COLOR,
    AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_TEXT_DECORATION_COLOR,
    AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_TEXT_DECORATION_SKIP,
    AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_TEXT_DECORATION_SKIP_INK,
    AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_TEXT_DECORATION_THICKNESS,
    AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_TEXT_TRANSFORM,
    AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_VERTICAL_ALIGN,
    AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_VISIBILITY,
    AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_Z_INDEX,
    AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_POSITION,
    AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_TOP,
    AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_RIGHT,
    AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BOTTOM,
    AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_LEFT,
    AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BORDER,
    AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BORDER_COLOR,
    AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BORDER_STYLE,
    AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BORDER_WIDTH,
    AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BORDER_RADIUS,
    AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FLOAT,
    AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_CLEAR,
    AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_OVERFLOW,
    AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_TEXT_OVERFLOW,
    AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_WHITE_SPACE,
    AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_CURSOR,
    AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FILTER,
    AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_OPACITY,
    AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_TRANSFORM,
    AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_TRANSITION,
    AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_ANIMATION,
    AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BOX_SHADOW,
    AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_TEXT_SHADOW,
    AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_OUTLINE,
    AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_OUTLINE_COLOR,
    AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_OUTLINE_STYLE,
    AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_OUTLINE_WIDTH,
    AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BACKGROUND_COLOR,
    AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BACKGROUND_IMAGE,
    AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BACKGROUND_REPEAT,
    AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BACKGROUND_POSITION,
    AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BACKGROUND_SIZE,
    AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BACKGROUND_ATTACHMENT,
    AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BACKGROUND_CLIP,
    AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BACKGROUND_ORIGIN,
    AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BACKGROUND_BLEND_MODE,
    AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_DISPLAY,
    AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FLEX,
    AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FLEX_DIRECTION,
    AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_TEXT_ANCHOR,
    AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_TEXT_COMBINE_UPRIGHT,
    AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FLEX_WRAP,
    AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FLEX_FLOW,
    AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FLEX_GROW,
    AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FLEX_SHRINK,
    AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FLEX_BASIS,
    AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_JUSTIFY_CONTENT,
    AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_ALIGN_ITEMS,
    AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_ALIGN_SELF,
    AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_ALIGN_CONTENT,
    AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_ORDER,
    AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_GRID,
    AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FILL,
    AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_STROKE,
    AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_STROKE_WIDTH,
    AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_STROKE_LINECAP,
    AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_STROKE_LINEJOIN,
    AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_STROKE_DASHARRAY,
    AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_STROKE_DASHOFFSET,
    AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FILL_OPACITY,
    AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_STROKE_OPACITY,
    AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_SHAPE_RENDERING,
    AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_TEXT_RENDERING,
    AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_IMAGE_RENDERING,
    AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_COLOR_INTERPOLATION,
    AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_COLOR_RENDERING,
    AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_VECTOR_EFFECT,
    AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_MASK,
    AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_CLIP_PATH,

    AST_LAYOUT_ATTRIBUTE_TYPE_ERROR,
} ast_layout_attribute_type_t;

typedef struct ast_layout_block_t {
    ast_block_type_t type;
    ast_type_t parent_type;
    ast_layout_node_type_t parent_node_type;
    char* text_content;

    struct hashmap_t* attributes;
    struct hashmap_t* styles;
    struct hashmap_t* new_styles;
    array_node_layout_t* children;

    void (*destroy)(void* node);
    void (*print)(void* node);
} ast_layout_block_t;

typedef struct ast_layout_attribute_t {
    ast_layout_attribute_type_t type;
    
    char* key;
    array_value_t* values;

    ast_layout_node_type_t parent_node_type;

    location_t key_location;
    location_t value_location;

    char* final_key;
    char* final_value;

    bool isStyle;
    bool isContent;
    bool ignoreMe;

    void (*destroy)(void* node);
    void (*print)(void* node);
} ast_layout_attribute_t;

typedef struct ast_layout_t {
    ast_layout_block_t* block;

    void (*destroy)(void* node);
    void (*print)(void* node);
} ast_layout_t;

typedef struct ast_layout_node_t {
    char* tag;
    ast_layout_node_type_t type;
    ast_layout_block_t* block;

    void (*destroy)(void* node);
    void (*print)(void* node);
} ast_layout_node_t;

/**
 * 
 * @function ast_layout_node_print
 * @beief Print the AST layout node
 * @params {ast_layout_node_t*} value - AST layout node
 * @returns {void}
 * 
 */
void ast_layout_node_print(ast_layout_node_t* value);

/**
 * 
 * @function ast_layout_node_destroy
 * @brief Free the AST node layout attribute
 * @params {ast_layout_node_t*} value - AST layout node
 * @returns {void}
 * 
 */
void ast_layout_node_destroy(ast_layout_node_t* value);

/**
 * 
 * @function ast_layout_create
 * @brief Create a new AST node layout attribute
 * @returns {ast_layout_attribute_t*} - Pointer to the created AST node layout attribute
 * 
 */
ast_layout_t* ast_layout_create();

/**
 * 
 * @function ast_layout_destroy
 * @brief Create a new AST node layout attribute
 * @params {ast_layout_t*} value - AST layout node
 * @returns {void}
 * 
 */
void ast_layout_destroy(ast_layout_t* value);

/**
 * 
 * @function ast_layout_block_create
 * @brief Create a new AST node layout block
 * @params {ast_type_t} node_type - Node type
 * @params {ast_layout_node_type_t} layout_node_type - Layout node type
 * @returns {ast_layout_block_t*} - Pointer to the created AST node layout block
 * 
 */
ast_layout_block_t* ast_layout_block_create(ast_type_t node_type, ast_layout_node_type_t layout_node_type);

/**
 * 
 * @function ast_layout_block_print
 * @brief Free the AST node layout block
 * @params {ast_layout_block_t*} value - AST layout block
 * @returns {void}
 * 
 */
void ast_layout_block_print(ast_layout_block_t* value);

/**
 * 
 * @function ast_layout_block_destroy
 * @brief Free the AST node layout block
 * @params {ast_layout_block_t*} value - AST layout block
 * @returns {void}
 * 
 */
void ast_layout_block_destroy(ast_layout_block_t* value);

/**
 * 
 * @function ast_layout_node_create
 * @brief Create a new AST node layout attribute
 * @params {ast_layout_node_type_t} layout_node_type - Layout node type
 * @returns {ast_layout_node_t*} - Pointer to the created AST node layout attribute
 * 
 */
ast_layout_node_t* ast_layout_node_create(ast_layout_node_type_t layout_node_type);

/**
 * 
 * @function ast_layout_attribute_destroy
 * @brief Create a new AST node layout attribute
 * @params {ast_layout_attribute_t*} value - AST layout attribute
 * @returns {void}
 * 
 */
void ast_layout_attribute_destroy(ast_layout_attribute_t* value);

/**
 * 
 * @function ast_layout_attribute_print
 * @brief Print the AST layout attribute
 * @params {ast_layout_attribute_t*} value - AST layout attribute
 * @returns {void}
 * 
 */
void ast_layout_attribute_print(ast_layout_attribute_t* value);

/**
 * 
 * @function ast_layout_print
 * @brief Print the AST layout
 * @params {ast_layout_t*} value - AST layout
 * @returns {void}
 * 
 */
void ast_layout_print(ast_layout_t* value);

/**
 * 
 * @function ast_layout_attribute_create
 * @brief Create a new AST node layout attribute
 * @params {ast_layout_attribute_type_t} type - Type of the layout attribute
 * @params {const char*} key - Key of the attribute
 * @params {array_value_t*} values - Values of the attribute
 * @params {ast_layout_node_type_t} parent_node_type - Parent node type
 * @params {location_t} last_name - Last name of the attribute
 * @params {location_t} first_value - First value of the attribute
 * @returns {ast_layout_attribute_t*} - Pointer to the created AST node layout attribute
 * 
 */
ast_layout_attribute_t* ast_layout_attribute_create(ast_layout_attribute_type_t type, char* key, array_value_t* values, ast_layout_node_type_t parent_node_type, location_t last_name, location_t first_value);

/**
 * 
 * @function name_to_ast_layout_node_type
 * @brief Convert name to AST layout node type
 * @params {char*} name - Name
 * @returns {ast_layout_node_type_t} type - Layout Node Type
 * 
 */
ast_layout_node_type_t name_to_ast_layout_node_type(char* name);

/**
 * 
 * @function token_to_ast_layout_node_type
 * @brief Convert token to AST layout node type
 * @params {token_t*} token - Token
 * @returns {ast_layout_node_type_t} type - Layout Node Type
 * 
 */
ast_layout_node_type_t token_to_ast_layout_node_type(token_t* token);

/**
 * 
 * @function ast_layout_node_type_to_name
 * @brief Convert AST layout attribute type to name
 * @params {ast_layout_node_type_t} type - Layout Attribute Type
 * @returns {char*} name - Name
 * 
 */
char* ast_layout_node_type_to_name(ast_layout_node_type_t type);

/**
 * 
 * @function name_to_ast_layout_node_type
 * @brief Convert name to AST layout node type
 * @params {char*} name - Name
 * @returns {ast_layout_attribute_type_t} type - Layout Attribute Type
 * 
 */
ast_layout_attribute_type_t name_to_ast_layout_attribute_type(char* name);

/**
 * 
 * @function token_to_ast_layout_attribute_type
 * @brief Convert token to AST layout attribute type
 * @params {char*} name - Name
 * @params {token_t*} token - Token
 * @params {ast_layout_node_type_t} parent_node_type - Parent node type
 * @returns {ast_layout_node_type_t} type - Layout Node Type
 * 
 */
ast_layout_attribute_type_t token_to_ast_layout_attribute_type(char* name, token_t* token, ast_layout_node_type_t parent_node_type);

/**
 * 
 * @function ast_layout_attribute_type_to_name
 * @brief Convert AST layout attribute type to name
 * @params {ast_layout_attribute_type_t} type - Layout Attribute Type
 * @returns {char*} name - Name
 * 
 */
char* ast_layout_attribute_type_to_name(ast_layout_attribute_type_t type);

/**
 * 
 * @function ast_layout_attribute_copy
 * @brief Copy the AST layout attribute
 * @params {ast_layout_attribute_t*} value - AST layout attribute
 * @returns {ast_layout_attribute_t*} - Pointer to the copied AST layout attribute
 * 
 */
ast_layout_attribute_t* ast_layout_attribute_copy(ast_layout_attribute_t* value);

#endif
