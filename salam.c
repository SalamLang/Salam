//
// THE SALAM PROGRAMMING LANGUAGE
//

#define _SALAM_LANGUAGE_

#include <stdio.h>
#include <wchar.h>
#include <stdlib.h>
#include <locale.h>
#include <string.h>
#include <limits.h>
#include <stdbool.h>

bool debug_enabled = true;

#ifndef MB_CUR_MAX
	#define MB_CUR_MAX (MB_LEN_MAX)
#endif

#define print_error(fmt, ...) fprintf(stderr, fmt, ##__VA_ARGS__)

#define print_message(fmt, ...) if (debug_enabled) fprintf(stdout, fmt, ##__VA_ARGS__)

#define CREATE_MEMORY_OBJECT(result, type, length, fmt, ...) \
	result = (type *)malloc(sizeof(type) * (length));  \
	if (!result) { \
		fprintf(stderr, fmt,  __VA_ARGS__);\
		exit(EXIT_FAILURE); \
	}

struct ast_expression;

struct ast_literal_t;

typedef enum {
	MESSAGE_NAME,
	MESSAGE_ENTRY_POINT_FUNCTION_NAME,
	MESSAGE_WELCOME,
	MESSAGE_LEXER_UNEXPECTED_CHAR,
	MESSAGE_LEXER_COMMENT_MULTI_NOT_CLOSED,
	MESSAGE_LEXER_NUMBER_FLOAT_NEED_NUMBER_AFTER_DOT,
	MESSAGE_LEXER_TOKEN_UNREAD_UNICODE,
	MESSAGE_LEXER_TOKEN_READ_UNICODE,
	MESSAGE_LEXER_STRING_READ_MEMORY,
	MESSAGE_LEXER_STRING_UNKNOWN_ESCAPE,
	MESSAGE_LEXER_STRING_CONVERT_MULTIBYTE,
	MESSAGE_LEXER_STRING_GET_LENGTH_UNICODE,
	MESSAGE_LEXER_IDENTIFIER_CONVERT_MULTIBYTE,
	MESSAGE_LEXER_CHAR_LENGTH_ISSUE,
	MESSAGE_LEXER_ARRAY_NOT_CLOSED,
	MESSAGE_PARSER_UNEXPECTED_TOKEN,
	MESSAGE_PARSER_BLOCK_MEMORY_ISSUE,
	MESSAGE_PARSER_BAD_TOKEN_AS_STATEMENT,
	MESSAGE_TOKEN_TRUE,
	MESSAGE_TOKEN_FALSE,
	MESSAGE_TOKEN_NULL,
	MESSAGE_TOKEN_UNKNOWN,
	MESSAGE_TOKEN_NUMBER_INT,
	MESSAGE_TOKEN_NUMBER_FLOAT,
	MESSAGE_TOKEN_BOOL,
	MESSAGE_TOKEN_STRING,
	MESSAGE_TOKEN_ARRAY,
	MESSAGE_TOKEN_OR,
	MESSAGE_TOKEN_AND,
	MESSAGE_TOKEN_FUNCTION_TYPE,
	MESSAGE_TOKEN_FUNCTION_EVEN,
	MESSAGE_TOKEN_FUNCTION_ODD,
	MESSAGE_TOKEN_FUNCTION_READ,
	MESSAGE_TOKEN_FUNCTION_LENGTH,
	MESSAGE_TOKEN_FUNCTION_STRING,
	MESSAGE_INTERPRETER_FUNCTION_CALL_FIRST_ARGUMENT_SHOULD_BE_ONLY_A_STRING,
	MESSAGE_LEXER_FILE_NOT_EXISTS,
	MESSAGE_MEMORY_ALLOCATE_ERROR,
	MESSAGE_MEMORY_REALLOCATE_ERROR,
	MESSAGE_INTERPRETER_MAIN_RETURN_CODE,
	MESSAGE_INTERPRETER_UNKNOWN_NODE_AS_INTERPRETER_ONCE,
	MESSAGE_COUNT,
} message_key_t;

typedef enum {
	LANGUAGE_PERSIAN,
	// LANGUAGE_ARABIC,
	LANGUAGE_COUNT,
} language_t;

typedef enum {
	VALUE_TYPE_NULL,
	VALUE_TYPE_INT,
	VALUE_TYPE_FLOAT,
	VALUE_TYPE_BOOL,
	VALUE_TYPE_STRING,
	VALUE_TYPE_ARRAY_LITERAL,
	VALUE_TYPE_ARRAY_EXPRESSION,
} ast_literal_type_t;

typedef struct {
	char* name;
	struct ast_expression_t* value;
} ast_argument_t;

typedef struct {
	size_t size;
	size_t length;
	void** data;
} array_t;

typedef struct ast_literal_t {
	ast_literal_type_t type;
	union {
		int int_value;
		bool bool_value;
		float float_value;
		char* string_value;
	};

	size_t size_value;
	struct ast_expression_t** array_expression_value;
	struct ast_literal_t** array_literal_value;

	struct ast_expression_t** main; // To have a reference into parser ast so we can free it later after interpreter
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

typedef enum {
	// Values
	TOKEN_TYPE_IDENTIFIER,
	TOKEN_TYPE_INT,
	TOKEN_TYPE_FLOAT,
	TOKEN_TYPE_STRING,
	
	// Keywords
	TOKEN_TYPE_PAGE, // صفحه
	TOKEN_TYPE_END, // خاتمه
	TOKEN_TYPE_FUNCTION, // تابع
	TOKEN_TYPE_RETURN, // برگشت
	TOKEN_TYPE_BREAK, // توقف
	TOKEN_TYPE_CONTINUE, // ادامه
	TOKEN_TYPE_PRINT, // نمایش
	TOKEN_TYPE_IF, // اگر
	TOKEN_TYPE_NULL, // پوچ
	TOKEN_TYPE_UNTIL, // تا
	TOKEN_TYPE_REPEAT, // تکرار
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
	TOKEN_TYPE_BRACKETS_OPEN, // [
	TOKEN_TYPE_BRACKETS_CLOSE, // ]

	TOKEN_TYPE_PLUS, // +
	TOKEN_TYPE_COLON, // :
	TOKEN_TYPE_DOT, // .
	TOKEN_TYPE_MINUS, // -
	TOKEN_TYPE_MULTIPLY, // *
	TOKEN_TYPE_DIVIDE, // /
	TOKEN_TYPE_MODULE, // %
	TOKEN_TYPE_COMMA, // ,
	TOKEN_TYPE_EQUAL, // =
	TOKEN_TYPE_EQUAL_EQUAL, // ==
	TOKEN_TYPE_NOT_EQUAL, // !=
	TOKEN_TYPE_NOT, // !
	TOKEN_TYPE_LESS_THAN, // <
	TOKEN_TYPE_GREATER_THAN, // >
	TOKEN_TYPE_LESS_THAN_EQUAL, // <=
	TOKEN_TYPE_GREATER_THAN_EQUAL, // >=

	// Others
	TOKEN_TYPE_EOF,
	TOKEN_TYPE_ERROR,
} token_type_t;

typedef struct {
	const char* keyword;
	token_type_t token_type;
} keyword_mapping_t;

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
	char* data;
	size_t length;
	size_t index;
	size_t line;
	size_t column;

	array_t* tokens;
	size_t last_index;
	size_t last_line;
	size_t last_column;
} lexer_t;

struct ast_node;

typedef struct {
	char* name;
	struct ast_node* body;
	array_t* arguments;
} ast_function_declaration_t;

typedef struct {
	struct ast_expression_t* expression;
	struct ast_literal_t* expression_value;
} ast_statement_return_t;

typedef struct {
	struct ast_expression_t* expression;
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
	struct ast_expression_t* left;
	struct ast_expression_t* right;
} ast_expression_binary_t;

typedef struct {
	struct ast_expression_t* left;
	struct ast_expression_t* right;
} ast_expression_assignment_t;

typedef enum {
	AST_EXPRESSION_ERROR,
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

typedef struct ast_expression_t {
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
	AST_STATEMENT_REPEAT,
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
} ast_statement_repeat_t;

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
		ast_statement_repeat_t* statement_repeat;
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

// Function declarations
char* read_dynamic_string();
char* intToString(int value);
char* literal_type2name(ast_literal_type_t type);
unsigned int hash(const char* str, size_t capacity);
SymbolTable* createSymbolTable(size_t capacity);
void pushSymbolTable(SymbolTableStack** ts, bool is_function_call);
void popSymbolTable(SymbolTableStack** ts);
static SymbolTableEntry* findSymbolInParentScopes(SymbolTableStack* ts, const char* identifier);
void addToSymbolTable(SymbolTableStack* ts, const char* identifier, ast_literal_t* value);
ast_literal_t* findInSymbolTableCurrent(SymbolTableStack* currentScope, const char* identifier, bool checkEverythingEvenIsFuncCall);
ast_literal_t* findInSymbolTable(SymbolTableStack* currentScope, const char* identifier, bool wantsGlobal, bool checkEverythingEvenIsFuncCall);
char* token_op_type2str(ast_expression_type_t type);
char* token_type2str(token_type_t type);
char* file_read(char* file_Name);
token_t* token_create(token_type_t type, const char* value, int a, int b, int c, int b2, int c2);
array_t* array_create(size_t size);
void* array_pop(array_t* arr);
void array_push(array_t* arr, void* data);
void array_free(array_t* arr);
void token_print(token_t* t);
void array_print(array_t* arr);
lexer_t* lexer_create(const char* data);
void lexer_free(lexer_t** lexer);
bool is_number(wchar_t ch);
bool is_alpha(wchar_t ch);
bool is_ident(wchar_t ch);
wchar_t read_token(lexer_t* lexer);
wchar_t unread_token(lexer_t* lexer);
char digit_wchar2char(wchar_t ch);
void read_number(lexer_t* lexer, wchar_t ch);
void read_comment_singleline(lexer_t* lexer);
void read_comment_multiline(lexer_t* lexer);
void read_string(lexer_t* lexer, wchar_t ch);
size_t mb_strlen(char* identifier);
void read_identifier(lexer_t* lexer, wchar_t ch);
size_t wchar_length(wchar_t wide_char);
void lexer_lex(lexer_t* lexer);
void help();
parser_t* parser_create(lexer_t** lexer);
void parser_free(parser_t** parser);
void print_xml_ast_expression(ast_expression_t* expr, int indent_level);
void print_xml_ast_node(ast_node_t* node, int indent_level);
void print_xml_ast_tree(parser_t* parser);
int main(int argc, char** argv);

// Global variables
SymbolTableStack* symbolTableStack = NULL;
SymbolTableStack* symbolGlobalTableStack = NULL;
language_t language = LANGUAGE_PERSIAN;

#include "messages.h"

// Helper functions
const char* get_message(language_t language, message_key_t key) {
	if (language >= LANGUAGE_COUNT || key >= MESSAGE_COUNT) {
		return NULL;
	}
	return messages[language][key];
}

char* read_dynamic_string() {
	size_t current_size = 52;
	char* input;
	CREATE_MEMORY_OBJECT(input, char, current_size, "Error: read_dynamic_string<input> - Memory allocation error in %s:%d\n",  __FILE__, __LINE__);

	size_t length = 0;

	while (1) {
		int c = getchar();

		if (c == '\n' || c == EOF) {
			input[length] = '\0';
			break;
		}

		if (length + 1 >= current_size) {
			current_size *= 2;
			char* new_input = realloc(input, current_size);
			if (!new_input) {
				free(input);
				perror(messages[language][MESSAGE_MEMORY_REALLOCATE_ERROR]);

				exit(EXIT_FAILURE);
			}
			input = new_input;
		}

		input[length++] = c;
	}

	return input;
}

char* intToString(int value)
{
	int sign = (value < 0) ? -1 : 1;

	int temp = value;
	int numDigits = (value == 0) ? 1 : 0;
	while (temp != 0) {
		temp /= 10;
		numDigits++;
	}

	char* result;
	CREATE_MEMORY_OBJECT(result, char, numDigits + 1 + (sign == -1 ? 1 : 0), "Error: intToString<result> - Memory allocation error in %s:%d\n",  __FILE__, __LINE__);
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

// Functions
char* literal_type2name(ast_literal_type_t type)
{
	switch (type) {
		case VALUE_TYPE_NULL: return "NULL";
		case VALUE_TYPE_INT: return "INT";
		case VALUE_TYPE_FLOAT: return "FLOAT";
		case VALUE_TYPE_BOOL: return "BOOL";
		case VALUE_TYPE_STRING: return "STRING";
		case VALUE_TYPE_ARRAY_EXPRESSION: case VALUE_TYPE_ARRAY_LITERAL: return "ARRAY";
		default: return "TYPE_UNKNOWN";
	}
}

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
	SymbolTable* table;
	CREATE_MEMORY_OBJECT(table, SymbolTable, 1, "Error: createSymbolTable<table> - Memory allocation error in %s:%d\n",  __FILE__, __LINE__);
	table->entries = (SymbolTableEntry**) calloc(capacity, sizeof(SymbolTableEntry*) );
	table->size = 0;
	table->capacity = capacity;
	return table;
}

void pushSymbolTable(SymbolTableStack** ts, bool is_function_call)
{
	// print_message("create scope with %d state\n", is_function_call ? 1 : 0);
	SymbolTable* table = createSymbolTable(16);
	SymbolTableStack* newScope;
	CREATE_MEMORY_OBJECT(newScope, SymbolTableStack, 1, "Error: pushSymbolTable<newScope> - Memory allocation error in %s:%d\n",  __FILE__, __LINE__);
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

	SymbolTableEntry* entry;
	CREATE_MEMORY_OBJECT(entry, SymbolTableEntry, 1, "Error: addToSymbolTable<entry> - Memory allocation error in %s:%d\n",  __FILE__, __LINE__);
	entry->identifier = strdup(identifier);
	entry->data = value;
	entry->next = table->entries[index];

	table->entries[index] = entry;
	table->size++;
}

ast_literal_t* findInSymbolTableCurrent(SymbolTableStack* currentScope, const char* identifier, bool checkEverythingEvenIsFuncCall)
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

ast_literal_t* findInSymbolTable(SymbolTableStack* currentScope, const char* identifier, bool wantsGlobal, bool checkEverythingEvenIsFuncCall)
{
	while (currentScope != NULL) {
		// print_message("looking for %s on a scope %d\n", identifier, currentScope->is_function_call ? 1 : 0);
		ast_literal_t* data = findInSymbolTableCurrent(currentScope, identifier, checkEverythingEvenIsFuncCall);
		if (data != NULL) {
			return data;
		}

		if (checkEverythingEvenIsFuncCall == false && currentScope->is_function_call == true) {
			// print_message("this scope is call enabled, so break loop (%d)!\n", wantsGlobal ? 1 : 0);
			break;
		}
		currentScope = currentScope->next;
	}

	if (wantsGlobal == true) {
		return findInSymbolTable(symbolGlobalTableStack, identifier, false, checkEverythingEvenIsFuncCall);
	}
	return NULL;
}

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
		// Keywords
		case TOKEN_TYPE_END: return "END";
		case TOKEN_TYPE_PAGE: return "PAGE";
		case TOKEN_TYPE_UNTIL: return "UNTIL";
		case TOKEN_TYPE_REPEAT: return "REPEAT";
		case TOKEN_TYPE_IDENTIFIER: return "IDENTIFIER";
		case TOKEN_TYPE_INT: return "NUMBER_INT";
		case TOKEN_TYPE_FLOAT: return "NUMBER_FLOAT";
		case TOKEN_TYPE_STRING: return "STRING";
		case TOKEN_TYPE_FUNCTION: return "FUNCTION";
		case TOKEN_TYPE_RETURN: return "RETURN";
		case TOKEN_TYPE_PRINT: return "PRINT";
		case TOKEN_TYPE_IF: return "IF";
		case TOKEN_TYPE_TRUE: return "TRUE";
		case TOKEN_TYPE_FALSE: return "FALSE";
		case TOKEN_TYPE_ELSEIF: return "ELSEIF";

		// Operators
		case TOKEN_TYPE_OR: return "OR";
		case TOKEN_TYPE_AND: return "AND";
		case TOKEN_TYPE_SECTION_OPEN: return "SECTION_OPEN";
		case TOKEN_TYPE_SECTION_CLOSE: return "SECTION_CLOSE";
		case TOKEN_TYPE_PARENTHESE_OPEN: return "PARENTHESIS_OPEN";
		case TOKEN_TYPE_PARENTHESE_CLOSE: return "PARENTHESIS_CLOSE";
		case TOKEN_TYPE_BRACKETS_OPEN: return "BRACKETS_OPEN";
		case TOKEN_TYPE_BRACKETS_CLOSE: return "BRACKETS_CLOSE";
		case TOKEN_TYPE_PLUS: return "PLUS";
		case TOKEN_TYPE_DIVIDE: return "DIVIDE";
		case TOKEN_TYPE_MODULE: return "MODULE";
		case TOKEN_TYPE_MINUS: return "MINUS";
		case TOKEN_TYPE_MULTIPLY: return "MULTIPLY";
		case TOKEN_TYPE_COMMA: return "COMMA";
		case TOKEN_TYPE_EQUAL: return "EQUAL";
		case TOKEN_TYPE_EQUAL_EQUAL: return "EQUAL_EQUAL";
		case TOKEN_TYPE_NOT_EQUAL: return "NOT_EQUAL";
		case TOKEN_TYPE_NOT: return "NOT";
		case TOKEN_TYPE_LESS_THAN: return "LESS_THAN";
		case TOKEN_TYPE_GREATER_THAN: return "GREATER_THAN";
		case TOKEN_TYPE_LESS_THAN_EQUAL: return "LESS_THAN_EQUAL";
		case TOKEN_TYPE_GREATER_THAN_EQUAL: return "GREATER_THAN_EQUAL";
		case TOKEN_TYPE_COLON: return "COLON";
		case TOKEN_TYPE_DOT: return "DOT";

		// Others
		case TOKEN_TYPE_EOF: return "EOF";
		case TOKEN_TYPE_ERROR: return "ERROR";
		default: return "TOK_UNKNOWN";
	}
}

char* file_read(char* file_Name)
{
	FILE* file = fopen(file_Name, "r");
	if (file == NULL) {
		print_error(messages[language][MESSAGE_LEXER_FILE_NOT_EXISTS], file_Name);
		return NULL;
	}

	fseek(file, 0, SEEK_END);
	long file_size = ftell(file);
	fseek(file, 0, SEEK_SET);

	char* file_data;
	CREATE_MEMORY_OBJECT(file_data, char, file_size + 1, "Error: file_read<file_data> - Memory allocation error in %s:%d\n",  __FILE__, __LINE__);
	fread(file_data, 1, file_size, file);
	file_data[file_size] = 0;

	fclose(file);
	return file_data;
}

token_t* token_create(token_type_t type, const char* value, int a, int b, int c, int b2, int c2)
{
	token_t* t;
	CREATE_MEMORY_OBJECT(t, token_t, 1, "Error: token_create<t> - Memory allocation error in %s:%d\n",  __FILE__, __LINE__);
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

	array_t* arr;
	CREATE_MEMORY_OBJECT(arr, array_t, 1, "Error: array_create<arr> - Memory allocation error in %s:%d\n",  __FILE__, __LINE__);
	arr->length = 0;
	arr->size = size > min_size ? size : min_size;
	arr->data = (void*) malloc(sizeof(void*) * arr->size);
	if (!arr->data) {
		perror(messages[language][MESSAGE_MEMORY_ALLOCATE_ERROR]);

		exit(EXIT_FAILURE);
	}

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
	print_message("%d ", t->type);
	// print_message("...\n");
	// print_message("%zu - ", t->location.length);
	print_message("%s - ", token_type2str(t->type));
	print_message("%s\n", t->value);
}

void array_print(array_t* arr)
{
	// print_message("Array Length: %zu\n", arr->length);
	// print_message("Array Size: %zu\n", arr->size);
	// print_message("Array Contents:\n");

	for (size_t i = 0; i < arr->length; i++) {
		token_t* t = arr->data[i];
		print_message("[%zu]: ", i);
		token_print(t);
	}
}

lexer_t* lexer_create(const char* data)
{
	lexer_t* lexer;
	CREATE_MEMORY_OBJECT(lexer, lexer_t, 1, "Error: lexer_create<lexer> - Memory allocation error in %s:%d\n",  __FILE__, __LINE__);
	lexer->index = 0;
	lexer->tokens = array_create(10);

	if (data == NULL) {
		lexer->data = "";
		lexer->length = 0;
	} else {
		lexer->data = (char*) data;
		lexer->length = strlen(data);
	}

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
		(ch >= L'آ' && ch <= L'ی') ||
		ch == L'_' ||
		(ch >= 'a' && ch <= 'z') ||
		(ch >= 'A' && ch <= 'Z')
	)
	&&
	!(
		ch == '+' ||
		ch == '-' ||
		ch == '*' ||
		ch == '/' ||
		ch == '=' ||
		ch == '>' ||
		ch == '<' ||
		ch == ',' ||
		ch == '(' ||
		ch == ')' ||
		ch == '{' ||
		ch == '}'
	);
}

bool is_ident(wchar_t ch)
{
	return is_alpha(ch) || is_number(ch);
}

wchar_t read_token(lexer_t* lexer)
{
	lexer->last_index = lexer->index;
	lexer->last_line = lexer->line;
	lexer->last_column = lexer->column;

	wchar_t current_char;
    int char_size = mbtowc(&current_char, &lexer->data[lexer->index], MB_CUR_MAX);
	if (char_size < 0) {
		print_error(messages[language][MESSAGE_LEXER_TOKEN_READ_UNICODE]);

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

	printf("\tread token: %d - '%lc'\n", char_size, current_char);

	return current_char;
}

wchar_t unread_token(lexer_t* lexer)
{
	lexer->index = lexer->last_index;
	lexer->line = lexer->last_line;
	lexer->column = lexer->last_column;

	wchar_t current_char;
	int char_size = mbtowc(&current_char, &lexer->data[lexer->index], MB_CUR_MAX);
	if (char_size < 0) {
		print_error(messages[language][MESSAGE_LEXER_TOKEN_UNREAD_UNICODE]);

		exit(EXIT_FAILURE);
		return 0;
	}

	return current_char;
}

char digit_wchar2char(wchar_t ch)
{
	// TODO: Arabic digits

	// English digits
	if (ch >= '0' && ch <= '9') {
		return ch;
	}
	// Persian digits
	else {
		return ch - L'۰' + '0';
	}
}

void read_number(lexer_t* lexer, wchar_t ch)
{
	char number[21];
	int i = 0;
	while (is_number(ch)) {
		number[i] = digit_wchar2char(ch);

		i++;
		ch = read_token(lexer);
	}

	bool isFloat = false;
	bool hasEmptyDot = false;

	if (ch == '.') {
		ch = read_token(lexer);

		if (is_number(ch)) {
			print_message("it's a dot");
			number[i++] = '.';
			isFloat = true;

			while (is_number(ch)) {
				number[i] = digit_wchar2char(ch);
				
				i++;
				ch = read_token(lexer);
			}
		}
		else {
			hasEmptyDot = true;
		}
	}

	number[i] = 0;

	token_t* t = token_create(isFloat ? TOKEN_TYPE_FLOAT : TOKEN_TYPE_INT, number, i, lexer->line, lexer->column - i - (hasEmptyDot ? 1 : 0), lexer->line, lexer->column);
	array_push(lexer->tokens, t);

	if (hasEmptyDot) {
		token_t* t = token_create(TOKEN_TYPE_DOT, ".", i, lexer->line, lexer->column - 1, lexer->line, lexer->column);
		array_push(lexer->tokens, t);
	}

	unread_token(lexer);
}

void read_comment_singleline(lexer_t* lexer)
{
	// Eating until finding \n or EOF
	while (1) {
		if (lexer->data[lexer->index] == '\n' || lexer->data[lexer->index] == '\0') {
			break;
		}

		lexer->index++;
	}
}

void read_comment_multiline(lexer_t* lexer)
{
	// Eating until finding */
	// EOF is not allowed

	// TODO: Eating first character lexer->index++;
	while (1) {
		if (lexer->data[lexer->index] == '\0') {
			print_error(messages[language][MESSAGE_LEXER_COMMENT_MULTI_NOT_CLOSED]);

			exit(EXIT_FAILURE);
		} else if (lexer->data[lexer->index - 1] == '*' && lexer->data[lexer->index] == '/') {
			lexer->index++;

			break;
		}

		lexer->index++;
	}
}

void read_string(lexer_t* lexer, wchar_t ch)
{
	size_t allocated_size = 20;

	char* string;
	CREATE_MEMORY_OBJECT(string, char, allocated_size, "Error: read_string<string> - Memory allocation error in %s:%d\n",  __FILE__, __LINE__);

	size_t i = 0;

	while (ch != L'"') {
		if (i >= allocated_size - 1) {
			allocated_size *= 2;
			char* temp = (char*)realloc(string, sizeof(char) * allocated_size);
			if (temp == NULL) {
				print_error(messages[language][MESSAGE_LEXER_STRING_READ_MEMORY]);
				free(string);

				exit(EXIT_FAILURE);
			}
			string = temp;
		}

		if (ch == L'\\') {
			ch = read_token(lexer);
			if (ch == L'n') {
				string[i++] = '\n';
			} else if (ch == L't') {
				string[i++] = '\t';
			} else if (ch == L'"') {
				string[i++] = '"';
			} else if (ch == L'\'') {
				string[i++] = '\'';
			} else if (ch == L'\\') {
				string[i++] = '\\';
			} else {
				print_error(messages[language][MESSAGE_LEXER_STRING_UNKNOWN_ESCAPE]);
				free(string);

				exit(EXIT_FAILURE);
			}
		} else {
			int char_size = wctomb(&string[i], ch);
			if (char_size < 0) {
				print_error(messages[language][MESSAGE_LEXER_STRING_CONVERT_MULTIBYTE]);
				free(string);

				exit(EXIT_FAILURE);
			}
			i += char_size;
		}

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
		perror(messages[language][MESSAGE_LEXER_STRING_GET_LENGTH_UNICODE]);

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
			print_error(messages[language][MESSAGE_LEXER_IDENTIFIER_CONVERT_MULTIBYTE]);

			exit(EXIT_FAILURE);
		}
		i += char_size;
		ch = read_token(lexer);
	}
	identifier[i] = 0;

	int mapping_index = 0;
	token_type_t type = TOKEN_TYPE_IDENTIFIER;
	while (keyword_mapping[language][mapping_index].keyword != NULL) {
		if (strcmp(identifier, keyword_mapping[language][mapping_index].keyword) == 0) {
			type = keyword_mapping[language][mapping_index].token_type;
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
		perror(messages[language][MESSAGE_LEXER_CHAR_LENGTH_ISSUE]);
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

		printf("====>>> %lc\n", current_wchar);

		if (current_wchar == L'\u200C') {
			lexer->index++;
			lexer->column++;
			continue;
		} else if (current_wchar == '[') {
			token_t* t = token_create(TOKEN_TYPE_BRACKETS_OPEN, "[", 1, lexer->line, lexer->column - 1, lexer->line, lexer->column);
			array_push(lexer->tokens, t);
		} else if (current_wchar == ']') {
			token_t* t = token_create(TOKEN_TYPE_BRACKETS_CLOSE, "]", 1, lexer->line, lexer->column - 1, lexer->line, lexer->column);
			array_push(lexer->tokens, t);
		} else if (current_wchar == '%' || current_wchar == L'٪') {
			token_t* t = token_create(TOKEN_TYPE_MODULE, "%", 1, lexer->line, lexer->column - 1, lexer->line, lexer->column);
			array_push(lexer->tokens, t);
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
		} else if (current_wchar == ':') {
			token_t* t = token_create(TOKEN_TYPE_COLON, ":", 1, lexer->line, lexer->column - 1, lexer->line, lexer->column);
			array_push(lexer->tokens, t);
		} else if (current_wchar == '.') {
			token_t* t = token_create(TOKEN_TYPE_DOT, ".", 1, lexer->line, lexer->column - 1, lexer->line, lexer->column);
			array_push(lexer->tokens, t);
		} else if (current_wchar == '*') {
			token_t* t = token_create(TOKEN_TYPE_MULTIPLY, "*", 1, lexer->line, lexer->column - 1, lexer->line, lexer->column);
			array_push(lexer->tokens, t);
		} else if (current_wchar == '/') {
			if (lexer->index < lexer->length && lexer->data[lexer->index] == L'/') {
				lexer->index++;
				lexer->column++;
				read_comment_singleline(lexer);
			} else if (lexer->index < lexer->length && lexer->data[lexer->index] == L'*') {
				lexer->index++;
				lexer->column++;
				read_comment_multiline(lexer);
			} else {
				token_t* t = token_create(TOKEN_TYPE_DIVIDE, "/", 1, lexer->line, lexer->column - 1, lexer->line, lexer->column);
				array_push(lexer->tokens, t);
			}
		} else if (current_wchar == ',') {
			token_t* t = token_create(TOKEN_TYPE_COMMA, ",", 1, lexer->line, lexer->column - 1, lexer->line, lexer->column);
			array_push(lexer->tokens, t);
		} else if (current_wchar == '-') {
			token_t* t = token_create(TOKEN_TYPE_MINUS, "-", 1, lexer->line, lexer->column - 1, lexer->line, lexer->column);
			array_push(lexer->tokens, t);
		} else if (current_wchar == L'=') {
			if (lexer->index < lexer->length && lexer->data[lexer->index] == L'=') {
				token_t* t = token_create(TOKEN_TYPE_EQUAL_EQUAL, "==", 2, lexer->line, lexer->column - 2, lexer->line, lexer->column);
				array_push(lexer->tokens, t);
				lexer->index++;
				lexer->column++;
			} else {
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
			print_error(messages[language][MESSAGE_LEXER_UNEXPECTED_CHAR], current_char, lexer->line, lexer->column - 1);

			token_t* t = token_create(TOKEN_TYPE_ERROR, (char[]){current_char,'\0'}, 1, lexer->line, lexer->column - 1, lexer->line, lexer->column);
			array_push(lexer->tokens, t);
			lexer->index++;
		}
	}

	if (lexer->data[lexer->index] == 0) {
		token_t* t = token_create(TOKEN_TYPE_EOF, "\0", 1, lexer->line, lexer->column - 1, lexer->line, lexer->column);
		array_push(lexer->tokens, t);
	}
}

void help()
{
	printf("%s", messages[language][MESSAGE_WELCOME]);
}

parser_t* parser_create(lexer_t** lexer)
{
	parser_t* parser;
	CREATE_MEMORY_OBJECT(parser, parser_t, 1, "Error: parser_create<parser> - Memory allocation error in %s:%d\n",  __FILE__, __LINE__);

	parser->lexer = lexer;
	parser->token_index = 0;
	parser->functions = array_create(5);
	parser->expressions = array_create(5);

	return parser;
}

void parser_free(parser_t** parser)
{
	if (parser == NULL || *parser == NULL) {
		return;
	}

	free(*parser);
	*parser = NULL;
	print_message("*parser is null %d\n", *parser == NULL ? 1 : 0);
	print_message("parser is null %d\n", parser == NULL ? 1 : 0);
}

int main(int argc, char** argv)
{
    // setlocale(LC_ALL, "");
    setlocale(LC_ALL, "C.UTF-8");

	if (argc == 1 || argc > 3) {
		help();
		return 0;
	}

	if (argc == 3 && (strcmp(argv[1], "--code") != 0 && strcmp(argv[1], "--ast") != 0)) {
		help();
		return 0;
	}

	char* file_data;
	// bool isAst = false;
	// bool passingCode = false;

	if (argc == 2) {
		file_data = file_read(argv[1]);
	} else {
		if (strcmp(argv[1], "--ast") == 0) {
			// isAst = true;
		} else if (strcmp(argv[1], "--code") == 0) {
			// isAst = false;
		} else {
			print_message("Second argument should be either --ast or --code\n");
			help();
			return 0;
		}

		// passingCode = true;
		file_data = argv[2];
	}

    lexer_t* lexer = lexer_create(file_data);
    lexer_lex(lexer);

    array_print(lexer->tokens);

    print_message("free lexer\n");
    lexer_free(&lexer);
    print_message("end lexer free\n");

	return 0;
}
