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
 * @function is_begin_block_token
 * @brief Check if the token is a begin block
 * @params {token_t*} token - Token
 * @returns {bool}
 * 
 */
bool is_begin_block_token(token_t* token)
{
	return token->type == TOKEN_LEFT_BRACE;
}

/**
 * 
 * @function is_close_block_token
 * @brief Check if the token is an end block
 * @params {token_t*} token - Token
 * @returns {bool}
 * 
 */
bool is_close_block_token(token_t* token)
{
	return token->type == TOKEN_RIGHT_BRACE;
}

/**
 * 
 * @function expect_open_block
 * @brief Expect an open block
 * @returns {void}
 * 
 */
void expect_open_block(lexer_t* lexer)
{
	expect(lexer, TOKEN_LEFT_BRACE);
}

/**
 * 
 * @function expect_open_block
 * @brief Expect an open block
 * @returns {void}
 * 
 */
void expect_close_block(lexer_t* lexer)
{
	expect(lexer, TOKEN_RIGHT_BRACE);
}

/**
 * 
 * @function match_next_open_block
 * @brief Match the next open block
 * @params {lexer_t*} lexer - Lexer
 * @returns {bool}
 * 
 */
bool match_next_open_block(lexer_t* lexer)
{
	return match_next(lexer, TOKEN_LEFT_BRACE);
}

/**
 * 
 * @function match_next_close_block
 * @brief Match the next close block
 * @params {lexer_t*} lexer - Lexer
 * @returns {bool}
 * 
 */
bool match_next_close_block(lexer_t* lexer)
{
	return match_next(lexer, TOKEN_RIGHT_BRACE);
}

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
 * @returns {ast_layout_value_t*} - AST Layout value
 * 
 */
ast_layout_value_t* parser_parse_value(lexer_t* lexer)
{
    DEBUG_ME;
	token_t* token = PARSER_CURRENT;

	PARSER_NEXT;

	ast_layout_value_t* value = ast_layout_value_create(token_value(token));

	return value;
}

/**
 * 
 * @function parser_parse_layout_value
 * @brief Parse the layout value
 * @params {lexer_t*} lexer - Lexer
 * @returns {ast_layout_attribute_value_t*} - AST Layout attribute value
 * 
 */
ast_layout_attribute_value_t* parser_parse_layout_value(lexer_t* lexer)
{
    DEBUG_ME;
	token_t* token = PARSER_CURRENT;

	PARSER_NEXT;
	
	ast_layout_attribute_value_t* value = ast_layout_attribute_value_create(token_value(token));

	return value;
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
	
	expect_open_block(lexer);

	while (PARSER_CURRENT->type != TOKEN_RIGHT_BRACE) {
		ast_node_t* node = parser_parse_node(lexer);

		if (node == NULL) {
			error(2, "Expected a node at line %d, column %d, but got %s", PARSER_CURRENT->location.end_line, PARSER_CURRENT->location.end_column, token_name(PARSER_CURRENT->type));
			continue;
		}
		else {
			array_push(block->children, node);
		}
	}

	expect_close_block(lexer);
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
	array_t* values = array_layout_attribute_value_create(1);

	token_t* last_name = PARSER_CURRENT;
	string_t* name = string_create(16);
	string_append_str(name, token_value(PARSER_CURRENT));
	PARSER_NEXT; // Eating the identifier token

	while (match(lexer, TOKEN_MINUS)) {
		PARSER_NEXT; // Eating the minus token
		string_append_char(name, '-');

		if (match(lexer, TOKEN_IDENTIFIER)) {
			string_append_str(name, token_value(PARSER_CURRENT));
			last_name = PARSER_CURRENT;
			PARSER_NEXT; // Eating the identifier token
		}
		else {
			error(2, "Expected an identifier after the dash in the attribute name at line %d, column %d, but got %s", PARSER_CURRENT->location.end_line, PARSER_CURRENT->location.end_column, token_name(PARSER_CURRENT->type));
			break;
		}
	}

	token_t* first_value = PARSER_CURRENT;
	ast_layout_attribute_value_t* value = parser_parse_layout_value(lexer);

	array_push(values, value);

	if (match(lexer, TOKEN_COMMA)) {
		while (match(lexer, TOKEN_COMMA)) {
			PARSER_NEXT; // Eating the comma token

			ast_layout_attribute_value_t* new_value = parser_parse_layout_value(lexer);

			if (new_value != NULL) {
				array_push(values, new_value);
			}
		}
	}

	ast_layout_attribute_type_t attribute_key_type = token_to_ast_layout_attribute_type(name->data, last_name, block->parent_node_type);

	ast_layout_attribute_t* attribute = ast_layout_attribute_create(attribute_key_type, name->data, values, last_name->location, first_value->location);
	if (!token_belongs_to_ast_layout_node(block,  attribute_key_type, attribute)) {
		error(2, "Attribute '%s' does not belong to node '%s' at line %d, column %d", name, ast_layout_node_type_to_name(block->parent_node_type), last_name->location.end_line, last_name->location.end_column);
	}

	char* attribute_key_name = ast_layout_attribute_type_to_name(attribute_key_type);
	
    if (is_style_attribute(attribute_key_type)) {
		if (hashmap_has(cast(hashmap_t*, block->styles), attribute_key_name)) {
			error(2, "Style attribute '%s' already defined in the '%s' block at line %d, column %d", attribute_key_name, ast_layout_node_type_to_name(block->parent_node_type), last_name->location.end_line, last_name->location.end_column);
		}
		else {
			hashmap_put(cast(hashmap_t*, block->styles), attribute_key_name, attribute);
		}
	}
	else {
		if (hashmap_has(cast(hashmap_t*, block->attributes), attribute_key_name)) {
			error(2, "Attribute '%s' already defined in the '%s' block at line %d, column %d", attribute_key_name, ast_layout_node_type_to_name(block->parent_node_type), last_name->location.end_line, last_name->location.end_column);
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
	
	if (node != NULL) {
		array_push(block->children, node);
	}
	else {
		error(2, "Expected a layout node at line %d, column %d, but got %s", PARSER_CURRENT->location.end_line, PARSER_CURRENT->location.end_column, token_name(PARSER_CURRENT->type));
	}
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
	expect_open_block(lexer);

	while (PARSER_CURRENT->type != TOKEN_RIGHT_BRACE) {
		if (match(lexer, TOKEN_IDENTIFIER) && match_next_open_block(lexer)) {
			parser_parse_layout_block_children(block, lexer);
		}
		else if (match(lexer, TOKEN_IDENTIFIER)) {
			parser_parse_layout_block_attribute(block, lexer);
		}
		else {
			unknown_scope(lexer, "layout block");
		}
	}

	expect_close_block(lexer);

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
 * @function parser_parse_expressions_maybe
 * @brief Parse the expressions maybe
 * @params {lexer_t*} lexer - Lexer
 * @returns {array_value_t*} - Array of AST values
 * 
 */
array_value_t* parser_parse_expressions_maybe(lexer_t* lexer)
{
	DEBUG_ME;
	if (
		match(lexer, TOKEN_NUMBER_FLOAT) ||
		match(lexer, TOKEN_NUMBER_INT) ||
		match(lexer, TOKEN_STRING) ||
		match(lexer, TOKEN_PLUS) ||
		match(lexer, TOKEN_IDENTIFIER) ||
		match(lexer, TOKEN_MINUS)
		// TODO: add mroe types
	) {
		return parser_parse_expressions(lexer);
	}

	return array_value_create(1);
	// return NULL;
}

/**
 * 
 * @function parser_parse_expressions
 * @brief Parse the expressions
 * @params {lexer_t*} lexer - Lexer
 * @returns {array_value_t*} - Array of AST values
 * 
 */
array_value_t* parser_parse_expressions(lexer_t* lexer)
{
	DEBUG_ME;
	array_value_t* values = array_value_create(1);

	ast_value_t* value = parser_parse_expression(lexer);
	array_push(values, value);

	while (match(lexer, TOKEN_COMMA)) {
		PARSER_NEXT; // Eat the comma token

		ast_value_t* new_value = parser_parse_expression(lexer);
		if (new_value == NULL) {
			error(2, "Expected an expression at line %d, column %d, but got %s", PARSER_CURRENT->location.end_line, PARSER_CURRENT->location.end_column, token_name(PARSER_CURRENT->type));
		}

		array_push(values, new_value);
	}

	return values;
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
	token_t* token = PARSER_CURRENT;

	ast_value_type_t* type = NULL;
	ast_value_t* value = NULL;
	char* data = memory_allocate(16 * sizeof(char));
	
	if (match(lexer, TOKEN_IDENTIFIER)) {
		PARSER_NEXT;
		strcpy(data, token->data.string);

		type = ast_type_create(AST_TYPE_KIND_STRING, token->location);
		value = ast_value_create(type, data);
		return value;
	}
	else if (match(lexer, TOKEN_STRING)) {
		PARSER_NEXT;
		strcpy(data, token->data.string);

		type = ast_type_create(AST_TYPE_KIND_STRING, token->location);
		value = ast_value_create(type, data);
		return value;
	}
	else if (match(lexer, TOKEN_NUMBER_INT)) {
		PARSER_NEXT;
		strcpy(data, int2string(token->data.number_int));

		type = ast_type_create(AST_TYPE_KIND_STRING, token->location);
		value = ast_value_create(type, data);
		return value;
	}
	else if (match(lexer, TOKEN_NUMBER_FLOAT)) {
		PARSER_NEXT;
		strcpy(data, float2string(token->data.number_float));

		type = ast_type_create(AST_TYPE_KIND_STRING, token->location);
		value = ast_value_create(type, data);
		return value;
	}
	else if (match(lexer, TOKEN_BOOLEAN)) {
		PARSER_NEXT;
		strcpy(data, token->data.boolean ? "true" : "false");

		type = ast_type_create(AST_TYPE_KIND_STRING, token->location);
		value = ast_value_create(type, data);
		return value;
	}
	else {
		error(2, "Expected an expression at line %d, column %d, but got %s", token->location.end_line, token->location.end_column, token_name(token->type));
	}

	return NULL;
}

/**
 * 
 * @function parser_parse_print
 * @brief Parse the print
 * @params {lexer_t*} lexer - Lexer
 * @returns {ast_node_t*} - AST node
 * 
 */
ast_node_t* parser_parse_print(lexer_t* lexer)
{
	DEBUG_ME;
	ast_node_t* node = ast_node_create(AST_NODE_TYPE_PRINT, PARSER_CURRENT->location);

	PARSER_NEXT; // Eat the print token

	array_value_t* values = parser_parse_expressions_maybe(lexer);
	node->data.print = ast_print_create(values);

	return node;
}

/**
 * 
 * @function parser_parse_return
 * @brief Parse the return
 * @params {lexer_t*} lexer - Lexer
 * @returns {ast_node_t*} - AST node
 * 
 */
ast_node_t* parser_parse_return(lexer_t* lexer)
{
	DEBUG_ME;
	ast_node_t* node = ast_node_create(AST_NODE_TYPE_RETURN, PARSER_CURRENT->location);

	PARSER_NEXT; // Eat the return token

	array_value_t* values = parser_parse_expressions(lexer);
	node->data.returns = ast_return_create(values);

	return node;
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
			if (match_next_open_block(lexer)) {
				PARSER_NEXT; // Eat the else token

				ast_node_t* else_if = ast_node_create(AST_NODE_TYPE_ELSE_IF, PARSER_CURRENT->location);
				
				else_if->data.ifclause = ast_else_create();
				parser_parse_block(lexer, else_if->data.ifclause->block);

				array_push(node->data.ifclause->else_blocks, else_if);

				break; // Stop the loop, this is the last else
			}
			// else if {}
			else if (match_next(lexer, TOKEN_IF)) {
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
	else if (match(lexer, TOKEN_RETURN)) {
		return parser_parse_return(lexer);
	}
	else if (match(lexer, TOKEN_PRINT)) {
		return parser_parse_print(lexer);
	}
	else {
		error(2, "Unknown token type '%s' at line %d, column %d", token_name(PARSER_CURRENT->type), PARSER_CURRENT->location.end_line, PARSER_CURRENT->location.end_column);
	}

	// unknown_scope(lexer, "node");

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
			error(2, "Expected a node at line %d, column %d, but got %s", PARSER_CURRENT->location.end_line, PARSER_CURRENT->location.end_column, token_name(PARSER_CURRENT->type));
			continue;
		}
		else if (node->type == AST_NODE_TYPE_LAYOUT) {
			// Free the previous layout if it exists
			if (ast->layout != NULL) {
				ast_layout_destroy(ast->layout);
			}

			// Set the layout
			ast->layout = node->data.layout;
			
			// Free the node as we don't need it anymore, we just need layout node
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
