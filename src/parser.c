#include "parser.h"

#define PARSER_NEXT lexer->token_index++
#define PARSER_PREV lexer->token_index--

#define PARSER_CURRENT ((token_t*)lexer->tokens->data[lexer->token_index])
#define PARSER_CURRENT_NEXT ((token_t*)lexer->tokens->data[lexer->token_index + 1])
#define PARSER_CURRENT_PREV ((token_t*)lexer->tokens->data[lexer->token_index + 1])

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
	expect(lexer, TOKEN_TYPE_OPEN_BLOCK);
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
	expect(lexer, TOKEN_TYPE_CLOSE_BLOCK);
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
 * @function match_open_block
 * @brief Match the open block
 * @params {lexer_t*} lexer - Lexer
 * @returns {bool}
 *
 */
bool match_open_block(lexer_t* lexer)
{
	return match(lexer, TOKEN_TYPE_OPEN_BLOCK);
}

/**
 *
 * @function match_close_block
 * @brief Match the close block
 * @params {lexer_t*} lexer - Lexer
 * @returns {bool}
 *
 */
bool match_close_block(lexer_t* lexer)
{
	return match(lexer, TOKEN_TYPE_CLOSE_BLOCK);
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
		error_parser(2, "Expected token type %s, got %s at line %d, column %d", token_type_keyword(token_type), token_type_keyword(PARSER_CURRENT->type), PARSER_CURRENT->location.end_line, PARSER_CURRENT->location.end_column);
	}

	PARSER_NEXT;
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
 * @returns {ast_value_t*} - AST Layout value
 *
 */
ast_value_t* parser_parse_value(lexer_t* lexer)
{
	DEBUG_ME;
	token_t* token = PARSER_CURRENT;

	PARSER_NEXT;

	if (token->type == TOKEN_IDENTIFIER) {
		// TODO
		error_parser(2, "Identifier '%s' is not defined at line %d, column %d", token->data.string, token->location.end_line, token->location.end_column);
	}

	ast_value_type_t* type = ast_value_type_create(AST_TYPE_KIND_STRING, token->location);

	ast_value_t* value = ast_value_create(type, token_value_stringify(token));

	return value;
}

/**
 *
 * @function parser_parse_layout_value
 * @brief Parse the layout value
 * @params {lexer_t*} lexer - Lexer
 * @returns {ast_value_t*} - AST Layout attribute value
 *
 */
ast_value_t*
parser_parse_layout_value(lexer_t* lexer)
{
	DEBUG_ME;
	return parser_parse_value(lexer);
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

	while (PARSER_CURRENT->type != TOKEN_TYPE_CLOSE_BLOCK) {
		ast_node_t* node = parser_parse_node(lexer);

		if (node == NULL) {
			error_parser(2, "Expected a node at line %d, column %d, but got %s", PARSER_CURRENT->location.end_line, PARSER_CURRENT->location.end_column, token_type_keyword(PARSER_CURRENT->type));
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
 * @params {char*} name - Name of the node
 * @params {token_t*} last_name - Last token
 * @returns {ast_layout_node_t*} - AST layout node
 *
 */
ast_layout_node_t* parser_parse_layout_node(lexer_t* lexer, char* name, token_t* last_name)
{
	DEBUG_ME;
	ast_layout_node_type_t type = enduser_name_to_ast_layout_node_type(name);

	if (type == AST_LAYOUT_TYPE_ERROR) {
		error_ast(2, "Unknown layout node '%s' at line %d, column %d", name, last_name->location.end_line, last_name->location.end_column);
	}

	ast_layout_node_t* node = ast_layout_node_create(type);

	parser_parse_layout_block(node->block, lexer);

	return node;
}

/**
 *
 * @function parser_parse_layout_block_style_state
 * @brief Parse the block style state
 * @params {ast_layout_block_t*} block - AST layout block node
 * @params {lexer_t*} lexer - Lexer
 * @params {char*} name - Name of the attribute
 * @params {token_t*} last_name - Last token
 * @returns {void}
 *
 */
void parser_parse_layout_block_style_state(ast_layout_block_t* block, lexer_t* lexer, char* name, token_t* last_name)
{
	DEBUG_ME;
	ast_layout_attribute_style_state_type style_state_type = enduser_name_to_ast_layout_attribute_style_state_type(name);

	if (style_state_type == AST_LAYOUT_ATTRIBUTE_STYLE_STATE_TYPE_ERROR) {
		error_parser(2, "Unknown style state '%s' at line %d, column %d", name, last_name->location.end_line, last_name->location.end_column);
	}

	expect_open_block(lexer);

	if (hashmap_has(block->states, name) == true) {
		error_parser(2, "Style state '%s' already defined in the '%s' block at line %d, column %d", name, ast_layout_node_type_to_enduser_name(block->parent_node_type), last_name->location.end_line, last_name->location.end_column);
	}

	ast_layout_style_state_t* state_styles = ast_layout_style_state_create();

	while (PARSER_CURRENT->type != TOKEN_TYPE_CLOSE_BLOCK) {
		token_t* last_name2 = PARSER_CURRENT;
		string_t* name2 = parser_parse_layout_name(lexer, &last_name2);

		PARSER_CURRENT->print(PARSER_CURRENT);

		parser_parse_layout_block_attribute(true, block, state_styles->normal, lexer, name2->data, last_name2);

		string_destroy(name2);
	}

	hashmap_put(block->states, name, state_styles);

	expect_close_block(lexer);
}

/**
 *
 * @function parser_parse_layout_values
 * @brief Parse the layout values
 * @params {lexer_t*} lexer - Lexer
 * @returns {array_value_t*} - Array of AST values
 *
 */
array_value_t* parser_parse_layout_values(lexer_t* lexer)
{
	array_value_t* values = array_value_create(1);

	ast_value_t* value = parser_parse_layout_value(lexer);

	array_push(values, value);

	if (match(lexer, TOKEN_COMMA)) {
		while (match(lexer, TOKEN_COMMA)) {
			PARSER_NEXT; // Eating the comma token

			ast_value_t* new_value = parser_parse_layout_value(lexer);

			if (new_value != NULL) {
				array_push(values, new_value);
			}
		}
	}

	return values;
}

/**
 *
 * @function parser_parse_layout_block_attribute
 * @brief Parse the block attribute
 * @params {bool} onlyStyle - Only style
 * @params {ast_layout_block_t*} block - AST layout block node
 * @params {hashmap_t*} normal - Normal hashmap
 * @params {lexer_t*} lexer - Lexer
 * @params {char*} name - Name of the attribute
 * @params {token_t*} last_name - Last token
 * @returns {void}
 *
 */
void parser_parse_layout_block_attribute(bool onlyStyle, ast_layout_block_t* block, hashmap_t* normal, lexer_t* lexer, char* name, token_t* last_name)
{
	DEBUG_ME;
	token_t* first_value = PARSER_CURRENT; // TODO

	if (match(lexer, TOKEN_TYPE_OPEN_BLOCK)) {
		error_parser(2, "Nonsupported layout element '%s' at line %d, column %d", name, last_name->location.end_line, last_name->location.end_column);
	}
	expect(lexer, TOKEN_ASSIGN);

	array_value_t* values = parser_parse_layout_values(lexer);

	ast_layout_attribute_type_t attribute_key_type = token_to_ast_layout_attribute_type(name, last_name, block->parent_node_type);

	ast_layout_attribute_t* attribute = ast_layout_attribute_create(attribute_key_type, name, values, block->parent_node_type, PARSER_CURRENT->location, first_value->location);
	if (!token_belongs_to_ast_layout_node(attribute_key_type, attribute)) {
		attribute->destroy(attribute);
		// values->destroy(values);

		error_parser(2, "Attribute '%s' does not belong to node '%s' at line %d, column %d", name, ast_layout_node_type_to_enduser_name(block->parent_node_type), last_name->location.end_line, last_name->location.end_column);
	}

	char* attribute_key_name = ast_layout_attribute_type_to_name(attribute_key_type);

	if (is_style_attribute(attribute_key_type)) {
		if (hashmap_has(normal, attribute_key_name)) {
			attribute->destroy(attribute);
			// values->destroy(values);

			error_parser(2, "Style attribute '%s' already defined in the '%s' block at line %d, column %d", attribute_key_name, ast_layout_node_type_to_enduser_name(block->parent_node_type), last_name->location.end_line, last_name->location.end_column);
		}
		else {
			hashmap_put(normal, attribute_key_name, attribute);
		}
	}
	else if (onlyStyle != true) {
		if (hashmap_has(block->attributes, attribute_key_name)) {
			attribute->destroy(attribute);
			// values->destroy(values);

			error_parser(2, "Attribute '%s' already defined in the '%s' block at line %d, column %d", attribute_key_name, ast_layout_node_type_to_enduser_name(block->parent_node_type), last_name->location.end_line, last_name->location.end_column);
		}
		else {
			hashmap_put(block->attributes, attribute_key_name, attribute);
		}
	}
	else {
		attribute->destroy(attribute);
		// values->destroy(values);

		error_parser(2, "Attribute '%s' is not a style attribute at line %d, column %d", attribute_key_name, last_name->location.end_line, last_name->location.end_column);
	}
}

/**
 *
 * @function parser_parse_layout_block_children
 * @brief Parse the block children
 * @params {ast_layout_block_t*} block - AST layout block node
 * @params {lexer_t*} lexer - Lexer
 * @params {char*} name - Name of the attribute
 * @params {token_t*} last_name - Last token
 * @returns {void}
 *
 */
void parser_parse_layout_block_children(ast_layout_block_t* block, lexer_t* lexer, char* name, token_t* last_name)
{
	DEBUG_ME;
	ast_layout_node_t* node = parser_parse_layout_node(lexer, name, last_name);

	if (node != NULL) {
		array_push(block->children, node);
	}
	else {
		error_parser(2, "Expected a layout node at line %d, column %d, but got %s", PARSER_CURRENT->location.end_line, PARSER_CURRENT->location.end_column, token_type_keyword(PARSER_CURRENT->type));
	}
}

/**
 *
 * @function parser_parse_layout_name
 * @brief Parse the layout name
 * @params {lexer_t*} lexer - Lexer
 * @params {token_t**} last_name - Last token
 * @returns {string_t*} - String
 *
 */
string_t* parser_parse_layout_name(lexer_t* lexer, token_t** last_name)
{
	DEBUG_ME;
	string_t* name = string_create(16);

	string_append_str(name, token_value_stringify(PARSER_CURRENT));

	PARSER_NEXT; // Eating the identifier token

	while (match(lexer, TOKEN_MINUS) || match(lexer, TOKEN_IDENTIFIER)) {
		if (PARSER_CURRENT->type == TOKEN_MINUS) {
			PARSER_NEXT; // Eating the minus token

			string_append_char(name, '-');
		}
		else {
			string_append_char(name, ' ');
		}

		if (match(lexer, TOKEN_IDENTIFIER)) {
			string_append_str(name, token_value_stringify(PARSER_CURRENT));
			*last_name = PARSER_CURRENT;

			PARSER_NEXT; // Eating the identifier token
		}
		else {
			error_parser(2, "Expected an identifier after the dash in the attribute name '%s' at line %d, column %d, but got %s", name->data, PARSER_CURRENT->location.end_line, PARSER_CURRENT->location.end_column, token_type_keyword(PARSER_CURRENT->type));
			break;
		}
	}

	return name;
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

	while (PARSER_CURRENT->type != TOKEN_TYPE_CLOSE_BLOCK) {
		if (match(lexer, TOKEN_IDENTIFIER) || match(lexer, TOKEN_PRINT)) {
			token_t* last_name = PARSER_CURRENT;
			string_t* name = parser_parse_layout_name(lexer, &last_name);

			if (enduser_name_to_ast_layout_node_type(name->data) != AST_LAYOUT_TYPE_ERROR) {
				parser_parse_layout_block_children(block, lexer, name->data, last_name);

				string_destroy(name);
			}
			else if (block->states != NULL && enduser_name_to_ast_layout_attribute_style_state_type(name->data) != AST_LAYOUT_ATTRIBUTE_STYLE_STATE_TYPE_ERROR) {
				parser_parse_layout_block_style_state(block, lexer, name->data, last_name);

				string_destroy(name);
			}
			else if (enduser_name_to_ast_layout_attribute_type(name->data) != AST_LAYOUT_ATTRIBUTE_TYPE_ERROR) {
				parser_parse_layout_block_attribute(false, block, block->styles->normal, lexer, name->data, last_name);

				string_destroy(name);
			}
			else {
				error_parser(2, "The '%s' is not a valid layout node, style state or attribute at line %d, column %d", name->data, last_name->location.end_line, last_name->location.end_column);
				return;
			}
		}
		else {
			error_parser(2, "Unknown token '%s' inside a layout block it should be name of an element or an attribute at line %d, column %d", token_type_keyword(PARSER_CURRENT->type), PARSER_CURRENT->location.end_line, PARSER_CURRENT->location.end_column);
		}
	}

	validate_layout_block(block);

	expect_close_block(lexer);
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
	ast_node_t* node = ast_node_create(AST_TYPE_LAYOUT, PARSER_CURRENT->location);

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
	ast_node_t* node = ast_node_create(AST_TYPE_FUNCTION, PARSER_CURRENT->location);

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
			error_parser(2, "Expected an expression at line %d, column %d, but got %s", PARSER_CURRENT->location.end_line, PARSER_CURRENT->location.end_column, token_type_keyword(PARSER_CURRENT->type));
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

	if (match(lexer, TOKEN_IDENTIFIER)) {
		PARSER_NEXT;

		type = ast_value_type_create(AST_TYPE_KIND_STRING, token->location);
		value = ast_value_create(type, token->data.string);


		return value;
	}
	else if (match(lexer, TOKEN_STRING)) {
		PARSER_NEXT;

		type = ast_value_type_create(AST_TYPE_KIND_STRING, token->location);
		value = ast_value_create(type, token->data.string);

		return value;
	}
	else if (match(lexer, TOKEN_NUMBER_INT)) {
		PARSER_NEXT;

		type = ast_value_type_create(AST_TYPE_KIND_INT, token->location);
		value = ast_value_create(type, NULL);
		value->data.int_value = token->data.number_int;

		return value;
	}
	else if (match(lexer, TOKEN_NUMBER_FLOAT)) {
		PARSER_NEXT;

		type = ast_value_type_create(AST_TYPE_KIND_STRING, token->location);
		value = ast_value_create(type, NULL);
		value->data.float_value = token->data.number_float;

		return value;
	}
	else if (match(lexer, TOKEN_BOOLEAN)) {
		PARSER_NEXT;

		type = ast_value_type_create(AST_TYPE_KIND_STRING, token->location);
		value = ast_value_create(type, NULL);
		value->data.bool_value = token->data.boolean;

		return value;
	}
	else {
		error_parser(2, "Expected an expression at line %d, column %d, but got %s", token->location.end_line, token->location.end_column, token_type_keyword(token->type));
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
	ast_node_t* node = ast_node_create(AST_TYPE_PRINT, PARSER_CURRENT->location);

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
	ast_node_t* node = ast_node_create(AST_TYPE_RETURN, PARSER_CURRENT->location);

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
	ast_node_t* node = ast_node_create(AST_TYPE_IF, PARSER_CURRENT->location);

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

				ast_node_t* else_if = ast_node_create(AST_TYPE_ELSE_IF, PARSER_CURRENT->location);

				else_if->data.ifclause = ast_else_create();
				parser_parse_block(lexer, else_if->data.ifclause->block);

				array_push(node->data.ifclause->else_blocks, else_if);

				break; // Stop the loop, this is the last else
			}
			// else if {}
			else if (match_next(lexer, TOKEN_IF)) {
				PARSER_NEXT; // Eat the else token

				ast_node_t* else_if = ast_node_create(AST_TYPE_IF, PARSER_CURRENT->location);
				PARSER_NEXT; // Eat the sub if token

				ast_value_t* condition = parser_parse_expression(lexer);
				else_if->data.ifclause = ast_elseif_create(condition);

				parser_parse_block(lexer, else_if->data.ifclause->block);

				array_push(node->data.ifclause->else_blocks, else_if);
			}
			else {
				error_parser(2, "Expected a block or an else if at line %d, column %d, but got %s", PARSER_CURRENT->location.end_line, PARSER_CURRENT->location.end_column, token_type_keyword(PARSER_CURRENT->type));
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
		error_parser(2, "Unknown token '%s' as statement at line %d, column %d", token_type_keyword(PARSER_CURRENT->type), PARSER_CURRENT->location.end_line, PARSER_CURRENT->location.end_column);
	}

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

	while (lexer->token_index < lexer->tokens->length) {
		if (PARSER_CURRENT->type == TOKEN_EOF) break;

		ast_node_t* node = parser_parse_node(lexer);

		if (node == NULL) {
			error_parser(2, "Expected a node at line %d, column %d, but got %s", PARSER_CURRENT->location.end_line, PARSER_CURRENT->location.end_column, token_type_keyword(PARSER_CURRENT->type));
			continue;
		}
		else if (node->type == AST_TYPE_LAYOUT) {
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
		else if (node->type == AST_TYPE_FUNCTION) {
			array_push(ast->functions, node->data.function);

			ast_node_destroy_notall(node);
		}
	}

	return ast;
}
