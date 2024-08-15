#ifndef _AST_H_
#define _AST_H_

#include <stdio.h>
#include <string.h>

#include "base.h"
#include "memory.h"
#include "array.h"
#include "lexer.h"
#include "hashmap.h"

typedef enum {
    AST_NODE_TYPE_IF,
    AST_NODE_TYPE_ELSE_IF,
    AST_NODE_TYPE_BLOCK,
    AST_NODE_TYPE_IMPORT,
    AST_NODE_TYPE_FUNCTION,
    AST_NODE_TYPE_LAYOUT,
    AST_NODE_TYPE_ERROR,
} ast_type_t;

typedef enum {
    AST_NODE_BLOCK_TYPE_LAYOUT,
    AST_NODE_BLOCK_TYPE_FUNCTION,
    AST_NODE_BLOCK_TYPE_IF,
    AST_NODE_BLOCK_TYPE_ELSE_IF,
    AST_NODE_BLOCK_TYPE_ERROR,
} ast_block_type_t;

typedef enum {
    AST_LAYOUT_NODE_TYPE_PARAGRAPH,
    AST_LAYOUT_NODE_TYPE_PARAGRAPH_RAW,
    AST_LAYOUT_NODE_TYPE_BUTTON,
    AST_LAYOUT_NODE_TYPE_INPUT,
    AST_LAYOUT_NODE_TYPE_TEXTAREA,
    AST_LAYOUT_NODE_TYPE_SPAN,
    AST_LAYOUT_NODE_TYPE_LABEL,
    AST_LAYOUT_NODE_TYPE_HEADER,
    AST_LAYOUT_NODE_TYPE_FOOTER,
    AST_LAYOUT_NODE_TYPE_NAV,
    AST_LAYOUT_NODE_TYPE_MAIN,
    AST_LAYOUT_NODE_TYPE_SECTION,
    AST_LAYOUT_NODE_TYPE_ARTICLE,
    AST_LAYOUT_NODE_TYPE_ASIDE,
    AST_LAYOUT_NODE_TYPE_SELECT,
    AST_LAYOUT_NODE_TYPE_OPTION,
    AST_LAYOUT_NODE_TYPE_IFRAME,
    AST_LAYOUT_NODE_TYPE_CANVAS,
    AST_LAYOUT_NODE_TYPE_BLOCKQUOTE,
    AST_LAYOUT_NODE_TYPE_PRE,
    AST_LAYOUT_NODE_TYPE_CODE,
    AST_LAYOUT_NODE_TYPE_BR,
    AST_LAYOUT_NODE_TYPE_HR,
    AST_LAYOUT_NODE_TYPE_STRONG,
    AST_LAYOUT_NODE_TYPE_EM,
    AST_LAYOUT_NODE_TYPE_ITALIC,
    AST_LAYOUT_NODE_TYPE_BOLD,
    AST_LAYOUT_NODE_TYPE_UNDERLINE,
    AST_LAYOUT_NODE_TYPE_S,
    AST_LAYOUT_NODE_TYPE_SMALL,
    AST_LAYOUT_NODE_TYPE_BIG,
    AST_LAYOUT_NODE_TYPE_SUB,
    AST_LAYOUT_NODE_TYPE_SUP,
    AST_LAYOUT_NODE_TYPE_CENTER,
    AST_LAYOUT_NODE_TYPE_DEL,
    AST_LAYOUT_NODE_TYPE_INS,
    AST_LAYOUT_NODE_TYPE_MARK,
    AST_LAYOUT_NODE_TYPE_Q,
    AST_LAYOUT_NODE_TYPE_CITE,
    AST_LAYOUT_NODE_TYPE_DFN,
    AST_LAYOUT_NODE_TYPE_ADDRESS,
    AST_LAYOUT_NODE_TYPE_TIME,
    AST_LAYOUT_NODE_TYPE_PROGRESS,
    AST_LAYOUT_NODE_TYPE_METER,
    AST_LAYOUT_NODE_TYPE_DETAILS,
    AST_LAYOUT_NODE_TYPE_SUMMARY,
    AST_LAYOUT_NODE_TYPE_DIALOG,
    AST_LAYOUT_NODE_TYPE_MENU,
    AST_LAYOUT_NODE_TYPE_MENUITEM,
    AST_LAYOUT_NODE_TYPE_COMMAND,
    AST_LAYOUT_NODE_TYPE_LEGEND,
    AST_LAYOUT_NODE_TYPE_FIELDSET,
    AST_LAYOUT_NODE_TYPE_CAPTION,
    AST_LAYOUT_NODE_TYPE_COL,
    AST_LAYOUT_NODE_TYPE_COLGROUP,
    AST_LAYOUT_NODE_TYPE_TABLE_HEADER,
    AST_LAYOUT_NODE_TYPE_TABLE_BODY,
    AST_LAYOUT_NODE_TYPE_TABLE_FOOTER,
    AST_LAYOUT_NODE_TYPE_UL,
    AST_LAYOUT_NODE_TYPE_OL,
    AST_LAYOUT_NODE_TYPE_LI,
    AST_LAYOUT_NODE_TYPE_LINK,
    AST_LAYOUT_NODE_TYPE_IMG,
    AST_LAYOUT_NODE_TYPE_TABLE,
    AST_LAYOUT_NODE_TYPE_TABLE_TR,
    AST_LAYOUT_NODE_TYPE_TABLE_TD,
    AST_LAYOUT_NODE_TYPE_TABLE_TH,
    AST_LAYOUT_NODE_TYPE_VIDEO,
    AST_LAYOUT_NODE_TYPE_AUDIO,
    AST_LAYOUT_NODE_TYPE_FORM,
    AST_LAYOUT_NODE_TYPE_DIV,
    AST_LAYOUT_NODE_TYPE_SCRIPT,
    AST_LAYOUT_NODE_TYPE_STYLE,

    AST_LAYOUT_NODE_TYPE_NONE,
    AST_LAYOUT_NODE_TYPE_ERROR,
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
    AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_TEXT_ALIGN,
    AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_TEXT_DECORATION,
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

struct ast_t;

typedef struct {
    ast_block_type_t type;
    ast_type_t parent_type;

    array_node_t* children;

    void (*destroy)(void* node);
    void (*print)(void* node);
} ast_block_t;

typedef struct ast_import_t {
    array_t* path;

    void (*destroy)(void* node);
    void (*print)(void* node);
} ast_import_t;

typedef enum ast_value_kind_t {
    AST_TYPE_KIND_VOID,
    AST_TYPE_KIND_INT,
    AST_TYPE_KIND_FLOAT,
    // AST_TYPE_KIND_DOUBLE,
    AST_TYPE_KIND_CHAR,
    AST_TYPE_KIND_STRING,
    AST_TYPE_KIND_BOOL,
    AST_TYPE_KIND_STRUCT,
    AST_TYPE_KIND_ENUM,
    AST_TYPE_KIND_POINTER,
    AST_TYPE_KIND_ARRAY,
    AST_TYPE_KIND_FUNCTION,
} ast_value_kind_t;

typedef struct ast_value_type_t {
    ast_value_kind_t kind;
    location_t location;

    void (*destroy)(void* node);
    void (*print)(void* node);
} ast_value_type_t;

typedef struct ast_function_t {
    char* name;
    array_function_parameter_t* parameters;
    ast_block_t* block;
    ast_value_type_t* return_type;

    void (*destroy)(void* node);
    void (*print)(void* node);
} ast_function_t;

typedef struct ast_value_t {
    char* data;
    ast_value_type_t* type;

    void (*destroy)(void* node);
    void (*print)(void* node);
} ast_value_t;

typedef struct ast_if_t {
    ast_value_t* condition;
    ast_block_t* block;
    array_block_t* else_blocks; // NULLABLE for else if blocks

    void (*destroy)(void* node);
    void (*print)(void* node);
} ast_if_t; 

typedef struct ast_layout_block_t {
    ast_block_type_t type;
    ast_type_t parent_type;
    ast_layout_node_type_t parent_node_type;
    char* text_content;

    struct hashmap_t* attributes;
    struct hashmap_t* styles;
    array_node_layout_t* children;

    void (*destroy)(void* node);
    void (*print)(void* node);
} ast_layout_block_t;

typedef struct ast_layout_attribute_t {
    ast_layout_attribute_type_t type;
    char* key;
    array_t* values;
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


typedef struct ast_function_parameter_t {
    char* name;
    ast_value_type_t* type;

    void (*destroy)(void* node);
    void (*print)(void* node);
} ast_function_parameter_t;

typedef union {
    ast_block_t* block;
    ast_import_t* import;
    ast_function_t* function;
    ast_if_t* ifclause;

    ast_layout_t* layout;
} ast_union_t;

typedef struct ast_t {
    ast_type_t type;
    location_t location;
    ast_union_t data;

    void (*destroy)(void* node);
    void (*print)(void* node);
} ast_node_t;

typedef struct {
    ast_layout_t* layout;
    array_function_t* functions;

    void (*destroy)(void* node);
    void (*print)(void* node);
} ast_t;

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
 * @function ast_node_destroy_notall
 * @brief Free the AST node (not all)
 * @params {ast_node_t*} value - AST node
 * @returns {void}
 * 
 */
void ast_node_destroy_notall(ast_node_t* value);

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
 * @function ast_node_create
 * @brief Create a new AST node
 * @params {ast_type_t} type - Type of the AST node
 * @params {location_t} location - Location of the AST node
 * @returns {ast_node_t*} - Pointer to the created AST node
 * 
 */
ast_node_t* ast_node_create(ast_type_t type, location_t location);

/**
 * 
 * @function ast_node_print
 * @brief Print the AST node
 * @params {ast_node_t*} node - AST node
 * @returns {void}
 * 
 */
void ast_node_print(ast_node_t* node);

/**
 * 
 * @function ast_node_destroy
 * @brief Free the AST node
 * @params {ast_node_t*} value - AST node
 * @returns {void}
 * 
 */
void ast_node_destroy(ast_node_t* value);

/**
 * 
 * @function ast_create
 * @brief Create a new AST
 * @returns {ast_t*} - Pointer to the created AST
 * 
 */
ast_t* ast_create();

/**
 * 
 * @function ast_debug
 * @brief Debug the AST
 * @params {ast_t*} ast - AST
 * @returns {void}
 * 
 */
void ast_debug(ast_t* ast);

/**
 * 
 * @function ast_destroy
 * @brief Free the AST
 * @params {ast_t*} ast - AST
 * @returns {void}
 * 
 */
void ast_destroy(ast_t* ast);

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
 * @function ast_block_create
 * @brief Create a new AST block node
 * @params {ast_block_type_t} type - Block type
 * @params {ast_type_t} parent_type - Parent type
 * @returns {ast_block_t*} - AST block node
 * 
 */
ast_block_t* ast_block_create(ast_block_type_t type, ast_type_t parent_type);

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
 * @params {array_t*} values - Values of the attribute
 * @returns {ast_layout_attribute_t*} - Pointer to the created AST node layout attribute
 * 
 */
ast_layout_attribute_t* ast_layout_attribute_create(ast_layout_attribute_type_t type, char* key, array_t* values);

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
 * @function ast_function_create
 * @brief Create a new AST node function
 * @params {char*} name - Name of the function
 * @returns {ast_function_t*} - Pointer to the created AST node function
 * 
 */
ast_function_t* ast_function_create(char* name);

/**
 * 
 * @function ast_function_parameter_destroy
 * @brief Free the AST function parameter
 * @params {ast_function_parameter_t*} value - AST function parameter
 * @returns {void}
 * 
 */
void ast_function_parameter_destroy(ast_function_parameter_t* value);

/**
 * 
 * @function ast_function_parameter_print
 * @brief Print the AST function parameter
 * @params {ast_function_parameter_t*} value - AST function parameter
 * @returns {void}
 * 
 */
void ast_function_parameter_print(ast_function_parameter_t* value);

/**
 * 
 * @function ast_function_print
 * @brief Print the AST function
 * @params {ast_function_t*} value - AST function
 * @returns {void}
 * 
 */
void ast_function_print(ast_function_t* value);

/**
 * 
 * @function ast_function_destroy
 * @brief Free the AST function
 * @params {ast_function_t*} value - AST function
 * @returns {void}
 * 
 */
void ast_function_destroy(ast_function_t* value);

/**
 * 
 * @function ast_type_create
 * @brief Create a new AST value type
 * @params {ast_value_kind_t} kind - Kind of the value type
 * @params {location_t} location - Location of the value type
 * @returns {ast_value_type_t*} - Pointer to the created AST value type
 * 
 */
ast_value_type_t* ast_type_create(ast_value_kind_t kind, location_t location);

/**
 * 
 * @function ast_type_print
 * @brief Print the AST value type
 * @params {ast_value_type_t*} type - AST value type
 * @returns {void}
 * 
 */
void ast_type_print(ast_value_type_t* type);

/**
 * 
 * @function ast_type_name
 * @brief Get the name of the AST value type
 * @params {ast_value_type_t*} type - AST value type
 * @returns {char*} - Name of the AST value type
 * 
 */
char* ast_type_name(ast_value_type_t* type);

/**
 * 
 * @function ast_type_destroy
 * @brief Free the AST value type
 * @params {ast_value_type_t*} type - AST value type
 * @returns {void}
 * 
 */
void ast_type_destroy(ast_value_type_t* type);

/**
 * 
 * @function ast_print
 * @brief Print the AST
 * @params {ast_t*} ast - AST
 * @returns {void}
 * 
 */
void ast_print(ast_t* ast);

/**
 * 
 * @function ast_block_print
 * @brief Print the AST block node
 * @params {ast_block_t*} block - AST block node
 * @returns {void}
 * 
 */
void ast_block_print(ast_block_t* block);

/**
 * 
 * @function ast_block_destroy
 * @brief Free the AST block node
 * @params {ast_block_t*} block - AST block node
 * @returns {void}
 * 
 */
void ast_block_destroy(ast_block_t* block);

/**
 * 
 * @function ast_if_create
 * @brief Create a new AST node if
 * @returns {ast_if_t*} - Pointer to the created AST node if
 * 
 */
void ast_if_print(ast_if_t* node);

/**
 * 
 * @function ast_elseif_create
 * @brief Create a new AST node else if
 * @params {ast_value_t*} condition - Condition of the else if
 * @returns {ast_if_t*} - Pointer to the created AST node else if
 * 
 */
ast_if_t* ast_elseif_create(ast_value_t* condition);

/**
 * 
 * @function ast_if_destroy
 * @brief Free the AST if node
 * @params {ast_if_t*} node - AST if node
 * @returns {void}
 * 
 */
void ast_if_destroy(ast_if_t* node);

/**
 * 
 * @function ast_if_create
 * @brief Create a new AST node if
 * @params {ast_value_t*} condition - Condition of the if
 * @returns {ast_if_t*} - Pointer to the created AST node if
 * 
 */
ast_if_t* ast_if_create(ast_value_t* condition);

/**
 * 
 * @function ast_value_create
 * @brief Create a new AST value
 * @params {ast_value_type_t*} type - Value type
 * @params {void*} data - Value data
 * @returns {ast_value_t*} - Pointer to the created AST value
 * 
 */
ast_value_t* ast_value_create(ast_value_type_t* type, void* data);

/**
 * 
 * @function ast_value_print
 * @brief Print the AST value
 * @params {ast_value_t*} value - AST Value
 * @returns {void}
 * 
 */
void ast_value_print(ast_value_t* value);

/**
 * 
 * @function ast_value_destroy
 * @brief Free the AST value
 * @params {ast_value_t*} value - AST Value
 * @returns {void}
 * 
 */
void ast_value_destroy(ast_value_t* value);

/**
 * 
 * @function ast_block_type_name
 * @brief Print the AST block type
 * @params {ast_block_type_t} type - AST block type
 * @returns {char*} - Name of the AST block type
 */
char* ast_block_type_name(ast_block_type_t type);

/**
 * 
 * @function ast_else_create
 * @brief Create a new AST node else
 * @returns {ast_if_t*} - Pointer to the created AST node else
 * 
 */
ast_if_t* ast_else_create();

#endif
