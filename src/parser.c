#include "parser.h"

#define PARSER_NEXT lexer->token_index++
#define PARSER_PREV lexer->token_index--

#define PARSER_CURRENT ((token_t*)lexer->tokens->data[lexer->token_index])
#define PARSER_CURRENT_NEXT ((token_t*)lexer->tokens->data[lexer->token_index + 1])
#define PARSER_CURRENT_PREV ((token_t*)lexer->tokens->data[lexer->token_index + 1])

// #define PARSER_CURRENT ((token_t*)array_get(lexer->tokens, lexer->token_index))
// #define PARSER_CURRENT_NEXT ((token_t*)array_get(lexer->tokens, lexer->token_index + 1))
// #define PARSER_CURRENT_PREV ((token_t*)array_get(lexer->tokens, lexer->token_index - 1))

/**
 * 
 * @function match
 * @brief Match the token type
 * @params {lexer_t*} lexer - Lexer
 * @params {token_type_t} token_type - Token type
 * @returns {bool}
 * 
 */
bool match(lexer_t* lexer, token_type_t token_type)
{
	return PARSER_CURRENT->type == token_type;
}

/**
 * 
 * @function expect
 * @brief Expect the token type
 * @params {lexer_t*} lexer - Lexer
 * @params {token_type_t} token_type - Token type
 * @returns {void}
 * 
 */
void expect(lexer_t* lexer, token_type_t token_type)
{
	if (PARSER_CURRENT->type != token_type) {
		error(2, "Expected token type %s, got %s at line %d, column %d", token_name(token_type), token_name(PARSER_CURRENT->type), PARSER_CURRENT->location.end_line, PARSER_CURRENT->location.end_column);
	}

	PARSER_NEXT;
}

/**
 * 
 * @function unknown
 * @brief Unknown token type
 * @params {lexer_t*} lexer - Lexer
 * @returns {void}
 * 
 */
void unknown(lexer_t* lexer)
{
	error(2, "Unexpected token type %s at line %d, column %d", token_name(PARSER_CURRENT->type), PARSER_CURRENT->location.end_line, PARSER_CURRENT->location.end_column);
}

/**
 * 
 * @function unknown_scope
 * @brief Unknown token type in a specific scope
 * @params {lexer_t*} lexer - Lexer
 * @params {char*} scope - Scope
 * @returns {void}
 * 
 */
void unknown_scope(lexer_t* lexer, char* scope)
{
	error(2, "Unknown token type %s in scope %s at line %zu, column %zu\n", token_name(PARSER_CURRENT->type), scope, PARSER_CURRENT->location.end_line, PARSER_CURRENT->location.end_column);
}

/**
 * 
 * @function ast_block_create
 * @brief Create a new AST block node
 * @returns {ast_block_t*} - AST block node
 * 
 */
ast_block_t* ast_block_create()
{
	ast_block_t* block = malloc(sizeof(ast_block_t));
	block->children = array_create(sizeof(ast_node_t*), 4);

	return block;
}

/**
 * 
 * @function match_next
 * @brief Match the next token type
 * @params {lexer_t*} lexer - Lexer
 * @params {token_type_t} token_type - Token type
 * @returns {bool}
 * 
 */
bool match_next(lexer_t* lexer, token_type_t token_type)
{
	if (lexer->token_index + 1 >= lexer->tokens->capacity) {
		return false;
	}
	return PARSER_CURRENT_NEXT->type == token_type;
}

/**
 * 
 * @function match_prev
 * @brief Match the previous token type
 * @params {lexer_t*} lexer - Lexer
 * @params {token_type_t} token_type - Token type
 * @returns {bool}
 * 
 */
bool match_prev(lexer_t* lexer, token_type_t token_type)
{
	if (lexer->token_index == 0) {
		return false;
	}
	return PARSER_CURRENT_PREV->type == token_type;
}

token_t* parser_parse_value(lexer_t* lexer)
{
	token_t* token = PARSER_CURRENT;

	PARSER_NEXT;

	return token;
}

/**
 * 
 * @function parser_parse_block
 * @brief Parse the block
 * @params {lexer_t*} lexer - Lexer
 * @params {ast_block_type_t} type - Block type
 * @params {ast_type_t} block_parent_type - Block parent type
 * @returns {ast_block_t*} - AST block node
 * 
 */
ast_block_t* parser_parse_block(lexer_t* lexer, ast_block_type_t type, ast_type_t block_parent_type)
{
	ast_block_t* block = ast_block_create();
	block->type = type;
	block->parent_type = block_parent_type;

	expect(lexer, TOKEN_LEFT_BRACE);

	expect(lexer, TOKEN_RIGHT_BRACE);

	return block;
}

/**
 * 
 * @function parser_parse_layout_node
 * @brief Parsing layout node
 * @params {ast_layout_node_type_t} parent_node_type - Parent node type
 * @params {lexer_t*} lexer - Lexer
 * @returns {ast_layout_node_t*} - AST layout node
 * 
 */
ast_layout_node_t* parser_parse_layout_node(lexer_t* lexer, ast_layout_block_t* block)
{	
	ast_layout_node_type_t type = token_to_ast_layout_node_type(PARSER_CURRENT);
	PARSER_NEXT;

	ast_layout_node_t* node = ast_layout_node_create(type);

	parser_parse_layout_block(node->block, lexer);
	
	return node;
}

/**
 * 
 * @function parser_parse_layout_block_attribute
 * @brief Parse the block attribute
 * @params {ast_layout_block_t*} block - AST layout block node
 * @params {lexer_t*} lexer - Lexer
 * @returns {void}
 * 
 */
void parser_parse_layout_block_attribute(ast_layout_block_t* block, lexer_t* lexer)
{
	token_t* token = PARSER_CURRENT;
	array_t* values = array_create(sizeof(token_t*), 1);
	values->destroy = cast(void (*)(void*), array_token_destroy);

	PARSER_NEXT; // Eating the identifier token
	PARSER_NEXT; // Eating the colon token

	token_t* value = parser_parse_value(lexer);

	token_t* value_copy = token_copy(value);

	array_push(values, value_copy);

	if (match(lexer, TOKEN_COMMA)) {
		while (match(lexer, TOKEN_COMMA)) {
			PARSER_NEXT; // Eating the comma token

			token_t* new_value = parser_parse_value(lexer);

			token_t* new_value_copy = token_copy(new_value);
			array_push(values, new_value_copy);
		}
	}

	ast_layout_attribute_type_t attribute_key_type = token_to_ast_layout_attribute_type(token, block->parent_node_type);
	token->print(token);

	if (!token_belongs_to_ast_layout_node(block,  attribute_key_type)) {
		error(2, "Attribute '%s' does not belong to node '%s' at line %d, column %d", token_value(token), ast_layout_node_type_to_name(block->parent_node_type), token->location.end_line, token->location.end_column);
	}

	ast_layout_attribute_t* attribute = ast_layout_attribute_create(attribute_key_type, token->data.string, values);
	char* attribute_key_name = ast_layout_attribute_type_to_name(attribute_key_type);

	hashmap_put(cast(hashmap_t*, block->attributes), attribute_key_name, attribute);
}

/**
 * 
 * @function parser_parse_layout_block_children
 * @brief Parse the block children
 * @params {ast_layout_block_t*} block - AST layout block node
 * @params {lexer_t*} lexer - Lexer
 * @returns {void}
 * 
 */
void parser_parse_layout_block_children(ast_layout_block_t* block, lexer_t* lexer)
{
	ast_layout_node_t* node = parser_parse_layout_node(lexer, block);

	array_push(block->children, node);
}

/**
 * 
 * @function parser_parse_layout_block
 * @brief Parse the block
 * @params {ast_layout_block_t*} block - AST layout block
 * @params {lexer_t*} lexer - Lexer
 * @params {ast_type_t} block_parent_type - Block parent type
 * @returns {void}
 * 
 */
void parser_parse_layout_block(ast_layout_block_t* block, lexer_t* lexer)
{
	expect(lexer, TOKEN_LEFT_BRACE);

	while (PARSER_CURRENT->type != TOKEN_RIGHT_BRACE) {
		if (match(lexer, TOKEN_IDENTIFIER) && match_next(lexer, TOKEN_COLON)) {
			parser_parse_layout_block_attribute(block, lexer);
		}
		else if (match(lexer, TOKEN_IDENTIFIER) && match_next(lexer, TOKEN_LEFT_BRACE)) {
			parser_parse_layout_block_children(block, lexer);
		}
		else {
			unknown_scope(lexer, "layout block");
		}
	}

	expect(lexer, TOKEN_RIGHT_BRACE);

	validate_layout_block(block);
}

/**
 * 
 * @function parser_parse_layout
 * @brief Parse the layout
 * @params {lexer_t*} lexer - Lexer
 * @returns {ast_node_t*} - AST node
 * 
 */
ast_node_t* parser_parse_layout(lexer_t* lexer)
{
	ast_node_t* node = ast_node_create(AST_NODE_TYPE_LAYOUT, PARSER_CURRENT->location);

	PARSER_NEXT; // Eat the layout token

	node->data.layout = ast_layout_create();

	parser_parse_layout_block(node->data.layout->block, lexer);

	return node;
}

/**
 * 
 * @function parser_parse_node
 * @brief Parse the node
 * @params {lexer_t*} lexer - Lexer
 * @returns {ast_node_t*} - AST node
 * 
 */
ast_node_t* parser_parse_node(lexer_t* lexer)
{
	if (match(lexer, TOKEN_LAYOUT)) {
		return parser_parse_layout(lexer);
	}
	
	unknown_scope(lexer, "node");

	return NULL;
}

/**
 * 
 * @function parser_parse
 * @brief Parse the tokens
 * @params {lexer_t*} lexer - Lexer
 * @returns {ast_t*} - AST
 * 
 */
ast_t* parser_parse(lexer_t* lexer)
{
	ast_t* ast = ast_create();

	while (lexer->token_index < lexer->tokens->capacity) {
		if (PARSER_CURRENT->type == TOKEN_EOF) break;

		ast_node_t* node = parser_parse_node(lexer);
		if (node == NULL) continue;

		if (node->type == AST_NODE_TYPE_LAYOUT) {
			if (ast->layout != NULL) {
				ast_layout_destroy(ast->layout);
			}

			ast->layout = node->data.layout;
			
			if (node != NULL) {
				memory_destroy(node);
			}
		}
	}

	return ast;
}
