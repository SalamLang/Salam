#include "generator.h"

/**
 *
 * @function generator_create
 * @brief Create a generator
 * @params {ast_t*} ast - AST tree
 * @returns {generator_t*}
 *
 */
generator_t* generator_create(ast_t* ast)
{
	DEBUG_ME;
	if (ast == NULL) {
		error_generator(2, "AST tree is NULL and not correct!");
	}

	generator_t* generator = memory_allocate(sizeof(generator_t));
	generator->ast = ast;
	generator->html = string_create(4096);
	generator->css = string_create(4096);
	generator->media_css = string_create(512);
	generator->js = string_create(4096);

	generator->inlineCSS = false;
	generator->inlineJS = false;

	generator->identifier = malloc(sizeof(generator_identifier_t));

	generator_identifier_init(generator->identifier);

	return generator;
}

/**
 *
 * @function generator_destroy
 * @brief Destroy a generator
 * @params {generator_t*} generator
 * @returns {void}
 *
 */
void generator_destroy(generator_t* generator)
{
	DEBUG_ME;
	if (generator != NULL) {
		if (generator->html != NULL) {
			generator->html->destroy(generator->html);
		}

		if (generator->media_css != NULL) {
			generator->media_css->destroy(generator->media_css);
		}

		if (generator->css != NULL) {
			generator->css->destroy(generator->css);
		}

		if (generator->js != NULL) {
			generator->js->destroy(generator->js);
		}

		if (generator->identifier != NULL) {
			generator_identifier_destroy(generator->identifier);
		}

		memory_destroy(generator);
	}
}

/**
 *
 * @function generator_debug
 * @brief Debug the generator
 * @params {generator_t*} generator - Generator
 * @returns {void}
 *
 */
void generator_debug(generator_t* generator)
{
	DEBUG_ME;
	if (generator == NULL) {
		printf("generator is NULL\n");
		return;
	}
	else {
		printf("generator->html: ");
		if (generator->html != NULL) {
			generator->html->print(generator->html);
		}
		else {
			printf("NULL\n");
		}

		printf("generator->media_css: ");
		if (generator->media_css != NULL) {
			generator->media_css->print(generator->media_css);
		}
		else {
			printf("NULL\n");
		}

		printf("generator->css: ");
		if (generator->css != NULL) {
			generator->css->print(generator->css);
		}
		else {
			printf("NULL\n");
		}

		printf("generator->js: ");
		if (generator->js != NULL) {
			generator->js->print(generator->js);
		}
		else {
			printf("NULL\n");
		}
	}
}

/**
 *
 * @function generator_save
 * @brief Save the generator
 * @params {generator_t*} generator - Generator
 * @returns {void}
 *
 */
void generator_save(generator_t* generator, const char* html_output, const char* css_output, const char* js_output)
{
	DEBUG_ME;
	if (generator == NULL) {
		return;
	}
	else {
		if (generator->html != NULL) {
			file_writes(html_output, generator->html->data);
		}

		if (generator->css != NULL) {
			file_writes(css_output, generator->css->data);
			file_appends(css_output, generator->media_css->data);
		}

		if (generator->js != NULL) {
			file_writes(js_output, generator->js->data);
		}
	}
}

/**
 *
 * @function generator_code_node
 * @brief Generate the code for the node
 * @params {generator_t*} generator - Generator
 * @params {ast_node_t*} node - Node
 * @returns {string_t*} code - Code
 *
 */
string_t* generator_code_node(generator_t* generator, ast_node_t* node)
{
	DEBUG_ME;
	string_t* code = string_create(1024);

	switch (node->type) {
		case AST_TYPE_ERROR:
			break;

		case AST_TYPE_IMPORT:
			break;

		case AST_TYPE_LAYOUT:
			break;

		case AST_TYPE_BLOCK: {
			string_t* block_code = generator_code_block(generator, node->data.block);

			if (block_code != NULL) {
				string_append(code, block_code);

				block_code->destroy(block_code);
			}
		}
		break;

		case AST_TYPE_FUNCTION: {
			string_t* function_code = generator_code_function(generator, node->data.function);

			if (function_code != NULL) {
				string_append(code, function_code);

				function_code->destroy(function_code);
			}
		}
		break;

		case AST_TYPE_IF:
		case AST_TYPE_ELSE_IF: {
			string_t* if_code = generator_code_if(generator, node->data.ifclause);

			if (if_code != NULL) {
				string_append(code, if_code);

				if_code->destroy(if_code);
			}
		}
		break;

		case AST_TYPE_RETURN: {
			string_t* return_code = generator_code_return(generator, node->data.returns);

			if (return_code != NULL) {
				string_append(code, return_code);

				return_code->destroy(return_code);
			}
		}
		break;

		case AST_TYPE_PRINT: {
			string_t* print_code = generator_code_print(generator, node->data.print);

			if (print_code != NULL) {
				string_append(code, print_code);

				print_code->destroy(print_code);
			}
		}
		break;
	}

	return code;
}

/**
 *
 * @function generator_code_type
 * @brief Generate the code for the type
 * @params {generator_t*} generator - Generator
 * @params {ast_value_type_t*} type - Type
 * @returns {string_t*} code - Code
 *
 */
string_t* generator_code_type(generator_t* generator, ast_value_type_t* type)
{
	DEBUG_ME;
	string_t* code = string_create(1024);

	if (generator) {}

	switch (type->kind) {
		case AST_TYPE_KIND_VOID:
			string_append_str(code, "void");

			return code;

		case AST_TYPE_KIND_NULL:
			string_append_str(code, "null");
			
			return code;

		case AST_TYPE_KIND_INT:
			string_append_str(code, "int");

			return code;

		case AST_TYPE_KIND_FLOAT:
			string_append_str(code, "float");

			return code;

		case AST_TYPE_KIND_CHAR:
			string_append_str(code, "char");

			return code;

		case AST_TYPE_KIND_STRING:
			string_append_str(code, "string");
			
			return code;

		case AST_TYPE_KIND_BOOL:
			string_append_str(code, "bool");

			return code;

		case AST_TYPE_KIND_ARRAY:
			string_append_str(code, "array");

			return code;

		// case AST_TYPE_KIND_MAP:
		//     string_append_str(code, "map");
		// 
		//     return code;

		case AST_TYPE_KIND_STRUCT:
			string_append_str(code, "struct");

			return code;

		case AST_TYPE_KIND_ENUM:
			string_append_str(code, "enum");

			return code;

		// case AST_TYPE_KIND_UNION:
		//     string_append_str(code, "union");
		// 
		//     return code;

		case AST_TYPE_KIND_POINTER:
			string_append_str(code, "pointer");

			return code;

		case AST_TYPE_KIND_FUNCTION:
			string_append_str(code, "function");

			return code;
	}

	string_append_str(code, "unknown type");

	return code;
}

/**
 *
 * @function generator_code_value
 * @brief Generate the code for the value
 * @params {generator_t*} generator - Generator
 * @params {ast_value_t*} value - Value
 * @returns {string_t*} code - Code
 *
 */
string_t* generator_code_value(generator_t* generator, ast_value_t* value)
{
	DEBUG_ME;
	string_t* code = string_create(1024);

	if (generator) {}

	if (value == NULL) {
		error_generator(2, "Value is NULL in value statement");

		return code;
	}

	if (value->type != NULL) {
		switch (value->type->kind) {
			case AST_TYPE_KIND_VOID:
				string_append_str(code, "VOID");

				return code;

			case AST_TYPE_KIND_INT:
				string_append_str(code, int2string(value->data.int_value));

				return code;

			case AST_TYPE_KIND_FLOAT:
				string_append_str(code, float2string(value->data.float_value));

				return code;

			case AST_TYPE_KIND_CHAR:
				string_append_char(code, value->data.char_value);

				return code;

			case AST_TYPE_KIND_NULL:
				string_append_str(code, "NULL");

				return code;

			case AST_TYPE_KIND_STRING:
				string_append_char(code, '"');
				string_append_str(code, value->data.string_value);
				string_append_char(code, '"');

				return code;

			case AST_TYPE_KIND_BOOL:
				string_append_str(code, value->data.bool_value ? "true" : "false");

				return code;

			case AST_TYPE_KIND_STRUCT:
				string_append_str(code, "STRUCT");

				return code;

			case AST_TYPE_KIND_ENUM:
				string_append_str(code, "ENUM");

				return code;

			case AST_TYPE_KIND_POINTER:
				string_append_str(code, "POINTER");

				return code;

			case AST_TYPE_KIND_ARRAY:
				string_append_str(code, "ARRAY");

				return code;

			case AST_TYPE_KIND_FUNCTION:
				string_append_str(code, "FUNCTION");
				
				return code;
		}
	}

	string_append_str(code, "unknown value");

	return code;
}

/**
 *
 * @function generator_code_values
 * @brief Generate the code for the values
 * @params {generator_t*} generator - Generator
 * @params {array_t*} values - Values
 * @returns {string_t*} code - Code
 *
 */
string_t* generator_code_values(generator_t* generator, array_t* values)
{
	DEBUG_ME;
	string_t* code = string_create(1024);

	if (values != NULL) {
		for (size_t i = 0; i < values->length; i++) {
			ast_value_t* value = array_get(values, i);

			if (value != NULL) {
				string_t* value_code = generator_code_value(generator, value);

				if (value_code != NULL) {
					string_append(code, value_code);

					if (i != values->length - 1) {
						string_append_str(code, ", ");
					}

					value_code->destroy(value_code);
				}
			}
		}
	}

	return code;
}

/**
 *
 * @function generator_code_print
 * @brief Generate the code for the print
 * @params {generator_t*} generator - Generator
 * @params {ast_print_t*} print - Print
 * @returns {string_t*} code - Code
 *
 */
string_t* generator_code_print(generator_t* generator, ast_print_t* print)
{
	DEBUG_ME;
	string_t* code = string_create(1024);

	if (print->values == NULL) {
		error_generator(2, "Print values are NULL in print statement");
	}

	string_t* values_code = generator_code_values(generator, print->values);

	if (values_code == NULL) {
		error_generator(2, "Error generating code for print values in print statement");
	}

	// string_append_str(code, "print");
	string_append_str(code, "console.log");
	string_append_char(code, '(');
	string_append(code, values_code);
	string_append_char(code, ')');
	string_append_char(code, ';');
	string_append_char(code, '\n');

	values_code->destroy(values_code);

	return code;
}

/**
 *
 * @function generator_code_return
 * @brief Generate the code for the return
 * @params {generator_t*} generator - Generator
 * @params {ast_return_t*} returns - Returns
 * @returns {string_t*} code - Code
 *
 */
string_t* generator_code_return(generator_t* generator, ast_return_t* returns)
{
	DEBUG_ME;
	string_t* code = string_create(1024);

	if (returns->values == NULL) {
		error_generator(2, "Return values are NULL in return statement");
	}

	if (returns->values->length == 0) {
		string_append_str(code, "return;");
		string_append_char(code, '\n');
	}
	else {
		string_t* values_code = generator_code_values(generator, returns->values);

		if (values_code == NULL) {
			error_generator(2, "Error generating code for return values in return statement");
		}

		string_append_str(code, "return");
		string_append_char(code, '(');
		string_append(code, values_code);
		string_append_char(code, ')');
		string_append_char(code, ';');
		string_append_char(code, '\n');

		values_code->destroy(values_code);
	}

	return code;
}

/**
 *
 * @function generator_code_if
 * @brief Generate the code for the if clause
 * @params {generator_t*} generator - Generator
 * @params {ast_if_t*} ifclause - If clause
 * @returns {string_t*} code - Code
 *
 */
string_t* generator_code_if(generator_t* generator, ast_if_t* ifclause)
{
	DEBUG_ME;
	string_t* code = string_create(1024);

	if (ifclause->condition != NULL) {
		string_t* condition_code = generator_code_value(generator, ifclause->condition);

		if (condition_code == NULL) {
			error_generator(2, "Error generating code for if clause condition");
		}

		string_append_str(code, "if (");
		string_append(code, condition_code);
		string_append_str(code, ") ");

		if (condition_code != NULL) {
			condition_code->destroy(condition_code);
		}
	}

	string_t* block_code = generator_code_block(generator, ifclause->block);

	if (block_code != NULL) {
		string_append(code, block_code);

		block_code->destroy(block_code);
	}

	if (ifclause->else_blocks != NULL) {
		size_t elseif_length = ifclause->else_blocks->length;

		if (elseif_length > 0) {
			for (size_t i = 0; i < elseif_length; i++) {
				ast_if_t* else_if = array_get(ifclause->else_blocks, i);

				string_t* else_if_code = generator_code_if(generator, else_if);

				if (else_if_code != NULL) {
					if (i != elseif_length - 1) {
						string_append_str(code, "else ");
					}

					string_append(code, else_if_code);

					else_if_code->destroy(else_if_code);
				}
			}
		}
	}

	return code;
}

/**
 *
 * @function generator_code_block
 * @brief Generate the code for the block
 * @params {generator_t*} generator - Generator
 * @params {ast_block_t*} block - Block
 * @returns {string_t*} code - Code
 *
 */
string_t* generator_code_block(generator_t* generator, ast_block_t* block)
{
	DEBUG_ME;
	string_t* code = string_create(1024);

	string_append_char(code, '{');
	string_append_char(code, '\n');

	if (block != NULL) {
		for (size_t i = 0; i < block->children->length; i++) {
			ast_node_t* node = array_get(block->children, i);

			if (node != NULL) {
				string_t* node_code = generator_code_node(generator, node);

				if (node_code != NULL) {
					string_append(code, node_code);

					node_code->destroy(node_code);
				}
			}
		}
	}

	string_append_char(code, '}');
	string_append_char(code, '\n');

	return code;
}

/**
 *
 * @function generator_code_function
 * @brief Generate the code for the function
 * @params {generator_t*} generator - Generator
 * @params {ast_function_t*} function - Function
 * @returns {string_t*} code - Code
 *
 */
string_t* generator_code_function(generator_t* generator, ast_function_t* function)
{
	DEBUG_ME;
	string_t* code = string_create(1024);

	string_append_str(code, "function ");
	string_append_str(code, function->name);
	string_append_char(code, '(');
	string_append_char(code, ')');

	string_t* code_block = generator_code_block(generator, function->block);
	string_append(code, code_block);
	code_block->destroy(code_block);

	return code;
}

/**
 *
 * @function generator_code_functions
 * @brief Generate the code for the functions
 * @params {generator_t*} generator - Generator
 * @returns {void}
 *
 */
void generator_code_functions(generator_t* generator)
{
	DEBUG_ME;
	if (generator != NULL && generator->ast != NULL && generator->ast->functions != NULL && generator->ast->functions->data != NULL && generator->ast->functions->length > 0) {
		for (size_t i = 0; i < generator->ast->functions->length; i++) {
			ast_function_t* function = array_get(generator->ast->functions, i);

			if (function != NULL) {
				string_t* function_code = generator_code_function(generator, function);

				if (function_code != NULL) {
					string_append(generator->js, function_code);

					function_code->destroy(function_code);
				}
			}
		}
	}
}

/**
 *
 * @function generator_code
 * @brief Generate code from AST
 * @params {generator_t*} generator - Generator
 * @returns {void}
 *
 */
void generator_code(generator_t* generator)
{
	DEBUG_ME;
	if (generator->ast == NULL) {
		error_generator(2, "AST tree is NULL and is not valid!");
	}

	if (generator->ast->functions != NULL) {
		generator_code_functions(generator);
	}

	generator_code_layout(generator);
}
