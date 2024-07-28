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

#define print_error(fmt, ...) fprintf(stderr, fmt, ##__VA_ARGS__)

#define print_message(fmt, ...) if (debug_enabled) fprintf(stdout, fmt, ##__VA_ARGS__)

#define CREATE_MEMORY_OBJECT(result, type, length, fmt, ...) \
	result = (type *)malloc(sizeof(type) * (length));  \
	if (!result) { \
		fprintf(stderr, fmt,  __VA_ARGS__);\
		exit(EXIT_FAILURE); \
	}

// Types
typedef enum {
	// Values
	TOKEN_TYPE_IDENTIFIER,
	TOKEN_TYPE_INT,
	TOKEN_TYPE_FLOAT,
	TOKEN_TYPE_STRING,
	
	// Keywords
	TOKEN_TYPE_FUNCTION, // تابع
	TOKEN_TYPE_RETURN, // برگشت
	TOKEN_TYPE_BREAK, // توقف
	TOKEN_TYPE_CONTINUE, // ادامه
	TOKEN_TYPE_PRINT, // نمایش
	TOKEN_TYPE_IF, // اگر

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

// Headers
char* file_read(char* file_Name);
lexer_t* lexer_create(const char* data);
void lexer_free(lexer_t** lexer);
array_t* array_create(size_t size);
void* array_pop(array_t* arr);
void array_push(array_t* arr, void* data);
void array_free(array_t* arr);
void array_print(array_t* arr);
void lexer_lex(lexer_t* lexer);
token_t* token_create(token_type_t type, const char* value, int a, int b, int c, int b2, int c2);

// Functions
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
		perror("MESSAGE_MEMORY_ALLOCATE_ERROR");

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

void array_print(array_t* arr)
{
	print_message("Array Length: %zu\n", arr->length);
	print_message("Array Size: %zu\n", arr->size);
	print_message("Array Contents:\n");

	for (size_t i = 0; i < arr->length; i++) {
		print_message("[%zu]: ...\n", i);
	}
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
			print_error("MESSAGE_LEXER_UNEXPECTED_CHAR", current_char, lexer->line, lexer->column - 1);

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

char* file_read(char* file_Name)
{
	FILE* file = fopen(file_Name, "r");
	if (file == NULL) {
		print_error("Error: this file %s not exists!", file_Name);
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

int main(int argc, char** argv)
{
    if (argc < 2) {
        print_error("Error: no input file!\n");
        return 1;
    }

    char* file_data = file_read(argv[1]);

    lexer_t* lexer = lexer_create(file_data);
    lexer_lex(lexer);

    // parser_t* parser = parser_create(&lexer);
    // parser_parse(parser);

    return 0;
}
