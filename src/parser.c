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
    DEBUG_ME;
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
    DEBUG_ME;
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
    DEBUG_ME;
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
    DEBUG_ME;
	error(2, "Unknown token type %s in scope %s at line %zu, column %zu\n", token_name(PARSER_CURRENT->type), scope, PARSER_CURRENT->location.end_line, PARSER_CURRENT->location.end_column);
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
    DEBUG_ME;
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
    DEBUG_ME;
	if (lexer->token_index == 0) {
		return false;
	}
	return PARSER_CURRENT_PREV->type == token_type;
}

/**
 * 
 * @function parser_parse_value
 * @brief Parse the value
 * @params {lexer_t*} lexer - Lexer
 * @returns {token_t*} - Token
 * 
 */
token_t* parser_parse_value(lexer_t* lexer)
{
    DEBUG_ME;
	token_t* token = PARSER_CURRENT;

	PARSER_NEXT;

	return token;
}

/**
 * 
 * @function parser_parse_block
 * @brief Parse the block
 * @params {lexer_t*} lexer - Lexer
 * @params {ast_block_t*} block - AST Block
 * @returns {void}
 * 
 */
void parser_parse_block(lexer_t* lexer, ast_block_t* block)
{
    DEBUG_ME;

	expect(lexer, TOKEN_LEFT_BRACE);

	while (PARSER_CURRENT->type != TOKEN_RIGHT_BRACE) {
		ast_node_t* node = parser_parse_node(lexer);

		if (node == NULL) {
			continue;
		}
		else {
			array_push(block->children, node);
		}
	}

	expect(lexer, TOKEN_RIGHT_BRACE);
}

/**
 * 
 * @function parser_parse_layout_node
 * @brief Parsing layout node
 * @params {lexer_t*} lexer - Lexer
 * @returns {ast_layout_node_t*} - AST layout node
 * 
 */
ast_layout_node_t* parser_parse_layout_node(lexer_t* lexer)
{
    DEBUG_ME;
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
    DEBUG_ME;
	// token
	array_t* values = array_create(sizeof(token_t*), 1);
	values->destroy = cast(void (*)(void*), array_token_destroy);

	token_t* name_token = PARSER_CURRENT;
	string_t* name = string_create(16);
	string_append_str(name, token_value(PARSER_CURRENT));
	PARSER_NEXT; // Eating the identifier token

	while (match(lexer, TOKEN_MINUS)) {
		PARSER_NEXT; // Eating the minus token
		string_append_char(name, '-');

		if (match(lexer, TOKEN_IDENTIFIER)) {
			string_append_str(name, token_value(PARSER_CURRENT));
			name_token = PARSER_CURRENT;
			PARSER_NEXT; // Eating the identifier token
		}
		else {
			error(2, "Expected an identifier after the dash in the attribute name at line %d, column %d, but got %s", PARSER_CURRENT->location.end_line, PARSER_CURRENT->location.end_column, token_name(PARSER_CURRENT->type));
			break;
		}
	}

	if (match(lexer, TOKEN_COLON)) {
		PARSER_NEXT; // Eating the left brace token
	}
	else {
		error(2, "Expected a colon after the attribute name at line %d, column %d, but got %s", PARSER_CURRENT->location.end_line, PARSER_CURRENT->location.end_column, token_name(PARSER_CURRENT->type));
	}

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

	ast_layout_attribute_type_t attribute_key_type = token_to_ast_layout_attribute_type(name->data, name_token, block->parent_node_type);

	ast_layout_attribute_t* attribute = ast_layout_attribute_create(attribute_key_type, name->data, values);
	if (!token_belongs_to_ast_layout_node(block,  attribute_key_type, attribute)) {
		error(2, "Attribute '%s' does not belong to node '%s' at line %d, column %d", name, ast_layout_node_type_to_name(block->parent_node_type), name_token->location.end_line, name_token->location.end_column);
	}

	char* attribute_key_name = ast_layout_attribute_type_to_name(attribute_key_type);
	
    if (is_style_attribute(attribute_key_type)) {
		if (hashmap_has(cast(hashmap_t*, block->styles), attribute_key_name)) {
			error(2, "Style attribute '%s' already defined in the '%s' block at line %d, column %d", attribute_key_name, ast_layout_node_type_to_name(block->parent_node_type), name_token->location.end_line, name_token->location.end_column);
		}
		else {
			hashmap_put(cast(hashmap_t*, block->styles), attribute_key_name, attribute);
		}
	}
	else {
		if (hashmap_has(cast(hashmap_t*, block->attributes), attribute_key_name)) {
			error(2, "Attribute '%s' already defined in the '%s' block at line %d, column %d", attribute_key_name, ast_layout_node_type_to_name(block->parent_node_type), name_token->location.end_line, name_token->location.end_column);
		}
		else {
			hashmap_put(cast(hashmap_t*, block->attributes), attribute_key_name, attribute);
		}
	}

	if (name != NULL) name->destroy(name);
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
    DEBUG_ME;
	ast_layout_node_t* node = parser_parse_layout_node(lexer);

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
    DEBUG_ME;
	expect(lexer, TOKEN_LEFT_BRACE);

	while (PARSER_CURRENT->type != TOKEN_RIGHT_BRACE) {
		if (match(lexer, TOKEN_IDENTIFIER) && (match_next(lexer, TOKEN_COLON) || match_next(lexer, TOKEN_MINUS))) {
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
    DEBUG_ME;
	ast_node_t* node = ast_node_create(AST_NODE_TYPE_LAYOUT, PARSER_CURRENT->location);

	PARSER_NEXT; // Eat the layout token

	node->data.layout = ast_layout_create();

	parser_parse_layout_block(node->data.layout->block, lexer);

	return node;
}

/**
 * 
 * @function parser_parse_function
 * @brief Parse the function
 * @params {lexer_t*} lexer - Lexer
 * @returns {ast_node_t*} - AST node
 * 
 */
ast_node_t* parser_parse_function(lexer_t* lexer)
{
    DEBUG_ME;
	ast_node_t* node = ast_node_create(AST_NODE_TYPE_FUNCTION, PARSER_CURRENT->location);

	PARSER_NEXT; // Eat the function token

	token_t* function_name = PARSER_CURRENT;
	expect(lexer, TOKEN_IDENTIFIER);
	node->data.function = ast_function_create(function_name->data.string);

	// Optional ()
	if (match(lexer, TOKEN_LEFT_PAREN)) {
		PARSER_NEXT; // Eat the left parenthesis token

		expect(lexer, TOKEN_RIGHT_PAREN);
	}

	parser_parse_block(lexer, node->data.function->block);

	return node;
}

/**
 * 
 * @function parser_parse_expression
 * @brief Parse the expression
 * @params {lexer_t*} lexer - Lexer
 * @returns {ast_value_t*} - AST value
 * 
 */
ast_value_t* parser_parse_expression(lexer_t* lexer)
{
	DEBUG_ME;
	ast_value_t* value = NULL;
	token_t* token = PARSER_CURRENT;

	token->print(token);

	if (match(lexer, TOKEN_IDENTIFIER)) {
		PARSER_NEXT;
		value = ast_value_create(ast_type_create(AST_TYPE_KIND_STRING));
		value->data = strdup(token->data.string);
	}
	else if (match(lexer, TOKEN_STRING)) {
		PARSER_NEXT;
		value = ast_value_create(ast_type_create(AST_TYPE_KIND_STRING));
		value->data = strdup(token->data.string);
	}
	else if (match(lexer, TOKEN_NUMBER_INT)) {
		PARSER_NEXT;
		value = ast_value_create(ast_type_create(AST_TYPE_KIND_STRING));
		value->data = strdup(int2string(token->data.number_int));
	}
	else if (match(lexer, TOKEN_NUMBER_FLOAT)) {
		PARSER_NEXT;
		value = ast_value_create(ast_type_create(AST_TYPE_KIND_STRING));
		value->data = strdup(float2string(token->data.number_float));
	}
	else if (match(lexer, TOKEN_BOOLEAN)) {
		PARSER_NEXT;
		value = ast_value_create(ast_type_create(AST_TYPE_KIND_STRING));
		value->data = token->data.boolean ? strdup("true") : strdup("false");
	}
	else {
		value->data = NULL;
	}
	
	if (value == NULL || value->data) {
		error(2, "Expected an expression at line %d, column %d, but got %s", token->location.end_line, token->location.end_column, token_name(token->type));
	}

	return value;
}

/**
 * 
 * @function parser_parse_if
 * @brief Parse the if
 * @params {lexer_t*} lexer - Lexer
 * @returns {ast_node_t*} - AST node
 * 
 */
ast_node_t* parser_parse_if(lexer_t* lexer)
{
	DEBUG_ME;
	ast_node_t* node = ast_node_create(AST_NODE_TYPE_IF, PARSER_CURRENT->location);

	PARSER_NEXT; // Eat the if token

	ast_value_t* condition = parser_parse_expression(lexer);
	node->data.ifclause = ast_if_create(condition);


	parser_parse_block(lexer, node->data.ifclause->block);

	// Optional else and then one or more if or else if
	while (true) {
		if (match(lexer, TOKEN_ELSE)) {
			// else {} and stop
			if (match(lexer, TOKEN_LEFT_BRACE)) {
				PARSER_NEXT; // Eat the else token

				ast_node_t* else_if = ast_node_create(AST_NODE_TYPE_ELSE_IF, PARSER_CURRENT->location);
				
				else_if->data.ifclause = ast_else_create();
				parser_parse_block(lexer, else_if->data.ifclause->block);

				array_push(node->data.ifclause->else_blocks, else_if);

				break; // Stop the loop, this is the last else
			}
			// else if {}
			else if (match(lexer, TOKEN_IF)) {
				PARSER_NEXT; // Eat the else token

				ast_node_t* else_if = ast_node_create(AST_NODE_TYPE_IF, PARSER_CURRENT->location);
				PARSER_NEXT; // Eat the sub if token
				
				ast_value_t* condition = parser_parse_expression(lexer);
				else_if->data.ifclause = ast_elseif_create(condition);

				parser_parse_block(lexer, else_if->data.ifclause->block);

				array_push(node->data.ifclause->else_blocks, else_if);
			}
			else {
				error(2, "Expected a block or an else if at line %d, column %d, but got %s", PARSER_CURRENT->location.end_line, PARSER_CURRENT->location.end_column, token_name(PARSER_CURRENT->type));
			}
		}
		else {
			break; // Stop the loop, no else or else if
		}
	}

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
    DEBUG_ME;
	if (match(lexer, TOKEN_LAYOUT)) {
		return parser_parse_layout(lexer);
	}
	else if (match(lexer, TOKEN_FUNCTION)) {
		return parser_parse_function(lexer);
	}
	else if (match(lexer, TOKEN_IF)) {
		return parser_parse_if(lexer);
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
    DEBUG_ME;
	ast_t* ast = ast_create();

	while (lexer->token_index < lexer->tokens->capacity) {
		if (PARSER_CURRENT->type == TOKEN_EOF) break;

		ast_node_t* node = parser_parse_node(lexer);
		if (node == NULL) {
			continue;
		}
		else if (node->type == AST_NODE_TYPE_LAYOUT) {
			if (ast->layout != NULL) {
				ast_layout_destroy(ast->layout);
			}

			ast->layout = node->data.layout;
			
			if (node != NULL) {
				memory_destroy(node);
			}
		}
		else if (node->type == AST_NODE_TYPE_FUNCTION) {
			array_push(ast->functions, node->data.function);
			ast_node_destroy_notall(node);
		}
	}

	return ast;
}
