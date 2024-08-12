#include "parser.h"

#define PARSER_CURRENT ((token_t*)lexer->tokens->data[lexer->token_index])
#define PARSER_NEXT lexer->token_index++
#define PARSER_PREV lexer->token_index--
#define PARSER_CURRENT_NEXT ((token_t*)lexer->tokens->data[lexer->token_index + 1])
#define PARSER_CURRENT_PREV ((token_t*)lexer->tokens->data[lexer->token_index + 1])

/**
 * 
 * @function match
 * @brief Match the token type
 * @param {lexer_t*} lexer - Lexer
 * @param {token_type_t} token_type - Token type
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
 * @param {lexer_t*} lexer - Lexer
 * @param {token_type_t} token_type - Token type
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
 * @param {lexer_t*} lexer - Lexer
 * @returns {void}
 * 
 */
void unknown(lexer_t* lexer)
{
	error(2, "Unexpected token type %s at line %d, column %d", token_name(PARSER_CURRENT->type), PARSER_CURRENT->location.end_line, PARSER_CURRENT->location.end_column);
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
 * @param {lexer_t*} lexer - Lexer
 * @param {token_type_t} token_type - Token type
 * @returns {bool}
 * 
 */
bool match_next(lexer_t* lexer, token_type_t token_type)
{
	if (lexer->token_index + 1 >= lexer->tokens->size) {
		return false;
	}
	return PARSER_CURRENT_NEXT->type == token_type;
}

/**
 * 
 * @function match_prev
 * @brief Match the previous token type
 * @param {lexer_t*} lexer - Lexer
 * @param {token_type_t} token_type - Token type
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

/**
 * 
 * @function parser_parse_block
 * @brief Parse the block
 * @param {lexer_t*} lexer - Lexer
 * @param {ast_block_type_t} type - Block type
 * @param {ast_type_t} block_parent_type - Block parent type
 * @returns {ast_block_t*} - AST block node
 * 
 */
ast_block_t* parser_parse_block(lexer_t* lexer, ast_block_type_t type, ast_type_t block_parent_type)
{
	ast_block_t* block = ast_block_create();
	block->type = type;
	block->parent_type = block_parent_type;

	expect(lexer, TOKEN_LEFT_BRACE);

	// while (PARSER_CURRENT->type != TOKEN_RIGHT_BRACE) {
	//     ast_node_t* child = ast_node_create(AST_NODE_TYPE_ERROR, PARSER_CURRENT->location);

	//     if (match(lexer, TOKEN_LAYOUT)) {
	//         child = parser_parse_layout(lexer);
	//     }
	//     else {
	//         unknown(lexer);
	//     }

	//     // if (block_parent_type == AST_NODE_TYPE_LAYOUT) {
	//     //     array_push(node->children, child);
	//     // }
	//     // else {
	//     //     array_push(node->children, child);
	//     // }
	// }

	expect(lexer, TOKEN_RIGHT_BRACE);

	return block;
}

/**
 * 
 * @function parser_parse_layout_node
 * @brief Parsing layout node
 * @returns {ast_layout_node_t*} - AST layout node
 * 
 */
ast_layout_node_t* parser_parse_layout_node()
{
	ast_layout_node_t* node = ast_layout_node_create(AST_NODE_LAYOUT_NODE_TYPE_DIV);

	return node;
}

/**
 * 
 * @function parser_parse_layout_block
 * @brief Parse the block
 * @param {ast_layout_block_t*} - AST layout block node
 * @param {lexer_t*} lexer - Lexer
 * @param {ast_type_t} block_parent_type - Block parent type
 * @returns {void}
 * 
 */
void parser_parse_layout_block(ast_layout_block_t* block, lexer_t* lexer, ast_type_t block_parent_type)
{
	block->parent_type = block_parent_type;

	expect(lexer, TOKEN_LEFT_BRACE);

	while (PARSER_CURRENT->type != TOKEN_RIGHT_BRACE) {
		token_t* token = PARSER_CURRENT;

		if (match(lexer, TOKEN_IDENTIFIER) && match_next(lexer, TOKEN_COLON)) {
			array_t* values = array_create(1, sizeof(token_t*));
			token_t* value = PARSER_CURRENT;

			PARSER_NEXT; // Eating the identifier token
			PARSER_NEXT; // Eating the colon token
			PARSER_NEXT; // Eating the value token

			array_push(values, value);

			ast_layout_attribute_t* attribute = ast_layout_attribute_create(token->data.string, values);
			hashmap_put(cast(hashmap_t*, block->attributes), token->data.string, attribute);
			
			// printf("key is %s\n", token->data.string);
			// printf("key is %s\n", attribute->key);
			// values->print(values);
			// printf("---------------------------->>>\n");
			// attribute->print(attribute);
			// attribute->free(attribute);
			
			printf("Put %s attribute\n", token->data.string);
		}
		else {
			unknown(lexer);
		}
	}

	expect(lexer, TOKEN_RIGHT_BRACE);
}

/**
 * 
 * @function parser_parse_layout
 * @brief Parse the layout
 * @param {lexer_t*} lexer - Lexer
 * @returns {ast_node_t*} - AST node
 * 
 */
ast_node_t* parser_parse_layout(lexer_t* lexer)
{
	ast_node_t* node = ast_node_create(AST_NODE_TYPE_LAYOUT, PARSER_CURRENT->location);

	PARSER_NEXT; // Eat the layout token

	node->data.layout = ast_layout_create();

	parser_parse_layout_block(node->data.layout->block, lexer, AST_NODE_TYPE_LAYOUT);

	return node;
}

/**
 * 
 * @function parser_parse_node
 * @brief Parse the node
 * @param {lexer_t*} lexer - Lexer
 * @returns {ast_node_t*} - AST node
 * 
 */
ast_node_t* parser_parse_node(lexer_t* lexer)
{
	if (match(lexer, TOKEN_LAYOUT)) {
		return parser_parse_layout(lexer);
	}

	unknown(lexer);
	return NULL;
}

/**
 * 
 * @function parser_parse
 * @brief Parse the tokens
 * @param {lexer_t*} lexer - Lexer
 * @returns {ast_t*} - AST
 * 
 */
ast_t* parser_parse(lexer_t* lexer)
{
	ast_t* ast = ast_create();

	while (lexer->token_index < lexer->tokens->size) {
		if (PARSER_CURRENT->type == TOKEN_EOF) break;

		ast_node_t* node = parser_parse_node(lexer);
		if (node == NULL) continue;

		if (node->type == AST_NODE_TYPE_LAYOUT) {
			if (ast->layout != NULL) {
				ast_layout_destroy(ast->layout);
			}

			ast->layout = node->data.layout;
			free(node);
		}
	}

	return ast;
}
