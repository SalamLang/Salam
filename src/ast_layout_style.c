#include "ast_layout_style.h"

/**
 *
 * @function enduser_name_to_ast_layout_attribute_style_type
 * @brief Convert style end-user attribute name to AST layout node type
 * @params {char*} name - Name
 * @returns {ast_layout_attribute_type_t} type - Layout Attribute Type
 *
 */
ast_layout_attribute_type_t enduser_name_to_ast_layout_attribute_style_type(char* name)
{
    DEBUG_ME;
	ast_layout_attribute_type_t type = AST_LAYOUT_ATTRIBUTE_TYPE_ERROR;

	if (false) {}

    #undef ADD_LAYOUT_ATTRIBUTE_STYLE_TYPE
    #undef ADD_LAYOUT_ATTRIBUTE_STYLE_TYPE_REPEAT
    #undef ADD_LAYOUT_ATTRIBUTE_STYLE_TYPE_HIDE

	#define ADD_LAYOUT_ATTRIBUTE_STYLE_TYPE(TYPE, NAME, NAME_LOWER, ENDUSER_NAME, GENERATED_NAME, FILTER, ALLOWED_VALUES, SUBTAGS) else if (strcmp(name, ENDUSER_NAME) == 0) { type = TYPE; return type; }
	#define ADD_LAYOUT_ATTRIBUTE_STYLE_TYPE_REPEAT(TYPE, NAME, NAME_LOWER, ENDUSER_NAME, GENERATED_NAME, FILTER, ALLOWED_VALUES, SUBTAGS) else if (strcmp(name, ENDUSER_NAME) == 0) { type = TYPE; return type; }
	#define ADD_LAYOUT_ATTRIBUTE_STYLE_TYPE_HIDE(TYPE, NAME, NAME_LOWER, ENDUSER_NAME, GENERATED_NAME, FILTER, ALLOWED_VALUES, SUBTAGS) 

	#include "ast_layout_attribute_style_type.h"

	return type;
}

/**
 *
 * @function name_to_ast_layout_attribute_style_type
 * @brief Convert style attribute name to AST layout node type
 * @params {char*} name - Name
 * @returns {ast_layout_attribute_type_t} type - Layout Attribute Type
 *
 */
ast_layout_attribute_type_t name_to_ast_layout_attribute_style_type(char* name)
{
    DEBUG_ME;
	ast_layout_attribute_type_t type = AST_LAYOUT_ATTRIBUTE_TYPE_ERROR;

	if (false) {}

    #undef ADD_LAYOUT_ATTRIBUTE_STYLE_TYPE
    #undef ADD_LAYOUT_ATTRIBUTE_STYLE_TYPE_REPEAT
    #undef ADD_LAYOUT_ATTRIBUTE_STYLE_TYPE_HIDE

	#define ADD_LAYOUT_ATTRIBUTE_STYLE_TYPE(TYPE, NAME, NAME_LOWER, ENDUSER_NAME, GENERATED_NAME, FILTER, ALLOWED_VALUES, SUBTAGS) else if (strcmp(name, NAME_LOWER) == 0) { type = TYPE; return type; }
	#define ADD_LAYOUT_ATTRIBUTE_STYLE_TYPE_REPEAT(TYPE, NAME, NAME_LOWER, ENDUSER_NAME, GENERATED_NAME, FILTER, ALLOWED_VALUES, SUBTAGS) else if (strcmp(name, NAME_LOWER) == 0) { type = TYPE; return type; }
	#define ADD_LAYOUT_ATTRIBUTE_STYLE_TYPE_HIDE(TYPE, NAME, NAME_LOWER, ENDUSER_NAME, GENERATED_NAME, FILTER, ALLOWED_VALUES, SUBTAGS) else if (strcmp(name, NAME_LOWER) == 0) { type = TYPE; return type; }

	#include "ast_layout_attribute_style_type.h"

	return type;
}


/**
 *
 * @function enduser_name_to_ast_layout_attribute_style_state_type
 * @brief Convert state type enduser name to AST layout node type
 * @params {char*} name - Name
 * @returns {ast_layout_attribute_style_state_type} type - Layout Attribute Style State Type
 *
 */
ast_layout_attribute_style_state_type enduser_name_to_ast_layout_attribute_style_state_type(char* name)
{
	DEBUG_ME;
	ast_layout_attribute_style_state_type type = AST_LAYOUT_ATTRIBUTE_STYLE_STATE_TYPE_ERROR;

    #undef ADD_LAYOUT_ATTRIBUTE_STYLE_STATE_TYPE

    #define ADD_LAYOUT_ATTRIBUTE_STYLE_STATE_TYPE(TYPE, NAME, NAME_LOWER, ENDUSER_NAME, GENERATED_NAME) else if (strcmp(name, ENDUSER_NAME) == 0) { type = TYPE; return type; }

	if (false) {}
	#include "ast_layout_attribute_style_state_type.h"

	return type;
}


/**
 *
 * @function ast_layout_style_state_print
 * @brief Print the AST layout style state
 * @params {ast_layout_style_state_t*} ast - AST layout style state
 * @returns {void}
 *
 */
void ast_layout_style_state_print(ast_layout_style_state_t* ast)
{
	DEBUG_ME;
	printf("Style State\n");
	if (ast != NULL) {
		if (ast->normal != NULL) {
			printf("Normal\n");
			ast->normal->print(ast->normal);
		}
		else {
			printf("Normal: NULL\n");
		}

		if (ast->new != NULL) {
			printf("New\n");
			ast->new->print(ast->new);
		}
		else {
			printf("New: NULL\n");
		}
	}
	else {
		printf("NULL\n");
	}
}


/**
 *
 * @function ast_layout_style_state_create
 * @brief Create a new AST layout style state
 * @returns {ast_layout_style_state_t*} - Pointer to the created AST layout style state
 *
 */
ast_layout_style_state_t* ast_layout_style_state_create()
{
	DEBUG_ME;
	ast_layout_style_state_t* ast = memory_allocate(sizeof(ast_layout_style_state_t));

	ast->normal = hashmap_create_layout_attribute(1);
	ast->new = hashmap_create_layout_attribute(1);

	ast->print = cast(void (*)(void*), ast_layout_style_state_print);
	ast->destroy = cast(void (*)(void*), ast_layout_style_state_destroy);

	return ast;
}

/**
 *
 * @function ast_layout_style_state_destroy
 * @brief Destroy the AST layout style state
 * @params {ast_layout_style_state_t*} ast - AST layout style state
 * @returns {void}
 *
 */
void ast_layout_style_state_destroy(ast_layout_style_state_t* ast)
{
	DEBUG_ME;
	if (ast != NULL) {
		if (ast->normal != NULL) {
			ast->normal->destroy(ast->normal);
		}

		if (ast->new != NULL) {
			ast->new->destroy(ast->new);
		}

		memory_destroy(ast);
	}
}

/**
 *
 * @function name_to_ast_layout_attribute_style_state_type
 * @brief Convert state type name to AST layout node type
 * @params {char*} name - Name
 * @returns {ast_layout_attribute_style_state_type} type - Layout Attribute Style State Type
 *
 */
ast_layout_attribute_style_state_type name_to_ast_layout_attribute_style_state_type(char* name)
{
	DEBUG_ME;
	ast_layout_attribute_style_state_type type = AST_LAYOUT_ATTRIBUTE_STYLE_STATE_TYPE_ERROR;

    #undef ADD_LAYOUT_ATTRIBUTE_STYLE_STATE_TYPE

    #define ADD_LAYOUT_ATTRIBUTE_STYLE_STATE_TYPE(TYPE, NAME, NAME_LOWER, ENDUSER_NAME, GENERATED_NAME) else if (strcmp(name, NAME) == 0) { type = TYPE; return type; }

	if (false) {}
	#include "ast_layout_attribute_style_state_type.h"

	return type;
}

/**
 *
 * @function ast_layout_attribute_style_state_has_any_sub_value
 * @brief Check if the AST layout style state has any sub value
 * @params {ast_layout_style_state_t*} value - AST layout style state
 * @returns {bool} - True if the AST layout style state has any sub value, false otherwise
 *
 */
bool ast_layout_style_state_has_any_sub_value(ast_layout_style_state_t* value)
{
	DEBUG_ME;
	if (value->normal->length > 0) {
		return hashmap_layout_attribute_has_any_sub_value(value->normal);
	}

	if (value->new->length > 0) {
		return hashmap_layout_attribute_has_any_sub_value(value->new);
	}

	return false;
}
