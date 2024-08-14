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
	ast_node_t* node = memory_allocate(sizeof(ast_node_t));
	node->type = type;
	node->location = location;
	node->print = cast(void (*)(void*), ast_node_print);
	node->destroy = cast(void (*)(void*), ast_node_destroy);

	return node;
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
	if (value != NULL) {
		memory_destroy(value);
	}
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
 * @params {ast_type_t} node_type - Node type
 * @params {ast_layout_node_type_t} layout_node_type - Layout node type
 * @returns {ast_layout_block_t*} - Pointer to the created AST node layout block
 * 
 */
ast_layout_block_t* ast_layout_block_create(ast_type_t node_type, ast_layout_node_type_t layout_node_type)
{
	ast_layout_block_t* block = memory_allocate(sizeof(ast_layout_block_t));
	block->type = AST_NODE_BLOCK_TYPE_LAYOUT;
	block->parent_type = node_type;
	block->parent_node_type = layout_node_type;
	block->text_content = NULL;

	block->attributes = cast(struct hashmap_t*, hashmap_create(16));
	cast(hashmap_t*, block->attributes)->print = cast(void (*)(void*), hashmap_print_layout_attribute);
	cast(hashmap_t*, block->attributes)->destroy = cast(void (*)(void*), hashmap_destroy_layout_attribute);

	block->styles = cast(struct hashmap_t*, hashmap_create(16));
	cast(hashmap_t*, block->styles)->print = cast(void (*)(void*), hashmap_print_layout_attribute);
	cast(hashmap_t*, block->styles)->destroy = cast(void (*)(void*), hashmap_destroy_layout_attribute);

	block->children = array_create(sizeof(ast_layout_node_t*), 16);
	block->children->destroy = cast(void (*)(void*), array_layout_node_destroy);
	block->children->print = cast(void (*)(void*), array_layout_node_print);

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
 * @params {array_t*} values - Values of the attribute
 * @returns {ast_layout_attribute_t*} - Pointer to the created AST node layout attribute
 * 
 */
ast_layout_attribute_t* ast_layout_attribute_create(ast_layout_attribute_type_t type, char* key, array_t* values)
{
	ast_layout_attribute_t* attribute = memory_allocate(sizeof(ast_layout_attribute_t));
	attribute->type = type;
	attribute->key = strdup(key);
	attribute->values = values;
	attribute->isStyle = false;
	attribute->isContent = false;
	attribute->ignoreMe = false;

	attribute->print = cast(void (*)(void*), ast_layout_attribute_print);
	attribute->destroy = cast(void (*)(void*), ast_layout_attribute_destroy);

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
	if (value != NULL) {
		if (value->key != NULL) {
			memory_destroy(value->key);
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
	printf("ast_layout_block_destroy\n");
	if (value != NULL) {
		hashmap_t* attributes = cast(hashmap_t*, value->attributes);
		hashmap_t* styles = cast(hashmap_t*, value->styles);
		array_t* children = value->children;
		
		if (attributes != NULL) {
			attributes->destroy(attributes);
		}

		if (styles != NULL) {
			styles->destroy(styles);
		}

		if (value->text_content != NULL) {
			memory_destroy(value->text_content);
		}

		printf("ast_layout_block_destroy->children\n");
		if (children != NULL) {
			children->destroy(children);
		}
		printf("ast_layout_block_destroy->children (after)\n");

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
	ast_layout_node_t* node = memory_allocate(sizeof(ast_layout_node_t));
	node->tag = NULL;
	node->type = layout_node_type;
	node->block = ast_layout_block_create(AST_NODE_TYPE_LAYOUT, layout_node_type);
	
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
	if (value != NULL) {
		printf("ast_layout_node_destroy\n");
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
 * @function ast_block_create
 * @brief Create a new AST block node
 * @params {ast_block_type_t} type - Block type
 * @params {ast_type_t} parent_type - Parent type
 * @returns {ast_block_t*} - AST block node
 * 
 */
ast_block_t* ast_block_create(ast_block_type_t type, ast_type_t parent_type)
{
	ast_block_t* block = malloc(sizeof(ast_block_t));
	block->type = type;
	block->parent_type = parent_type;
	block->children = array_create(sizeof(ast_node_t*), 4);

	return block;
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
	ast_function_t* node = memory_allocate(sizeof(ast_function_t));
	node->name = name;

	node->return_type = ast_type_create(AST_TYPE_KIND_VOID);

	node->parameters = array_create(sizeof(ast_function_parameter_t*), 16);
	node->parameters->destroy = cast(void (*)(void*), array_function_parameter_destroy);
	node->parameters->print = cast(void (*)(void*), array_function_parameter_print);

	node->block = ast_block_create(AST_NODE_BLOCK_TYPE_FUNCTION, AST_NODE_TYPE_FUNCTION);

	node->print = cast(void (*)(void*), ast_function_print);
	node->destroy = cast(void (*)(void*), ast_function_destroy);

	return node;
}

/**
 * 
 * @function ast_type_name
 * @brief Get the name of the AST value type
 * @params {ast_value_type_t*} type - AST value type
 * @returns {char*} - Name of the AST value type
 * 
 */
char* ast_type_name(ast_value_type_t* type)
{
	switch (type->kind) {
		case AST_TYPE_KIND_VOID:
			return "void";

		case AST_TYPE_KIND_INT:
			return "int";

		case AST_TYPE_KIND_FLOAT:
			return "float";

		case AST_TYPE_KIND_CHAR:
			return "char";

		case AST_TYPE_KIND_STRING:
			return "string";

		case AST_TYPE_KIND_BOOL:
			return "bool";

		case AST_TYPE_KIND_STRUCT:
			return "struct";
		
		case AST_TYPE_KIND_ENUM:
			return "enum";
		
		case AST_TYPE_KIND_ARRAY:
			return "array";
		
		case AST_TYPE_KIND_POINTER:
			return "pointer";
		
		case AST_TYPE_KIND_FUNCTION:
			return "function";
	}

	return "unknown";
}

/**
 * 
 * @function ast_type_print
 * @brief Print the AST value type
 * @params {ast_value_type_t*} type - AST value type
 * @returns {void}
 * 
 */
void ast_type_print(ast_value_type_t* type)
{
	printf("Type: ");

	switch (type->kind) {
		case AST_TYPE_KIND_VOID:
			printf("void");
			printf("\n");
			return;

		case AST_TYPE_KIND_INT:
			printf("int");
			printf("\n");
			return;

		case AST_TYPE_KIND_FLOAT:
			printf("float");
			printf("\n");
			return;

		case AST_TYPE_KIND_CHAR:
			printf("char");
			printf("\n");
			return;

		case AST_TYPE_KIND_STRING:
			printf("string");
			printf("\n");
			return;

		case AST_TYPE_KIND_BOOL:
			printf("bool");
			printf("\n");
			return;
		
		case AST_TYPE_KIND_STRUCT:
			printf("struct");
			printf("\n");
			return;
		
		case AST_TYPE_KIND_ENUM:
			printf("enum");
			printf("\n");
			return;
		
		case AST_TYPE_KIND_ARRAY:
			printf("array");
			printf("\n");
			return;
		
		case AST_TYPE_KIND_POINTER:
			printf("pointer");
			printf("\n");
			return;
		
		case AST_TYPE_KIND_FUNCTION:
			printf("function");
			printf("\n");
			return;
	}
	
	printf("unknown type");
	printf("\n");
}

/**
 * 
 * @function ast_type_create
 * @brief Create a new AST value type
 * @params {ast_value_kind_t} kind - Kind of the value type
 * @returns {ast_value_type_t*} - Pointer to the created AST value type
 * 
 */
ast_value_type_t* ast_type_create(ast_value_kind_t kind)
{
	ast_value_type_t* type = memory_allocate(sizeof(ast_value_type_t));
	type->kind = kind;

	type->print = cast(void (*)(void*), ast_type_print);
	type->destroy = cast(void (*)(void*), ast_type_destroy);

	return type;	
}

/**
 * 
 * @function ast_type_destroy
 * @brief Free the AST value type
 * @params {ast_value_type_t*} type - AST value type
 * @returns {void}
 * 
 */
void ast_type_destroy(ast_value_type_t* type)
{
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
	printf("Function: %s\n", value->name);

	array_t* parameters = value->parameters;
	size_t parameters_capacity = array_length(parameters);

	printf("Return Type: ");
	value->return_type->print(value->return_type);
	printf("\n");

	printf("Parameters: %zu\n", parameters_capacity);
	for (size_t i = 0; i < parameters_capacity; i++) {
		ast_function_parameter_t* parameter = cast(ast_function_parameter_t*, array_get(parameters, i));

		parameter->print(parameter);
	}

	value->block->print(value->block);
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
	if (value != NULL) {
		if (value->name != NULL) {
			memory_destroy(value->name);
		}

		if (value->parameters != NULL) {
			value->parameters->destroy(value->parameters);
		}

		if (value->block != NULL) {
			value->block->destroy(value->block);
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
	ast_layout_t* node = memory_allocate(sizeof(ast_layout_t));

	node->block = ast_layout_block_create(AST_NODE_TYPE_LAYOUT, AST_LAYOUT_NODE_TYPE_NONE);
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
	printf("ast_layout_destroy\n");
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
	printf("Layout\n");
	value->block->print(value->block);
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
	switch (node->type) {
		case AST_NODE_TYPE_FUNCTION:
			printf("Function\n");
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
 * @params {ast_t*} ast - AST
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
 * @params {ast_t*} ast - AST
 * @returns {void}
 * 
 */
void ast_destroy(ast_t* ast)
{
	printf("ast_destroy\n");
	if (ast != NULL) {
		ast->layout->destroy(ast->layout);

		memory_destroy(ast);
	}
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
	ast_layout_node_type_t type = AST_LAYOUT_NODE_TYPE_ERROR;

	if (strcmp(name, "paragraph") == 0) {
		type = AST_LAYOUT_NODE_TYPE_PARAGRAPH;
	}
	else if (strcmp(name, "paragraph_raw") == 0) {
		type = AST_LAYOUT_NODE_TYPE_PARAGRAPH_RAW;
	}
	else if (strcmp(name, "button") == 0) {
		type = AST_LAYOUT_NODE_TYPE_BUTTON;
	}
	else if (strcmp(name, "input") == 0) {
		type = AST_LAYOUT_NODE_TYPE_INPUT;
	}
	else if (strcmp(name, "textarea") == 0) {
		type = AST_LAYOUT_NODE_TYPE_TEXTAREA;
	}
	else if (strcmp(name, "span") == 0) {
		type = AST_LAYOUT_NODE_TYPE_SPAN;
	}
	else if (strcmp(name, "label") == 0) {
		type = AST_LAYOUT_NODE_TYPE_LABEL;
	}
	else if (strcmp(name, "header") == 0) {
		type = AST_LAYOUT_NODE_TYPE_HEADER;
	}
	else if (strcmp(name, "ul") == 0) {
		type = AST_LAYOUT_NODE_TYPE_UL;
	}
	else if (strcmp(name, "ol") == 0) {
		type = AST_LAYOUT_NODE_TYPE_OL;
	}
	else if (strcmp(name, "li") == 0) {
		type = AST_LAYOUT_NODE_TYPE_LI;
	}
	else if (strcmp(name, "link") == 0) {
		type = AST_LAYOUT_NODE_TYPE_LINK;
	}
	else if (strcmp(name, "img") == 0) {
		type = AST_LAYOUT_NODE_TYPE_IMG;
	}
	else if (strcmp(name, "table") == 0) {
		type = AST_LAYOUT_NODE_TYPE_TABLE;
	}
	else if (strcmp(name, "tr") == 0) {
		type = AST_LAYOUT_NODE_TYPE_TABLE_TR;
	}
	else if (strcmp(name, "td") == 0) {
		type = AST_LAYOUT_NODE_TYPE_TABLE_TD;
	}
	else if (strcmp(name, "video") == 0) {
		type = AST_LAYOUT_NODE_TYPE_VIDEO;
	}
	else if (strcmp(name, "audio") == 0) {
		type = AST_LAYOUT_NODE_TYPE_AUDIO;
	}
	else if (strcmp(name, "form") == 0) {
		type = AST_LAYOUT_NODE_TYPE_FORM;
	}
	else if (strcmp(name, "box") == 0) {
		type = AST_LAYOUT_NODE_TYPE_DIV;
	}

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
	if (token->type != TOKEN_IDENTIFIER) {
		error(2, "Expected token type to be identifier as layout node type, got %s at line %d, column %d", token_name(token->type), token->location.end_line, token->location.end_column);
	}
	
	ast_layout_node_type_t type = name_to_ast_layout_node_type(token->data.string);

	if (type == AST_LAYOUT_NODE_TYPE_ERROR) {
		error(2, "Unknown layout node '%s' at line %d, column %d", token->data.string, token->location.end_line, token->location.end_column);
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
	switch (type) {
		case AST_LAYOUT_NODE_TYPE_PARAGRAPH: return "paragraph";
		case AST_LAYOUT_NODE_TYPE_PARAGRAPH_RAW: return "paragraph_raw";
		case AST_LAYOUT_NODE_TYPE_BUTTON: return "button";
		case AST_LAYOUT_NODE_TYPE_INPUT: return "input";
		case AST_LAYOUT_NODE_TYPE_TEXTAREA: return "textarea";
		case AST_LAYOUT_NODE_TYPE_SPAN: return "span";
		case AST_LAYOUT_NODE_TYPE_LABEL: return "label";
		case AST_LAYOUT_NODE_TYPE_HEADER: return "header";
		case AST_LAYOUT_NODE_TYPE_FOOTER: return "footer";
		case AST_LAYOUT_NODE_TYPE_NAV: return "nav";
		case AST_LAYOUT_NODE_TYPE_MAIN: return "main";
		case AST_LAYOUT_NODE_TYPE_SECTION: return "section";
		case AST_LAYOUT_NODE_TYPE_ARTICLE: return "article";
		case AST_LAYOUT_NODE_TYPE_ASIDE: return "aside";
		case AST_LAYOUT_NODE_TYPE_SELECT: return "select";
		case AST_LAYOUT_NODE_TYPE_OPTION: return "option";
		case AST_LAYOUT_NODE_TYPE_IFRAME: return "iframe";
		case AST_LAYOUT_NODE_TYPE_CANVAS: return "canvas";
		case AST_LAYOUT_NODE_TYPE_BLOCKQUOTE: return "blockquote";
		case AST_LAYOUT_NODE_TYPE_PRE: return "pre";
		case AST_LAYOUT_NODE_TYPE_CODE: return "code";
		case AST_LAYOUT_NODE_TYPE_BR: return "br";
		case AST_LAYOUT_NODE_TYPE_HR: return "hr";
		case AST_LAYOUT_NODE_TYPE_STRONG: return "strong";
		case AST_LAYOUT_NODE_TYPE_EM: return "em";
		case AST_LAYOUT_NODE_TYPE_ITALIC: return "italic";
		case AST_LAYOUT_NODE_TYPE_BOLD: return "bold";
		case AST_LAYOUT_NODE_TYPE_UNDERLINE: return "underline";
		case AST_LAYOUT_NODE_TYPE_S: return "s";
		case AST_LAYOUT_NODE_TYPE_SMALL: return "small";
		case AST_LAYOUT_NODE_TYPE_BIG: return "big";
		case AST_LAYOUT_NODE_TYPE_SUB: return "sub";
		case AST_LAYOUT_NODE_TYPE_SUP: return "sup";
		case AST_LAYOUT_NODE_TYPE_CENTER: return "center";
		case AST_LAYOUT_NODE_TYPE_DEL: return "del";
		case AST_LAYOUT_NODE_TYPE_INS: return "ins";
		case AST_LAYOUT_NODE_TYPE_MARK: return "mark";
		case AST_LAYOUT_NODE_TYPE_Q: return "q";
		case AST_LAYOUT_NODE_TYPE_CITE: return "cite";
		case AST_LAYOUT_NODE_TYPE_DFN: return "dfn";
		case AST_LAYOUT_NODE_TYPE_ADDRESS: return "address";
		case AST_LAYOUT_NODE_TYPE_TIME: return "time";
		case AST_LAYOUT_NODE_TYPE_PROGRESS: return "progress";
		case AST_LAYOUT_NODE_TYPE_METER: return "meter";
		case AST_LAYOUT_NODE_TYPE_DETAILS: return "details";
		case AST_LAYOUT_NODE_TYPE_SUMMARY: return "summary";
		case AST_LAYOUT_NODE_TYPE_DIALOG: return "dialog";
		case AST_LAYOUT_NODE_TYPE_MENU: return "menu";
		case AST_LAYOUT_NODE_TYPE_MENUITEM: return "menuitem";
		case AST_LAYOUT_NODE_TYPE_COMMAND: return "command";
		case AST_LAYOUT_NODE_TYPE_LEGEND: return "legent";
		case AST_LAYOUT_NODE_TYPE_FIELDSET: return "fieldset";
		case AST_LAYOUT_NODE_TYPE_CAPTION: return "caption";
		case AST_LAYOUT_NODE_TYPE_COL: return "col";
		case AST_LAYOUT_NODE_TYPE_COLGROUP: return "colgroup";
		case AST_LAYOUT_NODE_TYPE_TABLE_HEADER: return "thead";
		case AST_LAYOUT_NODE_TYPE_TABLE_BODY: return "tbody";
		case AST_LAYOUT_NODE_TYPE_TABLE_FOOTER: return "tfoot";
		case AST_LAYOUT_NODE_TYPE_UL: return "ul";
		case AST_LAYOUT_NODE_TYPE_OL: return "ol";
		case AST_LAYOUT_NODE_TYPE_LI: return "li";
		case AST_LAYOUT_NODE_TYPE_LINK: return "a";
		case AST_LAYOUT_NODE_TYPE_IMG: return "img";
		case AST_LAYOUT_NODE_TYPE_TABLE: return "table";
		case AST_LAYOUT_NODE_TYPE_TABLE_TR: return "tr";
		case AST_LAYOUT_NODE_TYPE_TABLE_TD: return "td";
		case AST_LAYOUT_NODE_TYPE_TABLE_TH: return "th";
		case AST_LAYOUT_NODE_TYPE_VIDEO: return "video";
		case AST_LAYOUT_NODE_TYPE_AUDIO: return "audio";
		case AST_LAYOUT_NODE_TYPE_FORM: return "form";
		case AST_LAYOUT_NODE_TYPE_DIV: return "div";
		case AST_LAYOUT_NODE_TYPE_SCRIPT: return "script";
		case AST_LAYOUT_NODE_TYPE_STYLE: return "style";
		
		case AST_LAYOUT_NODE_TYPE_NONE: return "layout";
		case AST_LAYOUT_NODE_TYPE_ERROR: return "error";
	}

	return "error!!!";
}

/**
 * 
 * @function name_to_ast_layout_node_type
 * @brief Convert name to AST layout node type
 * @params {char*} name - Name
 * @returns {ast_layout_attribute_type_t} type - Layout Attribute Type
 * 
 */
ast_layout_attribute_type_t name_to_ast_layout_attribute_type(char* name)
{
	ast_layout_attribute_type_t type = AST_LAYOUT_ATTRIBUTE_TYPE_ERROR;

	if (strcmp(name, "class") == 0) {
		type = AST_LAYOUT_ATTRIBUTE_TYPE_CLASS;
	}
	else if (strcmp(name, "id") == 0) {
		type = AST_LAYOUT_ATTRIBUTE_TYPE_ID;
	}
	else if (strcmp(name, "content") == 0) {
		type = AST_LAYOUT_ATTRIBUTE_TYPE_CONTENT;
	}
	else if (strcmp(name, "src") == 0) {
		type = AST_LAYOUT_ATTRIBUTE_TYPE_SRC;
	}
	else if (strcmp(name, "dir") == 0) {
		type = AST_LAYOUT_ATTRIBUTE_TYPE_DIR;
	}
	else if (strcmp(name, "lang") == 0) {
		type = AST_LAYOUT_ATTRIBUTE_TYPE_LANG;
	}
	else if (strcmp(name, "title") == 0) {
		type = AST_LAYOUT_ATTRIBUTE_TYPE_TITLE;
	}
	else if (strcmp(name, "alt") == 0) {
		type = AST_LAYOUT_ATTRIBUTE_TYPE_ALT;
	}
	else if (strcmp(name, "author") == 0) {
		type = AST_LAYOUT_ATTRIBUTE_TYPE_AUTHOR;
	}
	else if (strcmp(name, "description") == 0) {
		type = AST_LAYOUT_ATTRIBUTE_TYPE_DESCRIPTION;
	}
	else if (strcmp(name, "keywords") == 0) {
		type = AST_LAYOUT_ATTRIBUTE_TYPE_KEYWORDS;
	}
	else if (strcmp(name, "icon") == 0) {
		type = AST_LAYOUT_ATTRIBUTE_TYPE_ICON;
	}
	else if (strcmp(name, "name") == 0) {
		type = AST_LAYOUT_ATTRIBUTE_TYPE_NAME;
	}
	else if (strcmp(name, "charset") == 0) {
		type = AST_LAYOUT_ATTRIBUTE_TYPE_CHARSET;
	}
	else if (strcmp(name, "type") == 0) {
		type = AST_LAYOUT_ATTRIBUTE_TYPE_TYPE;
	}

	// css attributes
	else if (strcmp(name, "background") == 0) {
		type = AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BACKGROUND;
	}
	else if (strcmp(name, "color") == 0) {
		type = AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_COLOR;
	}
	else if (strcmp(name, "font") == 0) {
		type = AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FONT;
	}
	else if (strcmp(name, "font-family") == 0) {
		type = AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FONT_FAMILY;
	}
	else if (strcmp(name, "font-size") == 0) {
		type = AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FONT_SIZE;
	}
	else if (strcmp(name, "font-style") == 0) {
		type = AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FONT_STYLE;
	}
	else if (strcmp(name, "font-weight") == 0) {
		type = AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FONT_WEIGHT;
	}
	else if (strcmp(name, "height") == 0) {
		type = AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_HEIGHT;
	}
	else if (strcmp(name, "width") == 0) {
		type = AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_WIDTH;
	}
	else if (strcmp(name, "margin") == 0) {
		type = AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_MARGIN;
	}
	else if (strcmp(name, "margin-left") == 0) {
		type = AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_MARGIN_LEFT;
	}
	else if (strcmp(name, "margin-right") == 0) {
		type = AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_MARGIN_RIGHT;
	}
	else if (strcmp(name, "margin-top") == 0) {
		type = AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_MARGIN_TOP;
	}
	else if (strcmp(name, "margin-bottom") == 0) {
		type = AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_MARGIN_BOTTOM;
	}
	else if (strcmp(name, "padding") == 0) {
		type = AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_PADDING;
	}
	else if (strcmp(name, "padding-left") == 0) {
		type = AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_PADDING_LEFT;
	}
	else if (strcmp(name, "padding-right") == 0) {
		type = AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_PADDING_RIGHT;
	}
	else if (strcmp(name, "padding-top") == 0) {
		type = AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_PADDING_TOP;
	}
	else if (strcmp(name, "padding-bottom") == 0) {
		type = AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_PADDING_BOTTOM;
	}
	else if (strcmp(name, "text-align") == 0) {
		type = AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_TEXT_ALIGN;
	}
	else if (strcmp(name, "text-decoration") == 0) {
		type = AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_TEXT_DECORATION;
	}
	else if (strcmp(name, "text-transform") == 0) {
		type = AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_TEXT_TRANSFORM;
	}
	else if (strcmp(name, "vertical-align") == 0) {
		type = AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_VERTICAL_ALIGN;
	}
	else if (strcmp(name, "visibility") == 0) {
		type = AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_VISIBILITY;
	}
	else if (strcmp(name, "z-index") == 0) {
		type = AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_Z_INDEX;
	}
	else if (strcmp(name, "position") == 0) {
		type = AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_POSITION;
	}
	else if (strcmp(name, "top") == 0) {
		type = AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_TOP;
	}
	else if (strcmp(name, "right") == 0) {
		type = AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_RIGHT;
	}
	else if (strcmp(name, "bottom") == 0) {
		type = AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BOTTOM;
	}
	else if (strcmp(name, "left") == 0) {
		type = AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_LEFT;
	}
	else if (strcmp(name, "border") == 0) {
		type = AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BORDER;
	}
	else if (strcmp(name, "border-color") == 0) {
		type = AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BORDER_COLOR;
	}
	else if (strcmp(name, "border-style") == 0) {
		type = AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BORDER_STYLE;
	}
	else if (strcmp(name, "border-width") == 0) {
		type = AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BORDER_WIDTH;
	}
	else if (strcmp(name, "border-radius") == 0) {
		type = AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BORDER_RADIUS;
	}
	else if (strcmp(name, "float") == 0) {
		type = AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FLOAT;
	}
	else if (strcmp(name, "clear") == 0) {
		type = AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_CLEAR;
	}
	else if (strcmp(name, "overflow") == 0) {
		type = AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_OVERFLOW;
	}
	else if (strcmp(name, "text-overflow") == 0) {
		type = AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_TEXT_OVERFLOW;
	}
	else if (strcmp(name, "white-space") == 0) {
		type = AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_WHITE_SPACE;
	}
	else if (strcmp(name, "cursor") == 0) {
		type = AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_CURSOR;
	}
	else if (strcmp(name, "filter") == 0) {
		type = AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FILTER;
	}
	else if (strcmp(name, "opacity") == 0) {
		type = AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_OPACITY;
	}
	else if (strcmp(name, "transform") == 0) {
		type = AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_TRANSFORM;
	}
	else if (strcmp(name, "transition") == 0) {
		type = AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_TRANSITION;
	}
	else if (strcmp(name, "animation") == 0) {
		type = AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_ANIMATION;
	}
	else if (strcmp(name, "box-shadow") == 0) {
		type = AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BOX_SHADOW;
	}
	else if (strcmp(name, "text-shadow") == 0) {
		type = AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_TEXT_SHADOW;
	}
	else if (strcmp(name, "outline") == 0) {
		type = AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_OUTLINE;
	}
	else if (strcmp(name, "outline-color") == 0) {
		type = AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_OUTLINE_COLOR;
	}
	else if (strcmp(name, "outline-style") == 0) {
		type = AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_OUTLINE_STYLE;
	}
	else if (strcmp(name, "outline-width") == 0) {
		type = AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_OUTLINE_WIDTH;
	}
	else if (strcmp(name, "background-color") == 0) {
		type = AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BACKGROUND_COLOR;
	}
	else if (strcmp(name, "background-image") == 0) {
		type = AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BACKGROUND_IMAGE;
	}
	else if (strcmp(name, "background-repeat") == 0) {
		type = AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BACKGROUND_REPEAT;
	}
	else if (strcmp(name, "background-position") == 0) {
		type = AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BACKGROUND_POSITION;
	}
	else if (strcmp(name, "background-size") == 0) {
		type = AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BACKGROUND_SIZE;
	}
	else if (strcmp(name, "background-attachment") == 0) {
		type = AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BACKGROUND_ATTACHMENT;
	}
	else if (strcmp(name, "background-clip") == 0) {
		type = AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BACKGROUND_CLIP;
	}
	else if (strcmp(name, "background-origin") == 0) {
		type = AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BACKGROUND_ORIGIN;
	}
	else if (strcmp(name, "background-blend-mode") == 0) {
		type = AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BACKGROUND_BLEND_MODE;
	}
	else if (strcmp(name, "display") == 0) {
		type = AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_DISPLAY;
	}
	else if (strcmp(name, "flex") == 0) {
		type = AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FLEX;
	}
	else if (strcmp(name, "flex-direction") == 0) {
		type = AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FLEX_DIRECTION;
	}
	else if (strcmp(name, "flex-wrap") == 0) {
		type = AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FLEX_WRAP;
	}
	else if (strcmp(name, "flex-flow") == 0) {
		type = AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FLEX_FLOW;
	}
	else if (strcmp(name, "flex-grow") == 0) {
		type = AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FLEX_GROW;
	}
	else if (strcmp(name, "flex-shrink") == 0) {
		type = AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FLEX_SHRINK;
	}
	else if (strcmp(name, "flex-basis") == 0) {
		type = AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FLEX_BASIS;
	}
	else if (strcmp(name, "justify-content") == 0) {
		type = AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_JUSTIFY_CONTENT;
	}
	else if (strcmp(name, "align-items") == 0) {
		type = AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_ALIGN_ITEMS;
	}
	else if (strcmp(name, "align-self") == 0) {
		type = AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_ALIGN_SELF;
	}
	else if (strcmp(name, "align-content") == 0) {
		type = AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_ALIGN_CONTENT;
	}
	else if (strcmp(name, "order") == 0) {
		type = AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_ORDER;
	}
	else if (strcmp(name, "grid") == 0) {
		type = AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_GRID;
	}
	else if (strcmp(name, "fill") == 0) {
		type = AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FILL;
	}
	else if (strcmp(name, "stroke") == 0) {
		type = AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_STROKE;
	}
	else if (strcmp(name, "stroke-width") == 0) {
		type = AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_STROKE_WIDTH;
	}
	else if (strcmp(name, "stroke-linecap") == 0) {
		type = AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_STROKE_LINECAP;
	}
	else if (strcmp(name, "stroke-linejoin") == 0) {
		type = AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_STROKE_LINEJOIN;
	}
	else if (strcmp(name, "stroke-dasharray") == 0) {
		type = AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_STROKE_DASHARRAY;
	}
	else if (strcmp(name, "stroke-dashoffset") == 0) {
		type = AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_STROKE_DASHOFFSET;
	}
	else if (strcmp(name, "fill-opacity") == 0) {
		type = AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FILL_OPACITY;
	}
	else if (strcmp(name, "stroke-opacity") == 0) {
		type = AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_STROKE_OPACITY;
	}
	else if (strcmp(name, "shape-rendering") == 0) {
		type = AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_SHAPE_RENDERING;
	}
	else if (strcmp(name, "text-rendering") == 0) {
		type = AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_TEXT_RENDERING;
	}
	else if (strcmp(name, "image-rendering") == 0) {
		type = AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_IMAGE_RENDERING;
	}
	else if (strcmp(name, "color-interpolation") == 0) {
		type = AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_COLOR_INTERPOLATION;
	}
	else if (strcmp(name, "color-rendering") == 0) {
		type = AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_COLOR_RENDERING;
	}
	else if (strcmp(name, "vector-effect") == 0) {
		type = AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_VECTOR_EFFECT;
	}
	else if (strcmp(name, "mask") == 0) {
		type = AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_MASK;
	}
	else if (strcmp(name, "clip-path") == 0) {
		type = AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_CLIP_PATH;
	}

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
	char* layout_element_name = ast_layout_node_type_to_name(parent_node_type);
		
	ast_layout_attribute_type_t type = name_to_ast_layout_attribute_type(name);

	if (type == AST_LAYOUT_ATTRIBUTE_TYPE_ERROR) {
		error(2, "Unknown layout attribute '%s' at line %d, column %d for '%s' element", name, token->location.end_line, token->location.end_column, layout_element_name);
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
	switch (type) {
		case AST_LAYOUT_ATTRIBUTE_TYPE_CLASS:
			return "class";
		case AST_LAYOUT_ATTRIBUTE_TYPE_ID:
			return "id";
		case AST_LAYOUT_ATTRIBUTE_TYPE_CONTENT:
			return "content";
		case AST_LAYOUT_ATTRIBUTE_TYPE_SRC:
			return "src";
		case AST_LAYOUT_ATTRIBUTE_TYPE_DIR:
			return "dir";
		case AST_LAYOUT_ATTRIBUTE_TYPE_LANG:
			return "lang";
		case AST_LAYOUT_ATTRIBUTE_TYPE_TITLE:
			return "title";
		case AST_LAYOUT_ATTRIBUTE_TYPE_ALT:
			return "alt";
		case AST_LAYOUT_ATTRIBUTE_TYPE_AUTHOR:
			return "author";
		case AST_LAYOUT_ATTRIBUTE_TYPE_DESCRIPTION:
			return "description";
		case AST_LAYOUT_ATTRIBUTE_TYPE_VIEWPORT:
			return "viewport";
		case AST_LAYOUT_ATTRIBUTE_TYPE_REFRESH:
			return "refresh";
		case AST_LAYOUT_ATTRIBUTE_TYPE_KEYWORDS:
			return "keywords";
		case AST_LAYOUT_ATTRIBUTE_TYPE_ICON:
			return "icon";
		case AST_LAYOUT_ATTRIBUTE_TYPE_NAME:
			return "name";
		case AST_LAYOUT_ATTRIBUTE_TYPE_CHARSET:
			return "charset";
		case AST_LAYOUT_ATTRIBUTE_TYPE_TYPE:
			return  "kind";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BACKGROUND:
			return  "background";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_COLOR:
			return  "color";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FONT:
			return  "font";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FONT_FAMILY:
			return  "font-family";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FONT_SIZE:
			return  "font-size";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FONT_STYLE:
			return  "font-style";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FONT_WEIGHT:
			return  "font-weight";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_HEIGHT:
			return  "height";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_WIDTH:
			return  "width";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_MARGIN:
			return  "margin";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_MARGIN_LEFT:
			return  "margin-left";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_MARGIN_RIGHT:
			return  "margin-right";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_MARGIN_TOP:
			return  "margin-top";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_MARGIN_BOTTOM:
			return  "margin-bottom";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_PADDING:
			return  "padding";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_PADDING_LEFT:
			return  "padding-left";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_PADDING_RIGHT:
			return  "padding-right";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_PADDING_TOP:
			return  "padding-top";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_PADDING_BOTTOM:
			return  "padding-bottom";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_TEXT_ALIGN:
			return  "text-align";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_TEXT_DECORATION:
			return  "text-decoration";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_TEXT_TRANSFORM:
			return  "text-transform";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_VERTICAL_ALIGN:
			return  "vertical-align";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_VISIBILITY:
			return  "visibility";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_Z_INDEX:
			return  "z-index";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_POSITION:
			return  "position";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_TOP:
			return  "top";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_RIGHT:
			return  "right";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BOTTOM:
			return  "bottom";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_LEFT:
			return  "left";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BORDER:
			return  "border";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BORDER_COLOR:
			return  "border-color";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BORDER_STYLE:
			return  "border-style";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BORDER_WIDTH:
			return  "border-width";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BORDER_RADIUS:
			return  "border-radius";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FLOAT:
			return  "float";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_CLEAR:
			return  "clear";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_OVERFLOW:
			return  "overflow";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_TEXT_OVERFLOW:
			return  "text-overflow";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_WHITE_SPACE:
			return  "white-space";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_CURSOR:
			return  "cursor";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FILTER:
			return  "filter";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_OPACITY:
			return  "opacity";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_TRANSFORM:
			return  "transform";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_TRANSITION:
			return  "transition";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_ANIMATION:
			return  "animation";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BOX_SHADOW:
			return  "box-shadow";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_TEXT_SHADOW:
			return  "text-shadow";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_OUTLINE:
			return  "outline";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_OUTLINE_COLOR:
			return  "outline-color";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_OUTLINE_STYLE:
			return  "outline-style";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_OUTLINE_WIDTH:
			return  "outline-width";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BACKGROUND_COLOR:
			return  "background-color";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BACKGROUND_IMAGE:
			return  "background-image";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BACKGROUND_REPEAT:
			return  "background-repeat";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BACKGROUND_POSITION:
			return  "background-position";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BACKGROUND_SIZE:
			return  "background-size";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BACKGROUND_ATTACHMENT:
			return  "background-attachment";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BACKGROUND_CLIP:
			return  "background-clip";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BACKGROUND_ORIGIN:
			return  "background-origin";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BACKGROUND_BLEND_MODE:
			return  "background-blend-mode";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_DISPLAY:
			return  "display";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FLEX:
			return  "flex";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FLEX_DIRECTION:
			return  "flex-direction";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FLEX_WRAP:
			return  "flex-wrap";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FLEX_FLOW:
			return  "flex-flow";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FLEX_GROW:
			return  "flex-grow";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FLEX_SHRINK:
			return  "flex-shrink";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FLEX_BASIS:
			return  "flex-basis";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_JUSTIFY_CONTENT:
			return  "justify-content";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_ALIGN_ITEMS:
			return  "align-items";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_ALIGN_SELF:
			return  "align-self";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_ALIGN_CONTENT:
			return  "align-content";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_ORDER:
			return  "order";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_GRID:
			return  "grid";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FILL:
			return  "fill";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_STROKE:
			return  "stroke";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_STROKE_WIDTH:
			return  "stroke-width";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_STROKE_LINECAP:
			return  "stroke-linecap";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_STROKE_LINEJOIN:
			return  "stroke-linejoin";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_STROKE_DASHARRAY:
			return  "stroke-dasharray";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_STROKE_DASHOFFSET:
			return  "stroke-dashoffset";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FILL_OPACITY:
			return  "fill-opacity";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_STROKE_OPACITY:
			return  "stroke-opacity";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_SHAPE_RENDERING:
			return  "shape-rendering";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_TEXT_RENDERING:
			return  "text-rendering";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_IMAGE_RENDERING:
			return  "image-rendering";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_COLOR_INTERPOLATION:
			return  "color-interpolation";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_COLOR_RENDERING:
			return  "color-rendering";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_VECTOR_EFFECT:
			return  "vector-effect";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_MASK:
			return  "mask";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_CLIP_PATH:
			return  "clip-path";
		
		case AST_LAYOUT_ATTRIBUTE_TYPE_ERROR:
			return "error";
	}

	return "error???";
}
