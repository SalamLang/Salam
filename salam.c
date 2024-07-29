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

#include "salam.h"

// Global variables
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
		case TOKEN_TYPE_SHARP: return "SHARP";

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

void lexer_free(lexer_t* lexer)
{
	if (lexer == NULL) {
		return;
	}

	if (lexer->tokens != NULL) {
		for (size_t i = 0; i < lexer->tokens->length; i++) {
			token_t* t = (token_t*) lexer->tokens->data[i];
			if (t != NULL) {
				if (t->value != NULL) {
					free(t->value);
					t->value = NULL;
				}

				free(t);
				t = NULL;
			}
		}

		if (lexer->tokens->data != NULL) {
			free(lexer->tokens->data);
			lexer->tokens->data = NULL;
		}

		free(lexer->tokens);
		lexer->tokens = NULL;
	}

	if (lexer->data != NULL) {
		free(lexer->data);
		lexer->data = NULL;
	}

	free(lexer);
	lexer = NULL;
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
		} else if (current_wchar == '#') {
			token_t* t = token_create(TOKEN_TYPE_SHARP, "#", 1, lexer->line, lexer->column - 1, lexer->line, lexer->column);
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

parser_t* parser_create(lexer_t* lexer)
{
	parser_t* parser;
	CREATE_MEMORY_OBJECT(parser, parser_t, 1, "Error: parser_create<parser> - Memory allocation error in %s:%d\n",  __FILE__, __LINE__);

	parser->lexer = lexer;
	parser->token_index = 0;
	parser->functions = array_create(5);
	parser->expressions = array_create(5);

	return parser;
}

ast_node_t* parser_page_contents(parser_t* parser)
{
	printf("parser_page_contents...");

	return NULL;
}

ast_node_t* parser_page(parser_t* parser)
{
	parser_token_eat_nodata(parser, TOKEN_TYPE_PAGE);
	parser_token_eat_nodata(parser, TOKEN_TYPE_COLON);
	
	parser_page_contents(parser);

	parser_token_eat_nodata(parser, TOKEN_TYPE_END);

	return NULL;
}

void token_free(token_t* token)
{
	free(token);
}

void parser_token_eat_nodata(parser_t* parser, token_type_t type)
{
	if (parser->lexer->tokens->length > parser->token_index) {
		token_t* token = (token_t*) parser->lexer->tokens->data[parser->token_index];

		if (token->type == type) {
			parser->token_index++;
		} else {
			token_free(token);
			print_message("Error: Expected %s\n", token_type2str(type));

			exit(EXIT_FAILURE);
		}
	} else {
		print_message("Error: Unexpected end of file\n");

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
			token_free(token);
			print_message("Error: Expected %s\n", token_type2str(type));

			exit(EXIT_FAILURE);
		}
	} else {
		print_message("Error: Unexpected end of file\n");

		exit(EXIT_FAILURE);
	}

	return NULL;
}

void parser_parse(parser_t* parser)
{
	if (parser->lexer->tokens->length == 1 &&
		((token_t*)parser->lexer->tokens->data[0])->type == TOKEN_TYPE_EOF
	) {
		return;
	}

	ast_node_t* expression_node;
	ast_node_t* function_node;
	ast_node_t* page_node;

	while (parser->token_index < parser->lexer->tokens->length) {
		token_t* current_token = (token_t*) parser->lexer->tokens->data[parser->token_index];

		switch (current_token->type) {
			case TOKEN_TYPE_EOF:
				parser->token_index++;
				return;

			// case TOKEN_TYPE_FUNCTION:
			// 	function_node = parser_function(parser);
			// 	if (parser->functions && function_node != NULL) {
			// 		array_push(parser->functions, function_node);
			// 	}
			// 	break;

			case TOKEN_TYPE_PAGE:
				page_node = parser_page(parser);
				break;

			default:
				// if (parser_expression_has(parser)) {
				// 	CREATE_MEMORY_OBJECT(expression_node, ast_node_t, 1, "Error: parser_parse<expression_node> - Memory allocation error in %s:%d\n",  __FILE__, __LINE__);

				// 	expression_node->type = AST_EXPRESSION;
				// 	expression_node->data.expression = parser_expression(parser);
				// 	if (parser->expressions && expression_node != NULL) {
				// 		array_push(parser->expressions, expression_node);
				// 	}
				// } else {
					print_error(messages[language][MESSAGE_PARSER_BAD_TOKEN_AS_STATEMENT], token_type2str(current_token->type));

					exit(EXIT_FAILURE);
				// }
				break;
		}
	}
}

void parser_free(parser_t* parser)
{
	if (parser == NULL) {
		return;
	}

	if (parser->functions != NULL) {
		if (parser->functions->data != NULL) {
			for (size_t i = 0; i < parser->functions->length; i++) {
				print_message("Free function %s\n", ((ast_node_t*) parser->functions->data[i])->data.function_declaration->name);
				if (parser->functions->data[i] != NULL) {
					// ast_node_free((ast_node_t**) &(parser->functions->data[i]));
					parser->functions->data[i] = NULL;
				}
			}

			free(parser->functions->data);
			parser->functions->data = NULL;
		}

		free(parser->functions);
		parser->functions = NULL;
	}

	if (parser->expressions != NULL) {
		if (parser->expressions->data != NULL) {
			for (size_t i = 0; i < parser->expressions->length; i++) {
				if (parser->expressions->data[i] != NULL) {
					// ast_node_free((ast_node_t**) &(parser->expressions->data[i]));
					parser->expressions->data[i] = NULL;
				}
			}

			free(parser->expressions->data);
			parser->expressions->data = NULL;
		}

		free(parser->expressions);
		parser->expressions = NULL;
	}

	free(parser);
	parser = NULL;
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
	bool passingCode = false;

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

		passingCode = true;
		file_data = argv[2];
	}

    lexer_t* lexer = lexer_create(file_data);
    lexer_lex(lexer);

	if (!passingCode) {
	    array_print(lexer->tokens);
	}

	parser_t* parser = parser_create(lexer);
	parser_parse(parser);
	parser_free(parser);

    print_message("free lexer\n");
    lexer_free(lexer);
    print_message("end lexer free\n");

	return 0;
}
