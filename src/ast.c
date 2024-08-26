#include "ast.h"

/**
 *
 * @function ast_node_create
 * @brief Create a new AST node
 * @params {ast_type_t} type - Type of the AST node
 * @params {location_t} location - Location of the AST node
 * @returns {ast_node_t*} - Pointer to the created AST node
 *
 */
ast_node_t* ast_node_create(ast_type_t type, location_t location)
{
	DEBUG_ME;
	ast_node_t* node = memory_allocate(sizeof(ast_node_t));
	node->type = type;
	node->location = location;

	node->print = cast(void (*)(void*), ast_node_print);
	node->destroy = cast(void (*)(void*), ast_node_destroy);

	return node;
}

/**
 *
 * @function ast_node_destroy_notall
 * @brief Free the AST node (not all)
 * @params {ast_node_t*} value - AST node
 * @returns {void}
 *
 */
void ast_node_destroy_notall(ast_node_t* value)
{
	DEBUG_ME;
	if (value != NULL) {
		memory_destroy(value);
	}
}

/**
 *
 * @function ast_node_destroy
 * @brief Free the AST node
 * @params {ast_node_t*} value - AST node
 * @returns {void}
 *
 */
void ast_node_destroy(ast_node_t* value)
{
	DEBUG_ME;
	if (value != NULL) {
		switch (value->type) {
			case AST_TYPE_IMPORT:
				value->data.import->destroy(value->data.import);
				break;

			case AST_TYPE_FUNCTION:
				value->data.function->destroy(value->data.function);
				break;

			case AST_TYPE_BLOCK:
				value->data.block->destroy(value->data.block);
				break;

			case AST_TYPE_LAYOUT:
				value->data.layout->destroy(value->data.layout);
				break;

			case AST_TYPE_RETURN:
				value->data.returns->destroy(value->data.returns);
				break;

			case AST_TYPE_IF:
			case AST_TYPE_ELSE_IF:
				value->data.ifclause->destroy(value->data.ifclause);
				break;

			case AST_TYPE_PRINT:
				value->data.print->destroy(value->data.print);
				break;

			case AST_TYPE_ERROR:
				break;
		}

		memory_destroy(value);
	}
}

/**
 *
 * @function ast_value_create
 * @brief Create a new AST node layout attribute value
 * @params {ast_value_type_t*} type - Type of the layout attribute value
 * @params {void*} value - Value of the layout attribute value
 * @returns {ast_value_t*} - Pointer to the created AST node layout attribute value
 *
 */
ast_value_t* ast_value_create(ast_value_type_t* type, void* value)
{
	DEBUG_ME;
	size_t value_length = strlen(value);
	ast_value_t* res = memory_allocate(sizeof(ast_value_t));

	res->type = type;

	if (type->kind == AST_TYPE_KIND_STRING) {
		size_t value_size = value_length < 1 ? 1 : value_length + 1;

		res->data.string_value = memory_allocate(value_size);

		memcpy(res->data.string_value, value, value_size);
	}

	res->print = cast(void (*)(void*), ast_value_print);
	res->destroy = cast(void (*)(void*), ast_value_destroy);
	res->get_data = cast(char* (*)(void*), ast_value_data);

	return res;
}

/**
 *
 * @function ast_value_data
 * @brief Get the string of the AST layout attribute value
 * @params {ast_value_t*} value - AST layout attribute value
 * @returns {char*} - String of the AST layout attribute value
 *
 */
char* ast_value_data(ast_value_t* value)
{
	DEBUG_ME;
	if (value->type->kind == AST_TYPE_KIND_STRING) {
		return value->data.string_value;
	}

	return "other value";
}

/**
 *
 * @function ast_block_create
 * @brief Create a new AST block node
 * @params {ast_block_type_t} type - Block type
 * @params {ast_type_t} parent_type - Parent type
 * @returns {ast_block_t*} - AST block node
 *
 */
ast_block_t* ast_block_create(ast_block_type_t type, ast_type_t parent_type)
{
	DEBUG_ME;
	ast_block_t* block = malloc(sizeof(ast_block_t));

	block->type = type;
	block->parent_type = parent_type;

	block->children = array_create(sizeof(ast_node_t*), 4);

	block->children->print = cast(void (*)(void*), array_node_print);
	block->children->destroy = cast(void (*)(void*), array_node_destroy);

	block->print = cast(void (*)(void*), ast_block_print);
	block->destroy = cast(void (*)(void*), ast_block_destroy);

	return block;
}

/**
 *
 * @function ast_print_create
 * @brief Create a new AST node print
 * @params {array_value_t*} values - Values of the print
 * @returns {ast_print_t*} - Pointer to the created AST node if
 *
 */
ast_print_t* ast_print_create(array_value_t* values)
{
	DEBUG_ME;
	ast_print_t* node = memory_allocate(sizeof(ast_print_t));

	node->values = values;

	node->print = cast(void (*)(void*), ast_print_print);
	node->destroy = cast(void (*)(void*), ast_print_destroy);

	return node;
}

/**
 *
 * @function ast_print_print
 * @brief Print the AST print node
 * @params {ast_print_t*} node - AST print node
 * @returns {void}
 *
 */
void ast_print_print(ast_print_t* node)
{
	DEBUG_ME;
	printf("Print\n");

	if (node->values != NULL) {
		printf("Values\n");
		node->values->print(node->values);
	}
}

/**
 *
 * @function ast_print_destroy
 * @brief Free the AST print node
 * @params {ast_print_t*} node - AST print node
 * @returns {void}
 *
 */
void ast_print_destroy(ast_print_t* node)
{
	DEBUG_ME;
	if (node != NULL) {
		if (node->values != NULL) {
			node->values->destroy(node->values);
		}

		memory_destroy(node);
	}
}

/**
 *
 * @function ast_return_create
 * @brief Create a new AST node return
 * @params {array_value_t*} values - Values of the return
 * @returns {ast_return_t*} - Pointer to the created AST node if
 *
 */
ast_return_t* ast_return_create(array_value_t* values)
{
	DEBUG_ME;
	ast_return_t* node = memory_allocate(sizeof(ast_return_t));

	node->values = values;

	node->print = cast(void (*)(void*), ast_return_print);
	node->destroy = cast(void (*)(void*), ast_return_destroy);

	return node;
}

/**
 *
 * @function ast_return_print
 * @brief Print the AST return node
 * @params {ast_return_t*} node - AST return node
 * @returns {void}
 */
void ast_return_print(ast_return_t* node)
{
	DEBUG_ME;
	printf("Return\n");

	if (node->values != NULL) {
		printf("Values\n");
		node->values->print(node->values);
	}
}

/**
 *
 * @function ast_return_destroy
 * @brief Free the AST return node
 * @params {ast_return_t*} node - AST return node
 * @returns {void}
 *
 */
void ast_return_destroy(ast_return_t* node)
{
	DEBUG_ME;
	if (node != NULL) {
		if (node->values != NULL) {
			node->values->destroy(node->values);
		}

		memory_destroy(node);
	}
}

/**
 *
 * @function ast_if_create
 * @brief Create a new AST node if
 * @params {ast_value_t*} condition - Condition of the if
 * @returns {ast_if_t*} - Pointer to the created AST node if
 *
 */
ast_if_t* ast_if_create(ast_value_t* condition)
{
	DEBUG_ME;
	ast_if_t* node = memory_allocate(sizeof(ast_if_t));

	node->condition = condition;

	node->block = ast_block_create(AST_BLOCK_TYPE_IF, AST_TYPE_IF); // TODO???

	node->block->print = cast(void (*)(void*), ast_block_print);
	node->block->destroy = cast(void (*)(void*), ast_block_destroy);

	node->else_blocks = array_create(sizeof(ast_if_t*), 16); // Can be NULL for sub else if

	node->else_blocks->print = cast(void (*)(void*), array_if_print);
	node->else_blocks->destroy = cast(void (*)(void*), array_if_destroy);

	node->print = cast(void (*)(void*), ast_if_print);
	node->destroy = cast(void (*)(void*), ast_if_destroy);

	return node;
}

/**
 *
 * @function ast_elseif_create
 * @brief Create a new AST node else if
 * @params {ast_value_t*} condition - Condition of the else if
 * @returns {ast_if_t*} - Pointer to the created AST node else if
 *
 */
ast_if_t* ast_elseif_create(ast_value_t* condition)
{
	DEBUG_ME;
	ast_if_t* node = memory_allocate(sizeof(ast_if_t));

	node->condition = condition;

	node->block = ast_block_create(AST_BLOCK_TYPE_IF, AST_TYPE_ELSE_IF); // TODO???
	node->block->print = cast(void (*)(void*), ast_block_print);
	node->block->destroy = cast(void (*)(void*), ast_block_destroy);

	node->else_blocks = NULL;

	node->print = cast(void (*)(void*), ast_if_print);
	node->destroy = cast(void (*)(void*), ast_if_destroy);

	return node;
}

/**
 *
 * @function ast_else_create
 * @brief Create a new AST node else
 * @returns {ast_if_t*} - Pointer to the created AST node else
 *
 */
ast_if_t* ast_else_create()
{
	DEBUG_ME;
	ast_if_t* node = memory_allocate(sizeof(ast_if_t));

	node->condition = NULL;

	node->block = ast_block_create(AST_BLOCK_TYPE_ELSE_IF, AST_TYPE_ELSE_IF); // TODO???
	node->block->print = cast(void (*)(void*), ast_block_print);
	node->block->destroy = cast(void (*)(void*), ast_block_destroy);

	node->else_blocks = NULL;

	node->print = cast(void (*)(void*), ast_if_print);
	node->destroy = cast(void (*)(void*), ast_if_destroy);

	return node;
}

/**
 *
 * @function ast_if_create
 * @brief Create a new AST node if
 * @returns {ast_if_t*} - Pointer to the created AST node if
 *
 */
void ast_if_print(ast_if_t* node)
{
	DEBUG_ME;
	printf("If\n");

	printf("Condition\n");
	if (node->condition != NULL) {
		node->condition->print(node->condition);
	}
	else {
		printf("NULL\n");
	}

	printf("Else Blocks\n");
	if (node->else_blocks != NULL) {
		node->else_blocks->print(node->else_blocks);
	}
	else {
		printf("NULL\n");
	}

	printf("Block\n");
	if (node->block != NULL) {
		node->block->print(node->block);
	}
	else {
		printf("NULL\n");
	}
}

/**
 *
 * @function ast_if_destroy
 * @brief Free the AST if node
 * @params {ast_if_t*} node - AST if node
 * @returns {void}
 *
 */
void ast_if_destroy(ast_if_t* node)
{
	DEBUG_ME;
	if (node != NULL) {
		DEBUG_ME;
		if (node->condition != NULL) {
			node->condition->destroy(node->condition);
		}

		DEBUG_ME;
		if (node->block != NULL) {
			node->block->destroy(node->block);
		}

		DEBUG_ME;
		if (node->else_blocks != NULL) {
			node->else_blocks->destroy(node->else_blocks);
		}

		DEBUG_ME;
		memory_destroy(node);
	}
}

/**
 *
 * @function ast_function_create
 * @brief Create a new AST node function
 * @params {char*} name - Name of the function
 * @returns {ast_function_t*} - Pointer to the created AST node function
 *
 */
ast_function_t* ast_function_create(char* name)
{
	DEBUG_ME;
	ast_function_t* node = memory_allocate(sizeof(ast_function_t));
	node->name = strdup(name);

	location_t return_location = { 0, 0, 0, 0, 0, 0 }; // TODO: Fix this
	node->return_type = ast_value_type_create(AST_TYPE_KIND_VOID, return_location);

	node->parameters = array_create(sizeof(ast_function_parameter_t*), 16);

	node->parameters->print = cast(void (*)(void*), array_function_parameter_print);
	node->parameters->destroy = cast(void (*)(void*), array_function_parameter_destroy);

	node->block = ast_block_create(AST_BLOCK_TYPE_FUNCTION, AST_TYPE_FUNCTION);

	node->block->print = cast(void (*)(void*), ast_block_print);
	node->block->destroy = cast(void (*)(void*), ast_block_destroy);

	node->print = cast(void (*)(void*), ast_function_print);
	node->destroy = cast(void (*)(void*), ast_function_destroy);

	return node;
}

/**
 *
 * @function ast_block_print
 * @brief Print the AST block node
 * @params {ast_block_t*} block - AST block node
 * @returns {void}
 *
 */
void ast_block_print(ast_block_t* block)
{
	DEBUG_ME;
	size_t children_capacity = array_length(block->children);

	printf("Block: %zu\n", children_capacity);
	for (size_t i = 0; i < children_capacity; i++) {
		ast_node_t* node = cast(ast_node_t*, array_get(block->children, i));

		node->print(node);
	}
}

/**
 *
 * @function ast_block_destroy
 * @brief Free the AST block node
 * @params {ast_block_t*} block - AST block node
 * @returns {void}
 *
 */
void ast_block_destroy(ast_block_t* block)
{
	DEBUG_ME;
	if (block != NULL) {
		if (block->children != NULL) {
			block->children->destroy(block->children);
		}

		memory_destroy(block);
	}
}

/**
 *
 * @function ast_value_type_name
 * @brief Get the name of the AST value type
 * @params {ast_value_type_t*} type - AST value type
 * @returns {char*} - Name of the AST value type
 *
 */
char* ast_value_type_name(ast_value_type_t* type)
{
	DEBUG_ME;
	switch (type->kind) {
		#undef ADD_VALUE_KIND
		#define ADD_VALUE_KIND(TYPE, NAME, NAME_LOWER) case TYPE: return NAME;

		#include "ast_value_kind.h"
	}

	return "unknown";
}

/**
 *
 * @function ast_value_type_print
 * @brief Print the AST value type
 * @params {ast_value_type_t*} type - AST value type
 * @returns {void}
 *
 */
void ast_value_type_print(ast_value_type_t* type)
{
	DEBUG_ME;
	printf("Type:\n");
	printf("%s\n", ast_value_type_name(type));
}

/**
 *
 * @function ast_value_type_create
 * @brief Create a new AST value type
 * @params {ast_value_kind_t} kind - Kind of the value type
 * @params {location_t} location - Location of the value type
 * @returns {ast_value_type_t*} - Pointer to the created AST value type
 *
 */
ast_value_type_t* ast_value_type_create(ast_value_kind_t kind, location_t location)
{
	DEBUG_ME;
	ast_value_type_t* type = memory_allocate(sizeof(ast_value_type_t));
	type->kind = kind;
	type->location = location;

	type->print = cast(void (*)(void*), ast_value_type_print);
	type->destroy = cast(void (*)(void*), ast_value_type_destroy);

	return type;
}

/**
 *
 * @function ast_value_type_destroy
 * @brief Free the AST value type
 * @params {ast_value_type_t*} type - AST value type
 * @returns {void}
 *
 */
void ast_value_type_destroy(ast_value_type_t* type)
{
	DEBUG_ME;
	if (type != NULL) {
		memory_destroy(type);
	}
}

/**
 *
 * @function ast_function_parameter_print
 * @brief Print the AST function parameter
 * @params {ast_function_parameter_t*} value - AST function parameter
 * @returns {void}
 *
 */
void ast_function_parameter_print(ast_function_parameter_t* value)
{
	DEBUG_ME;
	printf("Parameter: %s\n", value->name);
}

/**
 *
 * @function ast_function_parameter_destroy
 * @brief Free the AST function parameter
 * @params {ast_function_parameter_t*} value - AST function parameter
 * @returns {void}
 *
 */
void ast_function_parameter_destroy(ast_function_parameter_t* value)
{
	DEBUG_ME;
	if (value != NULL) {
		if (value->name != NULL) {
			memory_destroy(value->name);
		}

		memory_destroy(value);
	}
}

/**
 *
 * @function ast_function_print
 * @brief Print the AST function
 * @params {ast_function_t*} value - AST function
 * @returns {void}
 *
 */
void ast_function_print(ast_function_t* value)
{
	DEBUG_ME;
	printf("Function: %s\n", value->name);

	array_function_parameter_t* parameters = value->parameters;
	size_t parameters_capacity = array_length(parameters);

	printf("Return Type: ");
	value->return_type->print(value->return_type);
	printf("\n");

	printf("Parameters: %zu\n", parameters_capacity);
	for (size_t i = 0; i < parameters_capacity; i++) {
		ast_function_parameter_t* parameter = cast(ast_function_parameter_t*, array_get(parameters, i));

		parameter->print(parameter);
	}

	printf("Block: ");
	if (value->block != NULL) {
		printf("\n");

		value->block->print(value->block);
	}
	else {
		printf("NULL\n");
	}
}

/**
 *
 * @function ast_function_destroy
 * @brief Free the AST function
 * @params {ast_function_t*} value - AST function
 * @returns {void}
 *
 */
void ast_function_destroy(ast_function_t* value)
{
	DEBUG_ME;
	if (value != NULL) {
		if (value->name != NULL) {
			memory_destroy(value->name);
		}

		if (value->parameters != NULL) {
			value->parameters->destroy(value->parameters);
		}

		if (value->return_type != NULL) {
			value->return_type->destroy(value->return_type);
		}

		if (value->block != NULL) {
			value->block->destroy(value->block);
		}

		memory_destroy(value);
	}
}

/**
 *
 * @function ast_node_print
 * @brief Print the AST node
 * @params {ast_node_t*} node - AST node
 * @returns {void}
 *
 */
void ast_node_print(ast_node_t* node)
{
	DEBUG_ME;
	switch (node->type) {
		#undef ADD_TYPE
		#define ADD_TYPE(TYPE, NAME, NAME_LOWER) case TYPE: printf("%s\n", NAME); break;

		#include "ast_type.h"
	}
}

/**
 *
 * @function ast_block_type_name
 * @brief Print the AST block type
 * @params {ast_block_type_t} type - AST block type
 * @returns {char*} - Name of the AST block type
 */
char* ast_block_type_name(ast_block_type_t type)
{
	DEBUG_ME;
	switch (type) {
		#undef ADD_BLOCK_TYPE
		#define ADD_BLOCK_TYPE(TYPE, NAME, NAME_LOWER) case TYPE: printf("%s\n", NAME); break;

		#include "ast_block_type.h"
	}

	return "unknown block";
}

/**
 *
 * @function ast_create
 * @brief Create a new AST
 * @returns {ast_t*} - Pointer to the created AST
 *
 */
ast_t* ast_create()
{
	DEBUG_ME;
	ast_t* ast = memory_allocate(sizeof(ast_t));
	ast->layout = NULL;

	ast->functions = array_create(sizeof(ast_function_t*), 16);
	ast->functions->print = cast(void (*)(void*), array_function_print);
	ast->functions->destroy = cast(void (*)(void*), array_function_destroy);

	ast->print = cast(void (*)(void*), ast_print);
	ast->destroy = cast(void (*)(void*), ast_destroy);

	return ast;
}

/**
 *
 * @function ast_debug
 * @brief Debug the AST
 * @params {ast_t*} ast - AST
 * @returns {void}
 *
 */
void ast_debug(ast_t* ast)
{
	DEBUG_ME;
	printf("============= START AST DEBUG =============\n");

	printf("AST Layout\n");
	if (ast->layout != NULL) {
		ast->layout->print(ast->layout);
	}
	else {
		printf("NULL\n");
	}

	printf("============= END AST DEBUG =============\n");
}

/**
 *
 * @function ast_destroy
 * @brief Free the AST
 * @params {ast_t*} ast - AST
 * @returns {void}
 *
 */
void ast_destroy(ast_t* ast)
{
	DEBUG_ME;
	printf("ast_destroy\n");

	if (ast != NULL) {
		if (ast->layout != NULL) {
			ast_layout_destroy(ast->layout);
		}

		if (ast->functions != NULL) {
			ast->functions->destroy(ast->functions);
		}

		memory_destroy(ast);
	}
}

/**
 *
 * @function ast_print
 * @brief Print the AST
 * @params {ast_t*} ast - AST
 * @returns {void}
 *
 */
void ast_print(ast_t* ast)
{
	DEBUG_ME;
	printf("AST\n");

	printf("AST Layout: ");
	if (ast->layout != NULL) {
		ast->layout->print(ast->layout);
	}
	else {
		printf("NULL\n");
	}

	printf("AST Functions: ");
	if (ast->functions != NULL) {
		ast->functions->print(ast->functions);
	}
	else {
		printf("NULL\n");
	}
}

/**
 *
 * @function ast_value_destroy
 * @brief Free the AST value
 * @params {ast_value_t*} value - AST Value
 * @returns {void}
 *
 */
void ast_value_destroy(ast_value_t* value)
{
	DEBUG_ME;
	if (value != NULL) {
		if (value->type != NULL) {
			value->type->destroy(value->type);
		}

		if (value->data.string_value != NULL) {
			memory_destroy(value->data.string_value);
		}

		memory_destroy(value);
	}
}

/**
 *
 * @function ast_value_print
 * @brief Print the AST value
 * @params {ast_value_t*} value - AST Value
 * @returns {void}
 *
 */
void ast_value_print(ast_value_t* value)
{
	DEBUG_ME;
	printf("Value\n");

	if (value != NULL) {
		printf("Value Type:\n");

		if (value->type != NULL) {
			value->type->print(value->type);
		}
		else {
			printf("NULL\n");
		}

		printf("Value Data:\n");
		if (value->data.string_value != NULL) {
			printf("%s\n", value->data.string_value);
		}
		else {
			printf("NULL\n");
		}
	}
	else {
		printf("NULL\n");
	}
}

/**
 *
 * @function ast_value_copy
 * @brief Copy the AST layout attribute value
 * @params {ast_value_t*} value - AST Layout Attribute Value
 * @returns {ast_value_t*} - Copied AST Layout Attribute Value
 *
 */
ast_value_t* ast_value_copy(ast_value_t* value)
{
	DEBUG_ME;
	ast_value_type_t* type = ast_value_type_copy(value->type);
	
	ast_value_t* copy = ast_value_create(type, value->data.string_value);

	return copy;
}

/**
 *
 * @function ast_value_type_copy
 * @brief Copy the AST layout attribute value type
 * @params {ast_value_type_t*} type - AST Layout Attribute Value Type
 * @returns {ast_value_type_t*} - Copied AST Layout Attribute Value Type
 *
 */
ast_value_type_t* ast_value_type_copy(ast_value_type_t* type)
{
	DEBUG_ME;
	ast_value_type_t* copy = ast_value_type_create(type->kind, type->location);

	return copy;
}
