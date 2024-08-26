#ifndef _AST_LAYOUT_H_
#define _AST_LAYOUT_H_

#include <string.h>
#include <stdbool.h>

typedef enum {
    #undef ADD_LAYOUT_TYPE
    #undef ADD_LAYOUT_TYPE_HIDE
    #undef ADD_LAYOUT_TYPE_REPEAT

    #define ADD_LAYOUT_TYPE(TYPE, NAME, NAME_LOWER, GENERATED_NAME, ENDUSER_NAME, IS_MOTHER) TYPE,
    #define ADD_LAYOUT_TYPE_HIDE(TYPE, NAME, NAME_LOWER, GENERATED_NAME, ENDUSER_NAME, IS_MOTHER) TYPE,
    #define ADD_LAYOUT_TYPE_REPEAT(TYPE, NAME, NAME_LOWER, GENERATED_NAME, ENDUSER_NAME, IS_MOTHER)

    #include "ast_layout_type.h"
} ast_layout_node_type_t;

typedef enum {
    #undef ADD_LAYOUT_ATTRIBUTE_TYPE
    #undef ADD_LAYOUT_ATTRIBUTE_TYPE_REPEAT

    #define ADD_LAYOUT_ATTRIBUTE_TYPE(TYPE, NAME, NAME_LOWER, GENERATED_NAME, ENDUSER_NAME) TYPE,
    #define ADD_LAYOUT_ATTRIBUTE_TYPE_REPEAT(TYPE, NAME, NAME_LOWER, GENERATED_NAME, ENDUSER_NAME)

    #include "ast_layout_attribute_type.h"

    #undef ADD_LAYOUT_ATTRIBUTE_STYLE_TYPE
    #undef ADD_LAYOUT_ATTRIBUTE_STYLE_TYPE_REPEAT
    #undef ADD_LAYOUT_ATTRIBUTE_STYLE_TYPE_HIDE

    #define ADD_LAYOUT_ATTRIBUTE_STYLE_TYPE(TYPE, NAME, NAME_LOWER, ENDUSER_NAME, GENERATED_NAME, FILTER, ALLOWED_VALUES, SUBTAGS) TYPE,
    #define ADD_LAYOUT_ATTRIBUTE_STYLE_TYPE_REPEAT(TYPE, NAME, NAME_LOWER, ENDUSER_NAME, GENERATED_NAME, FILTER, ALLOWED_VALUES, SUBTAGS)
    #define ADD_LAYOUT_ATTRIBUTE_STYLE_TYPE_HIDE(TYPE, NAME, NAME_LOWER, ENDUSER_NAME, GENERATED_NAME, FILTER, ALLOWED_VALUES, SUBTAGS)

    #include "ast_layout_attribute_style_type.h"
} ast_layout_attribute_type_t;

#include "ast_layout_style.h"
#include "base.h"
#include "memory.h"
#include "ast.h"
#include "array.h"
#include "array_custom.h"
#include "hashmap.h"
#include "hashmap_custom.h"

typedef struct ast_layout_block_t {
    char* tag;
    ast_block_type_t type;
    ast_type_t parent_type;
    ast_layout_node_type_t parent_node_type;
    char* text_content;

    struct hashmap_t* attributes;
    ast_layout_style_state_t* styles;
    struct hashmap_t* states;

    array_node_layout_t* children;
    array_node_layout_t* meta_children;

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
 * @function enduser_name_to_ast_layout_attribute_type
 * @brief Convert enduser attribute name to AST layout node type
 * @params {char*} name - Name
 * @returns {ast_layout_attribute_type_t} type - Layout Attribute Type
 *
 */
ast_layout_attribute_type_t enduser_name_to_ast_layout_attribute_type(char* name);

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

/**
 *
 * @function name_to_ast_layout_node_type
 * @brief Convert attribute name to AST layout node type
 * @params {char*} name - Name
 * @returns {ast_layout_attribute_type_t} type - Layout Attribute Type
 *
 */
ast_layout_attribute_type_t name_to_ast_layout_attribute_type(char* name);

/**
 *
 * @function ast_layout_node_type_to_enduser_name
 * @brief Convert AST layout attribute type to enduser name
 * @params {ast_layout_node_type_t} type - Layout Attribute Type
 * @returns {char*} name - Name
 *
 */
char* ast_layout_node_type_to_enduser_name(ast_layout_node_type_t type);

/**
 *
 * @function enduser_name_to_ast_layout_node_type
 * @brief Convert enduser name to AST layout node type
 * @params {char*} name - Name
 * @returns {ast_layout_node_type_t} type - Layout Node Type
 *
 */
ast_layout_node_type_t enduser_name_to_ast_layout_node_type(char* name);

/**
 *
 * @function ast_layout_attribute_has_any_sub_value
 * @brief Check if the AST layout attribute has any sub value
 * @params {ast_layout_attribute_t*} value - AST layout attribute
 * @returns {bool} - True if the AST layout attribute has any sub value, false otherwise
 *
 */
bool ast_layout_attribute_has_any_sub_value(ast_layout_attribute_t* value);

#endif
