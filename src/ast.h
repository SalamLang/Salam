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
    AST_NODE_TYPE_BLOCK,
    AST_NODE_TYPE_IMPORT,
    AST_NODE_TYPE_FUNCTION,
    AST_NODE_TYPE_LAYOUT,
    AST_NODE_TYPE_ERROR,
} ast_type_t;

typedef enum {
    AST_NODE_BLOCK_TYPE_LAYOUT,
    AST_NODE_BLOCK_TYPE_FUNCTION,
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
    AST_LAYOUT_ATTRIBUTE_TYPE_STYLE,
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
    AST_LAYOUT_ATTRIBUTE_TYPE_ERROR,
} ast_layout_attribute_type_t;

struct ast_t;

typedef struct {
    ast_block_type_t type;
    array_node_t* children;
    ast_type_t parent_type;

    void (*destroy)(void* node);
    void (*print)(void* node);
} ast_block_t;

typedef struct ast_import_t {
    array_t* path;

    void (*destroy)(void* node);
    void (*print)(void* node);
} ast_import_t;

typedef struct ast_function_t {
    char* name;
    array_t* parameters;
    ast_block_t* block;

    void (*destroy)(void* node);
    void (*print)(void* node);
} ast_function_t;

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

typedef union {
    ast_block_t* block;
    ast_import_t* import;
    ast_function_t* function;
    ast_layout_t* layout;
    ast_layout_block_t* layout_block;
    ast_layout_node_t* layout_node;
} ast_union_t;

typedef struct ast_t {
    ast_type_t type;
    location_t location;

    void (*destroy)(void* node);
    void (*print)(void* node);

    ast_union_t data;
} ast_node_t;

typedef struct {
    ast_layout_t* layout;
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
 * @params {ast_layout_block_t*} parent_block - Parent block
 * @returns {ast_layout_node_t*} - Pointer to the created AST node layout attribute
 * 
 */
ast_layout_node_t* ast_layout_node_create(ast_layout_node_type_t layout_node_type, ast_layout_block_t* parent_block);

/**
 * 
 * @function ast_block_create
 * @brief Create a new AST block node
 * @returns {ast_block_t*} - AST block node
 * 
 */
ast_block_t* ast_block_create();

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
 * @params {token_t*} token - Token
 * @params {ast_layout_node_type_t} parent_node_type - Parent node type
 * @returns {ast_layout_node_type_t} type - Layout Node Type
 * 
 */
ast_layout_attribute_type_t token_to_ast_layout_attribute_type(token_t* token, ast_layout_node_type_t parent_node_type);

/**
 * 
 * @function ast_layout_attribute_type_to_name
 * @brief Convert AST layout attribute type to name
 * @params {ast_layout_attribute_type_t} type - Layout Attribute Type
 * @returns {char*} name - Name
 * 
 */
char* ast_layout_attribute_type_to_name(ast_layout_attribute_type_t type);

#endif
