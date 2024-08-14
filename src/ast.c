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
 * @params {ast_layout_block_t*} parent_block - Parent block
 * @returns {ast_layout_node_t*} - Pointer to the created AST node layout attribute
 * 
 */
ast_layout_node_t* ast_layout_node_create(ast_layout_node_type_t layout_node_type, ast_layout_block_t* parent_block)
{
	ast_layout_node_t* node = memory_allocate(sizeof(ast_layout_node_t));
	node->tag = NULL;
	node->type = layout_node_type;
	node->block = ast_layout_block_create(parent_block->parent_type, parent_block->parent_node_type);
	
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
	else if (strcmp(name, "style") == 0) {
		type = AST_LAYOUT_ATTRIBUTE_TYPE_STYLE;
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

	return type;
}

/**
 * 
 * @function token_to_ast_layout_attribute_type
 * @brief Convert token to AST layout attribute type
 * @params {token_t*} token - Token
 * @params {ast_layout_node_type_t} parent_node_type - Parent node type
 * @returns {ast_layout_node_type_t} type - Layout Node Type
 * 
 */
ast_layout_attribute_type_t token_to_ast_layout_attribute_type(token_t* token, ast_layout_node_type_t parent_node_type)
{
	char* layout_element_name = ast_layout_node_type_to_name(parent_node_type);

	if (token->type != TOKEN_IDENTIFIER) {
		error(2, "Expected token type to be identifier as layout attribute type, got %s at line %d, column %d", token_name(token->type), token->location.end_line, token->location.end_column);
	}
	
	ast_layout_attribute_type_t type = name_to_ast_layout_attribute_type(token->data.string);

	if (type == AST_LAYOUT_ATTRIBUTE_TYPE_ERROR) {
		error(2, "Unknown layout attribute '%s' at line %d, column %d for '%s' element", token->data.string, token->location.end_line, token->location.end_column, layout_element_name);
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
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE:
			return "style";
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
		case AST_LAYOUT_ATTRIBUTE_TYPE_KEYWORDS:
			return "keywords";
		case AST_LAYOUT_ATTRIBUTE_TYPE_ICON:
			return "icon";
		case AST_LAYOUT_ATTRIBUTE_TYPE_NAME:
			return "name";
		case AST_LAYOUT_ATTRIBUTE_TYPE_CHARSET:
			return "charset";
		case AST_LAYOUT_ATTRIBUTE_TYPE_TYPE:
			return "kind";

		default:
		case AST_LAYOUT_ATTRIBUTE_TYPE_ERROR:
			return "error";
	}
}
