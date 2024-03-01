#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <locale.h>
#include <stdbool.h>
#include <string.h>
#include <limits.h>

struct ast_expression;

char* intToString(int value)
{
    int sign = (value < 0) ? -1 : 1;

    int temp = value;
    int numDigits = (value == 0) ? 1 : 0;
    while (temp != 0) {
        temp /= 10;
        numDigits++;
    }

    char* result = (char*) malloc(numDigits + 1 + (sign == -1 ? 1 : 0));
    if (result != NULL) {
        if (sign == -1) {
            result[0] = '-';
        }

        for (int i = numDigits - 1 + (sign == -1 ? 1 : 0); i >= (sign == -1 ? 1 : 0); i--) {
            result[i] = '0' + abs(value % 10);
            value /= 10;
        }

        result[numDigits + (sign == -1 ? 1 : 0)] = '\0';
    }

    return result;
}

typedef enum {
	VALUE_TYPE_NULL,
	VALUE_TYPE_INT,
	VALUE_TYPE_FLOAT,
	VALUE_TYPE_BOOL,
	VALUE_TYPE_STRING,
	VALUE_TYPE_ARGUMENT,
} ast_literal_type_t;

char* literal_type2name(ast_literal_type_t type)
{
	switch (type) {
		case VALUE_TYPE_NULL: return "NULL";
		case VALUE_TYPE_INT: return "INT";
		case VALUE_TYPE_FLOAT: return "FLOAT";
		case VALUE_TYPE_BOOL: return "BOOL";
		case VALUE_TYPE_STRING: return "STRING";
		default: return "TYPE_UNKNOWN";
	}
}

struct ast_literal_t;

typedef struct {
	char* name;
	struct ast_expression_t* value;
} ast_argument_t;

typedef struct ast_literal_t {
	ast_literal_type_t type;
	union {
		int int_value;
		bool bool_value;
		float float_value;
		char* string_value;
		ast_argument_t* argument_value;
	};

	struct ast_expression_t* main; // To have a reference into parser ast so we can free it later after interpreter
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
	bool is_function_call;
	struct SymbolTableStack* next;
} SymbolTableStack;

SymbolTableStack* symbolTableStack = NULL;
SymbolTableStack* symbolGlobalTableStack = NULL;

typedef enum {
	// Values
	TOKEN_TYPE_IDENTIFIER,
	TOKEN_TYPE_NUMBER,
	TOKEN_TYPE_STRING,

	// Keywords
	TOKEN_TYPE_FUNCTION, // عملکرد
	TOKEN_TYPE_RETURN, // برگشت
	TOKEN_TYPE_BREAK, // توقف
	TOKEN_TYPE_CONTINUE, // ادامه
	TOKEN_TYPE_PRINT, // نمایش
	TOKEN_TYPE_IF, // اگر
	TOKEN_TYPE_NULL, // پوچ

	TOKEN_TYPE_UNTIL, // تا
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

	TOKEN_TYPE_COMMA, // ,

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
	{"ادامه", TOKEN_TYPE_CONTINUE},
	{"توقف", TOKEN_TYPE_BREAK},
	{"نمایش", TOKEN_TYPE_PRINT},
	{"واگرنه", TOKEN_TYPE_ELSEIF},
	{"اگر", TOKEN_TYPE_IF},
	{"پوچ", TOKEN_TYPE_NULL},
	{"تا", TOKEN_TYPE_UNTIL},
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
	array_t* arguments;
} ast_function_declaration_t;

typedef struct {
	struct ast_expression* expression;
	struct ast_literal_t* expression_value;
} ast_statement_return_t;

typedef struct {
	struct ast_expression* expression;
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
	AST_STATEMENT_CONTINUE,
	AST_STATEMENT_BREAK,
	AST_STATEMENT_PRINT,
	AST_STATEMENT_IF,
	AST_STATEMENT_UNTIL,
	AST_STATEMENT_ELSEIF,
	AST_BLOCK,
	AST_EXPRESSION,
} ast_node_type_t;

typedef struct {
	ast_expression_t* condition;
	struct ast_node* block;
} ast_statement_until_t;

typedef struct {
	ast_expression_t* condition;
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
		ast_statement_until_t* statement_until;
		ast_function_call_t* function_call;
	} data;
} ast_node_t;

typedef struct {
	char* name;
	ast_expression_t* expression;
} ast_variable_declaration_t;

typedef struct {
	lexer_t** lexer;
	size_t token_index;
	array_t* functions;
	array_t* expressions;
} parser_t;

typedef struct {
	int return_code;
	parser_t** parser;
	bool is_global_scope;
} interpreter_t;

unsigned int hash(const char* str, size_t capacity)
{
	unsigned int hash = 0;
	while (*str) {
		hash = (hash * 31) + (*str++);
	}
	return hash % capacity;
}

SymbolTable* createSymbolTable(size_t capacity)
{
	SymbolTable* table = (SymbolTable*) malloc(sizeof(SymbolTable));
	table->entries = (SymbolTableEntry**) calloc(capacity, sizeof(SymbolTableEntry*) );
	table->size = 0;
	table->capacity = capacity;
	return table;
}

void pushSymbolTable(SymbolTableStack** ts, bool is_function_call)
{
	// printf("create scope with %d state\n", is_function_call ? 1 : 0);
    SymbolTable* table = createSymbolTable(16);
    SymbolTableStack* newScope = (SymbolTableStack*) malloc(sizeof(SymbolTableStack));
    newScope->table = table;
	newScope->is_function_call = is_function_call;
    newScope->next = *ts;
    *ts = newScope;
}

void popSymbolTable(SymbolTableStack** ts)
{
	if (ts == NULL || *ts == NULL) {
		return;
	}

	SymbolTableStack* top =* ts;
	*ts = top->next;

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
				// ast_expression_free_data(&(entry->data));
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

static SymbolTableEntry* findSymbolInParentScopes(SymbolTableStack* ts, const char* identifier)
{
	if (ts == NULL) {
		return NULL;
	}

	SymbolTable* table = ts->table;
	unsigned int index = hash(identifier, table->capacity);

	SymbolTableEntry* entry = table->entries[index];
	while (entry != NULL) {
		if (strcmp(entry->identifier, identifier) == 0) {
			return entry;
		}
		entry = entry->next;
	}

	if (ts->is_function_call == true) return NULL;
	return findSymbolInParentScopes(ts->next, identifier);
}

void addToSymbolTable(SymbolTableStack* ts, const char* identifier, ast_literal_t* value)
{
	if (ts == NULL) {
		return;
	}

	SymbolTableEntry* existingEntry = findSymbolInParentScopes(ts, identifier);

	if (existingEntry != NULL) {
		existingEntry->data = value;
		return;
	}

	SymbolTable* table = ts->table;
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
	if (currentScope == NULL) {
		return NULL;
	}

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

ast_literal_t* findInSymbolTable(SymbolTableStack* currentScope, const char* identifier, bool wantsGlobal)
{
	while (currentScope != NULL) {
		// printf("looking for %s on a scope %d\n", identifier, currentScope->is_function_call ? 1 : 0);
		ast_literal_t* data = findInSymbolTableCurrent(currentScope, identifier);
		if (data != NULL) {
			return data;
		}

		if (currentScope->is_function_call == true) {
			// printf("this scope is call enabled, so break loop (%d)!\n", wantsGlobal ? 1 : 0);
			break;
		}
		currentScope = currentScope->next;
	}

	if (wantsGlobal == true) {
		return findInSymbolTable(symbolGlobalTableStack, identifier, false);
	}
	return NULL;
}

bool interpreter_expression_truly(ast_expression_t* expr, interpreter_t* interpreter);

interpreter_t* interpreter_interpret(interpreter_t* interpreter);
ast_node_t* interpreter_interpret_once(ast_node_t* node, interpreter_t* interpreter, token_type_t parent_type);

void interpreter_expression_data(ast_literal_t* data);

void ast_expression_free_data(ast_literal_t** val);

wchar_t read_token(lexer_t* lexer);
void read_number(lexer_t* lexer, wchar_t ch);
size_t mb_strlen(char* identifier);
void read_identifier(lexer_t* lexer, wchar_t ch);
size_t wchar_length(wchar_t wide_char);

lexer_t* lexer_create(const char* data);
void lexer_free(lexer_t** lexer);
void lexer_lex(lexer_t* lexer);

parser_t* parser_create(lexer_t** lexer);
void parser_free(parser_t** parser);
void parser_parse(parser_t* parser);
ast_node_t* parser_function(parser_t* parser);
ast_node_t* parser_block(parser_t* parser);
ast_node_t* parser_statement(parser_t* parser);
ast_node_t* parser_statement_return(parser_t* parser);
ast_node_t* parser_statement_print(parser_t* parser);
ast_expression_t* parser_expression(parser_t* parser);

ast_node_t* interpreter_statement_expression(ast_node_t* expr, interpreter_t* interpreter);
ast_literal_t* interpreter_expression(ast_expression_t* expr, interpreter_t* interpreter);
ast_literal_t* interpreter_expression_binary(ast_expression_t* expr, interpreter_t* interpreter);
ast_literal_t* interpreter_literal(ast_expression_t* expr);
ast_literal_t* interpreter_identifier(ast_expression_t* expr, interpreter_t* interpreter);

ast_node_t* interpreter_statement_print(ast_node_t* node, interpreter_t* interpreter);
ast_node_t* interpreter_statement_return(ast_node_t* node, interpreter_t* interpreter);
ast_node_t* interpreter_function_declaration(ast_node_t* node, interpreter_t* interpreter, array_t* arguments);
ast_node_t* interpreter_block(ast_node_t* node, interpreter_t* interpreter, token_type_t parent_type, array_t* arguments);

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
	PRECEDENCE_ANDOR = 1,     // AND OR
	PRECEDENCE_HIGHEST = 2,   // =
	PRECEDENCE_MULTIPY = 3,   // / *
	PRECEDENCE_SUM = 4,       // + -
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
	[TOKEN_TYPE_MULTIPY] = {PRECEDENCE_MULTIPY, NULL, led_plus_minus},
	[TOKEN_TYPE_DIVIDE] = {PRECEDENCE_MULTIPY, NULL, led_plus_minus},

	[TOKEN_TYPE_EQUAL] = {PRECEDENCE_HIGHEST, NULL, led_equal},

	[TOKEN_TYPE_NOT_EQUAL] = {PRECEDENCE_HIGHEST, NULL, led_equal_equal},
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
		case TOKEN_TYPE_UNTIL: return "UNTIL";
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
		case TOKEN_TYPE_DIVIDE: return "DIVIDE";
		case TOKEN_TYPE_MINUS: return "MINUS";
		case TOKEN_TYPE_MULTIPY: "MULTIPY";
		case TOKEN_TYPE_COMMA: return "COMMA";
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
	return (
		ch >= L'آ' && ch <= L'ی' || ch == L'_' || (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z')
	) && !(ch == '+' || ch == '-' || ch == '*' || ch == '/' || ch == '=' || ch == '>' || ch == '<' || ch == ','	);
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
		printf("Syntax Error: read token - invalid unicode character\n");
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
		printf("Syntax Error: nuread - invalid unicode character\n");
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
			number[i] = ch;
		} else {
			number[i] = ch - L'۰' + '0';
		}
		i++;
		ch = read_token(lexer);
	}
	number[i] = 0;

	token_t* t = token_create(TOKEN_TYPE_NUMBER, number, i, lexer->line, lexer->column - i, lexer->line, lexer->column);
	array_push(lexer->tokens, t);

	unread_token(lexer);
}

void read_string(lexer_t* lexer, wchar_t ch)
{
    size_t allocated_size = 20;
    char* string = (char*) malloc(sizeof(char) * allocated_size);
    if (string == NULL) {
        printf("Error: read_string - Memory allocation failed.\n");
        exit(EXIT_FAILURE);
    }

    size_t i = 0;

    while (ch != L'"') {
        if (i >= allocated_size - 1) {
            allocated_size *= 2;
            char* temp = (char*)realloc(string, sizeof(char) * allocated_size);
            if (temp == NULL) {
                printf("Error: read_string iterate - Memory reallocation failed.\n");
                free(string);
                exit(EXIT_FAILURE);
            }
            string = temp;
        }

        int char_size = wctomb(&string[i], ch);
        if (char_size < 0) {
            printf("Error: read_string - Failed to convert wide character to multibyte\n");
            free(string);
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
		perror("Error in mbstowcs - count length");
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
			printf("Error: read_identifier - Failed to convert wide character to multibyte\n");
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
		} else if (current_wchar == ',') {
			token_t* t = token_create(TOKEN_TYPE_COMMA, ",", 1, lexer->line, lexer->column - 1, lexer->line, lexer->column);
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
	printf("Welcome to Salam Programming Language!\n");
	printf("Salam is the first Persian/Iranian computer scripting language.\n");
	printf("\n");

	printf("Usage:\n");
	printf("  salam <filename>\t\t\t# Execute a Salam script\n");
	printf("\n");

	printf("Example:\n");
	printf("  salam my_script.salam\t\t# Run the Salam script 'my_script.salam'\n");
	printf("\n");

	printf("Feel free to explore and create using Salam!\n");
	printf("For more information, visit: https://salamlang.ir\n");
	printf("\n");
}

parser_t* parser_create(lexer_t** lexer)
{
	parser_t* parser = (parser_t*) malloc(sizeof(parser_t));
	parser->lexer = lexer;
	parser->token_index = 0;
	parser->functions = array_create(5);
	parser->expressions = array_create(5);

	return parser;
}

void debug_current_token(parser_t* parser)
{
	token_t* t = (token_t*) (*parser->lexer)->tokens->data[parser->token_index];
	printf("=========> Current token: %s - %s\n", token_type2str(t->type) ,t->value);
}

void ast_expression_free(ast_expression_t** expr);

void ast_expression_free_data(ast_literal_t** val)
{
	printf("ast_expression_free_data\n");

	if (val == NULL || *val == NULL) {
		printf("is null?\n");
		return;
	}

	printf("start checking type on ast_expression_free_data\n");

	if ((*val)->type == VALUE_TYPE_STRING) {
		printf("has string\n");
		printf("%s\n", (*val)->string_value);
		if ((*val)->string_value != "\0" && (*val)->string_value != NULL) {
			free((*val)->string_value);
			(*val)->string_value = NULL;
		}
	} else if ((*val)->type == VALUE_TYPE_NULL) {
		// Nothing to free
	} else if ((*val)->type == VALUE_TYPE_INT) {
		// Nothing to free
	} else if ((*val)->type == VALUE_TYPE_BOOL) {
		// Nothing to free
	} else if ((*val)->type == VALUE_TYPE_FLOAT) {
		// Nothing to free
	}

	if ((*val)->main != NULL) {
		printf("ast_expression_free_data main\n");
		ast_expression_free((ast_expression_t**) &((*val)->main));
	}

	printf("let's free it's at all\n");

	free(*val);
	*val = NULL;
}

void ast_expression_free_literal(ast_expression_t** expr)
{
	printf("AST_EXPRESSION_LITERAL\n");
	if ((*expr)->data.literal != NULL) {
		printf("....\n");
		if ((*expr)->data.literal == NULL) {
			printf("literal is null\n");
		} else {
			printf("literal is not null\n");
		}
		ast_expression_free_data(&((*expr)->data.literal));
		(*expr)->data.literal = NULL;
	}
}

void ast_expression_free_identifier(ast_expression_t** expr)
{
	printf("AST_EXPRESSION_IDENTIFIER\n");
	if ((*expr)->data.identifier != NULL) {
		if ((*expr)->data.identifier->name != NULL) {
			free((*expr)->data.identifier->name);
			(*expr)->data.identifier->name = NULL;
		}

		free((*expr)->data.identifier);
		(*expr)->data.identifier = NULL;
	}
}

void ast_expression_free_functioncall(ast_expression_t** expr)
{
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
}

void ast_expression_free_assignment(ast_expression_t** expr)
{
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
}

void ast_expression_free_binary(ast_expression_t** expr)
{
	printf("ast_expression_free_binary\n");

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
}

void ast_expression_free(ast_expression_t** expr)
{
	printf("ast_expression_free\n");

	if (expr == NULL || *expr == NULL) {
		return;
	}

	printf("---- expr type: %d\n", (*expr)->type);

	switch ((*expr)->type) {
		case AST_EXPRESSION_LITERAL:
			ast_expression_free_literal(expr);
			break;

		case AST_EXPRESSION_IDENTIFIER:
			ast_expression_free_identifier(expr);
			break;

		case AST_EXPRESSION_FUNCTION_CALL:
			ast_expression_free_functioncall(expr);
			break;

		case AST_EXPRESSION_ASSIGNMENT:
			ast_expression_free_assignment(expr);
			break;

		case AST_EXPRESSION_BINARY:
			ast_expression_free_binary(expr);
			break;
	}

	free(*expr);
	*expr = NULL;
}

void ast_node_free(ast_node_t** node)
{
	printf("ast_node_free\n");

	if (node == NULL || *node == NULL) {
		return;
	}

	printf("node type: %d\n", (*node)->type);

	switch ((*node)->type) {
		case AST_STATEMENT_UNTIL:
			// printf("free ast until\n");
			if ((*node)->data.statement_until != NULL) {
				if ((*node)->data.statement_until->condition != NULL) {
					ast_expression_free(&((*node)->data.statement_until->condition));
					(*node)->data.statement_until->condition = NULL;
				}

				if ((*node)->data.statement_until->block != NULL) {
					ast_node_free(&((*node)->data.statement_until->block));
					(*node)->data.statement_until->block = NULL;
				}

				free((*node)->data.statement_until);
				(*node)->data.statement_until = NULL;
			}
			break;

		case AST_STATEMENT_IF:
		case AST_STATEMENT_ELSEIF:
			// printf("free ast if/else\n");
			if ((*node)->data.statement_if != NULL) {
				if ((*node)->data.statement_if->condition != NULL) {
					ast_expression_free(&((*node)->data.statement_if->condition));
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
			printf("free ast function\n");
			if ((*node)->data.function_declaration != NULL) {
				// printf("free function name\n");
				if ((*node)->data.function_declaration->name != NULL) {
					free((*node)->data.function_declaration->name);
					(*node)->data.function_declaration->name = NULL;
				}
				
				// printf("free function body\n");
				if ((*node)->data.function_declaration->body != NULL) {
					ast_node_free(&((*node)->data.function_declaration->body));
					(*node)->data.function_declaration->body = NULL;
				}

				free((*node)->data.function_declaration);
				(*node)->data.function_declaration = NULL;
			}
			break;

		case AST_STATEMENT_RETURN:
			printf("free ast return\n");
			if ((*node)->data.statement_return != NULL) {
				if ((*node)->data.statement_return->expression != NULL) {
					ast_expression_free(&((*node)->data.statement_return->expression));
					(*node)->data.statement_return->expression = NULL;
				}

				if ((*node)->data.statement_return->expression_value != NULL) {
					ast_expression_free_data(&((*node)->data.statement_return->expression_value));
					(*node)->data.statement_return->expression_value = NULL;
				}

				free((*node)->data.statement_return);
				(*node)->data.statement_return = NULL;
			}
			break;

		case AST_STATEMENT_PRINT:
			printf("free ast print\n");
			if ((*node)->data.statement_print != NULL) {
				if ((*node)->data.statement_print->expression != NULL) {
					ast_expression_free(&((*node)->data.statement_print->expression));
					(*node)->data.statement_print->expression = NULL;
				}

				if ((*node)->data.statement_print->expression_value != NULL) {
					ast_expression_free_data(&((*node)->data.statement_print->expression_value));
					(*node)->data.statement_print->expression_value = NULL;
				}

				free((*node)->data.statement_print);
				(*node)->data.statement_print = NULL;
			}
			break;

		case AST_BLOCK:
			printf("free ast block\n");
			if ((*node)->data.block != NULL) {
				// printf("free block of func\n");
				if ((*node)->data.block->statements != NULL) {
					// printf("free stmts of func\n");
					for (size_t i = 0; i < (*node)->data.block->num_statements; i++) {
						// printf("free one of stmt of func\n");
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
			printf("free ast expr AST_EXPRESSION\n");
			if ((*node)->data.expression != NULL) {
				// printf("AST_EXPRESSION is not null so lets do it\n");
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
				printf("Free function %s\n", ((ast_node_t*) (*parser)->functions->data[i])->data.function_declaration->name);
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
	printf("*parser is null %d\n", *parser == NULL ? 1 : 0);
	printf("parser is null %d\n", parser == NULL ? 1 : 0);
}

void parser_token_next(parser_t* parser)
{
	if ((*parser->lexer)->tokens->length > parser->token_index) {
		parser->token_index++;
	} else {
		printf("Error: Unexpected end of file\n");
		exit(EXIT_FAILURE);
	}
}

token_t* parser_token_skip(parser_t* parser, token_type_t type)
{
	if ((*parser->lexer)->tokens->length > parser->token_index) {
		token_t* token = (token_t*) (*parser->lexer)->tokens->data[parser->token_index];

		if (token->type == type) {
			return (token_t*) (*parser->lexer)->tokens->data[parser->token_index++];
		}
	}

	return NULL;
}

bool parser_token_ifhas(parser_t* parser, token_type_t type)
{
	if ((*parser->lexer)->tokens->length > parser->token_index) {
		token_t* token = (token_t*) (*parser->lexer)->tokens->data[parser->token_index];

		if (token->type == type) {
			return true;
		}
	}

	return false;
}

bool parser_token_skip_ifhas(parser_t* parser, token_type_t type)
{
	if ((*parser->lexer)->tokens->length > parser->token_index) {
		token_t* token = (token_t*) (*parser->lexer)->tokens->data[parser->token_index];

		if (token->type == type) {
			parser->token_index++;
			return true;
		}
	}

	return false;
}

void parser_token_eat_nodata(parser_t* parser, token_type_t type)
{
	if ((*parser->lexer)->tokens->length > parser->token_index) {
		token_t* token = (token_t*) (*parser->lexer)->tokens->data[parser->token_index];

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
	if ((*parser->lexer)->tokens->length > parser->token_index) {
		token_t* token = (token_t*) (*parser->lexer)->tokens->data[parser->token_index];

		if (token->type == type) {
			return (token_t*) (*parser->lexer)->tokens->data[parser->token_index++];
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

	node->data.function_declaration = (ast_function_declaration_t*) malloc(sizeof(ast_function_declaration_t));
	node->data.function_declaration->name = strdup(name->value);
	node->data.function_declaration->arguments = array_create(3);
	
	if (parser_token_skip_ifhas(parser, TOKEN_TYPE_PARENTHESE_OPEN)) {
	    printf("Parsing parameters\n");

		while ((*parser->lexer)->tokens->length > parser->token_index && ((token_t*) (*parser->lexer)->tokens->data[parser->token_index])->type == TOKEN_TYPE_IDENTIFIER) {
			token_t* t = (*parser->lexer)->tokens->data[parser->token_index];
			// printf("we have an arg %s\n", t->value);
			array_push(node->data.function_declaration->arguments, strdup(t->value));
			parser->token_index++;

			if (parser_token_ifhas(parser, TOKEN_TYPE_COMMA) == false) {
				break;
			}
	    }

		parser_token_eat_nodata(parser, TOKEN_TYPE_PARENTHESE_CLOSE);
	}

	node->data.function_declaration->body = parser_block(parser);

	if (node->data.function_declaration->arguments->length == 0) {
		array_free(node->data.function_declaration->arguments);
		node->data.function_declaration->arguments = NULL;
	}

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
	node->data.statement_print->expression_value = NULL;
	node->data.statement_print->expression = (ast_expression_t*) parser_expression(parser);

	return node;
}

ast_node_t* parser_statement_return(parser_t* parser)
{
	printf("Parsing statement return\n");

	parser->token_index++;

	ast_node_t* node = (ast_node_t*) malloc(sizeof(ast_node_t));
	node->type = AST_STATEMENT_RETURN;

	node->data.statement_return = (ast_statement_return_t*) malloc(sizeof(ast_statement_return_t));
	node->data.statement_return->expression_value = NULL;
	node->data.statement_return->expression = (ast_expression_t*) parser_expression(parser);

	return node;
}

ast_node_t* parser_statement_break(parser_t* parser)
{
	printf("Parsing statement break\n");

	parser->token_index++;

	ast_node_t* node = (ast_node_t*) malloc(sizeof(ast_node_t));
	node->type = AST_STATEMENT_BREAK;

	return node;
}

ast_node_t* parser_statement_continue(parser_t* parser)
{
	printf("Parsing statement continue\n");

	parser->token_index++;

	ast_node_t* node = (ast_node_t*) malloc(sizeof(ast_node_t));
	node->type = AST_STATEMENT_CONTINUE;

	return node;
}

bool parser_expression_has(parser_t* parser)
{
	if ((*parser->lexer)->tokens->length > parser->token_index) {
		token_t* tok = (token_t*) (*parser->lexer)->tokens->data[parser->token_index];
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

	while ((*parser->lexer)->tokens->length > parser->token_index && ((token_t*) (*parser->lexer)->tokens->data[parser->token_index])->type == TOKEN_TYPE_ELSEIF) {
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

ast_node_t* parser_statement_until(parser_t* parser) {
	printf("Parsing statement until\n");

	parser->token_index++; // Eating UNTIL token

	ast_node_t* node = (ast_node_t*) malloc(sizeof(ast_node_t));
	node->type = AST_STATEMENT_UNTIL;
	node->data.statement_until = (ast_statement_until_t*) malloc(sizeof(ast_statement_until_t));
	node->data.statement_until->condition = parser_expression(parser);
	node->data.statement_until->block = parser_block(parser);

	return node;
}

ast_node_t* parser_statement(parser_t* parser)
{
	printf("Parsing statement\n");

	ast_node_t* stmt = NULL;

	if ((*parser->lexer)->tokens->length > parser->token_index) {
		token_t* tok = (token_t*) (*parser->lexer)->tokens->data[parser->token_index];

		switch (tok->type) {
			case TOKEN_TYPE_RETURN:
				stmt = parser_statement_return(parser);
				break;

			case TOKEN_TYPE_BREAK:
				stmt = parser_statement_break(parser);
				break;

			case TOKEN_TYPE_CONTINUE:
				stmt = parser_statement_continue(parser);
				break;

			case TOKEN_TYPE_UNTIL:
				stmt = parser_statement_until(parser);
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
					stmt = malloc(sizeof(ast_node_t));
					stmt->type = AST_EXPRESSION;
					stmt->data.expression = parser_expression(parser);
					return stmt;
				} else {
					printf("Error: Unexpected token as statement %s\n", token_type2str(tok->type));
					exit(EXIT_FAILURE);
				}
				break;
		}
	}
	
	if (stmt == NULL) {
		printf("Error: Unexpected token as statement %s\n", token_type2str(((token_t*) (*parser->lexer)->tokens->data[parser->token_index])->type));
		exit(EXIT_FAILURE);
	}
	return stmt;
}

ast_expression_t* parser_expression(parser_t* parser)
{
	printf("Parsing expression\n");

	return parser_expression_pratt(parser, PRECEDENCE_LOWEST);

	// ast_node_t* node = (ast_node_t*) malloc(sizeof(ast_node_t));
	// node->type = AST_EXPRESSION;
	// node->data.expression = parser_expression_pratt(parser, PRECEDENCE_LOWEST);
	
	// return node;
}

ast_expression_t* parser_expression_pratt(parser_t* parser, size_t precedence)
{
	printf("Parsing pratt\n");

	token_t* current_token = (token_t*) (*parser->lexer)->tokens->data[parser->token_index];
	parser->token_index++;

	ast_expression_t* left = token_infos[current_token->type].nud(parser, current_token);

	printf("Token as op: %s\n", token_type2str(((token_t*) (*parser->lexer)->tokens->data[parser->token_index])->type));
	while (precedence < token_infos[((token_t*) (*parser->lexer)->tokens->data[parser->token_index])->type].precedence) {
		current_token = (token_t*) (*parser->lexer)->tokens->data[parser->token_index];
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
	binary_op_expr->data.assignment = NULL;
	binary_op_expr->data.assignment = (ast_expression_assignment_t*) malloc(sizeof(ast_expression_assignment_t));
	binary_op_expr->data.assignment->left = left;
	binary_op_expr->data.assignment->right = parser_expression_pratt(parser, token_infos[token->type].precedence);

	return binary_op_expr;
}

ast_expression_t* led_equal_equal(parser_t* parser, token_t* token, ast_expression_t* left)
{
	printf("Parsing operator == or !=\n");

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
	literal_expr->data.literal->main = NULL;
	literal_expr->data.literal->type = VALUE_TYPE_BOOL;
	literal_expr->data.literal->bool_value = strcmp(token->value, "درست") == 0 ? true : false;

	return literal_expr;
}

ast_expression_t* nud_number(parser_t* parser, token_t* token)
{
	printf("Parsing number\n");

	ast_expression_t* literal_expr = (ast_expression_t*) malloc(sizeof(ast_expression_t));
	literal_expr->type = AST_EXPRESSION_LITERAL;

	literal_expr->data.literal = (ast_literal_t*) malloc(sizeof(ast_literal_t));
	literal_expr->data.literal->main = NULL;
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
	literal_expr->data.literal->main = NULL;
	literal_expr->data.literal->type = VALUE_TYPE_STRING;
	literal_expr->data.literal->string_value = strdup(token->value);

	return literal_expr;
}

ast_expression_t* nud_identifier(parser_t* parser, token_t* token)
{
	printf("Parsing identifier\n");

	ast_expression_t* expr = (ast_expression_t*) malloc(sizeof(ast_expression_t));

	// Check if current token is (
	if (parser_token_skip_ifhas(parser, TOKEN_TYPE_PARENTHESE_OPEN)) {
		expr->type = AST_EXPRESSION_FUNCTION_CALL;
		expr->data.function_call = (ast_function_call_t*) malloc(sizeof(ast_function_call_t));
		expr->data.function_call->name = strdup(token->value);
		expr->data.function_call->arguments = NULL;
		expr->data.function_call->arguments = array_create(3);

		// Eating parser_expression until find ( token
		while (parser_expression_has(parser)) {
			ast_expression_t* arg = parser_expression(parser);
			array_push(expr->data.function_call->arguments, arg);
			if (parser_token_ifhas(parser, TOKEN_TYPE_PARENTHESE_CLOSE)) {
				break;
			} else {
				parser_token_eat_nodata(parser, TOKEN_TYPE_COMMA);
			}
		}

		if (expr->data.function_call->arguments->length == 0) {
			array_free(expr->data.function_call->arguments);
			expr->data.function_call->arguments = NULL;
		}

		parser_token_eat_nodata(parser, TOKEN_TYPE_PARENTHESE_CLOSE);
	} else {
		expr->type = AST_EXPRESSION_IDENTIFIER;
		expr->data.identifier = (ast_identifier_t*) malloc(sizeof(ast_identifier_t));
		expr->data.identifier->name = strdup(token->value);
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

	ast_node_t* block_node = (ast_node_t*) malloc(sizeof(ast_node_t));
	block_node->type = AST_BLOCK;
	block_node->data.block = (ast_block_t*) malloc(sizeof(ast_block_t));
	block_node->data.block->num_statements = 0;
	block_node->data.block->statements = (ast_node_t**) malloc(sizeof(ast_node_t*) * (allocated_size + 1));

	parser_token_eat_nodata(parser, TOKEN_TYPE_SECTION_OPEN);

	while (
		(*parser->lexer)->tokens->length > parser->token_index &&
		((token_t*)(*parser->lexer)->tokens->data[parser->token_index])->type != TOKEN_TYPE_SECTION_CLOSE
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
	if ((*parser->lexer)->tokens->length == 1 &&
		((ast_node_t*) (*parser->lexer)->tokens->data[0])->type == TOKEN_TYPE_EOF
	) {
		return;
	}

	while (parser->token_index < (*parser->lexer)->tokens->length) {
		token_t* current_token = (token_t*) (*parser->lexer)->tokens->data[parser->token_index];

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
					ast_node_t* expression_node = malloc(sizeof(ast_node_t));
					expression_node->type = AST_EXPRESSION;
					expression_node->data.expression = parser_expression(parser);
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
				} else if (expr->data.literal->type == VALUE_TYPE_NULL) {
					printf("<Value>NULL</value>\n");
				} else if (expr->data.literal->type == VALUE_TYPE_INT) {
					printf("<Value>%d</Value>\n", expr->data.literal->int_value);
				} else if (expr->data.literal->type == VALUE_TYPE_BOOL) {
					printf("<Value>%s</Value>\n", expr->data.literal->bool_value ? "True" : "False");
				} else {
					printf("<!-- Unhandled Literal Type -->\n");
				}

				if (expr->data.literal->main != NULL) {
					print_indentation(indent_level + 2);
					printf("<Main>\n");

						print_indentation(indent_level + 3);
						print_xml_ast_expression((ast_expression_t*) expr->data.literal->main, indent_level + 3);

					print_indentation(indent_level + 2);
					printf("</Main>\n");
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
							printf("<Argument>\n");

								print_indentation(indent_level + 4);
								print_xml_ast_expression(expr->data.function_call->arguments->data[i], indent_level + 4);

							print_indentation(indent_level + 3);
							printf("<Argument>\n");
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
		case AST_STATEMENT_UNTIL:
			printf("<StatementUntil>\n");

				print_indentation(indent_level + 1);
				printf("<Condition>\n");

					print_indentation(indent_level + 2);
					print_xml_ast_expression(node->data.statement_if->condition, indent_level + 2);

				print_indentation(indent_level + 1);
				printf("</Condition>\n");

				print_xml_ast_node(node->data.statement_if->block, indent_level + 1);

			print_indentation(indent_level);
			printf("</StatementUntil>\n");
			break;

		case AST_STATEMENT_IF:
		case AST_STATEMENT_ELSEIF:
			printf("<StatementIf>\n");

				print_indentation(indent_level + 1);
				printf("<Condition>\n");

					print_indentation(indent_level + 2);
					print_xml_ast_expression(node->data.statement_if->condition, indent_level + 2);

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

				if (node->data.function_declaration->arguments == NULL || node->data.function_declaration->arguments->length == 0) {
					print_indentation(indent_level + 1);
					printf("<Arguments />\n");
				} else {
					print_indentation(indent_level + 1);
					printf("<Arguments>\n");

					for (size_t i = 0; i < node->data.function_declaration->arguments->length; i++) {
						char* ident = node->data.function_declaration->arguments->data[i];

						print_indentation(indent_level + 2);
						printf("<Argument>%s</Argument>\n", ident);
					}

					print_indentation(indent_level + 1);
					printf("</Arguments>\n");
				}

				print_xml_ast_node(node->data.function_declaration->body, indent_level + 1);

			print_indentation(indent_level);
			printf("</FunctionDeclaration>\n");
			break;

		case AST_STATEMENT_PRINT:
			printf("<StatementPrint>\n");

				print_indentation(indent_level + 1);
				print_xml_ast_expression(node->data.statement_print->expression, indent_level + 1);

			print_indentation(indent_level);
			printf("</StatementPrint>\n");
			break;
			
		case AST_STATEMENT_RETURN:
			printf("<StatementReturn>\n");

				print_indentation(indent_level + 1);
				print_xml_ast_expression(node->data.statement_return->expression, indent_level + 1);

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

	if (parser != NULL) {
		if (parser->functions != NULL) {
			print_indentation(1);
			printf("<Functions>\n");

				if (parser->functions != NULL) {
					for (size_t i = 0; i < parser->functions->length; i++) {
						print_xml_ast_node(parser->functions->data[i], 2);
					}
				}

			print_indentation(1);
			printf("</Functions>\n");
		}

		////////////////////////////////////////////
		////////////////////////////////////////////

		if (parser->expressions != NULL) {
			print_indentation(1);
			printf("<Expressions>\n");

				if (parser->expressions != NULL) {
					for (size_t i = 0; i < parser->expressions->length; i++) {
						print_xml_ast_node(parser->expressions->data[i], 2);
					}
				}
			
			print_indentation(1);
			printf("</Expressions>\n");
		}
	}

	printf("</AST>\n");
}

interpreter_t* interpreter_create(parser_t** parser)
{
	interpreter_t* interpreter = (interpreter_t*) malloc(sizeof(interpreter_t));
	interpreter->parser = parser;
	interpreter->is_global_scope = true;
	return interpreter;
}

ast_node_t* interpreter_statement_until(ast_node_t* node, interpreter_t* interpreter)
{
	// printf("Until\n");

	while (interpreter_expression_truly(node->data.statement_until->condition, interpreter) == true) {
		ast_node_t* returned = interpreter_block(node->data.statement_until->block, interpreter, TOKEN_TYPE_UNTIL, NULL);

		if (returned != NULL) {
			if (returned->type == AST_STATEMENT_RETURN) {
				return returned;
			} else if (returned->type == AST_STATEMENT_BREAK) {
				break;
			} else if (returned->type == AST_STATEMENT_CONTINUE) {
				continue;
			}
		}
	}

	return NULL;
}

ast_node_t* interpreter_statement_if(ast_node_t* node, interpreter_t* interpreter)
{
	// printf("If\n");

	if (interpreter_expression_truly(node->data.statement_if->condition, interpreter)) {
		return interpreter_block(node->data.statement_if->block, interpreter, TOKEN_TYPE_IF, NULL);
	} else {
		for (size_t i = 0; i < node->data.statement_if->num_elseifs; i++) {
			ast_node_t* elseif = (ast_node_t*) node->data.statement_if->elseifs[i];
			if (interpreter_expression_truly(elseif->data.statement_if->condition, interpreter)) {
				return interpreter_block(elseif->data.statement_if->block, interpreter, TOKEN_TYPE_ELSEIF, NULL);
			}
		}

		if (node->data.statement_if->else_block != NULL) {
			return interpreter_block(node->data.statement_if->else_block, interpreter, TOKEN_TYPE_ELSEIF, NULL);
		}
	}

	return NULL;
}

ast_node_t* interpreter_interpret_once(ast_node_t* node, interpreter_t* interpreter, token_type_t parent_type)
{
	// printf("Interpreter Interpret Once\n");

	switch (node->type) {
		case AST_BLOCK:
			return interpreter_block(node, interpreter, parent_type, NULL);
			break;

		case AST_FUNCTION_DECLARATION:
			return interpreter_function_declaration(node, interpreter, NULL);
			break;

		case AST_STATEMENT_RETURN:
			return interpreter_statement_return(node, interpreter);
			break;
		
		case AST_STATEMENT_IF:
			return interpreter_statement_if(node, interpreter);
			break;

		case AST_STATEMENT_UNTIL:
			return interpreter_statement_until(node, interpreter);
			break;

		case AST_STATEMENT_PRINT:
			return interpreter_statement_print(node, interpreter);
			break;

		case AST_EXPRESSION:
			return interpreter_statement_expression(node, interpreter);
			break;

		default:
			printf("interpreter_interpret - default\n");
			break;
	}

	return NULL;
}

interpreter_t* interpreter_interpret(interpreter_t* interpreter)
{
	// printf("Interpreter Interpret\n");

	if (interpreter == NULL || (*interpreter->parser) == NULL) {
		return NULL;
	}

	// Scope entry
	pushSymbolTable(&symbolGlobalTableStack, false);
	pushSymbolTable(&symbolTableStack, false);

	// Expressions
	interpreter->is_global_scope = true;
	if ((*interpreter->parser)->expressions != NULL) {
		for (size_t i = 0; i < (*interpreter->parser)->expressions->length; i++) {
			// printf("Interpreting global expression\n");
			ast_node_t* expression = (ast_node_t*) (*interpreter->parser)->expressions->data[i];
			interpreter_expression(expression->data.expression, interpreter);
		}
	}
	interpreter->is_global_scope = false;

	ast_node_t* main_returned = NULL;

	// Functions
	if ((*interpreter->parser)->functions != NULL) {
		for (size_t i = 0; i < (*interpreter->parser)->functions->length; i++) {
			ast_node_t* function = (ast_node_t*) (*interpreter->parser)->functions->data[i];

			if (strcmp(function->data.function_declaration->name, "سلام") == 0) {
				main_returned = interpreter_function_declaration(function, interpreter, NULL);
			}
		}
	}

	// Scope exit
	popSymbolTable(&symbolGlobalTableStack);
	popSymbolTable(&symbolTableStack);

	// print_xml_ast_node(main_returned, 3);
	if (main_returned != NULL && main_returned->type == AST_STATEMENT_RETURN) {
		printf("Main returned: ");
		interpreter_expression_data(main_returned->data.statement_return->expression_value);
	} else {
		printf("No return value from main function, so default!\n");
	}

	return interpreter;
}

ast_node_t* interpreter_function_declaration(ast_node_t* node, interpreter_t* interpreter, array_t* arguments)
{
	// printf("Function Declaration: %s\n", stmt->name);
	
	return interpreter_block(node->data.function_declaration->body, interpreter, TOKEN_TYPE_FUNCTION, arguments);
}

void interpreter_expression_data(ast_literal_t* data)
{
	if (data == NULL) {
		printf("NULL\n");
		return;
	} else if (data->type == VALUE_TYPE_NULL) {
		printf("NULL\n");
	} else if (data->type == VALUE_TYPE_INT) {
		printf("%d\n", data->int_value);
	} else if (data->type == VALUE_TYPE_FLOAT) {
		printf("%f\n", data->float_value);
	} else if (data->type == VALUE_TYPE_BOOL) {
		printf("%s\n", data->bool_value == true ? "True" : "False");
	} else if (data->type == VALUE_TYPE_STRING) {
		printf("%s\n", data->string_value);
	} else {
		printf("Unknown (%d)\n", data->type);
	}
}

ast_node_t* interpreter_statement_return(ast_node_t* node, interpreter_t* interpreter)
{
	// printf("Return Statement\n");

	node->data.statement_return->expression_value = (ast_literal_t*) interpreter_expression(node->data.statement_return->expression, interpreter);

	return node;
}

ast_node_t* interpreter_statement_print(ast_node_t* node, interpreter_t* interpreter)
{
	// printf("Print Statement\n");

	node->data.statement_print->expression_value = (ast_literal_t*) interpreter_expression(node->data.statement_print->expression, interpreter);

	interpreter_expression_data(node->data.statement_print->expression_value);

	return node;
}

ast_node_t* interpreter_block(ast_node_t* node, interpreter_t* interpreter, token_type_t parent_type, array_t* arguments)
{
	// printf("Block\n");

	ast_node_t* returned = NULL;

	// Scope entry
	if (interpreter->is_global_scope == false) {
		pushSymbolTable(&symbolTableStack, false);
		// pushSymbolTable(&symbolTableStack, parent_type == TOKEN_TYPE_FUNCTION ? true : false);
	}

	for (size_t i = 0; i < node->data.block->num_statements; i++) {
		ast_node_t* stmt = node->data.block->statements[i];

		if (parent_type != TOKEN_TYPE_UNTIL && (stmt->type == AST_STATEMENT_BREAK || stmt->type == AST_STATEMENT_CONTINUE)) {
			printf("Error: it's not possible to have break/continue inside a non-loop!\n");
			exit(EXIT_FAILURE);
			return NULL;
		}

		stmt = interpreter_interpret_once(stmt, interpreter, parent_type);
		if (stmt != NULL) {
			if (stmt->type == AST_STATEMENT_RETURN) {
				returned = stmt;
				break;
			} else if (stmt->type == AST_STATEMENT_BREAK) {
				returned = stmt;
				break;
			} else if (stmt->type == AST_STATEMENT_CONTINUE) {
				continue;
			}
		}
	}

	// Scope exit
	if (interpreter->is_global_scope == false) {
		popSymbolTable(&symbolTableStack);
	}

	return returned;
}

ast_literal_t* interpreter_literal(ast_expression_t* expr)
{
	if (expr == NULL) {
		return NULL;
	}

	return expr->data.literal;
}

ast_literal_t* interpreter_identifier(ast_expression_t* expr, interpreter_t* interpreter)
{
	// printf("Variable: %s (%d)\n", expr->data.identifier->name, interpreter->is_global_scope ? 1 : 0);

	ast_literal_t* val;

	if (interpreter->is_global_scope == true) {
		val = findInSymbolTable(symbolGlobalTableStack, expr->data.identifier->name, false);
	} else {
		val = findInSymbolTable(symbolTableStack, expr->data.identifier->name, true);
	}

	if (val != NULL) {
		return val;
	} else {
		printf("Error: Variable not found: %s\n", expr->data.identifier->name);
		exit(EXIT_FAILURE);
	}

	return NULL;
}

ast_literal_t* interpreter_expression_binary(ast_expression_t* expr, interpreter_t* interpreter) {
	bool invalid = false;

	ast_literal_t* left = (ast_literal_t*) interpreter_expression(expr->data.binary_op->left, interpreter);
	ast_literal_t* right = (ast_literal_t*) interpreter_expression(expr->data.binary_op->right, interpreter);

	ast_literal_t* res = (ast_literal_t*) malloc(sizeof(ast_literal_t));
	res->main = (struct ast_expression_t*) expr;

	bool isReverse = false;
	if (strcmp(expr->data.binary_op->operator, "!=") == 0) {
		isReverse = true;
	}

	if (left == NULL || right == NULL) {
		printf("Error: cannot calculate binary operator for invalid values!\n");
		invalid = true;
	} else if (left->type == VALUE_TYPE_INT && right->type == VALUE_TYPE_STRING && strcmp(expr->data.binary_op->operator, "+") == 0) {
		char* left_str = intToString(left->int_value);
		size_t leftlen = strlen(left_str);
		size_t rightlen = strlen(right->string_value);
		res->type = VALUE_TYPE_STRING;
		res->string_value = malloc(sizeof(char) * (leftlen + rightlen) + 1);
		strcpy(res->string_value, left_str);
		strcat(res->string_value, right->string_value);
		free(left_str);
	} else if (left->type == VALUE_TYPE_STRING && right->type == VALUE_TYPE_INT && strcmp(expr->data.binary_op->operator, "+") == 0) {
		char* right_str = intToString(right->int_value);
		size_t leftlen = strlen(left->string_value);
		size_t rightlen = strlen(right_str);
		res->type = VALUE_TYPE_STRING;
		res->string_value = malloc(sizeof(char) * (leftlen + rightlen) + 1);
		strcpy(res->string_value, left->string_value);
		strcat(res->string_value, right_str);
		free(right_str);
	} else if (left->type == VALUE_TYPE_STRING && right->type == VALUE_TYPE_STRING && strcmp(expr->data.binary_op->operator, "+") == 0) {
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
	} else if (strcmp(expr->data.binary_op->operator, "==") == 0 || strcmp(expr->data.binary_op->operator, "!=") == 0) {
		if (left->type == VALUE_TYPE_INT && right->type == TOKEN_TYPE_STRING) {
			res->type = VALUE_TYPE_STRING;
			char* left_str = intToString(left->int_value);
			res->type = VALUE_TYPE_BOOL;
			res->bool_value = strcmp(left_str, right->string_value) == 0 ? true : false;
			free(left_str);
		} else if (left->type == TOKEN_TYPE_STRING && right->type == VALUE_TYPE_INT) {
			res->type = VALUE_TYPE_STRING;
			char* right_var = intToString(right->int_value);
			res->type = VALUE_TYPE_BOOL;
			res->bool_value = strcmp(left->string_value, right_var) == 0 ? true : false;
			free(right_var);
		} else if (left->type == VALUE_TYPE_NULL && right->type == TOKEN_TYPE_NULL) {
			res->type = VALUE_TYPE_BOOL;
			res->bool_value = true;
		} else if (left->type == VALUE_TYPE_INT && right->type == VALUE_TYPE_FLOAT) {
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
	} else if (strcmp(expr->data.binary_op->operator, "+") == 0) {
		res->type = VALUE_TYPE_INT;
		res->int_value = left->int_value + right->int_value;
	} else if (strcmp(expr->data.binary_op->operator, "-") == 0) {
		res->type = VALUE_TYPE_INT;
		res->int_value = left->int_value - right->int_value;
	} else if (strcmp(expr->data.binary_op->operator, "*") == 0) {
		res->type = VALUE_TYPE_INT;
		res->int_value = left->int_value * right->int_value;
	} else if (strcmp(expr->data.binary_op->operator, "و") == 0) {
		res->type = VALUE_TYPE_BOOL;
		res->int_value = left->int_value && right->int_value;
	} else if (strcmp(expr->data.binary_op->operator, "یا") == 0) {
		res->type = VALUE_TYPE_BOOL;
		res->int_value = left->int_value || right->int_value;
	} else if (strcmp(expr->data.binary_op->operator, "/") == 0) {
		if (right->int_value == 0) {
			printf("Error: cannot divide by zero!\n");
			invalid = true;
		} else {
			res->type = VALUE_TYPE_INT;
			res->int_value = left->int_value / right->int_value;
		}
	} else {
		printf("Error: unknown operator: %s\n", expr->data.binary_op->operator);
		invalid = true;
	}

	if (invalid) {
		// ast_expression_free_data(&res);
		// res = NULL;
		// ast_expression_free_data(&left);
		// left = NULL;
		// ast_expression_free_data(&right);
		// right = NULL;
		exit(EXIT_FAILURE);
		return NULL;
	} else if (isReverse && res->type == VALUE_TYPE_BOOL) {
		res->bool_value = res->bool_value == true ? false : true;
	}

	return res;
}

ast_literal_t* interpreter_function_run(ast_node_t* function, array_t* arguments, interpreter_t* interpreter)
{
	if (function == NULL) {
		return NULL;
	}

	size_t function_arguments_count = function->data.function_declaration->arguments == NULL ? 0 : function->data.function_declaration->arguments->length;
	size_t arguments_count = arguments == NULL ? 0 : arguments->length;

	if (function_arguments_count != arguments_count) {
		if (arguments_count > function_arguments_count) {
			printf("Error: number of arguments is not match with the function - you are passing more arguments!\n");
		} else {
			printf("Error: number of arguments is not match with the function - you are passing less arguments!\n");
		}
		exit(EXIT_FAILURE);
		return NULL;
	}

	// Scope entry
	// printf("create a scope with function_call enabled\n");
	pushSymbolTable(&symbolTableStack, true);

	for (size_t i = 0; i < arguments_count; i++) {
		char* arg_name = function->data.function_declaration->arguments->data[i];
		ast_literal_t* arg_value = interpreter_expression((ast_expression_t*) arguments->data[i], interpreter);
		
		addToSymbolTable(symbolTableStack, arg_name, arg_value);
	}

	ast_node_t* returned = interpreter_function_declaration(function, interpreter, arguments);
	if (returned != NULL && returned->type == AST_STATEMENT_RETURN) {
		return returned->data.statement_return->expression_value;
	}
	
	// Scope exit
	popSymbolTable(&symbolTableStack);
	// printf("delete a scope with function_call enabled\n");

	return NULL;
}

ast_literal_t* interpreter_function_call(ast_expression_t* node, interpreter_t* interpreter)
{
	// printf("Function Call: %s\n", node->data.function_call->name);

	// Check if functions exists in (*interpreter->parser)->...
	bool exists = false;
	ast_node_t* func_exists = NULL;

	if ((*interpreter->parser)->functions != NULL) {
		for (size_t i = 0; i < (*interpreter->parser)->functions->length; i++) {
			ast_node_t* func = (*interpreter->parser)->functions->data[i];
			if (func != NULL && func->data.function_declaration != NULL && func->data.function_declaration->name != NULL && strcmp(func->data.function_declaration->name, node->data.function_call->name) == 0) {
				func_exists = func;
				exists = true;
				break;
			}
		}
	}
	
	if (exists == false || func_exists == NULL) {
		printf("Error: function not exists!\n");
		exit(EXIT_FAILURE);
		return NULL;
	}

	ast_literal_t* ret = interpreter_function_run(func_exists, node->data.function_call->arguments, interpreter);
	if (ret == NULL) {
		ast_literal_t* default_ret = malloc(sizeof(ast_literal_t));
		default_ret->type = VALUE_TYPE_NULL;
		default_ret->main = NULL;
		return default_ret;
	}

	return ret;
}

bool interpreter_expression_truly(ast_expression_t* expr, interpreter_t* interpreter)
{
	// printf("Truly\n");

	ast_literal_t* res = interpreter_expression(expr, interpreter);
	// ast_expression_free(&(expr));
	if (res->type == VALUE_TYPE_BOOL) {
		return res->bool_value;
	} else if (res->type == VALUE_TYPE_NULL) {
		return false;
	} else if (res->type == VALUE_TYPE_INT) {
		return res->int_value != 0;
	} else if (res->type == VALUE_TYPE_FLOAT) {
		return res->float_value != 0.0;
	} else if (res->type == VALUE_TYPE_STRING) {
		return res->string_value != NULL && strlen(res->string_value) > 0;
	}

	return false;
}

ast_literal_t* interpreter_expression_assignment(ast_expression_t* expr, interpreter_t* interpreter)
{
	// printf("Assignment\n");

	if (expr->data.assignment->left->type != AST_EXPRESSION_IDENTIFIER) {
		printf("Error: Assignment to non-variable\n");
		exit(EXIT_FAILURE);
		return NULL;
	}

	bool isNew = false;
	char* identifier = strdup(expr->data.assignment->left->data.identifier->name);

	// printf("============> assign %s variable\n", identifier);

	ast_literal_t* right = interpreter_expression(expr->data.assignment->right, interpreter);
	ast_literal_t* variable;

	if (interpreter->is_global_scope == true) {
		variable = findInSymbolTable(symbolGlobalTableStack, identifier, false);
	} else {
		variable = findInSymbolTable(symbolTableStack, identifier, true);
	}

	if (variable == NULL) {
		// printf("this is a new variable on this scope!\n");
		isNew = true;
		variable = (ast_literal_t*) malloc(sizeof(ast_literal_t));
	}

	variable->main = (struct ast_expression_t*) expr;
	variable->type = right->type;
	if (right->type == VALUE_TYPE_STRING) {
		variable->string_value = strdup(right->string_value);
	} else if (right->type == VALUE_TYPE_NULL) {
		// No value
	} else if (right->type == VALUE_TYPE_INT) {
		variable->int_value = right->int_value;
	} else if (right->type == VALUE_TYPE_BOOL) {
		variable->bool_value = right->bool_value;
	} else if (right->type == VALUE_TYPE_FLOAT) {
		variable->float_value = right->float_value;
	}

	if (isNew == true) {
		if (interpreter->is_global_scope == true) {
			addToSymbolTable(symbolGlobalTableStack, identifier, variable);
		} else {
			addToSymbolTable(symbolTableStack, identifier, variable);
		}
	}

	free(identifier);
	identifier = NULL;

	return variable;
}

ast_node_t* interpreter_statement_expression(ast_node_t* node, interpreter_t* interpreter)
{
	ast_literal_t* val = interpreter_expression(node->data.expression, interpreter);

	// node->type = AST_STATEMENT_EX...
	// TODO

	return node;
}

ast_literal_t* interpreter_expression(ast_expression_t* expr, interpreter_t* interpreter)
{
	if (expr == NULL) {
		return NULL;
	}

	ast_literal_t* lit = NULL;

	switch (expr->type) {
		case AST_EXPRESSION_LITERAL:
			lit = interpreter_literal(expr);
			break;

		case AST_EXPRESSION_IDENTIFIER:
			lit = interpreter_identifier(expr, interpreter);
			break;

		case AST_EXPRESSION_BINARY:
			lit = interpreter_expression_binary(expr, interpreter);
			break;

		case AST_EXPRESSION_FUNCTION_CALL:
			lit = interpreter_function_call(expr, interpreter);
			break;

		case AST_EXPRESSION_ASSIGNMENT:
			lit = interpreter_expression_assignment(expr, interpreter);
			break;

		default:
			printf("Error: default expr type: %d\n", expr->type);
			exit(EXIT_FAILURE);
			return NULL;
	}

	return lit;
}

void interpreter_free(interpreter_t** interpreter)
{
	if (interpreter == NULL || *interpreter == NULL) {
		return;
	}

	while (symbolTableStack != NULL) {
		popSymbolTable(&symbolTableStack);
	}
	if (symbolTableStack != NULL) {
		free(symbolTableStack);
	}

	while (symbolGlobalTableStack != NULL) {
		popSymbolTable(&symbolGlobalTableStack);
	}
	if (symbolGlobalTableStack != NULL) {
		free(symbolGlobalTableStack);
	}

	if (*interpreter != NULL) {
		free(*interpreter);
		*interpreter = NULL;
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

	lexer_t* lexer = lexer_create(file_data);
	lexer_lex(lexer);

	array_print(lexer->tokens);

	parser_t* parser = parser_create(&lexer);
	parser_parse(parser);

	print_xml_ast_tree(parser);

	printf("====================================\n");

	interpreter_t* interpreter = interpreter_create(&parser);
	interpreter_interpret(interpreter);

	printf("====================================\n");
	printf("DONE\n");

	// print_xml_ast_tree(parser);

	// printf("====================================\n");

	// printf("start signing...\n");
	// print_xml_ast_tree(parser);
	// printf("sign-done\n");
	// exit(EXIT_FAILURE);

	// printf("start second signing...\n");
	// print_xml_ast_tree(parser);
	// print_xml_ast_tree(*(interpreter->parser));
	// printf("second sign-done\n");

	// return 0;

	printf("free lexer\n");
	lexer_free(&lexer);
	printf("end lexer free\n");

	printf("free parser\n");
	parser_free(&parser);
	printf("end parser free\n");

	printf("out-after parser is null %d\n", parser == NULL ? 1 : 0);
	printf("out-after interp-parser is null %d\n", (*interpreter->parser) == NULL ? 1 : 0);

	printf("free interpreter\n");
	interpreter_free(&interpreter);
	printf("end interpreter free\n");

	// if (file_data != NULL) {
	// 	free(file_data);
	// 	file_data = NULL;
	// }

	printf("DONE\n");

	exit(EXIT_SUCCESS);
}
