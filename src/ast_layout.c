#include "ast_layout.h"

/**
 *
 * @function ast_layout_attribute_print
 * @brief Print the AST layout attribute
 * @params {ast_layout_attribute_t*} value - AST layout attribute
 * @returns {void}
 *
 */
void ast_layout_attribute_print(ast_layout_attribute_t* value)
{
    DEBUG_ME;
	printf("Layout Attribute: %s\n", value->key);
	printf("Value(s): ");

	if (value->values == NULL) {
		printf("NULL\n");
	}
	else {
		value->values->print(value->values);
	}
}

/**
 *
 * @function ast_layout_block_create
 * @brief Create a new AST node layout block
 * @params {ast_type_t} node_type - Node type
 * @params {ast_layout_node_type_t} layout_node_type - Layout node type
 * @returns {ast_layout_block_t*} - Pointer to the created AST node layout block
 *
 */
ast_layout_block_t* ast_layout_block_create(ast_type_t node_type, ast_layout_node_type_t layout_node_type)
{
    DEBUG_ME;
	ast_layout_block_t* block = memory_allocate(sizeof(ast_layout_block_t));
	block->tag = NULL;
	block->type = AST_BLOCK_TYPE_LAYOUT;
	block->parent_type = node_type;
	block->parent_node_type = layout_node_type;
	block->text_content = NULL;

	block->attributes = cast(struct hashmap_t*, hashmap_create(3));

	block->styles = ast_layout_style_state_create();

	block->states = hashmap_create_layout_attribute_style_state(1);

	block->children = array_create(sizeof(ast_layout_node_t*), 16);
	block->children->print = cast(void (*)(void*), array_layout_node_print);
	block->children->destroy = cast(void (*)(void*), array_layout_node_destroy);

	block->print = cast(void (*)(void*), ast_layout_block_print);
	block->destroy = cast(void (*)(void*), ast_layout_block_destroy);

	return block;
}

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
ast_layout_attribute_t* ast_layout_attribute_create(ast_layout_attribute_type_t type, char* key, array_value_t* values, ast_layout_node_type_t parent_node_type, location_t last_name, location_t first_value)
{
    DEBUG_ME;
	ast_layout_attribute_t* attribute = memory_allocate(sizeof(ast_layout_attribute_t));
	attribute->type = type;

	attribute->parent_node_type = parent_node_type;

	attribute->key = strdup(key);
	attribute->values = values;

	attribute->isStyle = false;
	attribute->isContent = false;
	attribute->ignoreMe = false;

	attribute->key_location = last_name;
	attribute->value_location = first_value;

	attribute->final_key = NULL;
	attribute->final_value = NULL;

	attribute->print = cast(void (*)(void*), ast_layout_attribute_print);
	attribute->destroy = cast(void (*)(void*), ast_layout_attribute_destroy);

	return attribute;
}

/**
 *
 * @function ast_layout_attribute_copy
 * @brief Copy the AST layout attribute
 * @params {ast_layout_attribute_t*} value - AST layout attribute
 * @returns {ast_layout_attribute_t*} - Pointer to the copied AST layout attribute
 *
 */
ast_layout_attribute_t* ast_layout_attribute_copy(ast_layout_attribute_t* value)
{
	DEBUG_ME;
	ast_layout_attribute_t* copy = ast_layout_attribute_create(value->type, value->key, array_value_copy(value->values), value->parent_node_type, value->key_location, value->value_location);

	copy->isStyle = value->isStyle;
	copy->isContent = value->isContent;
	copy->ignoreMe = value->ignoreMe;

	if (value->final_key != NULL) {
		copy->final_key = strdup(value->final_key);
	}

	if (value->final_value != NULL) {
		copy->final_value = strdup(value->final_value);
	}

	return copy;
}

/**
 *
 * @function ast_layout_attribute_destroy
 * @brief Destroy the AST layout attribute
 * @params {ast_layout_attribute_t*} value - AST layout attribute
 * @returns {void}
 *
 */
void ast_layout_attribute_destroy(ast_layout_attribute_t* value)
{
    DEBUG_ME;
	if (value != NULL) {
		if (value->key != NULL) {
			memory_destroy(value->key);
		}

		if (value->final_key != NULL) {
			memory_destroy(value->final_key);
		}

		if (value->final_value != NULL) {
			memory_destroy(value->final_value);
		}

		if (value->values != NULL) {
			value->values->destroy(value->values);
		}

		memory_destroy(value);
	}
}

/**
 *
 * @function ast_layout_block_destroy
 * @brief Free the AST node layout block
 * @params {ast_layout_block_t*} value - AST layout block
 * @returns {void}
 *
 */
void ast_layout_block_destroy(ast_layout_block_t* value)
{
    DEBUG_ME;
	if (value != NULL) {
		if (value->attributes != NULL) {
			value->attributes->destroy(value->attributes);
		}

		if (value->styles != NULL) {
			value->styles->destroy(value->styles);
		}

		if (value->tag != NULL) {
			memory_destroy(value->tag);
		}

		if (value->states != NULL) {
			value->states->destroy(value->states);
		}

		if (value->text_content != NULL) {
			memory_destroy(value->text_content);
		}

		if (value->children != NULL) {
			value->children->destroy(value->children);
		}

		memory_destroy(value);
	}
}

/**
 *
 * @function ast_layout_block_print
 * @brief Free the AST node layout block
 * @params {ast_layout_block_t*} value - AST layout block
 * @returns {void}
 *
 */
void ast_layout_block_print(ast_layout_block_t* value)
{
    DEBUG_ME;
	hashmap_t* attributes = cast(hashmap_t*, value->attributes);
	hashmap_t* styles = cast(hashmap_t*, value->styles);
	array_node_layout_t* children = value->children;
	size_t children_capacity = array_length(children);

	printf("Block attributes:\n");
	attributes->print(attributes);

	printf("Block styles:\n");
	styles->print(styles);

	printf("Block children:\n");
	for (size_t i = 0; i < children_capacity; i++) {
		ast_layout_node_t* node = cast(ast_layout_node_t*, array_get(value->children, i));

		node->print(node);
	}
}

/**
 *
 * @function ast_layout_node_create
 * @brief Create a new AST node layout attribute
 * @params {ast_layout_node_type_t} layout_node_type - Layout node type
 * @returns {ast_layout_node_t*} - Pointer to the created AST node layout attribute
 *
 */
ast_layout_node_t* ast_layout_node_create(ast_layout_node_type_t layout_node_type)
{
    DEBUG_ME;
	ast_layout_node_t* node = memory_allocate(sizeof(ast_layout_node_t));
	node->tag = NULL;
	node->type = layout_node_type;
	node->block = ast_layout_block_create(AST_TYPE_LAYOUT, layout_node_type);

	node->print = cast(void (*)(void*), ast_layout_node_print);
	node->destroy = cast(void (*)(void*), ast_layout_node_destroy);

	return node;
}

/**
 *
 * @function ast_layout_node_print
 * @beief Print the AST layout node
 * @params {ast_layout_node_t*} value - AST layout node
 * @returns {void}
 *
 */
void ast_layout_node_print(ast_layout_node_t* value)
{
    DEBUG_ME;
	printf("Layout Node: %s\n", value->tag);
	value->block->print(value->block);
}

/**
 *
 * @function ast_layout_node_destroy
 * @brief Free the AST node layout attribute
 * @params {ast_layout_node_t*} value - AST layout node
 * @returns {void}
 *
 */
void ast_layout_node_destroy(ast_layout_node_t* value)
{
    DEBUG_ME;
	if (value != NULL) {
		if (value->tag != NULL) {
			memory_destroy(value->tag);
		}

		if (value->block != NULL) {
			ast_layout_block_destroy(value->block);
		}

		memory_destroy(value);
	}
}

/**
 *
 * @function ast_layout_create
 * @brief Create a new AST node layout attribute
 * @returns {ast_layout_attribute_t*} - Pointer to the created AST node layout attribute
 *
 */
ast_layout_t* ast_layout_create()
{
    DEBUG_ME;
	ast_layout_t* node = memory_allocate(sizeof(ast_layout_t));

	node->block = ast_layout_block_create(AST_TYPE_LAYOUT, AST_LAYOUT_TYPE_NONE);

	node->print = cast(void (*)(void*), ast_layout_print);
	node->destroy = cast(void (*)(void*), ast_layout_destroy);

	return node;
}

/**
 *
 * @function ast_layout_destroy
 * @brief Create a new AST node layout attribute
 * @params {ast_layout_t*} value - AST layout node
 * @returns {void}
 *
 */
void ast_layout_destroy(ast_layout_t* value)
{
    DEBUG_ME;
	if (value != NULL) {
		if (value->block != NULL) {
			value->block->destroy(value->block);
		}

		memory_destroy(value);
	}
}

/**
 *
 * @function ast_layout_print
 * @brief Print the AST layout
 * @params {ast_layout_t*} value - AST layout
 * @returns {void}
 *
 */
void ast_layout_print(ast_layout_t* value)
{
    DEBUG_ME;
	printf("Layout\n");
	value->block->print(value->block);
}

/**
 *
 * @function name_to_ast_layout_node_type
 * @brief Convert name to AST layout node type
 * @params {char*} name - Name
 * @returns {ast_layout_node_type_t} type - Layout Node Type
 *
 */
ast_layout_node_type_t name_to_ast_layout_node_type(char* name)
{
    DEBUG_ME;
	ast_layout_node_type_t type = AST_LAYOUT_TYPE_ERROR;

    #undef ADD_LAYOUT_TYPE
	#undef ADD_LAYOUT_TYPE_HIDE
	#undef ADD_LAYOUT_TYPE_REPEAT

	#define ADD_LAYOUT_TYPE(TYPE, NAME, NAME_LOWER, GENERATED_NAME, ENDUSER_NAME, IS_MOTHER) else if (strcmp(name, NAME) == 0) { type = TYPE; return type; }
	#define ADD_LAYOUT_TYPE_HIDE(TYPE, NAME, NAME_LOWER, GENERATED_NAME, ENDUSER_NAME, IS_MOTHER) else if (strcmp(name, NAME) == 0) { type = TYPE; return type; }
	#define ADD_LAYOUT_TYPE_REPEAT(TYPE, NAME, NAME_LOWER, GENERATED_NAME, ENDUSER_NAME, IS_MOTHER) else if (strcmp(name, NAME) == 0) { type = TYPE; return type; }

	if (false) {}
	#include "ast_layout_type.h"

	return type;
}

/**
 *
 * @function enduser_name_to_ast_layout_node_type
 * @brief Convert enduser name to AST layout node type
 * @params {char*} name - Name
 * @returns {ast_layout_node_type_t} type - Layout Node Type
 *
 */
ast_layout_node_type_t enduser_name_to_ast_layout_node_type(char* name)
{
    DEBUG_ME;
	ast_layout_node_type_t type = AST_LAYOUT_TYPE_ERROR;

    #undef ADD_LAYOUT_TYPE
	// #undef ADD_LAYOUT_TYPE_HIDE
	#undef ADD_LAYOUT_TYPE_REPEAT

	#define ADD_LAYOUT_TYPE(TYPE, NAME, NAME_LOWER, GENERATED_NAME, ENDUSER_NAME, IS_MOTHER) else if (strcmp(name, ENDUSER_NAME) == 0) { type = TYPE; return type; }
	// #define ADD_LAYOUT_TYPE_HIDE(TYPE, NAME, NAME_LOWER, GENERATED_NAME, ENDUSER_NAME, IS_MOTHER) else if (strcmp(name, ENDUSER_NAME) == 0) { type = TYPE; return type; }
	#define ADD_LAYOUT_TYPE_REPEAT(TYPE, NAME, NAME_LOWER, GENERATED_NAME, ENDUSER_NAME, IS_MOTHER) else if (strcmp(name, ENDUSER_NAME) == 0) { type = TYPE; return type; }

	if (false) {}
	#include "ast_layout_type.h"

	return type;
}

/**
 *
 * @function token_to_ast_layout_node_type
 * @brief Convert token to AST layout node type
 * @params {token_t*} token - Token
 * @returns {ast_layout_node_type_t} type - Layout Node Type
 *
 */
ast_layout_node_type_t token_to_ast_layout_node_type(token_t* token)
{
    DEBUG_ME;
	if (token->type != TOKEN_IDENTIFIER) {
		error_ast(2, "Expected token type to be identifier as layout node type, got %s at line %d, column %d", token_name(token->type), token->location.end_line, token->location.end_column);
	}

	ast_layout_node_type_t type = enduser_name_to_ast_layout_node_type(token->data.string);

	if (type == AST_LAYOUT_TYPE_ERROR) {
		error_ast(2, "Unknown layout node '%s' at line %d, column %d", token->data.string, token->location.end_line, token->location.end_column);
	}

	return type;
}

/**
 *
 * @function ast_layout_node_type_to_name
 * @brief Convert AST layout attribute type to name
 * @params {ast_layout_node_type_t} type - Layout Attribute Type
 * @returns {char*} name - Name
 *
 */
char* ast_layout_node_type_to_name(ast_layout_node_type_t type)
{
    DEBUG_ME;
	switch (type) {
		#undef ADD_LAYOUT_TYPE
	    #undef ADD_LAYOUT_TYPE_HIDE
		#undef ADD_LAYOUT_TYPE_REPEAT
		// #undef ADD_LAYOUT_ATTRIBUTE_TYPE

		#define ADD_LAYOUT_TYPE(TYPE, NAME, NAME_LOWER, GENERATED_NAME, ENDUSER_NAME, IS_MOTHER) case TYPE: return NAME;
		#define ADD_LAYOUT_TYPE_HIDE(TYPE, NAME, NAME_LOWER, GENERATED_NAME, ENDUSER_NAME, IS_MOTHER) case TYPE: return NAME;
		#define ADD_LAYOUT_TYPE_REPEAT(TYPE, NAME, NAME_LOWER, GENERATED_NAME, ENDUSER_NAME, IS_MOTHER)
		// #define ADD_LAYOUT_ATTRIBUTE_TYPE(TYPE, NAME, NAME_LOWER, GENERATED_NAME, ENDUSER_NAME) case TYPE: return NAME;

		#include "ast_layout_type.h"
		// #include "ast_layout_attribute_type.h"
	}

	return "error!!!";
}

/**
 *
 * @function ast_layout_node_type_to_enduser_name
 * @brief Convert AST layout attribute type to enduser name
 * @params {ast_layout_node_type_t} type - Layout Attribute Type
 * @returns {char*} name - Name
 *
 */
char* ast_layout_node_type_to_enduser_name(ast_layout_node_type_t type)
{
    DEBUG_ME;
	switch (type) {
		#undef ADD_LAYOUT_TYPE
	    #undef ADD_LAYOUT_TYPE_HIDE
		#undef ADD_LAYOUT_TYPE_REPEAT
		// #undef ADD_LAYOUT_ATTRIBUTE_TYPE

		#define ADD_LAYOUT_TYPE(TYPE, NAME, NAME_LOWER, GENERATED_NAME, ENDUSER_NAME, IS_MOTHER) case TYPE: return ENDUSER_NAME;
		#define ADD_LAYOUT_TYPE_HIDE(TYPE, NAME, NAME_LOWER, GENERATED_NAME, ENDUSER_NAME, IS_MOTHER) case TYPE: return ENDUSER_NAME;
		#define ADD_LAYOUT_TYPE_REPEAT(TYPE, NAME, NAME_LOWER, GENERATED_NAME, ENDUSER_NAME, IS_MOTHER)
		// #define ADD_LAYOUT_ATTRIBUTE_TYPE(TYPE, NAME, NAME_LOWER, GENERATED_NAME, ENDUSER_NAME) case TYPE: return ENDUSER_NAME;

		#include "ast_layout_type.h"
		// #include "ast_layout_attribute_type.h"
	}

	return "error!!!";
}

/**
 *
 * @function name_to_ast_layout_node_type
 * @brief Convert attribute name to AST layout node type
 * @params {char*} name - Name
 * @returns {ast_layout_attribute_type_t} type - Layout Attribute Type
 *
 */
ast_layout_attribute_type_t name_to_ast_layout_attribute_type(char* name)
{
    DEBUG_ME;
	ast_layout_attribute_type_t type = AST_LAYOUT_ATTRIBUTE_TYPE_ERROR;

    #undef ADD_LAYOUT_ATTRIBUTE_TYPE
    #undef ADD_LAYOUT_ATTRIBUTE_STYLE_TYPE
	#undef ADD_LAYOUT_ATTRIBUTE_STYLE_TYPE_HIDE

	#define ADD_LAYOUT_ATTRIBUTE_TYPE(TYPE, NAME, NAME_LOWER, GENERATED_NAME, ENDUSER_NAME) else if (strcmp(name, NAME_LOWER) == 0) { type = TYPE; return type; }
	#define ADD_LAYOUT_ATTRIBUTE_STYLE_TYPE(TYPE, NAME, NAME_LOWER, ENDUSER_NAME, GENERATED_NAME, FILTER, ALLOWED_VALUES, SUBTAGS) else if (strcmp(name, NAME_LOWER) == 0) { type = TYPE; return type; }
	#define ADD_LAYOUT_ATTRIBUTE_STYLE_TYPE_HIDE(TYPE, NAME, NAME_LOWER, ENDUSER_NAME, GENERATED_NAME, FILTER, ALLOWED_VALUES, SUBTAGS)

	if (false) {}
	#include "ast_layout_attribute_type.h"
	#include "ast_layout_attribute_style_type.h"

	return type;
}

/**
 *
 * @function enduser_name_to_ast_layout_attribute_type
 * @brief Convert enduser attribute name to AST layout node type
 * @params {char*} name - Name
 * @returns {ast_layout_attribute_type_t} type - Layout Attribute Type
 *
 */
ast_layout_attribute_type_t enduser_name_to_ast_layout_attribute_type(char* name)
{
    DEBUG_ME;
	ast_layout_attribute_type_t type = AST_LAYOUT_ATTRIBUTE_TYPE_ERROR;

    #undef ADD_LAYOUT_ATTRIBUTE_TYPE
    #undef ADD_LAYOUT_ATTRIBUTE_STYLE_TYPE
	#undef ADD_LAYOUT_ATTRIBUTE_STYLE_TYPE_HIDE

	#define ADD_LAYOUT_ATTRIBUTE_TYPE(TYPE, NAME, NAME_LOWER, GENERATED_NAME, ENDUSER_NAME) else if (strcmp(name, ENDUSER_NAME) == 0) { type = TYPE; return type; }
	#define ADD_LAYOUT_ATTRIBUTE_STYLE_TYPE(TYPE, NAME, NAME_LOWER, ENDUSER_NAME, GENERATED_NAME, FILTER, ALLOWED_VALUES, SUBTAGS) else if (strcmp(name, ENDUSER_NAME) == 0) { type = TYPE; return type; }
	#define ADD_LAYOUT_ATTRIBUTE_STYLE_TYPE_HIDE(TYPE, NAME, NAME_LOWER, ENDUSER_NAME, GENERATED_NAME, FILTER, ALLOWED_VALUES, SUBTAGS) else if (strcmp(name, ENDUSER_NAME) == 0) { type = TYPE; return type; }

	if (false) {}
	#include "ast_layout_attribute_type.h"
	#include "ast_layout_attribute_style_type.h"

	return type;
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

    #undef ADD_LAYOUT_ATTRIBUTE_TYPE
    #undef ADD_LAYOUT_ATTRIBUTE_STYLE_TYPE
    #undef ADD_LAYOUT_ATTRIBUTE_STYLE_TYPE_HIDE

	#define ADD_LAYOUT_ATTRIBUTE_TYPE(TYPE, NAME, NAME_LOWER, GENERATED_NAME, ENDUSER_NAME)
	#define ADD_LAYOUT_ATTRIBUTE_STYLE_TYPE(TYPE, NAME, NAME_LOWER, ENDUSER_NAME, GENERATED_NAME, FILTER, ALLOWED_VALUES, SUBTAGS) else if (strcmp(name, NAME_LOWER) == 0) { type = TYPE; return type; }
	#define ADD_LAYOUT_ATTRIBUTE_STYLE_TYPE_HIDE(TYPE, NAME, NAME_LOWER, GENERATED_NAME, ENDUSER_NAME, FILTER, ALLOWED_VALUES, SUBTAGS) else if (strcmp(name, NAME_LOWER) == 0) { type = TYPE; return type; }

	if (false) {}
	#include "ast_layout_attribute_style_type.h"

	return type;
}

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

    #undef ADD_LAYOUT_ATTRIBUTE_TYPE
    #undef ADD_LAYOUT_ATTRIBUTE_STYLE_TYPE
    #undef ADD_LAYOUT_ATTRIBUTE_STYLE_TYPE_HIDE

	#define ADD_LAYOUT_ATTRIBUTE_TYPE(TYPE, NAME, NAME_LOWER, GENERATED_NAME, ENDUSER_NAME)
	#define ADD_LAYOUT_ATTRIBUTE_STYLE_TYPE(TYPE, NAME, NAME_LOWER, ENDUSER_NAME, GENERATED_NAME, FILTER, ALLOWED_VALUES, SUBTAGS) else if (strcmp(name, ENDUSER_NAME) == 0) { type = TYPE; return type; }
	#define ADD_LAYOUT_ATTRIBUTE_STYLE_TYPE_HIDE(TYPE, NAME, NAME_LOWER, GENERATED_NAME, ENDUSER_NAME, FILTER, ALLOWED_VALUES, SUBTAGS) else if (strcmp(name, ENDUSER_NAME) == 0) { type = TYPE; return type; }

	if (false) {}
	#include "ast_layout_attribute_style_type.h"

	return type;
}

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
ast_layout_attribute_type_t token_to_ast_layout_attribute_type(char* name, token_t* token, ast_layout_node_type_t parent_node_type)
{
    DEBUG_ME;
	ast_layout_attribute_type_t type = enduser_name_to_ast_layout_attribute_type(name);

	if (type == AST_LAYOUT_ATTRIBUTE_TYPE_ERROR) {
		error_ast(2, "Unknown layout attribute '%s' at line %d, column %d for '%s' element", name, token->location.end_line, token->location.end_column, ast_layout_node_type_to_enduser_name(parent_node_type));
	}

	return type;
}

/**
 *
 * @function ast_layout_attribute_type_to_name
 * @brief Convert AST layout attribute type to name
 * @params {ast_layout_attribute_type_t} type - Layout Attribute Type
 * @returns {char*} name - Name
 *
 */
char* ast_layout_attribute_type_to_name(ast_layout_attribute_type_t type)
{
    DEBUG_ME;
	switch (type) {
		#undef ADD_LAYOUT_ATTRIBUTE_TYPE
		#undef ADD_LAYOUT_ATTRIBUTE_STYLE_TYPE
		#undef ADD_LAYOUT_ATTRIBUTE_STYLE_TYPE_HIDE

		#define ADD_LAYOUT_ATTRIBUTE_TYPE(TYPE, NAME, NAME_LOWER, GENERATED_NAME, ENDUSER_NAME) case TYPE: return NAME_LOWER;
		#define ADD_LAYOUT_ATTRIBUTE_STYLE_TYPE(TYPE, NAME, NAME_LOWER, ENDUSER_NAME, GENERATED_NAME, FILTER, ALLOWED_VALUES, SUBTAGS) case TYPE: return NAME_LOWER;
		#define ADD_LAYOUT_ATTRIBUTE_STYLE_TYPE_HIDE(TYPE, NAME, NAME_LOWER, GENERATED_NAME, ENDUSER_NAME, FILTER, ALLOWED_VALUES, SUBTAGS)

		#include "ast_layout_attribute_type.h"
		#include "ast_layout_attribute_style_type.h"
	}

	return "error???";
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

/**
 *
 * @function ast_layout_attribute_has_any_sub_value
 * @brief Check if the AST layout attribute has any sub value
 * @params {ast_layout_attribute_t*} value - AST layout attribute
 * @returns {bool} - True if the AST layout attribute has any sub value, false otherwise
 *
 */
bool ast_layout_attribute_has_any_sub_value(ast_layout_attribute_t* value)
{
	DEBUG_ME;
	// if (value->values != NULL && value->values->length > 0 && value->final_key != NULL && value->final_value != NULL) {
	if (value->values != NULL && value->values->length > 0) {
		return true;
	}

	return false;
}
