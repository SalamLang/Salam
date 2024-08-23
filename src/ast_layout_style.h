#ifndef _AST_LAYOUT_STYLE_H_
#define _AST_LAYOUT_STYLE_H_

#include <string.h>
#include <stdbool.h>

#include "base.h"
#include "memory.h"
#include "ast.h"
#include "array.h"
#include "array_custom.h"
#include "hashmap.h"
#include "hashmap_custom.h"
#include "ast_layout.h"

typedef enum {
    #undef ADD_LAYOUT_ATTRIBUTE_STYLE_FILTER
    #define ADD_LAYOUT_ATTRIBUTE_STYLE_FILTER(TYPE, NAME, NAME_LOWER) TYPE,

    #include "ast_layout_attribute_style_filter.h"
} ast_layout_attribute_style_filter_t;

typedef struct ast_layout_style_state_t {
    struct hashmap_t* normal;
    struct hashmap_t* new;

    void (*destroy)(void* node);
    void (*print)(void* node);
} ast_layout_style_state_t;

typedef enum ast_layout_attribute_style_state_type {
    #undef ADD_LAYOUT_ATTRIBUTE_STYLE_STATE_TYPE

    #define ADD_LAYOUT_ATTRIBUTE_STYLE_STATE_TYPE(TYPE, NAME, NAME_LOWER, ENDUSER_NAME, GENERATED_NAME) TYPE,

    #include "ast_layout_attribute_style_state_type.h"
} ast_layout_attribute_style_state_type;

/**
 *
 * @function name_to_ast_layout_attribute_style_type
 * @brief Convert style attribute name to AST layout node type
 * @params {char*} name - Name
 * @returns {ast_layout_attribute_type_t} type - Layout Attribute Type
 *
 */
ast_layout_attribute_type_t name_to_ast_layout_attribute_style_type(char* name);

/**
 *
 * @function enduser_name_to_ast_layout_attribute_style_type
 * @brief Convert style end-user attribute name to AST layout node type
 * @params {char*} name - Name
 * @returns {ast_layout_attribute_type_t} type - Layout Attribute Type
 *
 */
ast_layout_attribute_type_t enduser_name_to_ast_layout_attribute_style_type(char* name);

/**
 *
 * @function name_to_ast_layout_attribute_style_state_type
 * @brief Convert state type name to AST layout node type
 * @params {char*} name - Name
 * @returns {ast_layout_attribute_style_state_type} type - Layout Attribute Style State Type
 *
 */
ast_layout_attribute_style_state_type name_to_ast_layout_attribute_style_state_type(char* name);

/**
 *
 * @function enduser_name_to_ast_layout_attribute_style_state_type
 * @brief Convert state type enduser name to AST layout node type
 * @params {char*} name - Name
 * @returns {ast_layout_attribute_style_state_type} type - Layout Attribute Style State Type
 *
 */
ast_layout_attribute_style_state_type enduser_name_to_ast_layout_attribute_style_state_type(char* name);

/**
 *
 * @function ast_layout_style_state_destroy
 * @brief Destroy the AST layout style state
 * @params {ast_layout_style_state_t*} ast - AST layout style state
 * @returns {void}
 *
 */
void ast_layout_style_state_destroy(ast_layout_style_state_t* ast);

/**
 *
 * @function ast_layout_style_state_print
 * @brief Print the AST layout style state
 * @params {ast_layout_style_state_t*} ast - AST layout style state
 * @returns {void}
 *
 */
void ast_layout_style_state_print(ast_layout_style_state_t* ast);

/**
 *
 * @function ast_layout_style_state_create
 * @brief Create a new AST layout style state
 * @returns {ast_layout_style_state_t*} - Pointer to the created AST layout style state
 *
 */
ast_layout_style_state_t* ast_layout_style_state_create();

/**
 *
 * @function ast_layout_attribute_style_state_has_any_sub_value
 * @brief Check if the AST layout style state has any sub value
 * @params {ast_layout_style_state_t*} value - AST layout style state
 * @returns {bool} - True if the AST layout style state has any sub value, false otherwise
 *
 */
bool ast_layout_style_state_has_any_sub_value(ast_layout_style_state_t* value);

#endif
