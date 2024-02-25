#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <locale.h>
#include <stdbool.h>
#include <string.h>
#include <limits.h>

typedef struct {
} interpreter_state_t;

typedef enum {
	// values
	TOKEN_TYPE_IDENTIFIER,
	TOKEN_TYPE_NUMBER,
	TOKEN_TYPE_STRING,

	// keywords
	TOKEN_TYPE_FUNCTION,
	TOKEN_TYPE_RETURN,
	TOKEN_TYPE_PRINT,
	TOKEN_TYPE_IF,
	TOKEN_TYPE_ELSEIF,
	TOKEN_TYPE_AND,
	TOKEN_TYPE_OR,

	// symbols
	TOKEN_TYPE_SECTION_OPEN,
	TOKEN_TYPE_SECTION_CLOSE,
	TOKEN_TYPE_PARENTHESE_OPEN,
	TOKEN_TYPE_PARENTHESE_CLOSE,

	TOKEN_TYPE_PLUS,
	TOKEN_TYPE_MINUS,

	// others
	TOKEN_TYPE_EOF,
	TOKEN_TYPE_ERROR,
} token_type_t;

typedef struct {
	const char* keyword;
	token_type_t token_type;
} keyword_mapping_t;

keyword_mapping_t keyword_mapping[] = {
	{"عملکرد", TOKEN_TYPE_FUNCTION},
	{"برگشت", TOKEN_TYPE_RETURN},
	{"نمایش", TOKEN_TYPE_PRINT},
	{"واگرنه", TOKEN_TYPE_ELSEIF},
	{"اگر", TOKEN_TYPE_IF},
	{"و", TOKEN_TYPE_AND},
	{"یا", TOKEN_TYPE_OR},
	{NULL, TOKEN_TYPE_ERROR}
};

typedef struct {
	token_type_t type;
	char* value;
	struct {
		size_t length;
		size_t line;
		size_t column;
		size_t end_line;
		size_t end_column;
	} location;
} token_t;

typedef struct {
	size_t size;
	size_t length;
	void** data;
} array_t;

typedef struct {
	char* data;
	int index;
	int line;
	int column;

	array_t* tokens;
	int last_char_size;
} lexer_t;

struct ast_node;

typedef struct {
	char* name;
	struct ast_node* body;
} ast_function_declaration_t;

typedef struct {
	struct ast_node* expression;
} ast_statement_return_t;

typedef struct {
	struct ast_node* expression;
} ast_statement_print_t;

typedef struct {
	struct ast_node** statements;
	size_t num_statements;
} ast_block_t;

struct ast_statement_if_t;

typedef struct {
	struct ast_node* expression;
	ast_block_t* block;
	struct ast_statement_if_t* next;
} ast_statement_if_t;

typedef struct ast_literal {
	token_type_t literal_type;
	char* value;
} ast_literal_t;

typedef struct ast_identifier {
	char* name;
} ast_identifier_t;

typedef struct ast_binary_op {
	char* operator;
	struct ast_expression* left;
	struct ast_expression* right;
} ast_binary_op_t;

typedef enum {
	AST_EXPRESSION_LITERAL,
	AST_EXPRESSION_IDENTIFIER,
	AST_EXPRESSION_BINARY_OP,
} ast_expression_type_t;

typedef struct ast_expression {
	ast_expression_type_t type;

	union {
		ast_literal_t* literal;
		ast_identifier_t* identifier;
		ast_binary_op_t* binary_op;
	} data;
} ast_expression_t;

typedef enum {
	AST_FUNCTION_DECLARATION,
	AST_STATEMENT_RETURN,
	AST_STATEMENT_PRINT,
	AST_STATEMENT_IF,
	AST_STATEMENT_ELSEIF,
	AST_BLOCK,
	AST_EXPRESSION
} ast_node_type_t;

typedef struct ast_node {
	ast_node_type_t type;
	union {
		ast_function_declaration_t* function_declaration;
		ast_statement_return_t* statement_return;
		ast_statement_print_t* statement_print;
		ast_block_t* block;
		ast_expression_t* expression;
	} data;
} ast_node_t;

typedef struct {
	char* name;
	ast_expression_t* expression;
} ast_variable_declaration_t;

typedef struct {
	lexer_t* lexer;
	size_t token_index;
	ast_node_t* ast_tree;
} parser_t;

enum {
	PRECEDENCE_LOWEST,
	PRECEDENCE_SUM,       // +
	PRECEDENCE_DIFFERENCE, // -
};

wchar_t read_token(lexer_t* lexer);
void read_number(lexer_t* lexer, wchar_t ch);
size_t mb_strlen(char* identifier);
void read_identifier(lexer_t* lexer, wchar_t ch);
char* wchar_to_char(wchar_t wide_char);
size_t wchar_length(wchar_t wide_char);

lexer_t* lexer_create(const char* data);
void lexer_free(lexer_t* lexer);
void lexer_lex(lexer_t* lexer);

parser_t* parser_create(lexer_t* lexer);
void parser_free(parser_t* parser);
void parser_parse(parser_t* parser);
ast_node_t* parser_function(parser_t* parser);
ast_node_t* parser_block(parser_t* parser);
ast_node_t* parser_statement(parser_t* parser);
ast_node_t* parser_statement_return(parser_t* parser);
ast_node_t* parser_statement_print(parser_t* parser);
ast_node_t* parser_expression(parser_t* parser);

int evaluate_expression(ast_expression_t* expr, interpreter_state_t* state);
int evaluate_binary_operation(ast_binary_op_t* expr, interpreter_state_t* state);
int evaluate_literal(ast_literal_t* expr);
int evaluate_identifier(ast_identifier_t* expr, interpreter_state_t* state);

void interpret_statement_print(ast_statement_print_t* stmt, interpreter_state_t* state);
void interpret_statement_return(ast_statement_return_t* stmt, interpreter_state_t* state);
void interpret_function_declaration(ast_function_declaration_t* stmt, interpreter_state_t* state);
void interpret_block(ast_node_t* node, interpreter_state_t* state);

typedef ast_expression_t* (*nud_func_t)(parser_t* parser, token_t* token);
typedef ast_expression_t* (*led_func_t)(parser_t* parser, token_t* token, ast_expression_t* left);

typedef struct {
	int precedence;
	nud_func_t nud;
	led_func_t led;
} token_info_t;

ast_expression_t* nud_number(parser_t* parser, token_t* token);
ast_expression_t* nud_string(parser_t* parser, token_t* token);
ast_expression_t* nud_identifier(parser_t* parser, token_t* token);
ast_expression_t* nud_parentheses(parser_t* parser, token_t* token);
ast_expression_t* led_plus_minus(parser_t* parser, token_t* token, ast_expression_t* left);

ast_expression_t* parser_expression_pratt(parser_t* parser, int precedence);

token_info_t token_infos[] = {
	[TOKEN_TYPE_NUMBER] = {PRECEDENCE_LOWEST, nud_number, NULL},
	[TOKEN_TYPE_STRING] = {PRECEDENCE_LOWEST, nud_string, NULL},
	[TOKEN_TYPE_IDENTIFIER] = {PRECEDENCE_LOWEST, nud_identifier, NULL},
	[TOKEN_TYPE_PARENTHESE_OPEN] = {PRECEDENCE_LOWEST, nud_parentheses, NULL},
	[TOKEN_TYPE_PLUS] = {PRECEDENCE_SUM, NULL, led_plus_minus},
	[TOKEN_TYPE_MINUS] = {PRECEDENCE_DIFFERENCE, NULL, led_plus_minus},
};

char* token_op_type2str(ast_expression_type_t type)
{
	switch (type) {
		case AST_EXPRESSION_LITERAL: return "LITERAL";
		case AST_EXPRESSION_IDENTIFIER: return "IDENTIFIER";
		case AST_EXPRESSION_BINARY_OP: return "BINARY_OP";
		default: return "OP_UNKNOWN";
	}
}

char* token_type2str(token_type_t type)
{
	switch(type) {
		case TOKEN_TYPE_IDENTIFIER: return "IDENTIFIER";
		case TOKEN_TYPE_NUMBER: return "NUMBER";
		case TOKEN_TYPE_STRING: return "STRING";
		case TOKEN_TYPE_FUNCTION: return "FUNCTION";
		case TOKEN_TYPE_RETURN: return "RETURN";
		case TOKEN_TYPE_PRINT: return "PRINT";
		case TOKEN_TYPE_IF: return "IF";
		case TOKEN_TYPE_ELSEIF: return "ELSEIF";
		case TOKEN_TYPE_OR: return "OR";
		case TOKEN_TYPE_AND: return "AND";
		case TOKEN_TYPE_SECTION_OPEN: return "SECTION_OPEN";
		case TOKEN_TYPE_SECTION_CLOSE: return "SECTION_CLOSE";
		case TOKEN_TYPE_PARENTHESE_OPEN: return "PARENTHESIS_OPEN";
		case TOKEN_TYPE_PARENTHESE_CLOSE: return "PARENTHESIS_CLOSE";
		case TOKEN_TYPE_PLUS: return "PLUS";
		case TOKEN_TYPE_MINUS: return "MINUS";
		case TOKEN_TYPE_EOF: return "EOF";
		case TOKEN_TYPE_ERROR: return "ERROR";
		default: return "TOK_UNKNOWN";
	}
}

char* file_read(char* file_Name)
{
	FILE* file = fopen(file_Name, "r");
	if (file == NULL) {
		printf("Error: file not found\n");
		return NULL;
	}

	fseek(file, 0, SEEK_END);
	long file_size = ftell(file);
	fseek(file, 0, SEEK_SET);

	char* file_data = (char*)malloc(file_size + 1);
	fread(file_data, 1, file_size, file);
	file_data[file_size] = 0;

	fclose(file);
	return file_data;
}

token_t* token_create(token_type_t type, const char* value, int a, int b, int c, int b2, int c2)
{
	token_t* t = malloc(sizeof(token_t));
	t->type = type;
	t->value = strdup(value);
	t->location.length = a;
	t->location.line = b;
	t->location.column = c;
	t->location.end_line = b2;
	t->location.end_column = c2;

	return t;
}

array_t* array_create(size_t size)
{
	size_t min_size = 1;

	array_t* arr = malloc(sizeof(array_t));
	arr->length = 0;
	arr->size = size > min_size ? size : min_size;
	arr->data = malloc(sizeof(void*) * arr->size);
	return arr;
}

void array_push(array_t* arr, void* data)
{
	if (arr->length >= arr->size) {
		size_t new_size = arr->size * 2;
		arr->data = realloc(arr->data, sizeof(void*) * new_size);
		arr->size = new_size;
	}

	arr->data[arr->length++] = data;
}

void array_free(array_t* arr)
{
	free(arr->data);
	free(arr);
}

void token_print(token_t* t)
{
	printf("%d ", t->type);
	// printf("...\n");
	// printf("%zu - ", t->location.length);
	printf("%s - ", token_type2str(t->type));
	printf("%s\n", t->value);
}

void array_print(array_t* arr)
{
	// printf("Array Length: %zu\n", arr->length);
	// printf("Array Size: %zu\n", arr->size);
	// printf("Array Contents:\n");

	for (size_t i = 0; i < arr->length; i++) {
		token_t* t = arr->data[i];
		printf("[%zu]: ", i);
		token_print(t);
	}
}

lexer_t* lexer_create(const char* data)
{
	lexer_t* lexer = (lexer_t*)malloc(sizeof(lexer_t));
	lexer->data = (char*) data;
	lexer->index = 0;
	lexer->tokens = array_create(10);

	return lexer;
}

void lexer_free(lexer_t* lexer)
{
	array_free(lexer->tokens);
	free(lexer);
}

bool is_number(wchar_t ch)
{
	return ch >= L'۰' && ch <= L'۹';
}

bool is_alpha(wchar_t ch)
{
	return ch >= L'آ' && ch <= L'ی' || ch == L'_';
}

bool is_ident(wchar_t ch)
{
	return is_alpha(ch) || is_number(ch);
}

wchar_t read_token(lexer_t* lexer)
{
	wchar_t current_char;
	int char_size = mbtowc(&current_char, &lexer->data[lexer->index], MB_CUR_MAX);
	if (char_size < 0) {
		printf("Syntax Error: invalid unicode character\n");
		exit(EXIT_FAILURE);
		return 0;
	}

	if (current_char == '\n') {
		lexer->line++;
		lexer->column = 0;
	} else {
		lexer->column += char_size;
	}

	lexer->index += char_size;
	lexer->last_char_size = char_size;

	return current_char;
}

wchar_t unread_token(lexer_t* lexer)
{
	lexer->index -= lexer->last_char_size;
	lexer->column -= lexer->last_char_size;

	wchar_t current_char;
	int char_size = mbtowc(&current_char, &lexer->data[lexer->index], MB_CUR_MAX);
	if (char_size < 0) {
		printf("Syntax Error: invalid unicode character\n");
		exit(EXIT_FAILURE);
		return 0;
	}

	return current_char;
}

void read_number(lexer_t* lexer, wchar_t ch)
{
	char* number = (char*)malloc(sizeof(char) * 20);
	int i = 0;
	while (is_number(ch)) {
		number[i++] = ch - L'۰' + '0';
		ch = read_token(lexer);
	}
	number[i] = 0;

	size_t length = strlen(number);
	token_t* t = token_create(TOKEN_TYPE_NUMBER, number, length, lexer->line, lexer->column - length, lexer->line, lexer->column);
	array_push(lexer->tokens, t);

	unread_token(lexer);
}

void read_string(lexer_t* lexer, wchar_t ch)
{
	char* string = (char*)malloc(sizeof(char) * 1024);
	int i = 0;
	while (ch != L'"') {
		int char_size = wctomb(&string[i], ch);
		if (char_size < 0) {
			printf("Error: Failed to convert wide character to multibyte\n");
			exit(EXIT_FAILURE);
		}
		i += char_size;
		ch = read_token(lexer);
	}
	string[i] = 0;

	size_t length = strlen(string);
	token_t* t = token_create(TOKEN_TYPE_STRING, string, length, lexer->line, lexer->column - length, lexer->line, lexer->column);
	array_push(lexer->tokens, t);

	unread_token(lexer);
}

size_t mb_strlen(char* identifier)
{
	size_t wcs_len = mbstowcs(NULL, identifier, 0);
	if (wcs_len == (size_t)-1) {
		perror("Error in mbstowcs");
		exit(EXIT_FAILURE);
	}

	return wcs_len;
}

void read_identifier(lexer_t* lexer, wchar_t ch)
{
	char identifier[256];
	int i = 0;
	while (is_ident(ch)) {
		int char_size = wctomb(&identifier[i], ch);
		if (char_size < 0) {
			printf("Error: Failed to convert wide character to multibyte\n");
			exit(EXIT_FAILURE);
		}
		i += char_size;
		ch = read_token(lexer);
	}
	identifier[i] = 0;

	int mapping_index = 0;
	token_type_t type = TOKEN_TYPE_IDENTIFIER;
	while (keyword_mapping[mapping_index].keyword != NULL) {
		if (strcmp(identifier, keyword_mapping[mapping_index].keyword) == 0) {
			type = keyword_mapping[mapping_index].token_type;
			break;
		}
		mapping_index++;
	}

	size_t length = mb_strlen(identifier);
	token_t* t = token_create(type, identifier, length, lexer->line, lexer->column - length, lexer->line, lexer->column);
	array_push(lexer->tokens, t);

	unread_token(lexer);
}

char* wchar_to_char(wchar_t wide_char)
{
	char* mb_char = (char*)malloc(6);
	if (wcstombs(mb_char, &wide_char, 6) == (size_t)-1) {
		perror("Error in wcstombs");
		exit(EXIT_FAILURE);
	}

	return mb_char;
}

size_t wchar_length(wchar_t wide_char)
{
	char mb_char[MB_LEN_MAX];
	if (wcrtomb(mb_char, wide_char, NULL) == (size_t)-1) {
		perror("Error in wcrtomb");
		return 0;
	}

	return mbrlen(mb_char, MB_LEN_MAX, NULL);
}

void lexer_lex(lexer_t* lexer)
{
	while (lexer->data[lexer->index] != 0) {
		if (lexer->data[lexer->index] == '\a' || lexer->data[lexer->index] == '\r') {
			lexer->column++;
			lexer->index++;
			continue;
		} else if (lexer->data[lexer->index] == ' ' || lexer->data[lexer->index] == '\t') {
			lexer->column++;
			lexer->index++;
			continue;
		} else if (lexer->data[lexer->index] == '\n') {
			lexer->index++;
			lexer->line++;
			lexer->column = 0;
			continue;
		}

		wchar_t current_char = read_token(lexer);
		if (current_char == L'\u200C') {
			printf("Ignoring ZWNJ character at line %d, column %d\n", lexer->line, lexer->column - 1);
			lexer->index++;
			lexer->column++;
			continue;
		} else if (current_char == '{') {
			token_t* t = token_create(TOKEN_TYPE_SECTION_OPEN, "{", 1, lexer->line, lexer->column - 1, lexer->line, lexer->column);
			array_push(lexer->tokens, t);
		} else if (current_char == '}') {
			token_t* t = token_create(TOKEN_TYPE_SECTION_CLOSE, "}", 1, lexer->line, lexer->column - 1, lexer->line, lexer->column);
			array_push(lexer->tokens, t);
		} else if (current_char == '(') {
			token_t* t = token_create(TOKEN_TYPE_PARENTHESE_OPEN, "(", 1, lexer->line, lexer->column - 1, lexer->line, lexer->column);
			array_push(lexer->tokens, t);
		} else if (current_char == ')') {
			token_t* t = token_create(TOKEN_TYPE_PARENTHESE_CLOSE, ")", 1, lexer->line, lexer->column - 1, lexer->line, lexer->column);
			array_push(lexer->tokens, t);
		} else if (current_char == '+') {
			token_t* t = token_create(TOKEN_TYPE_PLUS, "+", 1, lexer->line, lexer->column - 1, lexer->line, lexer->column);
			array_push(lexer->tokens, t);
		} else if (current_char == '-') {
			token_t* t = token_create(TOKEN_TYPE_MINUS, "-", 1, lexer->line, lexer->column - 1, lexer->line, lexer->column);
			array_push(lexer->tokens, t);
		} else if (current_char == '\"') {
			current_char = read_token(lexer);
			read_string(lexer, current_char);
		} else if (is_number(current_char)) {
			read_number(lexer, current_char);
		} else if (is_alpha(current_char)) {
			read_identifier(lexer, current_char);
		} else {
			printf("Error: Unexpected character '%s' at line %d, column %d\n", wchar_to_char(current_char), lexer->line, lexer->column - 1);
			size_t length = wchar_length(current_char);
			token_t* t = token_create(TOKEN_TYPE_ERROR, wchar_to_char(current_char), length, lexer->line, lexer->column - length, lexer->line, lexer->column);
			array_push(lexer->tokens, t);
		}
	}

	if (lexer->data[lexer->index] == 0) {
		token_t* t = token_create(TOKEN_TYPE_EOF, "\0", 1, lexer->line, lexer->column - 1, lexer->line, lexer->column);
		array_push(lexer->tokens, t);
	}
}

void help()
{
	printf("Welcome to Sallam Programming Language!\n");
	printf("Sallam is the first Persian/Iranian computer scripting language.\n");
	printf("\n");

	printf("Usage:\n");
	printf("  sallam <filename>\t\t\t# Execute a Sallam script\n");
	printf("\n");

	printf("Example:\n");
	printf("  sallam my_script.sallam\t\t# Run the Sallam script 'my_script.sallam'\n");
	printf("\n");

	printf("Feel free to explore and create using Sallam!\n");
	printf("For more information, visit: https://sallam-lang.js.org\n");
	printf("\n");
}

parser_t* parser_create(lexer_t* lexer)
{
	parser_t* parser = malloc(sizeof(parser_t));
	parser->lexer = lexer;
	parser->token_index = 0;
	parser->ast_tree = NULL;

	return parser;
}

void ast_expression_free(ast_expression_t* expr)
{
	switch (expr->type) {
		case AST_EXPRESSION_LITERAL:
			free(expr->data.literal);
			break;
		case AST_EXPRESSION_IDENTIFIER:
			free(expr->data.identifier);
			break;
		case AST_EXPRESSION_BINARY_OP:
			free(expr->data.binary_op->operator);
			ast_expression_free(expr->data.binary_op->left);
			ast_expression_free(expr->data.binary_op->right);
			break;
	}
}

void ast_node_free(ast_node_t* node)
{
	if (node == NULL) {
		return;
	}

	switch (node->type) {
		case AST_FUNCTION_DECLARATION:
			free(node->data.function_declaration->name);
			ast_node_free((ast_node_t*) node->data.function_declaration->body);
			break;
		case AST_STATEMENT_RETURN:
			ast_node_free((ast_node_t*) node->data.statement_return->expression);
			break;
		case AST_BLOCK:
			for (size_t i = 0; i < node->data.block->num_statements; i++) {
				ast_node_free((ast_node_t*) node->data.block->statements[i]);
			}
			free(node->data.block->statements);
			break;
		case AST_EXPRESSION:
			ast_expression_free(node->data.expression);
			break;
	}

	free(node);
}

void parser_free(parser_t* parser)
{
	ast_node_free(parser->ast_tree);
	lexer_free(parser->lexer);
	free(parser);
}

void parser_token_next(parser_t* parser)
{
	if (parser->lexer->tokens->length > parser->token_index) {
		parser->token_index++;
	} else {
		printf("Error: Unexpected end of file\n");
		exit(EXIT_FAILURE);
	}
}

token_t* parser_token_skip(parser_t* parser, token_type_t type)
{
	if (parser->lexer->tokens->length > parser->token_index) {
		token_t* token = (token_t*)parser->lexer->tokens->data[parser->token_index];

		if (token->type == type) {
			return (token_t*)parser->lexer->tokens->data[parser->token_index++];
		}
	}

	return NULL;
}

token_t* parser_token_eat(parser_t* parser, token_type_t type)
{
	if (parser->lexer->tokens->length > parser->token_index) {
		token_t* token = (token_t*)parser->lexer->tokens->data[parser->token_index];

		if (token->type == type) {
			return (token_t*)parser->lexer->tokens->data[parser->token_index++];
		} else {
			printf("Error: Expected %s\n", token_type2str(type));
			exit(EXIT_FAILURE);
		}
	} else {
		printf("Error: Unexpected end of file\n");
		exit(EXIT_FAILURE);
	}

	return NULL;
}

ast_node_t* parser_function(parser_t* parser) {
	printf("Parsing function\n");

	ast_node_t* node = malloc(sizeof(ast_node_t));
	node->type = AST_FUNCTION_DECLARATION;

	parser_token_eat(parser, TOKEN_TYPE_FUNCTION);

	token_t* name = parser_token_eat(parser, TOKEN_TYPE_IDENTIFIER);
	token_print(name);

	// if (parser->lexer->tokens->length > parser->token_index && ((token_t*)parser->lexer->tokens->data[parser->token_index])->type == TOKEN_TYPE_PARENTHESE_OPEN) {
	//     printf("Parsing parameters\n");
	//     parser->token_index++;

	//     if (parser->lexer->tokens->length > parser->token_index && ((token_t*) parser->lexer->tokens->data[parser->token_index])->type == TOKEN_TYPE_PARENTHESE_CLOSE) {
	//         parser->token_index++;
	//     } else {
	//         printf("Error: Expected closing parenthesis\n");
	//         exit(EXIT_FAILURE);
	//     }
	// }

	node->data.function_declaration->name = strdup(name->value);
	node->data.function_declaration->body = parser_block(parser);

	return node;
}

ast_node_t* parser_statement_print(parser_t* parser) {
	printf("Parsing statement print\n");

	ast_node_t* node = malloc(sizeof(ast_node_t));
	node->type = AST_STATEMENT_PRINT;

	parser->token_index++;
	node->data.statement_return->expression = parser_expression(parser);

	printf("end of print stmt\n");

	return node;
}

ast_node_t* parser_statement_return(parser_t* parser) {
	printf("Parsing statement return\n");

	ast_node_t* node = malloc(sizeof(ast_node_t));
	node->type = AST_STATEMENT_RETURN;

	parser->token_index++;
	node->data.statement_return->expression = parser_expression(parser);

	return node;
}

ast_node_t* parser_statement(parser_t* parser)
{
	printf("Parsing statement\n");

	ast_node_t* stmt = NULL;

	if (parser->lexer->tokens->length > parser->token_index) {
		token_t* tok = (token_t*)parser->lexer->tokens->data[parser->token_index];

		switch (tok->type) {
			case TOKEN_TYPE_RETURN:
				stmt = parser_statement_return(parser);
				break;
			
			case TOKEN_TYPE_PRINT:
				stmt = parser_statement_print(parser);
				break;
		}
	}
	
	if (stmt == NULL) {
		printf("Error: Unexpected token as statement\n");
		exit(EXIT_FAILURE);
	}
	return stmt;
}

ast_expression_t* parser_primary(parser_t* parser)
{
	printf("Parsing primary\n");

	token_t* current_token = (token_t*)parser->lexer->tokens->data[parser->token_index];
	ast_expression_t* primary_node = NULL;

	if (token_infos[current_token->type].nud != NULL) {
		primary_node = token_infos[current_token->type].nud(parser, current_token);
	} else {
		printf("Error: Unexpected token in primary expression\n");
		token_print(current_token);
		exit(EXIT_FAILURE);
	}

	printf("end of primary\n");

	return primary_node;
}

ast_node_t* parser_expression(parser_t* parser)
{
	printf("Parsing expression\n");

	ast_node_t* node = malloc(sizeof(ast_node_t));
	node->type = AST_EXPRESSION;
	node->data.expression = parser_expression_pratt(parser, PRECEDENCE_LOWEST);

	printf("end of expression\n");

	return node;
}

ast_expression_t* parser_expression_pratt(parser_t* parser, int precedence)
{
	printf("Parsing pratt\n");

	token_t* current_token = (token_t*)parser->lexer->tokens->data[parser->token_index];
	parser->token_index++;

	ast_expression_t* left = token_infos[current_token->type].nud(parser, current_token);

	printf("before loop of pratt\n");

	while (precedence < token_infos[((token_t*)parser->lexer->tokens->data[parser->token_index])->type].precedence) {
		current_token = (token_t*)parser->lexer->tokens->data[parser->token_index];
		parser->token_index++;

		left = token_infos[current_token->type].led(parser, current_token, left);
	}

	printf("after loop of pratt\n");

	return left;
}

ast_expression_t* led_plus_minus(parser_t* parser, token_t* token, ast_expression_t* left)
{
	printf("Parsing operation binary\n");

	ast_expression_t* right = parser_expression_pratt(parser, token_infos[token->type].precedence);

	ast_expression_t* binary_op_expr = malloc(sizeof(ast_expression_t));
	binary_op_expr->type = AST_EXPRESSION_BINARY_OP;
	binary_op_expr->data.binary_op = malloc(sizeof(ast_binary_op_t));
	binary_op_expr->data.binary_op->operator = strdup(token->value);
	binary_op_expr->data.binary_op->left = left;
	binary_op_expr->data.binary_op->right = right;

	return binary_op_expr;
}

ast_expression_t* nud_number(parser_t* parser, token_t* token)
{
	printf("Parsing number\n");

	ast_expression_t* literal_expr = malloc(sizeof(ast_expression_t));
	literal_expr->type = AST_EXPRESSION_LITERAL;

	printf("middle of number\n");

	literal_expr->data.literal = malloc(sizeof(ast_literal_t));
	printf("next of middle number\n");
	literal_expr->data.literal->literal_type = token->type;
	printf("next of middle number\n");
	printf("-->%s\n", token->value);
	literal_expr->data.literal->value = token->value; // strdup

	printf("end of number\n");

	return literal_expr;
}

ast_expression_t* nud_string(parser_t* parser, token_t* token)
{
	printf("Parsing string\n");

	ast_expression_t* literal_expr = malloc(sizeof(ast_expression_t));
	literal_expr->type = AST_EXPRESSION_LITERAL;

	literal_expr->data.literal = malloc(sizeof(ast_literal_t));
	literal_expr->data.literal->literal_type = token->type;
	literal_expr->data.literal->value = token->value; // strdup

	return literal_expr;
}

ast_expression_t* nud_identifier(parser_t* parser, token_t* token)
{
	printf("Parsing identifier\n");

	ast_expression_t* identifier_expr = malloc(sizeof(ast_expression_t));
	identifier_expr->type = AST_EXPRESSION_IDENTIFIER;

	identifier_expr->data.identifier = malloc(sizeof(ast_identifier_t));
	identifier_expr->data.identifier->name = token->value; // strdup

	return identifier_expr;
}

ast_expression_t* nud_parentheses(parser_t* parser, token_t* token)
{
	printf("Parsing parentheses\n");

	ast_expression_t* expression_node = parser_expression_pratt(parser, PRECEDENCE_LOWEST);

	parser_token_eat(parser, TOKEN_TYPE_PARENTHESE_CLOSE);

	return expression_node;
}

ast_node_t* parser_block(parser_t* parser)
{
	printf("Parsing block\n");

	ast_block_t* block_data = malloc(sizeof(ast_block_t));
	block_data->num_statements = 0;
	block_data->statements = NULL;

	parser_token_eat(parser, TOKEN_TYPE_SECTION_OPEN);

	while (parser->lexer->tokens->length > parser->token_index && ((token_t*) parser->lexer->tokens->data[parser->token_index])->type != TOKEN_TYPE_SECTION_CLOSE) {
		ast_node_t* statement = parser_statement(parser);
		block_data->statements = realloc(block_data->statements, (block_data->num_statements + 1) * sizeof(ast_node_t*));
		block_data->statements[block_data->num_statements++] = statement;
	}

	ast_node_t* block_node = malloc(sizeof(ast_node_t));
	block_node->type = AST_BLOCK;
	block_node->data.block = block_data;

	parser_token_eat(parser, TOKEN_TYPE_SECTION_CLOSE);

	return block_node;
}

void parser_parse(parser_t* parser)
{
	while (parser->token_index < parser->lexer->tokens->length) {
		token_t* current_token = (token_t*)parser->lexer->tokens->data[parser->token_index];

		switch (current_token->type) {
			case TOKEN_TYPE_FUNCTION:
				ast_node_t* function_node = parser_function(parser);
				parser->ast_tree = function_node;
				break;

			case TOKEN_TYPE_SECTION_OPEN:
				ast_node_t* block_node = parser_block(parser);
				parser->ast_tree = block_node;
				break;

			case TOKEN_TYPE_RETURN:
				ast_node_t* return_node = parser_statement_return(parser);
				parser->ast_tree = return_node;
				break;

			default:
				ast_node_t* statement_node = parser_statement(parser);
				parser->ast_tree = statement_node;
				break;
		}

		parser->token_index++;
	}
}

void print_indentation(int indent_level)
{
	for (int i = 0; i < indent_level; i++) {
		printf("  ");
	}
}

void print_xml_ast_expression(ast_expression_t* expr, int indent_level)
{
	printf("<Expression>\n");

	switch (expr->type) {
		case AST_EXPRESSION_LITERAL:
			print_indentation(indent_level + 1);
			printf("<Literal>\n");

				print_indentation(indent_level + 2);
				printf("<Type>%s</Type>\n", token_type2str(expr->data.literal->literal_type));

				print_indentation(indent_level + 2);
				printf("<Value>%s</Value>\n", expr->data.literal->value);

			print_indentation(indent_level + 1);
			printf("</Literal>\n");
			break;

		case AST_EXPRESSION_IDENTIFIER:
			print_indentation(indent_level + 1);
			printf("<Identifier>\n");

				print_indentation(indent_level + 2);
				printf("<Name>%s</Name>\n", expr->data.identifier->name);

			print_indentation(indent_level + 1);
			printf("</Identifier>\n");
			break;

		case AST_EXPRESSION_BINARY_OP:
			print_indentation(indent_level + 1);
			printf("<BinaryOperation>\n");

				print_indentation(indent_level + 2);
				printf("<Operator>%s</Operator>\n", expr->data.binary_op->operator);

				print_indentation(indent_level + 2);
				printf("<Left>\n");

					print_indentation(indent_level + 3);
					print_xml_ast_expression(expr->data.binary_op->left, indent_level + 3);

				print_indentation(indent_level + 2);
				printf("</Left>\n");

				print_indentation(indent_level + 2);
				printf("<Right>\n");

					print_indentation(indent_level + 3);
					print_xml_ast_expression(expr->data.binary_op->right, indent_level + 3);

				print_indentation(indent_level + 2);
				printf("</Right>\n");

			print_indentation(indent_level + 1);
			printf("</BinaryOperation>\n");
			break;

		default:
			print_indentation(indent_level + 1);
			printf("<!-- Unhandled Expression Type -->\n");
			break;
	}

	print_indentation(indent_level);
	printf("</Expression>\n");
}

void print_xml_ast_node(ast_node_t* node, int indent_level)
{
	if (node == NULL) {
		return;
	}
	
	print_indentation(indent_level);

	switch (node->type) {
		case AST_FUNCTION_DECLARATION:
			printf("<FunctionDeclaration>\n");
			print_indentation(indent_level + 1);
			printf("<Name>%s</Name>\n", node->data.function_declaration->name);
	
				print_xml_ast_node(node->data.function_declaration->body, indent_level + 1);

			print_indentation(indent_level);
			printf("</FunctionDeclaration>\n");
			break;

		case AST_STATEMENT_PRINT:
			printf("<StatementPrint>\n");

				print_xml_ast_node(node->data.statement_print->expression, indent_level + 1);

			print_indentation(indent_level);
			printf("</StatementPrint>\n");
			break;
			
		case AST_STATEMENT_RETURN:
			printf("<StatementReturn>\n");

				print_xml_ast_node(node->data.statement_return->expression, indent_level + 1);

			print_indentation(indent_level);
			printf("</StatementReturn>\n");
			break;

		case AST_BLOCK:
			printf("<Block>\n");

			for (size_t i = 0; i < node->data.block->num_statements; i++) {
				print_xml_ast_node(node->data.block->statements[i], indent_level + 1);
			}

			print_indentation(indent_level);
			printf("</Block>\n");
			break;

		case AST_EXPRESSION:
			print_xml_ast_expression(node->data.expression, indent_level);
			break;

		default:
			print_indentation(indent_level);
			printf("<!-- Unhandled AST Node Type -->\n");
			break;
	}
}

void print_xml_ast_tree(ast_node_t* root)
{
	printf("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
	printf("<AST>\n");

		print_xml_ast_node(root, 1);

	printf("</AST>\n");
}

void interpret(ast_node_t* node, interpreter_state_t* state)
{
	if (node == NULL) {
		return;
	}

	switch (node->type) {
		case AST_FUNCTION_DECLARATION:
			interpret_function_declaration(node->data.function_declaration, state);
			break;

		case AST_STATEMENT_RETURN:
			interpret_statement_return(node->data.statement_return, state);
			break;

		case AST_STATEMENT_PRINT:
			interpret_statement_print(node->data.statement_print, state);
			break;

		// case AST_BLOCK:
		//     interpret_block(node, state);
		//     break;

		// case AST_EXPRESSION:
		// 	printf("expr...\n");
		// 	// interpret_expression(node, state);
		// 	break;

		default:
			break;
	}
}

void interpret_function_declaration(ast_function_declaration_t* stmt, interpreter_state_t* state)
{
	// printf("Function Declaration: %s\n", stmt->name);

	interpret_block(stmt->body, state);
}

void interpret_statement_return(ast_statement_return_t* stmt, interpreter_state_t* state)
{
	// printf("Return Statement\n");

	int res = evaluate_expression(stmt->expression->data.expression, state);
	printf("Return Result: %d\n", res);
}

void interpret_statement_print(ast_statement_print_t* stmt, interpreter_state_t* state)
{
	// printf("Print Statement\n");

	int res = evaluate_expression(stmt->expression->data.expression, state);
	printf("Print Result: %d\n", res);
}

void interpret_block(ast_node_t* node, interpreter_state_t* state)
{
	// printf("Block\n");

	for (size_t i = 0; i < node->data.block->num_statements; i++) {
		interpret(node->data.block->statements[i], state);
	}
}

int evaluate_literal(ast_literal_t* expr)
{
	// printf("Literal: %s\n", expr->value);

	if (expr->literal_type == TOKEN_TYPE_NUMBER)
		return atoi(expr->value);
	return 0;
}

int evaluate_identifier(ast_identifier_t* expr, interpreter_state_t* state)
{
	printf("Variable: %s\n", expr->name);

	return 0;
}

int evaluate_binary_operation(ast_binary_op_t* binary_op, interpreter_state_t* state) {
	const char* operator_str = binary_op->operator;

	int left = evaluate_expression(binary_op->left, state);
	int right = evaluate_expression(binary_op->right, state);

	if (strcmp(operator_str, "+") == 0) {
		return left + right;
	} else if (strcmp(operator_str, "-") == 0) {
		return left - right;
	} else if (strcmp(operator_str, "*") == 0) {
		return left * right;
	} else if (strcmp(operator_str, "/") == 0) {
		return right != 0 ? left / right : 0;
	} else {
		return 88;
	}
}

// int evaluate_function_call(ast_node_t* node, interpreter_state_t* state) {
//     printf("Function Call: %s\n", node->data.function_call.name);
//     return 0;
// }

int evaluate_expression(ast_expression_t* expr, interpreter_state_t* state) {
	int res = 66;
	if (expr == NULL) {
		return res;
	}

	switch (expr->type) {
		case AST_EXPRESSION_LITERAL:
			res = evaluate_literal(expr->data.literal);
			return res;

		case AST_EXPRESSION_IDENTIFIER:
			res = evaluate_identifier(expr->data.identifier, state);
			return res;

		case AST_EXPRESSION_BINARY_OP:
			res = evaluate_binary_operation(expr->data.binary_op, state);
			return res;

		// case AST_EXPRESION_FUNCTION_CALL:
		//     return evaluate_function_call(expr, state);

		default:
			printf("Error: default expr type: %d\n", expr->type);
			exit(EXIT_FAILURE);
			return res;
	}
}

int main(int argc, char** argv)
{
	setlocale(LC_ALL, "");

	if (argc == 1 || argc > 2) {
		help();
		return 0;
	}

	char* file_data = file_read(argv[1]);
	// printf("%s\n", file_data);

	lexer_t* lexer = lexer_create(file_data);
	lexer_lex(lexer);

	array_print(lexer->tokens);

	parser_t* parser = parser_create(lexer);
	parser_parse(parser);

	print_xml_ast_tree(parser->ast_tree);

	interpret(parser->ast_tree, NULL);

	exit(EXIT_SUCCESS);
}
