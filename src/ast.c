#include "ast.h"

/**
 * 
 * @function ast_node_create
 * @brief Create a new AST node
 * @param {ast_type_t} type - Type of the AST node
 * @param {location_t} location - Location of the AST node
 * @returns {ast_node_t*} - Pointer to the created AST node
 * 
 */
ast_node_t* ast_node_create(ast_type_t type, location_t location)
{
	ast_node_t* node = memory_allocate(sizeof(ast_node_t));
	node->type = type;
	node->location = location;
	node->print = cast(void (*)(void*), ast_node_print);
	node->free = cast(void (*)(void*), ast_node_destroy);

	return node;
}

/**
 * 
 * @function ast_node_destroy
 * @brief Free the AST node
 * @param {ast_node_t*} value - AST node
 * @returns {void}
 * 
 */
void ast_node_destroy(ast_node_t* value)
{
	memory_destroy(value);
}

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
	printf("Attribute: %s\n", value->key);
	array_t* values = value->values;

	values->print(values);
}

/**
 * 
 * @function ast_layout_block_create
 * @brief Create a new AST node layout block
 * @params {ast_type_t} parent_type - Parent token type
 * @returns {ast_layout_block_t*} - Pointer to the created AST node layout block
 * 
 */
ast_layout_block_t* ast_layout_block_create(ast_type_t parent_type)
{
	ast_layout_block_t* block = memory_allocate(sizeof(ast_layout_block_t));

	block->parent_type = parent_type;


	block->attributes = cast(struct hashmap_t*, hashmap_create(16));
	cast(hashmap_t*, block->attributes)->print = cast(void (*)(void*), hashmap_print_layout_attribute);

	block->styles = cast(struct hashmap_t*, hashmap_create(16));
	cast(hashmap_t*, block->styles)->print = cast(void (*)(void*), hashmap_print_layout_attribute);

	block->children = array_create(sizeof(ast_layout_node_t*), 16);

	block->print = cast(void (*)(void*), ast_layout_block_print);
	block->free = cast(void (*)(void*), ast_layout_block_destroy);

	return block;
}

/**
 * 
 * @function ast_layout_attribute_create
 * @brief Create a new AST node layout attribute
 * @params {const char*} key - Key of the attribute
 * @params {array_t*} values - Values of the attribute
 * @returns {ast_layout_attribute_t*} - Pointer to the created AST node layout attribute
 * 
 */
ast_layout_attribute_t* ast_layout_attribute_create(char* key, array_t* values)
{
	ast_layout_attribute_t* attribute = memory_allocate(sizeof(ast_layout_attribute_t));
	// printf("--> inside func - key is %s\n", key);
	attribute->key = strdup(key);
	// printf("--> inside func - key is %s\n", attribute->key);
	attribute->values = values;
	attribute->isStyle = false;

	attribute->print = cast(void (*)(void*), ast_layout_attribute_print);
	attribute->free = cast(void (*)(void*), ast_layout_attribute_destroy);

	return attribute;
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
	if (value->key != NULL) memory_destroy(value->key);
	array_destroy_custom(value->values, cast(void (*)(void*), token_destroy));

	memory_destroy(value);
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
	hashmap_destroy_custom(cast(hashmap_t*, value->attributes), cast(void (*)(void*), ast_layout_attribute_destroy));
	hashmap_destroy_custom(cast(hashmap_t*, value->styles), cast(void (*)(void*), ast_layout_attribute_destroy));
	array_destroy_custom(value->children, cast(void (*)(void*), ast_node_destroy));
	memory_destroy(value);
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
	hashmap_t* attributes = cast(hashmap_t*, value->attributes);
	hashmap_t* styles = cast(hashmap_t*, value->styles);
	array_node_layout_t* children = value->children;
	size_t children_size = array_size(children);

	printf("Block attributes:\n");
	attributes->print(attributes);

	printf("Block styles:\n");
	styles->print(styles);

	printf("Block children:\n");
	for (size_t i = 0; i < children_size; i++) {
		ast_layout_node_t* node = cast(ast_layout_node_t*, array_get(value->children, i));

		node->print(node);
	}
}

/**
 * 
 * @function ast_layout_node_create
 * @brief Create a new AST node layout attribute
 * @params {ast_layout_node_type_t} type - Type of the layout node
 * @returns {ast_layout_node_t*} - Pointer to the created AST node layout attribute
 * 
 */
ast_layout_node_t* ast_layout_node_create(ast_layout_node_type_t type)
{
	ast_layout_node_t* node = memory_allocate(sizeof(ast_layout_node_t));
	node->tag = NULL;
	node->type = type;

	node->print = cast(void (*)(void*), ast_layout_node_print);
	node->free = cast(void (*)(void*), ast_layout_node_destroy);

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
	memory_destroy(value->tag);
	ast_layout_block_destroy(value->block);
	memory_destroy(value);
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
	ast_layout_t* node = memory_allocate(sizeof(ast_layout_t));
	
	node->block = ast_layout_block_create(AST_NODE_TYPE_LAYOUT);
	node->print = cast(void (*)(void*), ast_layout_print);
	node->free = cast(void (*)(void*), ast_layout_destroy);

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
	ast_layout_block_destroy(value->block);
	memory_destroy(value);
}

/**
 * 
 * @function ast_layout_print
 * @brief Print the AST layout
 * @param {ast_layout_t*} value - AST layout
 * @returns {void}
 * 
 */
void ast_layout_print(ast_layout_t* value)
{
	printf("Layout\n");
	value->block->print(value->block);
}

/**
 * 
 * @function ast_node_print
 * @brief Print the AST node
 * @param {ast_node_t*} node - AST node
 * @returns {void}
 * 
 */
void ast_node_print(ast_node_t* node)
{
	switch (node->type) {
		case AST_NODE_TYPE_FUNCTION:
			printf("Function\n");
			break;
		case AST_NODE_TYPE_FUNCTION_ERROR:
			printf("Function Error\n");
			break;
		case AST_NODE_TYPE_BLOCK:
			printf("Block\n");
			break;
		case AST_NODE_TYPE_IMPORT:
			printf("Import\n");
			break;
		case AST_NODE_TYPE_LAYOUT:
			printf("Layout\n");
			break;
		case AST_NODE_TYPE_LAYOUT_BLOCK:
			printf("Layout Block\n");
			break;
		case AST_NODE_TYPE_LAYOUT_NODE:
			printf("Layout Node\n");
			break;
		case AST_NODE_TYPE_ERROR:
			printf("Error\n");
			break;
	}
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
	ast_t* ast = memory_allocate(sizeof(ast_t));
	ast->layout = NULL;
	
	return ast;
}

/**
 * 
 * @function ast_debug
 * @brief Debug the AST
 * @param {ast_t*} ast - AST
 * @returns {void}
 * 
 */
void ast_debug(ast_t* ast)
{
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
 * @param {ast_t*} ast - AST
 * @returns {void}
 * 
 */
void ast_destroy(ast_t* ast)
{
	if (ast->layout != NULL) {
		ast_layout_destroy(ast->layout);
	}

	memory_destroy(ast);
}
