#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <locale.h>
#include <stdbool.h>
#include <string.h>
#include <limits.h>

typedef enum {
	VALUE_TYPE_INT,
	VALUE_TYPE_FLOAT,
	VALUE_TYPE_BOOL,
	VALUE_TYPE_STRING,
} ast_literal_type_t;

char* literal_type2name(ast_literal_type_t type)
{
	switch (type) {
		case VALUE_TYPE_INT: return "INT";
		case VALUE_TYPE_FLOAT: return "FLOAT";
		case VALUE_TYPE_BOOL: return "BOOL";
		case VALUE_TYPE_STRING: return "STRING";
		default: return "TYPE_UNKNOWN";
	}
}

struct ast_literal_t;

typedef struct ast_literal_t {
	ast_literal_type_t type;
	union {
		int int_value;
		bool bool_value;
		float float_value;
		char* string_value;
	};

	struct ast_literal_t* left;
	struct ast_literal_t* right;
} ast_literal_t;

typedef struct SymbolTableEntry {
	char* identifier;
	ast_literal_t* data;
	ast_literal_t* prevdata;
	struct SymbolTableEntry* next;
} SymbolTableEntry;

typedef struct {
	SymbolTableEntry** entries;
	size_t size;
	size_t capacity;
} SymbolTable;

typedef struct SymbolTableStack {
	SymbolTable* table;
	struct SymbolTableStack* next;
} SymbolTableStack;

SymbolTableStack* symbolTableStack = NULL;

SymbolTable* createSymbolTable(size_t capacity)
{
	SymbolTable* table = (SymbolTable*) malloc(sizeof(SymbolTable));
	table->entries = (SymbolTableEntry**) calloc(capacity, sizeof(SymbolTableEntry*) );
	table->size = 0;
	table->capacity = capacity;
	return table;
}

void pushSymbolTable()
{
	SymbolTable* table = createSymbolTable(3);
	SymbolTableStack* newScope = (SymbolTableStack*) malloc(sizeof(SymbolTableStack));
	newScope->table = table;
	newScope->next = symbolTableStack;
	symbolTableStack = newScope;
}

void popSymbolTable()
{
	if (symbolTableStack == NULL) {
		return;
	}

	SymbolTableStack* top = symbolTableStack;
	symbolTableStack = top->next;

	SymbolTable* table = top->table;
	for (size_t i = 0; i < table->capacity; ++i) {
		SymbolTableEntry* entry = table->entries[i];
		while (entry != NULL) {
			SymbolTableEntry* next = entry->next;
			if (entry->identifier != NULL) {
				free(entry->identifier);
				entry->identifier = NULL;
			}
			if (entry->data != NULL) {
				// ast_expression_data_free(&(entry->data));
				entry->prevdata = entry->data;
				entry->data = NULL;
			}
			free(entry);

			entry = next;
		}
	}
	free(table->entries);
	table->entries = NULL;
	free(table);
	table = NULL;
	free(top);
	top = NULL;
}

unsigned int hash(const char* str, size_t capacity)
{
	unsigned int hash = 0;
	while (*str) {
		hash = (hash * 31) + (*str++);
	}
	return hash % capacity;
}

static SymbolTableEntry* findSymbolInParentScopes(SymbolTableStack* symbolTableStack, const char* identifier)
{
    if (symbolTableStack == NULL) {
        return NULL;
    }

    SymbolTable* table = symbolTableStack->table;
    unsigned int index = hash(identifier, table->capacity);

    SymbolTableEntry* entry = table->entries[index];
    while (entry != NULL) {
        if (strcmp(entry->identifier, identifier) == 0) {
            return entry;
        }
        entry = entry->next;
    }

    return findSymbolInParentScopes(symbolTableStack->next, identifier);
}

void addToSymbolTable(SymbolTableStack* symbolTableStack, const char* identifier, ast_literal_t* value)
{
    if (symbolTableStack == NULL) {
        return;
    }

    SymbolTableEntry* existingEntry = findSymbolInParentScopes(symbolTableStack, identifier);

    if (existingEntry != NULL) {
        existingEntry->data = value;
        return;
    }

    SymbolTable* table = symbolTableStack->table;
    unsigned int index = hash(identifier, table->capacity);

    SymbolTableEntry* entry = (SymbolTableEntry*) malloc(sizeof(SymbolTableEntry));
    entry->identifier = strdup(identifier);
    entry->data = value;
    entry->next = table->entries[index];
    table->entries[index] = entry;
    table->size++;
}

ast_literal_t* findInSymbolTableCurrent(SymbolTableStack* currentScope, const char* identifier)
{
	SymbolTable* table = currentScope->table;
	unsigned int index = hash(identifier, table->capacity);
	SymbolTableEntry* entry = table->entries[index];

	while (entry != NULL) {
		if (strcmp(entry->identifier, identifier) == 0) {
			return entry->data;
		}
		entry = entry->next;
	}

	return NULL;
}

ast_literal_t* findInSymbolTable(SymbolTableStack* currentScope, const char* identifier)
{
	while (currentScope != NULL) {
		ast_literal_t* data = findInSymbolTableCurrent(currentScope, identifier);
		if (data != NULL) {
			return data;
		}

		currentScope = currentScope->next;
	}

	return NULL;
}

typedef enum {
	// Values
	TOKEN_TYPE_IDENTIFIER,
	TOKEN_TYPE_NUMBER,
	TOKEN_TYPE_STRING,

	// Keywords
	TOKEN_TYPE_FUNCTION, // عملکرد
	TOKEN_TYPE_RETURN, // برگشت
	TOKEN_TYPE_PRINT, // نمایش
	TOKEN_TYPE_IF, // اگر
	TOKEN_TYPE_TRUE, // درست
	TOKEN_TYPE_FALSE, // غلط
	TOKEN_TYPE_ELSEIF, // واگرنه
	TOKEN_TYPE_AND, // و
	TOKEN_TYPE_OR, // یا

	// Symbols
	TOKEN_TYPE_SECTION_OPEN, // {
	TOKEN_TYPE_SECTION_CLOSE, // }
	TOKEN_TYPE_PARENTHESE_OPEN, // (
	TOKEN_TYPE_PARENTHESE_CLOSE, // )

	TOKEN_TYPE_PLUS, // +
	TOKEN_TYPE_MINUS, // -
	TOKEN_TYPE_MULTIPY, // *
	TOKEN_TYPE_DIVIDE, // /

	TOKEN_TYPE_EQUAL, // =
	TOKEN_TYPE_EQUAL_EQUAL, // ==
	TOKEN_TYPE_NOT_EQUAL, // !=
	TOKEN_TYPE_NOT, // !
	TOKEN_TYPE_LESS_THAN, // <
	TOKEN_TYPE_GREATER_THAN, // >
	TOKEN_TYPE_LESS_THAN_EQUAL, // <=
	TOKEN_TYPE_GREATER_THAN_EQUAL, // >=

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
	{"درست", TOKEN_TYPE_TRUE},
	{"غلط", TOKEN_TYPE_FALSE},
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
	size_t length;
	size_t index;
	size_t line;
	size_t column;

	array_t* tokens;
	size_t last_char_size;
} lexer_t;

struct ast_node;

typedef struct {
	char* name;
	struct ast_node* body;
} ast_function_declaration_t;

typedef struct {
	struct ast_node* expression;
	struct ast_literal_t* expression_value;
} ast_statement_return_t;

typedef struct {
	struct ast_node* expression;
	struct ast_literal_t* expression_value;
} ast_statement_print_t;

typedef struct {
	struct ast_node** statements;
	size_t num_statements;
} ast_block_t;

typedef struct {
	char* name;
} ast_identifier_t;

typedef struct {
	char* operator;
	struct ast_expression* left;
	struct ast_expression* right;
} ast_expression_binary_t;

typedef struct {
	struct ast_expression* left;
	struct ast_expression* right;
} ast_expression_assignment_t;

typedef enum {
	AST_EXPRESSION_VALUE,
	AST_EXPRESSION_LITERAL,
	AST_EXPRESSION_IDENTIFIER,
	AST_EXPRESSION_BINARY,
	AST_EXPRESSION_ASSIGNMENT,
	AST_EXPRESSION_FUNCTION_CALL,
} ast_expression_type_t;

typedef struct {
	char* name;
	array_t* arguments;
} ast_function_call_t;

typedef struct ast_expression {
	ast_expression_type_t type;

	union {
		ast_literal_t* literal;
		ast_identifier_t* identifier;
		ast_expression_binary_t* binary_op;
		ast_expression_assignment_t* assignment;
		ast_function_call_t* function_call;
	} data;
} ast_expression_t;

typedef enum {
	AST_FUNCTION_DECLARATION,
	AST_STATEMENT_RETURN,
	AST_STATEMENT_PRINT,
	AST_STATEMENT_IF,
	AST_STATEMENT_ELSEIF,
	AST_BLOCK,
	AST_EXPRESSION,
} ast_node_type_t;

struct ast_statement_if_t;

typedef struct {
	struct ast_node* condition;
	struct ast_node* block;
	struct ast_node_t** elseifs;
	struct ast_node* else_block;
	size_t num_elseifs;
} ast_statement_if_t;

typedef struct ast_node {
	ast_node_type_t type;
	union {
		ast_function_declaration_t* function_declaration;
		ast_statement_return_t* statement_return;
		ast_statement_print_t* statement_print;
		ast_block_t* block;
		ast_expression_t* expression;
		ast_statement_if_t* statement_if;
		ast_function_call_t* function_call;
	} data;
} ast_node_t;

typedef struct {
	char* name;
	ast_expression_t* expression;
} ast_variable_declaration_t;

typedef struct {
	lexer_t* lexer;
	size_t token_index;
	array_t* functions;
	array_t* expressions;
} parser_t;

typedef struct {
	int return_code;
	parser_t* parser;
} interpreter_t;

bool interpreter_expression_truly(ast_expression_t* expr, interpreter_t* interpreter);

interpreter_t* interpreter_interpret(interpreter_t* interpreter);
ast_node_t* interpreter_interpret_once(ast_node_t* node, interpreter_t* interpreter);

void interpreter_expression_data(ast_literal_t* data);

void ast_expression_data_free(ast_literal_t** val);

wchar_t read_token(lexer_t* lexer);
void read_number(lexer_t* lexer, wchar_t ch);
size_t mb_strlen(char* identifier);
void read_identifier(lexer_t* lexer, wchar_t ch);
size_t wchar_length(wchar_t wide_char);

lexer_t* lexer_create(const char* data);
void lexer_free(lexer_t** lexer);
void lexer_lex(lexer_t* lexer);

parser_t* parser_create(lexer_t* lexer);
void parser_free(parser_t** parser);
void parser_parse(parser_t* parser);
ast_node_t* parser_function(parser_t* parser);
ast_node_t* parser_block(parser_t* parser);
ast_node_t* parser_statement(parser_t* parser);
ast_node_t* parser_statement_return(parser_t* parser);
ast_node_t* parser_statement_print(parser_t* parser);
ast_node_t* parser_expression(parser_t* parser);

ast_literal_t* interpreter_expression(ast_expression_t* expr, interpreter_t* interpreter);
ast_literal_t* interpreter_operator_binary(ast_expression_binary_t* expr, interpreter_t* interpreter);
ast_literal_t* interpreter_literal(ast_literal_t* expr);
ast_literal_t* interpreter_identifier(ast_identifier_t* expr, interpreter_t* interpreter);

ast_statement_print_t* interpreter_statement_print(ast_statement_print_t* stmt, interpreter_t* interpreter);
ast_statement_return_t* interpreter_statement_return(ast_statement_return_t* stmt, interpreter_t* interpreter);
ast_function_declaration_t* interpreter_function_declaration(ast_function_declaration_t* stmt, interpreter_t* interpreter);
ast_block_t* interpreter_block(ast_block_t* block, interpreter_t* interpreter);

typedef ast_expression_t* (*nud_func_t)(parser_t* parser, token_t* token);
typedef ast_expression_t* (*led_func_t)(parser_t* parser, token_t* token, ast_expression_t* left);

typedef struct {
	size_t precedence;
	nud_func_t nud;
	led_func_t led;
} token_info_t;

ast_expression_t* nud_bool(parser_t* parser, token_t* token);
ast_expression_t* nud_number(parser_t* parser, token_t* token);
ast_expression_t* nud_string(parser_t* parser, token_t* token);
ast_expression_t* nud_identifier(parser_t* parser, token_t* token);
ast_expression_t* nud_parentheses(parser_t* parser, token_t* token);
ast_expression_t* led_plus_minus(parser_t* parser, token_t* token, ast_expression_t* left);
ast_expression_t* led_equal(parser_t* parser, token_t* token, ast_expression_t* left);
ast_expression_t* led_equal_equal(parser_t* parser, token_t* token, ast_expression_t* left);
ast_expression_t* led_and(parser_t* parser, token_t* token, ast_expression_t* left);
ast_expression_t* led_or(parser_t* parser, token_t* token, ast_expression_t* left);

ast_expression_t* parser_expression_pratt(parser_t* parser, size_t precedence);

enum {
	PRECEDENCE_LOWEST = 0,    // START FROM HERE
	PRECEDENCE_HIGHEST = 1,   // =
	PRECEDENCE_ANDOR = 2,     // AND OR
	PRECEDENCE_SUM = 3,       // + -
};

token_info_t token_infos[] = {
	[TOKEN_TYPE_TRUE] = {PRECEDENCE_LOWEST, nud_bool, NULL},
	[TOKEN_TYPE_FALSE] = {PRECEDENCE_LOWEST, nud_bool, NULL},
	[TOKEN_TYPE_NUMBER] = {PRECEDENCE_LOWEST, nud_number, NULL},
	[TOKEN_TYPE_NUMBER] = {PRECEDENCE_LOWEST, nud_number, NULL},
	[TOKEN_TYPE_IDENTIFIER] = {PRECEDENCE_LOWEST, nud_identifier, NULL},
	[TOKEN_TYPE_STRING] = {PRECEDENCE_LOWEST, nud_string, NULL},
	[TOKEN_TYPE_PARENTHESE_OPEN] = {PRECEDENCE_LOWEST, nud_parentheses, NULL},
	[TOKEN_TYPE_PLUS] = {PRECEDENCE_SUM, NULL, led_plus_minus},
	[TOKEN_TYPE_AND] = {PRECEDENCE_ANDOR, NULL, led_and},
	[TOKEN_TYPE_OR] = {PRECEDENCE_ANDOR, NULL, led_or},
	[TOKEN_TYPE_MINUS] = {PRECEDENCE_SUM, NULL, led_plus_minus},
	[TOKEN_TYPE_EQUAL] = {PRECEDENCE_HIGHEST, NULL, led_equal},
	[TOKEN_TYPE_EQUAL_EQUAL] = {PRECEDENCE_HIGHEST, NULL, led_equal_equal},
};

char* token_op_type2str(ast_expression_type_t type)
{
	switch (type) {
		case AST_EXPRESSION_LITERAL: return "LITERAL";
		case AST_EXPRESSION_IDENTIFIER: return "IDENTIFIER";
		case AST_EXPRESSION_BINARY: return "BINARY_OP";
		case AST_EXPRESSION_ASSIGNMENT: return "ASSIGNMENT";
		case AST_EXPRESSION_FUNCTION_CALL: return "FUNCTION_CALL";
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
		case TOKEN_TYPE_TRUE: return "TRUE";
		case TOKEN_TYPE_FALSE: return "FALSE";
		case TOKEN_TYPE_ELSEIF: return "ELSEIF";
		case TOKEN_TYPE_OR: return "OR";
		case TOKEN_TYPE_AND: return "AND";
		case TOKEN_TYPE_SECTION_OPEN: return "SECTION_OPEN";
		case TOKEN_TYPE_SECTION_CLOSE: return "SECTION_CLOSE";
		case TOKEN_TYPE_PARENTHESE_OPEN: return "PARENTHESIS_OPEN";
		case TOKEN_TYPE_PARENTHESE_CLOSE: return "PARENTHESIS_CLOSE";
		case TOKEN_TYPE_PLUS: return "PLUS";
		case TOKEN_TYPE_MINUS: return "MINUS";
		case TOKEN_TYPE_EQUAL: return "EQUAL";
		case TOKEN_TYPE_EQUAL_EQUAL: return "EQUAL_EQUAL";
		case TOKEN_TYPE_NOT_EQUAL: return "NOT_EQUAL";
		case TOKEN_TYPE_NOT: return "NOT";
		case TOKEN_TYPE_LESS_THAN: return "LESS_THAN";
		case TOKEN_TYPE_GREATER_THAN: return "GREATER_THAN";
		case TOKEN_TYPE_LESS_THAN_EQUAL: return "LESS_THAN_EQUAL";
		case TOKEN_TYPE_GREATER_THAN_EQUAL: return "GREATER_THAN_EQUAL";
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

	char* file_data = (char*) malloc(file_size + 1);
	fread(file_data, 1, file_size, file);
	file_data[file_size] = 0;

	fclose(file);
	return file_data;
}

token_t* token_create(token_type_t type, const char* value, int a, int b, int c, int b2, int c2)
{
	token_t* t = (token_t*) malloc(sizeof(token_t));
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

	array_t* arr = (array_t*) malloc(sizeof(array_t));
	arr->length = 0;
	arr->size = size > min_size ? size : min_size;
	arr->data = (void*) malloc(sizeof(void*) * arr->size);

	return arr;
}

void* array_pop(array_t* arr)
{
	if (arr->length == 0) {
		return NULL;
	}

	void* data = arr->data[--arr->length];
	return data;
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
	if (arr == NULL) {
		return;
	}

	if (arr->data != NULL) {
		for (size_t i = 0; i < arr->length; i++) {
			free(arr->data[i]);
		}
		free(arr->data);
		arr->data = NULL;
	}

	free(arr);
	arr = NULL;
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
	lexer_t* lexer = (lexer_t*) malloc(sizeof(lexer_t));
	lexer->data = (char*) data;
	lexer->index = 0;
	lexer->tokens = array_create(10);
	lexer->length = strlen(data);

	return lexer;
}

void lexer_free(lexer_t** lexer)
{
    if (lexer == NULL || *lexer == NULL) {
        return;
    }

    if ((*lexer)->tokens != NULL) {
        for (size_t i = 0; i < (*lexer)->tokens->length; i++) {
            token_t* t = (token_t*) (*lexer)->tokens->data[i];
            if (t != NULL) {
                if (t->value != NULL) {
                    free(t->value);
                    t->value = NULL;
                }

                free(t);
                t = NULL;
            }
        }

        if ((*lexer)->tokens->data != NULL) {
            free((*lexer)->tokens->data);
            (*lexer)->tokens->data = NULL;
        }

        free((*lexer)->tokens);
        (*lexer)->tokens = NULL;
    }

    if ((*lexer)->data != NULL) {
        free((*lexer)->data);
        (*lexer)->data = NULL;
    }

    free(*lexer);
    *lexer = NULL;
}

bool is_number(wchar_t ch)
{
	return (ch >= L'۰' && ch <= L'۹') || (ch >= '0' && ch <= '9');
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
	char number[21];
	int i = 0;
	while (is_number(ch)) {
		if (ch >= '0' && ch <= '9') {
			number[i++] = ch - '0';
		} else {
			number[i++] = ch - L'۰' + '0';
		}
		ch = read_token(lexer);
	}
	number[i] = 0;

	token_t* t = token_create(TOKEN_TYPE_NUMBER, number, i, lexer->line, lexer->column - i, lexer->line, lexer->column);
	array_push(lexer->tokens, t);

	unread_token(lexer);
}

void read_string(lexer_t* lexer, wchar_t ch)
{
	char* string = (char*) malloc(sizeof(char) * 1024); // 1023 + 1 for null terminator
	int i = 0;

	while (ch != L'"') {
		if (i >= 1023) {
			printf("Error: String length exceeds the maximum allowed length.\n");
			exit(EXIT_FAILURE);
		}

		int char_size = wctomb(&string[i], ch);
		if (char_size < 0) {
			printf("Error: Failed to convert wide character to multibyte\n");
			exit(EXIT_FAILURE);
		}
		i += char_size;

		ch = read_token(lexer);
	}

	string[i] = '\0';

	token_t* t = token_create(TOKEN_TYPE_STRING, string, i, lexer->line, lexer->column - i, lexer->line, lexer->column);
	array_push(lexer->tokens, t);

	free(string);
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
		char current_char = lexer->data[lexer->index];

		if (current_char == '\a' || current_char == '\r' || current_char == ' ' || current_char == '\t') {
			lexer->column++;
			lexer->index++;
			continue;
		} else if (current_char == '\n') {
			lexer->index++;
			lexer->line++;
			lexer->column = 0;
			continue;
		}

		wchar_t current_wchar = read_token(lexer);
		if (current_wchar == L'\u200C') {
			lexer->index++;
			lexer->column++;
			continue;
		} else if (current_wchar == '{') {
			token_t* t = token_create(TOKEN_TYPE_SECTION_OPEN, "{", 1, lexer->line, lexer->column - 1, lexer->line, lexer->column);
			array_push(lexer->tokens, t);
		} else if (current_wchar == '}') {
			token_t* t = token_create(TOKEN_TYPE_SECTION_CLOSE, "}", 1, lexer->line, lexer->column - 1, lexer->line, lexer->column);
			array_push(lexer->tokens, t);
		} else if (current_wchar == '(') {
			token_t* t = token_create(TOKEN_TYPE_PARENTHESE_OPEN, "(", 1, lexer->line, lexer->column - 1, lexer->line, lexer->column);
			array_push(lexer->tokens, t);
		} else if (current_wchar == ')') {
			token_t* t = token_create(TOKEN_TYPE_PARENTHESE_CLOSE, ")", 1, lexer->line, lexer->column - 1, lexer->line, lexer->column);
			array_push(lexer->tokens, t);
		} else if (current_wchar == '+') {
			token_t* t = token_create(TOKEN_TYPE_PLUS, "+", 1, lexer->line, lexer->column - 1, lexer->line, lexer->column);
			array_push(lexer->tokens, t);
		} else if (current_wchar == '*') {
			token_t* t = token_create(TOKEN_TYPE_MULTIPY, "*", 1, lexer->line, lexer->column - 1, lexer->line, lexer->column);
			array_push(lexer->tokens, t);
		} else if (current_wchar == '/') {
			token_t* t = token_create(TOKEN_TYPE_DIVIDE, "/", 1, lexer->line, lexer->column - 1, lexer->line, lexer->column);
			array_push(lexer->tokens, t);
		} else if (current_wchar == '-') {
			token_t* t = token_create(TOKEN_TYPE_MINUS, "-", 1, lexer->line, lexer->column - 1, lexer->line, lexer->column);
			array_push(lexer->tokens, t);
        } else if (current_wchar == L'=') {
            if (lexer->index < lexer->length && lexer->data[lexer->index] == L'=') {
                // wprintf(L"has ==\n");
                token_t* t = token_create(TOKEN_TYPE_EQUAL_EQUAL, "==", 2, lexer->line, lexer->column - 2, lexer->line, lexer->column);
                array_push(lexer->tokens, t);
                lexer->index++;
                lexer->column++;
            } else {
                // wprintf(L"has =\n");
                token_t* t = token_create(TOKEN_TYPE_EQUAL, "=", 1, lexer->line, lexer->column - 1, lexer->line, lexer->column);
                array_push(lexer->tokens, t);
            }
		} else if (current_char == '!') {
			if (lexer->index < lexer->length && lexer->data[lexer->index] == '=') {
				token_t* t = token_create(TOKEN_TYPE_NOT_EQUAL, "!=", 2, lexer->line, lexer->column - 2, lexer->line, lexer->column);
				array_push(lexer->tokens, t);
				lexer->index++;
				lexer->column++;
			} else {
				token_t* t = token_create(TOKEN_TYPE_NOT, "!", 1, lexer->line, lexer->column - 1, lexer->line, lexer->column);
				array_push(lexer->tokens, t);
			}
		} else if (current_char == '>') {
			if (lexer->index < lexer->length && lexer->data[lexer->index] == '=') {
				token_t* t = token_create(TOKEN_TYPE_GREATER_THAN_EQUAL, ">=", 2, lexer->line, lexer->column - 2, lexer->line, lexer->column);
				array_push(lexer->tokens, t);
				lexer->index++;
				lexer->column++;
			} else {
				token_t* t = token_create(TOKEN_TYPE_GREATER_THAN, ">", 1, lexer->line, lexer->column - 1, lexer->line, lexer->column);
				array_push(lexer->tokens, t);
			}
		} else if (current_char == '<') {
			if (lexer->index < lexer->length && lexer->data[lexer->index] == '=') {
				token_t* t = token_create(TOKEN_TYPE_LESS_THAN_EQUAL, "<=", 2, lexer->line, lexer->column - 2, lexer->line, lexer->column);
				array_push(lexer->tokens, t);
				lexer->index++;
				lexer->column++;
			} else {
				token_t* t = token_create(TOKEN_TYPE_LESS_THAN, "<", 1, lexer->line, lexer->column - 1, lexer->line, lexer->column);
				array_push(lexer->tokens, t);
			}
		} else if (current_wchar == '\"') {
			current_wchar = read_token(lexer);
			read_string(lexer, current_wchar);
		} else if (is_number(current_wchar)) {
			read_number(lexer, current_wchar);
		} else if (is_alpha(current_wchar)) {
			read_identifier(lexer, current_wchar);
		} else {
			printf("Error: Unexpected character '%c' at line %zu, column %zu\n", current_char, lexer->line, lexer->column - 1);

			token_t* t = token_create(TOKEN_TYPE_ERROR, (char[]){current_char,'\0'}, 1, lexer->line, lexer->column - 1, lexer->line, lexer->column);
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
	parser_t* parser = (parser_t*) malloc(sizeof(parser_t));
	parser->lexer = lexer;
	parser->token_index = 0;
	parser->functions = array_create(5);
	parser->expressions = NULL;// = array_create(5);

	return parser;
}

void debug_current_token(parser_t* parser)
{
	token_t* t = (token_t*) parser->lexer->tokens->data[parser->token_index];
	printf("=========> Current token: %s - %s\n", token_type2str(t->type) ,t->value);
}

void ast_expression_free(ast_expression_t** expr)
{
    printf("ast_expression_free\n");

    if (expr == NULL || *expr == NULL) {
        return;
    }

    switch ((*expr)->type) {
        case AST_EXPRESSION_LITERAL:
            if ((*expr)->data.literal != NULL) {
                ast_expression_data_free(&((*expr)->data.literal));
                (*expr)->data.literal = NULL;
            }
            break;

        case AST_EXPRESSION_IDENTIFIER:
            if ((*expr)->data.identifier != NULL) {
                if ((*expr)->data.identifier->name != NULL) {
                    free((*expr)->data.identifier->name);
                    (*expr)->data.identifier->name = NULL;
                }

                free((*expr)->data.identifier);
                (*expr)->data.identifier = NULL;
            }
            break;

        case AST_EXPRESSION_FUNCTION_CALL:
            if ((*expr)->data.function_call != NULL) {
                free((*expr)->data.function_call->name);
                (*expr)->data.function_call->name = NULL;

                if ((*expr)->data.function_call->arguments != NULL) {
                    for (size_t i = 0; i < (*expr)->data.function_call->arguments->length; i++) {
						ast_expression_free((ast_expression_t**)&((*expr)->data.function_call->arguments->data[i]));
                        (*expr)->data.function_call->arguments->data[i] = NULL;
                    }

                    free((*expr)->data.function_call->arguments->data);
                    (*expr)->data.function_call->arguments->data = NULL;

                    free((*expr)->data.function_call->arguments);
                    (*expr)->data.function_call->arguments = NULL;
                }
            }
            break;

        case AST_EXPRESSION_ASSIGNMENT:
            if ((*expr)->data.assignment != NULL) {
                if ((*expr)->data.assignment->left != NULL) {
                    ast_expression_free(&((*expr)->data.assignment->left));
                    (*expr)->data.assignment->left = NULL;
                }
                if ((*expr)->data.assignment->right != NULL) {
                    ast_expression_free(&((*expr)->data.assignment->right));
                    (*expr)->data.assignment->right = NULL;
                }

                free((*expr)->data.assignment);
                (*expr)->data.assignment = NULL;
            }
            break;

        case AST_EXPRESSION_BINARY:
            if ((*expr)->data.binary_op != NULL) {
                if ((*expr)->data.binary_op->left != NULL) {
                    ast_expression_free(&((*expr)->data.binary_op->left));
                    (*expr)->data.binary_op->left = NULL;
                }

                if ((*expr)->data.binary_op->right != NULL) {
                    ast_expression_free(&((*expr)->data.binary_op->right));
                    (*expr)->data.binary_op->right = NULL;
                }

                if ((*expr)->data.binary_op->operator != NULL) {
                    free((*expr)->data.binary_op->operator);
                    (*expr)->data.binary_op->operator = NULL;
                }

                free((*expr)->data.binary_op);
                (*expr)->data.binary_op = NULL;
            }
            break;
    }

    free(*expr);
    *expr = NULL;
}

void ast_node_free(ast_node_t** node)
{
    if (node == NULL || *node == NULL) {
        return;
    }

    switch ((*node)->type) {
        case AST_STATEMENT_IF:
        case AST_STATEMENT_ELSEIF:
            if ((*node)->data.statement_if != NULL) {
                if ((*node)->data.statement_if->condition != NULL) {
                    ast_node_free(&((*node)->data.statement_if->condition));
					(*node)->data.statement_if->condition = NULL;
                }

                if ((*node)->data.statement_if->block != NULL) {
                    ast_node_free(&((*node)->data.statement_if->block));
					(*node)->data.statement_if->block = NULL;
                }

                if ((*node)->data.statement_if->elseifs != NULL) {
                    for (size_t i = 0; i < (*node)->data.statement_if->num_elseifs; i++) {
                        ast_node_free((ast_node_t**) &((*node)->data.statement_if->elseifs[i]));
					(*node)->data.statement_if->elseifs[i] = NULL;
                    }
                    free((*node)->data.statement_if->elseifs);
					(*node)->data.statement_if->elseifs = NULL;
                }

                if ((*node)->data.statement_if->else_block != NULL) {
                    ast_node_free(&((*node)->data.statement_if->else_block));
					(*node)->data.statement_if->else_block = NULL;
                }

                free((*node)->data.statement_if);
				(*node)->data.statement_if = NULL;
            }
            break;

        case AST_FUNCTION_DECLARATION:
            if ((*node)->data.function_declaration != NULL) {
                if ((*node)->data.function_declaration->body != NULL) {
                    ast_node_free(&((*node)->data.function_declaration->body));
					(*node)->data.function_declaration->body = NULL;
                }

                if ((*node)->data.function_declaration->name != NULL) {
                    free((*node)->data.function_declaration->name);
					(*node)->data.function_declaration->name = NULL;
                }

                free((*node)->data.function_declaration);
				(*node)->data.function_declaration = NULL;
            }
            break;

        case AST_STATEMENT_RETURN:
            if ((*node)->data.statement_return != NULL) {
                if ((*node)->data.statement_return->expression != NULL) {
                    ast_node_free(&((*node)->data.statement_return->expression));
					(*node)->data.statement_return->expression = NULL;
                }

                if ((*node)->data.statement_return->expression_value != NULL) {
                    ast_expression_data_free(&((*node)->data.statement_return->expression_value));
					(*node)->data.statement_return->expression_value = NULL;
                }

                free((*node)->data.statement_return);
				(*node)->data.statement_return = NULL;
            }
            break;

        case AST_STATEMENT_PRINT:
            if ((*node)->data.statement_print != NULL) {
                if ((*node)->data.statement_print->expression != NULL) {
                    ast_node_free(&((*node)->data.statement_print->expression));
					(*node)->data.statement_print->expression = NULL;
                }

                if ((*node)->data.statement_print->expression_value != NULL) {
                    ast_expression_data_free(&((*node)->data.statement_print->expression_value));
					(*node)->data.statement_print->expression_value = NULL;
                }

                free((*node)->data.statement_print);
				(*node)->data.statement_print = NULL;
            }
            break;

        case AST_BLOCK:
            if ((*node)->data.block != NULL) {
                if ((*node)->data.block->statements != NULL) {
                    for (size_t i = 0; i < (*node)->data.block->num_statements; i++) {
                        ast_node_free(&((*node)->data.block->statements[i]));
						(*node)->data.block->statements[i] = NULL;
                    }
                    free((*node)->data.block->statements);
					(*node)->data.block->statements = NULL;
                }

                free((*node)->data.block);
				(*node)->data.block = NULL;
            }
            break;

        case AST_EXPRESSION:
            if ((*node)->data.expression != NULL) {
                ast_expression_free(&((*node)->data.expression));
				(*node)->data.expression = NULL;

            }
            break;
    }

    free(*node);
    *node = NULL;
}

void parser_free(parser_t** parser)
{
    if (parser == NULL || *parser == NULL) {
        return;
    }

    if ((*parser)->functions != NULL) {
        if ((*parser)->functions->data != NULL) {
            for (size_t i = 0; i < (*parser)->functions->length; i++) {
                ast_node_free((ast_node_t**) &((*parser)->functions->data[i]));
                (*parser)->functions->data[i] = NULL;
            }

            free((*parser)->functions->data);
            (*parser)->functions->data = NULL;
        }

        free((*parser)->functions);
        (*parser)->functions = NULL;
    }

    if ((*parser)->expressions != NULL) {
        if ((*parser)->expressions->data != NULL) {
			for (size_t i = 0; i < (*parser)->expressions->length; i++) {
				ast_node_free((ast_node_t**) &((*parser)->expressions->data[i]));
				(*parser)->expressions->data[i] = NULL;
			}

            free((*parser)->functions->data);
            (*parser)->functions->data = NULL;
		}

        free((*parser)->expressions);
        (*parser)->expressions = NULL;
    }

    free(*parser);
    *parser = NULL;
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
		token_t* token = (token_t*) parser->lexer->tokens->data[parser->token_index];

		if (token->type == type) {
			return (token_t*) parser->lexer->tokens->data[parser->token_index++];
		}
	}

	return NULL;
}

bool parser_token_skip_ifhas(parser_t* parser, token_type_t type)
{
	if (parser->lexer->tokens->length > parser->token_index) {
		token_t* token = (token_t*) parser->lexer->tokens->data[parser->token_index];

		if (token->type == type) {
			parser->token_index++;
			return true;
		}
	}

	return false;
}

void parser_token_eat_nodata(parser_t* parser, token_type_t type)
{
	if (parser->lexer->tokens->length > parser->token_index) {
		token_t* token = (token_t*) parser->lexer->tokens->data[parser->token_index];

		if (token->type == type) {
			parser->token_index++;
		} else {
			printf("Error: Expected %s\n", token_type2str(type));
			exit(EXIT_FAILURE);
		}
	} else {
		printf("Error: Unexpected end of file\n");
		exit(EXIT_FAILURE);
	}
}

token_t* parser_token_eat(parser_t* parser, token_type_t type)
{
	if (parser->lexer->tokens->length > parser->token_index) {
		token_t* token = (token_t*) parser->lexer->tokens->data[parser->token_index];

		if (token->type == type) {
			return (token_t*) parser->lexer->tokens->data[parser->token_index++];
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

ast_node_t* parser_function(parser_t* parser)
{
	printf("Parsing function\n");

	ast_node_t* node = (ast_node_t*) malloc(sizeof(ast_node_t));
	node->type = AST_FUNCTION_DECLARATION;

	parser_token_eat_nodata(parser, TOKEN_TYPE_FUNCTION);

	token_t* name = parser_token_eat(parser, TOKEN_TYPE_IDENTIFIER);

	// if (parser->lexer->tokens->length > parser->token_index && ((token_t*) parser->lexer->tokens->data[parser->token_index])->type == TOKEN_TYPE_PARENTHESE_OPEN) {
	//     printf("Parsing parameters\n");
	//     parser->token_index++;

	//     if (parser->lexer->tokens->length > parser->token_index && ((token_t*) parser->lexer->tokens->data[parser->token_index])->type == TOKEN_TYPE_PARENTHESE_CLOSE) {
	//         parser->token_index++;
	//     } else {
	//         printf("Error: Expected closing parenthesis\n");
	//         exit(EXIT_FAILURE);
	//     }
	// }

	node->data.function_declaration = (ast_function_declaration_t*) malloc(sizeof(ast_function_declaration_t));
	node->data.function_declaration->name = strdup(name->value);
	node->data.function_declaration->body = parser_block(parser);

	if (node->data.function_declaration->body == NULL) {
		return NULL;
	}

	return node;
}

ast_node_t* parser_statement_print(parser_t* parser)
{
	printf("Parsing statement print\n");

	parser->token_index++;

	ast_node_t* node = (ast_node_t*) malloc(sizeof(ast_node_t));
	node->type = AST_STATEMENT_PRINT;

	node->data.statement_print = (ast_statement_print_t*) malloc(sizeof(ast_statement_print_t));
	node->data.statement_print->expression = parser_expression(parser);

	return node;
}

ast_node_t* parser_statement_return(parser_t* parser)
{
	printf("Parsing statement return\n");

	parser->token_index++;

	ast_node_t* node = (ast_node_t*) malloc(sizeof(ast_node_t));
	node->type = AST_STATEMENT_RETURN;

	node->data.statement_return = (ast_statement_return_t*) malloc(sizeof(ast_statement_return_t));
	node->data.statement_return->expression = parser_expression(parser);

	return node;
}

bool parser_expression_has(parser_t* parser)
{
	if (parser->lexer->tokens->length > parser->token_index) {
		token_t* tok = (token_t*) parser->lexer->tokens->data[parser->token_index];
		if (tok->type == TOKEN_TYPE_IDENTIFIER ||
			tok->type == TOKEN_TYPE_PLUS ||
			tok->type == TOKEN_TYPE_MINUS ||
			tok->type == TOKEN_TYPE_PARENTHESE_OPEN ||
			tok->type == TOKEN_TYPE_STRING ||
			tok->type == TOKEN_TYPE_NUMBER
		) {
			return true;
		}
	}
	return false;
}

ast_node_t* parser_statement_if(parser_t* parser) {
	printf("Parsing statement if\n");

	parser->token_index++; // Eating IF token

	size_t allocated_size = 2;

	ast_node_t* node = (ast_node_t*) malloc(sizeof(ast_node_t));
	node->type = AST_STATEMENT_IF;
	node->data.statement_if = (ast_statement_if_t*) malloc(sizeof(ast_statement_if_t));
	node->data.statement_if->condition = parser_expression(parser);
	node->data.statement_if->block = parser_block(parser);
	node->data.statement_if->num_elseifs = 0;
	node->data.statement_if->elseifs = (struct ast_node_t**) malloc(sizeof(ast_node_t*) * (allocated_size + 1));
	node->data.statement_if->else_block = NULL;

	while (parser->lexer->tokens->length > parser->token_index && ((token_t*) parser->lexer->tokens->data[parser->token_index])->type == TOKEN_TYPE_ELSEIF) {
		parser->token_index++; // Eating ELSEIF token

		if (parser_expression_has(parser)) {
			ast_node_t* elseif = (ast_node_t*) malloc(sizeof(ast_node_t));
			elseif->type = AST_STATEMENT_ELSEIF;

			elseif->data.statement_if = (ast_statement_if_t*) malloc(sizeof(ast_statement_if_t));
			elseif->data.statement_if->condition = parser_expression(parser);
			elseif->data.statement_if->block = parser_block(parser);
			elseif->data.statement_if->num_elseifs = 0;
			elseif->data.statement_if->elseifs = NULL;
			elseif->data.statement_if->else_block = NULL;

			if (node->data.statement_if->num_elseifs >= allocated_size) {
				allocated_size *= 2;
				node->data.statement_if->elseifs = (struct ast_node_t**) realloc(
					node->data.statement_if->elseifs, sizeof(ast_node_t*) * (allocated_size + 1)
				);

				if (node->data.statement_if->elseifs == NULL) {
					fprintf(stderr, "Error in parsing block: Memory reallocation failed.\n");
					exit(EXIT_FAILURE);
					break;
				}
			}

			node->data.statement_if->elseifs[node->data.statement_if->num_elseifs++] = (struct ast_node_t*) elseif;
		} else {
			node->data.statement_if->else_block = parser_block(parser);
			break;
		}
	}

	return node;
}

ast_node_t* parser_statement(parser_t* parser)
{
	printf("Parsing statement\n");

	ast_node_t* stmt = NULL;

	if (parser->lexer->tokens->length > parser->token_index) {
		token_t* tok = (token_t*) parser->lexer->tokens->data[parser->token_index];

		switch (tok->type) {
			case TOKEN_TYPE_RETURN:
				stmt = parser_statement_return(parser);
				break;

			case TOKEN_TYPE_IF:
				stmt = parser_statement_if(parser);
				break;
			
			case TOKEN_TYPE_PRINT:
				stmt = parser_statement_print(parser);
				break;
			
			case TOKEN_TYPE_SECTION_OPEN:
				stmt = parser_block(parser);
				break;
			
			default:
				if (parser_expression_has(parser)) {
					stmt = parser_expression(parser);
				} else {
					printf("Error: Unexpected token as statement %s\n", token_type2str(tok->type));
					exit(EXIT_FAILURE);
				}
				break;
		}
	}
	
	if (stmt == NULL) {
		printf("Error: Unexpected token as statement %s\n", token_type2str(((token_t*) parser->lexer->tokens->data[parser->token_index])->type));
		exit(EXIT_FAILURE);
	}
	return stmt;
}

ast_node_t* parser_expression(parser_t* parser)
{
	printf("Parsing expression\n");

	ast_node_t* node = (ast_node_t*) malloc(sizeof(ast_node_t));
	node->type = AST_EXPRESSION;
	node->data.expression = parser_expression_pratt(parser, PRECEDENCE_LOWEST);
	
	return node;
}

ast_expression_t* parser_expression_pratt(parser_t* parser, size_t precedence)
{
	printf("Parsing pratt\n");

	token_t* current_token = (token_t*) parser->lexer->tokens->data[parser->token_index];
	parser->token_index++;

	ast_expression_t* left = token_infos[current_token->type].nud(parser, current_token);

	while (precedence < token_infos[((token_t*) parser->lexer->tokens->data[parser->token_index])->type].precedence) {
		current_token = (token_t*) parser->lexer->tokens->data[parser->token_index];
		parser->token_index++;

		left = token_infos[current_token->type].led(parser, current_token, left);
	}

	return left;
}

ast_expression_t* led_equal(parser_t* parser, token_t* token, ast_expression_t* left)
{
	printf("Parsing operator assignment\n");

	ast_expression_t* binary_op_expr = (ast_expression_t*) malloc(sizeof(ast_expression_t));
	binary_op_expr->type = AST_EXPRESSION_ASSIGNMENT;
	binary_op_expr->data.assignment = (ast_expression_assignment_t*) malloc(sizeof(ast_expression_assignment_t));
	binary_op_expr->data.assignment->left = left;
	binary_op_expr->data.assignment->right = parser_expression_pratt(parser, token_infos[token->type].precedence);

	return binary_op_expr;
}

ast_expression_t* led_equal_equal(parser_t* parser, token_t* token, ast_expression_t* left)
{
	printf("Parsing operator equal\n");

	ast_expression_t* right = parser_expression_pratt(parser, token_infos[token->type].precedence);

	ast_expression_t* binary_op_expr = (ast_expression_t*) malloc(sizeof(ast_expression_t));
	binary_op_expr->type = AST_EXPRESSION_BINARY;
	binary_op_expr->data.binary_op = (ast_expression_binary_t*) malloc(sizeof(ast_expression_binary_t));
	binary_op_expr->data.binary_op->left = left;
	binary_op_expr->data.binary_op->right = right;
	binary_op_expr->data.binary_op->operator = strdup(token->value);

	return binary_op_expr;
}

ast_expression_t* led_and(parser_t* parser, token_t* token, ast_expression_t* left)
{
	printf("Parsing operator and\n");

	ast_expression_t* right = parser_expression_pratt(parser, token_infos[token->type].precedence);

	ast_expression_t* binary_op_expr = (ast_expression_t*) malloc(sizeof(ast_expression_t));
	binary_op_expr->type = AST_EXPRESSION_BINARY;
	binary_op_expr->data.binary_op = (ast_expression_binary_t*) malloc(sizeof(ast_expression_binary_t));
	binary_op_expr->data.binary_op->operator = strdup(token->value);
	binary_op_expr->data.binary_op->left = left;
	binary_op_expr->data.binary_op->right = right;

	return binary_op_expr;
}

ast_expression_t* led_or(parser_t* parser, token_t* token, ast_expression_t* left)
{
	printf("Parsing operator and\n");

	ast_expression_t* right = parser_expression_pratt(parser, token_infos[token->type].precedence);

	ast_expression_t* binary_op_expr = (ast_expression_t*) malloc(sizeof(ast_expression_t));
	binary_op_expr->type = AST_EXPRESSION_BINARY;
	binary_op_expr->data.binary_op = (ast_expression_binary_t*) malloc(sizeof(ast_expression_binary_t));
	binary_op_expr->data.binary_op->operator = strdup(token->value);
	binary_op_expr->data.binary_op->left = left;
	binary_op_expr->data.binary_op->right = right;

	return binary_op_expr;
}

ast_expression_t* led_plus_minus(parser_t* parser, token_t* token, ast_expression_t* left)
{
	printf("Parsing operator binary\n");

	ast_expression_t* right = parser_expression_pratt(parser, token_infos[token->type].precedence);

	ast_expression_t* binary_op_expr = (ast_expression_t*) malloc(sizeof(ast_expression_t));
	binary_op_expr->type = AST_EXPRESSION_BINARY;
	binary_op_expr->data.binary_op = (ast_expression_binary_t*) malloc(sizeof(ast_expression_binary_t));
	binary_op_expr->data.binary_op->operator = strdup(token->value);
	binary_op_expr->data.binary_op->left = left;
	binary_op_expr->data.binary_op->right = right;

	return binary_op_expr;
}

ast_expression_t* nud_bool(parser_t* parser, token_t* token)
{
	printf("Parsing bool\n");

	ast_expression_t* literal_expr = (ast_expression_t*) malloc(sizeof(ast_expression_t));
	literal_expr->type = AST_EXPRESSION_LITERAL;

	literal_expr->data.literal = (ast_literal_t*) malloc(sizeof(ast_literal_t));
	literal_expr->data.literal->left = NULL;
	literal_expr->data.literal->right = NULL;
	literal_expr->data.literal->type = VALUE_TYPE_BOOL;
	literal_expr->data.literal->bool_value = strcmp(token->value, "درست") == 0;

	return literal_expr;
}

ast_expression_t* nud_number(parser_t* parser, token_t* token)
{
	printf("Parsing number\n");

	ast_expression_t* literal_expr = (ast_expression_t*) malloc(sizeof(ast_expression_t));
	literal_expr->type = AST_EXPRESSION_LITERAL;

	literal_expr->data.literal = (ast_literal_t*) malloc(sizeof(ast_literal_t));
	literal_expr->data.literal->left = NULL;
	literal_expr->data.literal->right = NULL;
	literal_expr->data.literal->type = VALUE_TYPE_INT;
	literal_expr->data.literal->int_value = atoi(token->value);

	return literal_expr;
}

ast_expression_t* nud_string(parser_t* parser, token_t* token)
{
	printf("Parsing string\n");

	ast_expression_t* literal_expr = (ast_expression_t*) malloc(sizeof(ast_expression_t));
	literal_expr->type = AST_EXPRESSION_LITERAL;

	literal_expr->data.literal = (ast_literal_t*) malloc(sizeof(ast_literal_t));
	literal_expr->data.literal->left = NULL;
	literal_expr->data.literal->right = NULL;
	literal_expr->data.literal->type = VALUE_TYPE_STRING;
	literal_expr->data.literal->string_value = token->value;

	return literal_expr;
}

ast_expression_t* nud_identifier(parser_t* parser, token_t* token)
{
	printf("Parsing identifier\n");

	char* identifier = strdup(token->value);

	ast_expression_t* expr = (ast_expression_t*) malloc(sizeof(ast_expression_t));

	// Check if current token is (
	if (parser_token_skip_ifhas(parser, TOKEN_TYPE_PARENTHESE_OPEN)) {
		expr->type = AST_EXPRESSION_FUNCTION_CALL;
		expr->data.function_call = (ast_function_call_t*) malloc(sizeof(ast_function_call_t));
		expr->data.function_call->name = identifier;
		expr->data.function_call->arguments = NULL;
		// expr->data.function_call->arguments = array_create(3);

		// Eating ) token
		parser_token_eat_nodata(parser, TOKEN_TYPE_PARENTHESE_CLOSE);
	} else {
		expr->type = AST_EXPRESSION_IDENTIFIER;
		expr->data.identifier = (ast_identifier_t*) malloc(sizeof(ast_identifier_t));
		expr->data.identifier->name = identifier;
	}
	
	return expr;
}

ast_expression_t* nud_parentheses(parser_t* parser, token_t* token)
{
	printf("Parsing parentheses\n");

	ast_expression_t* expression_node = parser_expression_pratt(parser, PRECEDENCE_LOWEST);

	parser_token_eat_nodata(parser, TOKEN_TYPE_PARENTHESE_CLOSE);

	return expression_node;
}

ast_node_t* parser_block(parser_t* parser)
{
    printf("Parsing block\n");

    size_t allocated_size = 5;

    ast_node_t* block_node = (ast_node_t*)malloc(sizeof(ast_node_t));
    block_node->type = AST_BLOCK;
    block_node->data.block = (ast_block_t*)malloc(sizeof(ast_block_t));
    block_node->data.block->num_statements = 0;
    block_node->data.block->statements = (ast_node_t**)malloc(sizeof(ast_node_t*) * (allocated_size + 1));

    parser_token_eat_nodata(parser, TOKEN_TYPE_SECTION_OPEN);

	while (
		parser->lexer->tokens->length > parser->token_index &&
		((token_t*)parser->lexer->tokens->data[parser->token_index])->type != TOKEN_TYPE_SECTION_CLOSE
	) {
		ast_node_t* statement = parser_statement(parser);

		if (block_node->data.block->num_statements >= allocated_size) {
			allocated_size *= 2;
			block_node->data.block->statements = (ast_node_t**)realloc(
				block_node->data.block->statements, sizeof(ast_node_t*) * (allocated_size + 1)
			);

			if (block_node->data.block->statements == NULL) {
				fprintf(stderr, "Error in parsing block: Memory reallocation failed.\n");
				exit(EXIT_FAILURE);
			}
		}

		block_node->data.block->statements[block_node->data.block->num_statements++] = statement;
	}

	if (block_node->data.block->num_statements == 0) {
		free(block_node->data.block->statements);
		block_node->data.block->statements = NULL;
	}

    parser_token_eat_nodata(parser, TOKEN_TYPE_SECTION_CLOSE);

    return block_node;
}

void parser_parse(parser_t* parser)
{
	if (parser->lexer->tokens->length == 1 &&
		((ast_node_t*) parser->lexer->tokens->data[0])->type == TOKEN_TYPE_EOF
	) {
		return;
	}

	while (parser->token_index < parser->lexer->tokens->length) {
		token_t* current_token = (token_t*) parser->lexer->tokens->data[parser->token_index];

		switch (current_token->type) {
			case TOKEN_TYPE_EOF:
				parser->token_index++;
				break;
			
			case TOKEN_TYPE_FUNCTION:
				ast_node_t* function_node = parser_function(parser);
				if (parser->functions && function_node != NULL) {
					array_push(parser->functions, function_node);
				}
				break;

			default:
				if (parser_expression_has(parser)) {
					ast_node_t* expression_node = parser_expression(parser);
					if (parser->expressions && expression_node != NULL) {
						array_push(parser->expressions, expression_node);
					}
				} else {
					printf("Error: bad token as statement %s\n", token_type2str(current_token->type));
					exit(EXIT_FAILURE);
				}
				break;
		}
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
				printf("<Type>%s</Type>\n", literal_type2name(expr->data.literal->type));

				print_indentation(indent_level + 2);
				if (expr->data.literal->type == VALUE_TYPE_STRING) {
					printf("<Value>%s</Value>\n", expr->data.literal->string_value);
				} else if (expr->data.literal->type == VALUE_TYPE_INT) {
					printf("<Value>%d</Value>\n", expr->data.literal->int_value);
				} else if (expr->data.literal->type == VALUE_TYPE_BOOL) {
					printf("<Value>%s</Value>\n", expr->data.literal->bool_value ? "True" : "False");
				} else {
					printf("<!-- Unhandled Literal Type -->\n");
				}

			print_indentation(indent_level + 1);
			printf("</Literal>\n");
			break;
		
		case AST_EXPRESSION_FUNCTION_CALL:
			print_indentation(indent_level + 1);
			printf("<FunctionCall>\n");

				print_indentation(indent_level + 2);
				printf("<Name>%s</Name>\n", expr->data.function_call->name);

				if (expr->data.function_call->arguments == NULL) {
					print_indentation(indent_level + 2);
					printf("<Arguments />\n");
				} else {
					print_indentation(indent_level + 2);
					printf("<Arguments>\n");

						for (size_t i = 0; i < expr->data.function_call->arguments->length; i++) {
							print_indentation(indent_level + 3);
							printf("<Argument ... />\n");
						}

					print_indentation(indent_level + 2);
					printf("</Arguments>\n");
				}

			print_indentation(indent_level + 1);
			printf("</FunctionCall>\n");
			break;

		case AST_EXPRESSION_IDENTIFIER:
			print_indentation(indent_level + 1);
			printf("<Identifier>\n");

				print_indentation(indent_level + 2);
				printf("<Name>%s</Name>\n", expr->data.identifier->name);

			print_indentation(indent_level + 1);
			printf("</Identifier>\n");
			break;

		case AST_EXPRESSION_BINARY:
			print_indentation(indent_level + 1);
			printf("<BinaryOperator>\n");

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
			printf("</BinaryOperator>\n");
			break;

		case AST_EXPRESSION_ASSIGNMENT:
			print_indentation(indent_level + 1);
			printf("<Assignment>\n");

				print_indentation(indent_level + 2);
				printf("<Left>\n");

					print_indentation(indent_level + 3);
					print_xml_ast_expression(expr->data.assignment->left, indent_level + 3);

				print_indentation(indent_level + 2);
				printf("</Left>\n");

				print_indentation(indent_level + 2);
				printf("<Right>\n");

					print_indentation(indent_level + 3);
					print_xml_ast_expression(expr->data.assignment->right, indent_level + 3);

				print_indentation(indent_level + 2);
				printf("</Right>\n");

			print_indentation(indent_level + 1);
			printf("</Assignment>\n");
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
		case AST_STATEMENT_IF:
		case AST_STATEMENT_ELSEIF:
			printf("<StatementIf>\n");

				print_indentation(indent_level + 1);
				printf("<Condition>\n");

					print_indentation(indent_level + 2);
					print_xml_ast_expression(node->data.statement_if->condition->data.expression, indent_level + 2);

				print_indentation(indent_level + 1);
				printf("</Condition>\n");

				print_xml_ast_node(node->data.statement_if->block, indent_level + 1);

				if (node->data.statement_if->num_elseifs > 0) {
					print_indentation(indent_level + 1);
					printf("<ElseIfBlocks>\n");

						for (size_t i = 0; i < node->data.statement_if->num_elseifs; i++) {
							print_xml_ast_node((ast_node_t*) node->data.statement_if->elseifs[i], indent_level + 2);
						}

					print_indentation(indent_level + 1);
					printf("</ElseIfBlocks>\n");
				}

				if (node->data.statement_if->else_block != NULL) {
					print_indentation(indent_level + 1);
					printf("<Else>\n");

						if (node->data.statement_if->else_block != NULL) {
							print_xml_ast_node(node->data.statement_if->else_block, indent_level + 2);
						}
					
					print_indentation(indent_level + 1);
					printf("</Else>\n");
				}

			print_indentation(indent_level);
			printf("</StatementIf>\n");
			break;

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

void print_xml_ast_tree(parser_t* parser)
{
	printf("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
	printf("<AST>\n");

		print_indentation(1);
		printf("<Functions>\n");

			if (parser->functions != NULL) {
				for (size_t i = 0; i < parser->functions->length; i++) {
					print_xml_ast_node(parser->functions->data[i], 2);
				}
			}

		print_indentation(1);
		printf("</Functions>\n");

		////////////////////////////////////////////
		////////////////////////////////////////////

		print_indentation(1);
		printf("<Expressions>\n");

			if (parser->expressions != NULL) {
				for (size_t i = 0; i < parser->expressions->length; i++) {
					print_xml_ast_node(parser->expressions->data[i], 2);
				}
			}
		
		print_indentation(1);
		printf("</Expressions>\n");

	printf("</AST>\n");
}

interpreter_t* interpreter_create(parser_t* parser)
{
	interpreter_t* interpreter = (interpreter_t*) malloc(sizeof(interpreter_t));
	interpreter->parser = parser;
	return interpreter;
}

ast_node_t* interpreter_statement_if(ast_statement_if_t* node, interpreter_t* interpreter)
{
    // printf("If\n");

    if (interpreter_expression_truly(node->condition->data.expression, interpreter)) {
        return interpreter_interpret_once(node->block, interpreter);
    } else {
        for (size_t i = 0; i < node->num_elseifs; i++) {
            ast_node_t* elseif = (ast_node_t*) node->elseifs[i];
			if (interpreter_expression_truly(elseif->data.statement_if->condition->data.expression, interpreter)) {
				return interpreter_interpret_once(elseif->data.statement_if->block, interpreter);
			}
        }

        if (node->else_block != NULL) {
            return interpreter_interpret_once(node->else_block, interpreter);
        }
    }

    return NULL;
}

ast_node_t* interpreter_interpret_once(ast_node_t* node, interpreter_t* interpreter)
{
	// printf("Interpreter Interpret Once\n");

	switch (node->type) {
		case AST_BLOCK:
			node->data.block = interpreter_block(node->data.block, interpreter);
			return node;
			break;

		case AST_FUNCTION_DECLARATION:
			node->data.function_declaration = interpreter_function_declaration(node->data.function_declaration, interpreter);
			return node;
			break;

		case AST_STATEMENT_RETURN:
			node->data.statement_return = interpreter_statement_return(node->data.statement_return, interpreter);
			return node;
			break;
		
		case AST_STATEMENT_IF:
			return interpreter_statement_if(node->data.statement_if, interpreter);
			break;

		case AST_STATEMENT_PRINT:
			node->data.statement_print = interpreter_statement_print(node->data.statement_print, interpreter);
			interpreter_expression_data(node->data.statement_print->expression_value);
			return node;
			break;

		case AST_EXPRESSION:
			ast_literal_t* val = interpreter_expression(node->data.expression, interpreter);

			node->data.expression = (ast_expression_t*) malloc(sizeof(ast_expression_t));
			node->data.expression->type = AST_EXPRESSION_LITERAL;
			node->data.expression->data.literal = val;
			return node;
			break;

		default:
			printf("interpreter_interpret - default\n");
			break;
	}

	return NULL;
}

interpreter_t* interpreter_interpret(interpreter_t* interpreter)
{
	printf("Interpreter Interpret\n");

	if (interpreter == NULL || interpreter->parser == NULL) {
		return NULL;
	}

	// Scope entry
	pushSymbolTable(symbolTableStack);

	// Expressions
	if (interpreter->parser->expressions != NULL) {
		for (size_t i = 0; i < interpreter->parser->expressions->length; i++) {
			printf("Interpreting global expression\n");
			ast_node_t* expression = (ast_node_t*) interpreter->parser->expressions->data[i];
			ast_literal_t* val = interpreter_expression(expression->data.expression, interpreter);
			expression->type = AST_EXPRESSION_LITERAL;
			expression->data.expression->data.literal = val;
			interpreter->parser->expressions->data[i] = expression;
		}
	}

	// Functions
	if (interpreter->parser->functions != NULL) {
		for (size_t i = 0; i < interpreter->parser->functions->length; i++) {
			ast_node_t* function = (ast_node_t*) interpreter->parser->functions->data[i];
			ast_function_declaration_t* val;

			if (strcmp(function->data.function_declaration->name, "سلام") == 0) {
				val = interpreter_function_declaration(function->data.function_declaration, interpreter);
				function->type = AST_FUNCTION_DECLARATION;
				function->data.function_declaration = val;

				interpreter->parser->functions->data[i] = function;
			}
		}
	}

	// Scope exit
	popSymbolTable(symbolTableStack);

	return interpreter;
}

ast_function_declaration_t* interpreter_function_declaration(ast_function_declaration_t* stmt, interpreter_t* interpreter)
{
	printf("Function Declaration: %s\n", stmt->name);

	stmt->body->data.block = interpreter_block(stmt->body->data.block, interpreter);

	return stmt;
}

void interpreter_expression_data(ast_literal_t* data)
{
	if (data == NULL) {
		printf("NULL\n");
		return;
	} else if (data->type == VALUE_TYPE_INT) {
		printf("%d\n", data->int_value);
	} else if (data->type == VALUE_TYPE_FLOAT) {
		printf("%f\n", data->float_value);
	} else if (data->type == VALUE_TYPE_BOOL) {
		printf("%s\n", data->bool_value == true ? "True" : "False");
	} else if (data->type == VALUE_TYPE_STRING) {
		printf("%s\n", data->string_value);
	} else {
		printf("Unknown\n");
	}
}

ast_statement_return_t* interpreter_statement_return(ast_statement_return_t* stmt, interpreter_t* interpreter)
{
	// printf("Return Statement\n");

	stmt->expression_value = interpreter_expression(stmt->expression->data.expression, interpreter);

	return stmt;
}

ast_statement_print_t* interpreter_statement_print(ast_statement_print_t* stmt, interpreter_t* interpreter)
{
	// printf("Print Statement\n");

	stmt->expression_value = interpreter_expression(stmt->expression->data.expression, interpreter);

	return stmt;
}

ast_block_t* interpreter_block(ast_block_t* block, interpreter_t* interpreter)
{
	// printf("Block\n");

	// Scope entry
	pushSymbolTable(symbolTableStack);

	for (size_t i = 0; i < block->num_statements; i++) {
		ast_node_t* node = interpreter_interpret_once(block->statements[i], interpreter);
		block->statements[i] = node;
		// TODO: break on return statement
	}

	// Scope exit
	popSymbolTable(symbolTableStack);

	return block;
}

ast_literal_t* interpreter_literal(ast_literal_t* expr)
{
	if (expr == NULL) {
		return NULL;
	}

	return expr;
}

void ast_expression_data_free(ast_literal_t** val)
{
    printf("ast_expression_data_free\n");

    if (val == NULL || *val == NULL) {
		printf("is null?\n");
        return;
    }

    if ((*val)->type == VALUE_TYPE_STRING) {
		printf("has string\n");
        if ((*val)->string_value != NULL) {
            free((*val)->string_value);
            (*val)->string_value = NULL;
        }
    } else if ((*val)->type == VALUE_TYPE_INT) {
        // Nothing to free
    } else if ((*val)->type == VALUE_TYPE_BOOL) {
        // Nothing to free
    } else if ((*val)->type == VALUE_TYPE_FLOAT) {
        // Nothing to free
    }

    if ((*val)->left != NULL) {
        printf("ast_expression_data_free left\n");
        ast_expression_data_free(&((*val)->left));
        (*val)->left = NULL;
    }

    if ((*val)->right != NULL) {
        printf("ast_expression_data_free right\n");
        ast_expression_data_free(&((*val)->right));
        (*val)->right = NULL;
    }

	printf("let's free it's at all\n");

    free(*val);
    *val = NULL;
}

ast_literal_t* interpreter_identifier(ast_identifier_t* expr, interpreter_t* interpreter)
{
	printf("Variable: %s\n", expr->name);

	ast_literal_t* val = findInSymbolTable(symbolTableStack, expr->name);
	if (val != NULL) {
		// printf("Variable found: %s\n", expr->name);
		return val;
	} else {
		printf("Error: Variable not found: %s\n", expr->name);
		exit(EXIT_FAILURE);
	}

	return NULL;
}

ast_literal_t* interpreter_operator_binary(ast_expression_binary_t* binary_op, interpreter_t* interpreter) {
	bool invalid = false;

	ast_literal_t* left = interpreter_expression(binary_op->left, interpreter);
	ast_literal_t* right = interpreter_expression(binary_op->right, interpreter);

	ast_literal_t* res = (ast_literal_t*) malloc(sizeof(ast_literal_t));
	res->left = left;
	res->right = right;
	// res->left = NULL;
	// res->right = NULL;

	if (left == NULL || right == NULL) {
		printf("Error: cannot calculate binary operator for NULL values!\n");
		invalid = true;
	} else if (left->type == VALUE_TYPE_STRING && right->type == VALUE_TYPE_STRING && strcmp(binary_op->operator, "+") == 0) {
		res->type = VALUE_TYPE_STRING;
		size_t leftlen = strlen(left->string_value);
		size_t rightlen = strlen(right->string_value);

		if (leftlen == 0 && rightlen == 0) {
			res->string_value = strdup("");
		} else if (leftlen != 0 && rightlen != 0) {
			size_t new_size = sizeof(char*) * (leftlen + rightlen + 1);

			res->string_value = (char*) malloc(new_size);

			strcpy(res->string_value, left->string_value);
			strcat(res->string_value, right->string_value);
		} else if (leftlen == 0) {
			res->string_value = strdup(right->string_value);
		} else if (rightlen == 0) {
			res->string_value = strdup(left->string_value);
		}
	} else if (strcmp(binary_op->operator, "==") == 0) {
		if (left->type == VALUE_TYPE_INT && right->type == VALUE_TYPE_FLOAT) {
			res->type = VALUE_TYPE_BOOL;
			res->bool_value = left->int_value == right->float_value ? true : false;
		} else if (left->type == VALUE_TYPE_FLOAT && right->type == VALUE_TYPE_INT) {
			res->type = VALUE_TYPE_BOOL;
			res->bool_value = left->float_value == right->int_value ? true : false;
		} else if (left->type != right->type) {
			res->type = VALUE_TYPE_BOOL;
			res->bool_value = false;
		} else if (left->type == VALUE_TYPE_STRING) {
			res->type = VALUE_TYPE_BOOL;
			res->bool_value = strcmp(left->string_value, right->string_value) == 0 ? true : false;
		} else if (left->type == VALUE_TYPE_INT) {
			res->type = VALUE_TYPE_BOOL;
			res->bool_value = left->int_value == right->int_value ? true : false;
		} else if (left->type == VALUE_TYPE_BOOL) {
			res->type = VALUE_TYPE_BOOL;
			res->bool_value = left->bool_value == right->bool_value ? true : false;
		} else if (left->type == VALUE_TYPE_FLOAT) {
			res->type = VALUE_TYPE_BOOL;
			res->bool_value = left->float_value == right->float_value ? true : false;
		} else {
			printf("Error: cannot compare unknown types!\n");
			invalid = true;
		}
	} else if ((left->type != VALUE_TYPE_INT && left->type != VALUE_TYPE_BOOL) || (right->type != VALUE_TYPE_INT && right->type != VALUE_TYPE_BOOL)) {
		printf("Error: cannot calculate binary operator for non-int values!\n");
		invalid = true;
	} else if (strcmp(binary_op->operator, "+") == 0) {
		res->type = VALUE_TYPE_INT;
		res->int_value = left->int_value + right->int_value;
	} else if (strcmp(binary_op->operator, "-") == 0) {
		res->type = VALUE_TYPE_INT;
		res->int_value = left->int_value - right->int_value;
	} else if (strcmp(binary_op->operator, "*") == 0) {
		res->type = VALUE_TYPE_INT;
		res->int_value = left->int_value * right->int_value;
	} else if (strcmp(binary_op->operator, "و") == 0) {
		res->type = VALUE_TYPE_BOOL;
		res->int_value = left->int_value && right->int_value;
	} else if (strcmp(binary_op->operator, "یا") == 0) {
		res->type = VALUE_TYPE_BOOL;
		res->int_value = left->int_value || right->int_value;
	} else if (strcmp(binary_op->operator, "/") == 0) {
		if (right->int_value == 0) {
			printf("Error: cannot divide by zero!\n");
			invalid = true;
		} else {
			res->type = VALUE_TYPE_INT;
			res->int_value = left->int_value / right->int_value;
		}
	} else {
		printf("Error: unknown operator: %s\n", binary_op->operator);
		invalid = true;
	}

	if (invalid) {
		ast_expression_data_free(&res);
		ast_expression_data_free(&left);
		ast_expression_data_free(&right);
		exit(EXIT_FAILURE);
		return NULL;
	}
	return res;
}

ast_literal_t* interpreter_function_call(ast_function_call_t* node, interpreter_t* interpreter)
{
    printf("Function Call: %s\n", node->name);

	// Check if functions exists in interpreter->parser->...
	bool exists = false;
	ast_function_declaration_t* func_exists = NULL;

	if (interpreter->parser->functions != NULL) {
		for (size_t i = 0; i < interpreter->parser->functions->length; i++) {
			ast_node_t* func = interpreter->parser->functions->data[i];
			if (func != NULL && func->data.function_declaration != NULL && func->data.function_declaration->name != NULL && strcmp(func->data.function_declaration->name, node->name) == 0) {
				func_exists = func->data.function_declaration;
				exists = true;
				break;
			}
		}
	}
	
	if (exists && func_exists != NULL) {
		// TODO: using `func_exists`
		ast_literal_t* val = (ast_literal_t*) malloc(sizeof(ast_literal_t));
		val->type = VALUE_TYPE_INT;
		val->left = NULL;
		val->right = NULL;
		val->int_value = 135;
		return val;
	} else {
		printf("Error: function not exists!\n");
		exit(EXIT_FAILURE);
		return NULL;
	}
}


bool interpreter_expression_truly(ast_expression_t* expr, interpreter_t* interpreter)
{
	// printf("Truly\n");

	ast_literal_t* res = interpreter_expression(expr, interpreter);
	if (res->type == VALUE_TYPE_BOOL) {
		return res->bool_value;
	} else if (res->type == VALUE_TYPE_INT) {
		return res->int_value != 0;
	} else if (res->type == VALUE_TYPE_FLOAT) {
		return res->float_value != 0.0;
	} else if (res->type == VALUE_TYPE_STRING) {
		return res->string_value != NULL && strlen(res->string_value) > 0;
	}

	return false;
}

ast_literal_t* interpreter_expression_assignment(ast_expression_assignment_t* expr, interpreter_t* interpreter)
{
	// printf("Assignment\n");

	if (expr->left->type == AST_EXPRESSION_IDENTIFIER) {
		ast_literal_t* right = interpreter_expression(expr->right, interpreter);

		char* identifier = expr->left->data.identifier->name;
		ast_literal_t* variable = findInSymbolTableCurrent(symbolTableStack, identifier);
		if (variable != NULL) {
			printf("Found an exiting variable %s\n", identifier);
			variable->left = NULL;
			variable->right = right;
			variable->type = right->type;

			if (right->type == VALUE_TYPE_STRING) {
				variable->string_value = right->string_value;
			} else if (right->type == VALUE_TYPE_INT) {
				variable->int_value = right->int_value;
			} else if (right->type == VALUE_TYPE_BOOL) {
				variable->bool_value = right->bool_value;
			} else if (right->type == VALUE_TYPE_FLOAT) {
				variable->float_value = right->float_value;
			}
		} else {
			printf("Saving %s variable\n", identifier);
			addToSymbolTable(symbolTableStack, identifier, right);
		}
		free(identifier);
		// ast_expression_data_free(&(variable)); // TODO
	} else {
		printf("Error: Assignment to non-variable\n");
		exit(EXIT_FAILURE);
	}

	return NULL;
}

ast_literal_t* interpreter_expression(ast_expression_t* expr, interpreter_t* interpreter)
{
	if (expr == NULL) {
		return NULL;
	}

	switch (expr->type) {
		case AST_EXPRESSION_LITERAL:
			return interpreter_literal(expr->data.literal);

		case AST_EXPRESSION_IDENTIFIER:
			return interpreter_identifier(expr->data.identifier, interpreter);

		case AST_EXPRESSION_BINARY:
			return interpreter_operator_binary(expr->data.binary_op, interpreter);

		case AST_EXPRESSION_FUNCTION_CALL:
		    return interpreter_function_call(expr->data.function_call, interpreter);

		case AST_EXPRESSION_ASSIGNMENT:
			return interpreter_expression_assignment(expr->data.assignment, interpreter);

		default:
			printf("Error: default expr type: %d\n", expr->type);
			exit(EXIT_FAILURE);
	}
	
	return NULL;
}

void interpreter_free(interpreter_t** interpreter)
{
	// while (symbolTableStack != NULL) {
	// 	popSymbolTable();
	// }
	// free(symbolTableStack);

	free(*interpreter);
	*interpreter = NULL;
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

	print_xml_ast_tree(parser);

	interpreter_t* interpreter = interpreter_create(parser);
	interpreter_interpret(interpreter);

	printf("====================================\n");

	printf("free lexer\n");
	lexer_free(&lexer);
	printf("end lexer free\n");

	printf("free parser\n");
	parser_free(&parser);
	printf("end parser free\n");

	printf("free interpreter\n");
	interpreter_free(&interpreter);
	printf("end interpreter free\n");

	exit(EXIT_SUCCESS);
}
