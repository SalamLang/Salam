#include "parser_layout.h"

/**
 *
 * @function parser_parse_layout
 * @brief Parse the layout
 * @params {lexer_t*} lexer - Lexer
 * @returns {ast_node_t*} - AST node
 *
 */
ast_node_t *parser_parse_layout(lexer_t *lexer)
{
	DEBUG_ME;
	ast_node_t *node = ast_node_create(AST_TYPE_LAYOUT, PARSER_CURRENT->location);

	PARSER_NEXT; // Eat the layout token

	node->data.layout = ast_layout_create();

	parser_parse_layout_block(node->data.layout->block, lexer);

	return node;
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
void parser_parse_layout_block(ast_layout_block_t *block, lexer_t *lexer)
{
	DEBUG_ME;
	expect_open_block(lexer);

	while (PARSER_CURRENT->type != TOKEN_TYPE_CLOSE_BLOCK)
	{
		if (match(lexer, TOKEN_IDENTIFIER) || match(lexer, TOKEN_PRINT))
		{
			token_t *last_name = PARSER_CURRENT;
			string_t *name = parser_parse_layout_name(lexer, &last_name);

			if (enduser_name_to_ast_layout_node_type(name->data) != AST_LAYOUT_TYPE_ERROR)
			{
				parser_parse_layout_block_children(block, lexer, name->data, last_name);

				string_destroy(name);
			}
			else if (block->states != NULL && enduser_name_to_ast_layout_attribute_style_state_type(name->data) != AST_LAYOUT_ATTRIBUTE_STYLE_STATE_TYPE_ERROR)
			{
				parser_parse_layout_block_style_state(block, lexer, name->data, last_name);

				string_destroy(name);
			}
			else if (enduser_name_to_ast_layout_attribute_type(name->data) != AST_LAYOUT_ATTRIBUTE_TYPE_ERROR)
			{
				parser_parse_layout_block_attribute(false, block, block->styles->normal, lexer, name->data, last_name);

				string_destroy(name);
			}
			else
			{
				error_parser(2, "The '%s' is not a valid layout node, style state or attribute at line %d, column %d", name->data, last_name->location.end_line, last_name->location.end_column);
				return;
			}
		}
		else
		{
			error_parser(2, "Unknown token '%s' inside a layout block it should be name of an element or an attribute at line %d, column %d", token_type_keyword(PARSER_CURRENT->type), PARSER_CURRENT->location.end_line, PARSER_CURRENT->location.end_column);
		}
	}

	validate_layout_block(block);

	expect_close_block(lexer);
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
string_t *parser_parse_layout_name(lexer_t *lexer, token_t **last_name)
{
	DEBUG_ME;
	string_t *name = string_create(16);

	string_append_str(name, token_value_stringify(PARSER_CURRENT));

	PARSER_NEXT; // Eating the identifier token

	while (match(lexer, TOKEN_MINUS) || match(lexer, TOKEN_IDENTIFIER))
	{
		if (PARSER_CURRENT->type == TOKEN_MINUS)
		{
			PARSER_NEXT; // Eating the minus token

			string_append_char(name, '-');
		}
		else
		{
			string_append_char(name, ' ');
		}

		if (match(lexer, TOKEN_IDENTIFIER))
		{
			string_append_str(name, token_value_stringify(PARSER_CURRENT));
			*last_name = PARSER_CURRENT;

			PARSER_NEXT; // Eating the identifier token
		}
		else
		{
			error_parser(2, "Expected an identifier after the dash in the attribute name '%s' at line %d, column %d, but got %s", name->data, PARSER_CURRENT->location.end_line, PARSER_CURRENT->location.end_column, token_type_keyword(PARSER_CURRENT->type));
			break;
		}
	}

	return name;
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
void parser_parse_layout_block_children(ast_layout_block_t *block, lexer_t *lexer, char *name, token_t *last_name)
{
	DEBUG_ME;
	ast_layout_node_t *node = parser_parse_layout_node(lexer, name, last_name);

	if (node != NULL)
	{
		if (node->type == AST_LAYOUT_TYPE_FONT)
		{
			if (block->parent_node_type != AST_LAYOUT_TYPE_NONE)
			{
				error_parser(2, "Font node is not allowed in the '%s' block at line %d, column %d", ast_layout_node_type_to_enduser_name(block->parent_node_type), last_name->location.end_line, last_name->location.end_column);
			}

			array_push(block->meta_children, node);
		}
		else if (node->type == AST_LAYOUT_TYPE_MEDIA)
		{
			array_push(block->meta_children, node);
		}
		else
		{
			array_push(block->children, node);
		}
	}
	else
	{
		error_parser(2, "Expected a layout node at line %d, column %d, but got %s", PARSER_CURRENT->location.end_line, PARSER_CURRENT->location.end_column, token_type_keyword(PARSER_CURRENT->type));
	}
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
void parser_parse_layout_block_attribute(bool onlyStyle, ast_layout_block_t *block, hashmap_t *normal, lexer_t *lexer, char *name, token_t *last_name)
{
	DEBUG_ME;
	token_t *first_value = PARSER_CURRENT; // TODO

	if (match(lexer, TOKEN_TYPE_OPEN_BLOCK))
	{
		error_parser(2, "Nonsupported layout element '%s' at line %d, column %d", name, last_name->location.end_line, last_name->location.end_column);
	}
	expect(lexer, TOKEN_ASSIGN);

	array_value_t *values = parser_parse_layout_values(lexer);

	ast_layout_attribute_type_t attribute_key_type = token_to_ast_layout_attribute_type(name, last_name, block->parent_node_type);

	ast_layout_attribute_t *attribute = ast_layout_attribute_create(attribute_key_type, name, values, block->parent_node_type, PARSER_CURRENT->location, first_value->location);
	if (!token_belongs_to_ast_layout_node(attribute_key_type, attribute))
	{
		attribute->destroy(attribute);

		error_parser(2, "Attribute '%s' does not belong to node '%s' at line %d, column %d", name, ast_layout_node_type_to_enduser_name(block->parent_node_type), last_name->location.end_line, last_name->location.end_column);
	}

	char *attribute_key_name = ast_layout_attribute_type_to_name(attribute_key_type);

	if (is_style_attribute(attribute_key_type))
	{
		if (hashmap_has(normal, attribute_key_name))
		{
			attribute->destroy(attribute);

			error_parser(2, "Style attribute '%s' already defined in the '%s' block at line %d, column %d", attribute_key_name, ast_layout_node_type_to_enduser_name(block->parent_node_type), last_name->location.end_line, last_name->location.end_column);
		}
		else
		{
			hashmap_put(normal, attribute_key_name, attribute);
		}
	}
	else if (onlyStyle != true)
	{
		if (hashmap_has(block->attributes, attribute_key_name))
		{
			attribute->destroy(attribute);

			error_parser(2, "Attribute '%s' already defined in the '%s' block at line %d, column %d", attribute_key_name, ast_layout_node_type_to_enduser_name(block->parent_node_type), last_name->location.end_line, last_name->location.end_column);
		}
		else
		{
			hashmap_put(block->attributes, attribute_key_name, attribute);
		}
	}
	else
	{
		attribute->destroy(attribute);

		error_parser(2, "Attribute '%s' is not a style attribute at line %d, column %d", attribute_key_name, last_name->location.end_line, last_name->location.end_column);
	}
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
ast_layout_node_t *parser_parse_layout_node(lexer_t *lexer, char *name, token_t *last_name)
{
	DEBUG_ME;
	ast_layout_node_type_t type = enduser_name_to_ast_layout_node_type(name);

	if (type == AST_LAYOUT_TYPE_ERROR)
	{
		error_ast(2, "Unknown layout node '%s' at line %d, column %d", name, last_name->location.end_line, last_name->location.end_column);
	}

	ast_layout_node_t *node = ast_layout_node_create(type);

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
void parser_parse_layout_block_style_state(ast_layout_block_t *block, lexer_t *lexer, char *name, token_t *last_name)
{
	DEBUG_ME;
	ast_layout_attribute_style_state_type style_state_type = enduser_name_to_ast_layout_attribute_style_state_type(name);

	if (style_state_type == AST_LAYOUT_ATTRIBUTE_STYLE_STATE_TYPE_ERROR)
	{
		error_parser(2, "Unknown style state '%s' at line %d, column %d", name, last_name->location.end_line, last_name->location.end_column);
	}

	expect_open_block(lexer);

	if (hashmap_has(block->states, name) == true)
	{
		error_parser(2, "Style state '%s' already defined in the '%s' block at line %d, column %d", name, ast_layout_node_type_to_enduser_name(block->parent_node_type), last_name->location.end_line, last_name->location.end_column);
	}

	ast_layout_style_state_t *state_styles = ast_layout_style_state_create();

	while (PARSER_CURRENT->type != TOKEN_TYPE_CLOSE_BLOCK)
	{
		token_t *last_name2 = PARSER_CURRENT;
		string_t *name2 = parser_parse_layout_name(lexer, &last_name2);

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
array_value_t *parser_parse_layout_values(lexer_t *lexer)
{
	array_value_t *values = array_value_create(1);

	ast_value_t *value = parser_parse_layout_value(lexer);

	array_push(values, value);

	if (match(lexer, TOKEN_COMMA))
	{
		while (match(lexer, TOKEN_COMMA))
		{
			PARSER_NEXT; // Eating the comma token

			ast_value_t *new_value = parser_parse_layout_value(lexer);

			if (new_value != NULL)
			{
				array_push(values, new_value);
			}
		}
	}

	return values;
}

/**
 *
 * @function parser_parse_layout_value
 * @brief Parse the layout value
 * @params {lexer_t*} lexer - Lexer
 * @returns {ast_value_t*} - AST Layout attribute value
 *
 */
ast_value_t *parser_parse_layout_value(lexer_t *lexer)
{
	DEBUG_ME;
	return parser_parse_value(lexer);
}
