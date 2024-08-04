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
char* token_type2str(token_type_t type)
{
	switch(type) {
		// Keywords
		case TOKEN_TYPE_END: return "END";
		case TOKEN_TYPE_LAYOUT: return "LAYOUT";
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

char* file_read(char* file_name)
{
	FILE* file = fopen(file_name, "r");
	if (file == NULL) {
		print_error(messages[language][MESSAGE_LEXER_FILE_NOT_EXISTS], file_name);
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
	array_t* arr;
	CREATE_MEMORY_OBJECT(arr, array_t, 1, "Error: array_create<arr> - Memory allocation error in %s:%d\n",  __FILE__, __LINE__);

	size_t min_size = 1;
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
	if (arr->length == 0) return NULL;

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
	if (arr == NULL) return;

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
	print_message("Array Length: %zu\n", arr->length);
	print_message("Array Size: %zu\n", arr->size);
	print_message("Array Contents:\n");

	for (size_t i = 0; i < arr->length; i++) {
		token_t* t = arr->data[i];
		print_message("[%zu]: ", i);
		token_print(t);
	}
}

unsigned long hash_function(const char *str)
{
	unsigned long hash = 5381;
	int c;

	while ((c = *str++)) {
		hash = ((hash << 5) + hash) + c;
	}

	return hash;
}

hashmap_t* hashmap_create()
{
	hashmap_t *map;
	map = (hashmap_t*) malloc(sizeof(hashmap_t));
	map->size = 16;
	map->length = 0;
	map->data = (hashmap_entry_t**) calloc(map->size, sizeof(hashmap_entry_t*));

	if (!map->data) {
		perror("Error: hashmap_create - Memory allocation error");
		exit(EXIT_FAILURE);
	}

	return map;
}

void hashmap_put(hashmap_t *map, const char *key, void *value)
{
	unsigned long hash = hash_function(key);

	size_t index = hash % map->size;
	hashmap_entry_t *entry = map->data[index];
	
	while (entry != NULL) {
		if (strcmp(entry->key, key) == 0) {
			free(entry->value);
			entry->value = NULL;

			entry->value = strdup(value);

			return;
		}

		entry = entry->next;
	}

	hashmap_entry_t *new_entry = (hashmap_entry_t*) malloc(sizeof(hashmap_entry_t));
	new_entry->key = strdup(key);
	new_entry->value = strdup(value);
	new_entry->next = map->data[index];
	map->data[index] = new_entry;
	map->length++;

	if ((float)map->length / map->size >= 0.75) {
		size_t new_length = map->size * 2;
		hashmap_entry_t **new_data = (hashmap_entry_t**) calloc(new_length, sizeof(hashmap_entry_t*));
		for (size_t i = 0; i < map->size; i++) {
			hashmap_entry_t *entry = map->data[i];

			while (entry) {
				hashmap_entry_t *next = entry->next;
				unsigned long new_index = hash_function(entry->key) % new_length;
				entry->next = new_data[new_index];
				new_data[new_index] = entry;
				entry = next;
			}
		}

		free(map->data);
		map->data = new_data;
		map->size = new_length;
	}
}

void* hashmap_get(hashmap_t *map, const char *key)
{
	unsigned long hash = hash_function(key);
	size_t index = hash % map->size;
	hashmap_entry_t *entry = map->data[index];

	while (entry != NULL) {
		if (strcmp(entry->key, key) == 0) {
			return entry->value;
		}
		entry = entry->next;
	}

	return NULL;
}

void* hashmap_remove(hashmap_t *map, const char *key)
{
	unsigned long hash = hash_function(key);

	size_t index = hash % map->size;
	hashmap_entry_t *entry = map->data[index];
	hashmap_entry_t *prev = NULL;

	while (entry != NULL) {
		if (strcmp(entry->key, key) == 0) {
			if (prev == NULL) {
				map->data[index] = entry->next;
			} else {
				prev->next = entry->next;
			}
			void *value = entry->value;
			free(entry->key);
			free(entry);
			map->length--;

			return value;
		}
		prev = entry;
		entry = entry->next;
	}

	return NULL;
}

void hashmap_free(hashmap_t *map)
{
	for (size_t i = 0; i < map->size; i++) {
		hashmap_entry_t *entry = map->data[i];

		while (entry) {
			hashmap_entry_t *next = entry->next;
			free(entry->key);
			free(entry->value);

			free(entry);
			entry = next;
		}
	}

	free(map->data);
	free(map);
}

void hashmap_print(hashmap_t *map)
{
	printf("Hashmap Size: %zu\n", map->length);
	printf("Hashmap Capacity: %zu\n", map->size);
	printf("Hashmap Contents:\n");

	for (size_t i = 0; i < map->size; i++) {
		hashmap_entry_t *entry = map->data[i];

		while (entry) {
			printf("[%zu] Key: %s, Value: %p\n", i, entry->key, entry->value);
			entry = entry->next;
		}
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
	if (lexer == NULL) return;

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
	lexer->last_length = lexer->length;

	wchar_t current_char;
	int char_size = mbtowc(&current_char, &lexer->data[lexer->index], MB_CUR_MAX);
	if (char_size < 0) {
		print_error("%s", messages[language][MESSAGE_LEXER_TOKEN_READ_UNICODE]);

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
	lexer->length = lexer->last_length;

	wchar_t current_char;
	int char_size = mbtowc(&current_char, &lexer->data[lexer->index], MB_CUR_MAX);
	if (char_size < 0) {
		print_error("%s", messages[language][MESSAGE_LEXER_TOKEN_UNREAD_UNICODE]);

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
			print_error("%s", messages[language][MESSAGE_LEXER_COMMENT_MULTI_NOT_CLOSED]);

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
		if (ch == '\0') break;
		if (i >= allocated_size - 1) {
			allocated_size *= 2;
			char* temp = (char*) realloc(string, sizeof(char) * allocated_size);
			if (temp == NULL) {
				print_error("%s", messages[language][MESSAGE_LEXER_STRING_READ_MEMORY]);
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
				print_error("%s", messages[language][MESSAGE_LEXER_STRING_UNKNOWN_ESCAPE]);
				free(string);

				exit(EXIT_FAILURE);
			}
		} else {
			int char_size = wctomb(&string[i], ch);
			if (char_size < 0) {
				print_error("%s", messages[language][MESSAGE_LEXER_STRING_CONVERT_MULTIBYTE]);
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

token_type_t convert_identifier_token_type(char* identifier)
{
	int mapping_index = 0;
	token_type_t type = TOKEN_TYPE_IDENTIFIER;

	while (keyword_mapping[language][mapping_index].keyword != NULL) {
		if (strcmp(identifier, keyword_mapping[language][mapping_index].keyword) == 0) {
			type = keyword_mapping[language][mapping_index].token_type;
			break;
		}

		mapping_index++;
	}

	return type;
}

ast_layout_type_t convert_layout_identifier_token_type(char* identifier)
{
	int mapping_index = 0;
	ast_layout_type_t type = AST_TYPE_LAYOUT_ERROR;

	while (layout_keyword_mapping[language][mapping_index].keyword != NULL) {
		if (strcmp(identifier, layout_keyword_mapping[language][mapping_index].keyword) == 0) {
			type = layout_keyword_mapping[language][mapping_index].layout_type;
			break;
		}

		mapping_index++;
	}

	return type;	
}

void read_identifier(lexer_t* lexer, wchar_t ch)
{
	char identifier[256];
	int i = 0;
	while (is_ident(ch)) {
		int char_size = wctomb(&identifier[i], ch);
		if (char_size < 0) {
			print_error("%s", messages[language][MESSAGE_LEXER_IDENTIFIER_CONVERT_MULTIBYTE]);

			exit(EXIT_FAILURE);
		}
		i += char_size;
		ch = read_token(lexer);
	}
	identifier[i] = 0;

	token_type_t type = convert_identifier_token_type(identifier);

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
	parser->layout = NULL;

	return parser;
}

ast_layout_node_t* parser_layout_element_single(ast_layout_type_t type, parser_t* parser)
{
	ast_layout_node_t* element;
	CREATE_MEMORY_OBJECT(element, ast_layout_node_t, 1, "Error: parser_layout_element_single<element> - Memory allocation error in %s:%d\n",  __FILE__, __LINE__);
	element->type = type;
	element->children = array_create(1);
	element->attributes = hashmap_create();
	element->is_mother = false;

	parser->token_index++; // Eating keyword

	return element;
}

ast_layout_node_t* parser_layout_element_mother(ast_layout_type_t type, parser_t* parser)
{
	printf("parser_layout_element_mother\n");
	ast_layout_node_t* element;
	CREATE_MEMORY_OBJECT(element, ast_layout_node_t, 1, "Error: parser_layout_element_mother<element> - Memory allocation error in %s:%d\n",  __FILE__, __LINE__);
	element->type = type;
	element->children = array_create(1);
	element->attributes = hashmap_create();
	element->is_mother = true;

	parser->token_index++; // Eating keyword

	parser_token_eat_nodata(parser, TOKEN_TYPE_COLON);

	while (parser->token_index < parser->lexer->tokens->length) {
		token_t* current_token = parser_token_get(parser);

		if (current_token->type == TOKEN_TYPE_END) break;

		ast_layout_type_t type = convert_layout_identifier_token_type(current_token->value);

		if (type == AST_TYPE_LAYOUT_ERROR) {
			parser->token_index++; // Eating the attr name

			parser_token_eat_nodata(parser, TOKEN_TYPE_COLON); // :
			
			token_t* attr_value = parser_token_get(parser);
			parser->token_index++;

			hashmap_put(element->attributes, current_token->value, attr_value->value);
		} else {
			array_push(element->children, parser_layout_element(parser));			
		}
	}

	parser_token_eat_nodata(parser, TOKEN_TYPE_END);

	return element;
}

ast_layout_node_t* parser_layout_element(parser_t* parser)
{
	token_t* current_token = parser_token_get(parser);

	if (current_token->type != TOKEN_TYPE_IDENTIFIER) return NULL;
	
	ast_layout_type_t type = convert_layout_identifier_token_type(current_token->value);

	switch (type) {
		case AST_TYPE_LAYOUT_TEXT:
		case AST_TYPE_LAYOUT_INPUT:
		case AST_TYPE_LAYOUT_BUTTON:
			return parser_layout_element_mother(type, parser);
			break;
		
		case AST_TYPE_LAYOUT_LINE:
		case AST_TYPE_LAYOUT_BREAK:
			return parser_layout_element_single(type, parser);
			break;

		default:
			return NULL;
			break;
	}
}

array_t* parser_layout_elements(parser_t* parser)
{
	array_t* elements = array_create(4);

	while (parser->token_index < parser->lexer->tokens->length) {
		token_t* current_token = parser_token_get(parser);

		if (current_token->type == TOKEN_TYPE_IDENTIFIER) {
			ast_layout_node_t* element = parser_layout_element(parser);
			if (element == NULL) {
				token_free(current_token);
				array_free(elements);

				print_message("Error: invalid element inside layout block!");
				
				exit(EXIT_FAILURE);
			}

			array_push(elements, element);
		}
		else {
			break;
		}
	}

	return elements;
}

ast_layout_t* parser_layout(parser_t* parser)
{
	printf("parser_layout\n");
	ast_layout_t* layout;

	CREATE_MEMORY_OBJECT(layout, ast_layout_t, 1, "Error: parser_layout<layout> - Memory allocation error in %s:%d\n",  __FILE__, __LINE__);

	parser_token_eat_nodata(parser, TOKEN_TYPE_LAYOUT);
	parser_token_eat_nodata(parser, TOKEN_TYPE_COLON);
	
	layout->elements = parser_layout_elements(parser);

	parser_token_eat_nodata(parser, TOKEN_TYPE_END);

	return layout;
}

void token_free(token_t* token)
{
	if (token->value != NULL) free(token->value);

	free(token);
}

void parser_token_eat_nodata(parser_t* parser, token_type_t type)
{
	if (parser->lexer->tokens->length > parser->token_index) {
		token_t* token = parser_token_get(parser);

		if (token->type == type) {
			parser->token_index++;
		} else {
			token_free(token);
			print_message("Error: Expected nodata %s\n", token_type2str(type));

			exit(EXIT_FAILURE);
		}
	} else {
		print_message("Error: Unexpected end of file\n");

		exit(EXIT_FAILURE);
	}
}

token_t* parser_token_get(parser_t* parser)
{
	if (parser->lexer->tokens->length > parser->token_index) {
		return (token_t*) parser->lexer->tokens->data[parser->token_index];
	}

	return NULL;
}

token_t* parser_token_eat(parser_t* parser, token_type_t type)
{
	if (parser->lexer->tokens->length > parser->token_index) {
		token_t* token = parser_token_get(parser);

		if (token->type == type) {
			return (token_t*) parser->lexer->tokens->data[parser->token_index++];
		} else {
			token_free(token);
			print_message("Error: Expected data %s\n", token_type2str(type));

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
	// Empty tokens
	if (parser->lexer->tokens->length == 0) return;

	// Only one EOF token
	if (parser->lexer->tokens->length == 1 && ((token_t*)parser->lexer->tokens->data[0])->type == TOKEN_TYPE_EOF) return;

	ast_node_t* layout_node;
	ast_node_t* function_node;
	ast_node_t* expression_node;

	while (parser->token_index < parser->lexer->tokens->length) {
		token_t* current_token = parser_token_get(parser);

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

			case TOKEN_TYPE_LAYOUT:
				parser->layout = parser_layout(parser);
				break;
			
			case TOKEN_TYPE_IDENTIFIER:

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

void ast_layout_node_free(ast_layout_node_t* node)
{
	if (node == NULL) return;

	if (node->attributes != NULL) {
		if (node->attributes->data != NULL) {
			for (size_t i = 0; i < node->attributes->size; i++) {
				hashmap_entry_t *entry = node->attributes->data[i];

				while (entry != NULL) {
					if (entry->key != NULL) {
						free(entry->key);
						entry->key = NULL;
					}

					if (entry->value != NULL) {
						free(entry->value);
						entry->value = NULL;
					}

					hashmap_entry_t *buf = entry;

					entry = entry->next;

					free(buf);
					buf = NULL;
				}

				free(node->attributes->data[i]);
				node->attributes->data[i] = NULL;
			}

			free(node->attributes->data);
			node->attributes->data = NULL;
		}

		free(node->attributes);
		node->attributes = NULL;
	}

	if (node->children != NULL) {
		if (node->children->data != NULL) {
			for (size_t i = 0; i < node->children->length; i++) {
				if (node->children->data[i] != NULL) {
					ast_layout_node_free((ast_layout_node_t*) node->children->data[i]);
				}
			}

			free(node->children->data);
			node->children->data = NULL;
		}

		free(node->children);
		node->children = NULL;
	}

	free(node);
	node = NULL;
}

char* ast_node_type_string(ast_node_type_t type)
{
	switch (type) {
		case AST_TYPE_FUNCTION: return "function";
		case AST_TYPE_LAYOUT: return "layout";
		default: "none";
	}
}

char* ast_layout_type_string(ast_layout_type_t type)
{
	switch (type) {
		case AST_TYPE_LAYOUT_BUTTON: return "button";
		case AST_TYPE_LAYOUT_TEXT: return "text";
		case AST_TYPE_LAYOUT_INPUT: return "input";
		case AST_TYPE_LAYOUT_LINE: return "line";
		case AST_TYPE_LAYOUT_BREAK: return "break";

		case AST_TYPE_LAYOUT_ERROR:
		default:
			return "error";
	}
}

char* generate_layout_type_string(ast_layout_type_t type)
{
	switch (type) {
		case AST_TYPE_LAYOUT_BUTTON: return "button";
		case AST_TYPE_LAYOUT_TEXT: return "text";
		case AST_TYPE_LAYOUT_INPUT: return "input";
		case AST_TYPE_LAYOUT_LINE: return "br";
		case AST_TYPE_LAYOUT_BREAK: return "hr";

		case AST_TYPE_LAYOUT_ERROR:
		default:
			return "error";
	}
}

void ast_node_free(ast_node_t* node)
{
	if (node == NULL) return;

	switch (node->type) {
		case AST_TYPE_FUNCTION:

			break;
		
		case AST_TYPE_LAYOUT:
			if (node->data.layout != NULL) {
				if (node->data.layout->elements != NULL) {
					if (node->data.layout->elements->data != NULL) {
						for (size_t i = 0; i < node->data.layout->elements->length; i++) {
							if (node->data.layout->elements->data[i] != NULL) {
								ast_layout_node_free((ast_layout_node_t*) node->data.layout->elements->data[i]);
							}
						}
					}

					free(node->data.layout->elements->data);
					node->data.layout->elements->data = NULL;

					free(node->data.layout->elements);
					node->data.layout->elements = NULL;
				}

				free(node->data.layout);
				node->data.layout = NULL;
			}
			break;
		
		default:
			break;
	}

	free(node);
	node = NULL;
}

void parser_free(parser_t* parser)
{
	if (parser == NULL) {
		return;
	}

	if (parser->layout != NULL) {
		if (parser->layout->elements->data != NULL) {
			for (size_t i = 0; i < parser->layout->elements->length; i++) {
				if (parser->layout->elements->data[i] != NULL) {
					ast_layout_node_free((ast_layout_node_t*) parser->layout->elements->data[i]);
				}
			}
		}
		free(parser->layout->elements->data);
		parser->layout->elements->data = NULL;

		free(parser->layout->elements);
		parser->layout->elements = NULL;

		free(parser->layout);
		parser->layout = NULL;
	}

	if (parser->functions != NULL) {
		if (parser->functions->data != NULL) {
			for (size_t i = 0; i < parser->functions->length; i++) {
				print_message("Free function %s\n", ((ast_node_t*) parser->functions->data[i])->data.function_declaration->name);
				if (parser->functions->data[i] != NULL) {
					ast_node_free((ast_node_t*) parser->functions->data[i]);
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
					ast_node_free((ast_node_t*) parser->expressions->data[i]);
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
}

string_t* string_create(size_t initial_size)
{
	string_t* str;
	CREATE_MEMORY_OBJECT(str, string_t, 1, "Error: string_create<str> - Memory allocation error in %s:%d\n",  __FILE__, __LINE__);
	str->size = initial_size;
	str->length = 0;

	CREATE_MEMORY_OBJECT(str->data, char, initial_size, "Error: string_create<str_data> - Memory allocation error in %s:%d\n",  __FILE__, __LINE__);
	if (str->data == NULL) {
		fprintf(stderr, "Memory allocation failed\n");
		exit(1);
	}
	str->data[0] = '\0';

	return str;
}

void string_append_char(string_t* str, char c)
{
	if (str->length + 1 >= str->size) {
		str->size *= 2;
		str->data = (char*) realloc(str->data, str->size * sizeof(char));
		if (str->data == NULL) {
			fprintf(stderr, "Memory reallocation failed\n");
			exit(1);
		}
	}
	str->data[str->length] = c;
	str->length++;
	str->data[str->length] = '\0';
}

void string_append_str(string_t* str, const char* suffix)
{
	size_t suffix_len = strlen(suffix);
	while (str->length + suffix_len >= str->size) {
		str->size *= 2;
		str->data = (char*) realloc(str->data, str->size * sizeof(char));
		if (str->data == NULL) {
			fprintf(stderr, "Memory reallocation failed\n");
			exit(1);
		}
	}
	strcpy(str->data + str->length, suffix);
	str->length += suffix_len;
}

void string_free(string_t* str)
{
	if (str == NULL) return;

	free(str->data);
	str->data = NULL;

	str->size = 0;
	str->length = 0;

	free(str);
	str = NULL;
}

void string_print(string_t* str)
{
	if (str == NULL || str->data == NULL) printf("NULL\n");
	else printf("%s\n", str->data);
}

void ast_layout_ident(string_t* str, int ident)
{
	for (int i = 0; i < ident; i++) string_append_char(str, '\t');
}

string_t* ast_layout_string(ast_layout_node_t* element, parser_t* parser, int ident)
{
	string_t* str = string_create(10);
	
	char* element_name = ast_layout_type_string(element->type);
	
	ast_layout_ident(str, ident);

	string_append_str(str, "<layout_");
	string_append_str(str, element_name);

	if (element->attributes != NULL && element->attributes->size > 0) {
		for (size_t i = 0; i < element->attributes->size; i++) {
			hashmap_entry_t *entry = element->attributes->data[i];

			while (entry) {
				string_append_char(str, ' ');

				string_append_str(str, entry->key);
				string_append_char(str, '=');
				string_append_char(str, '\"');
				string_append_str(str, entry->value);
				string_append_char(str, '\"');

				entry = entry->next;
			}
		}
	}

	if (element->is_mother) {
		string_append_str(str, ">\n");

		if (element->children != NULL) {
			for (size_t j = 0; j < element->children->length; j++) {
				ast_layout_node_t* child = (ast_layout_node_t*) element->children->data[j];

				string_t* buf = ast_layout_string(child, parser, ident + 1);
				string_append(str, buf);
				string_free(buf);
			}
		}
		
		ast_layout_ident(str, ident);

		string_append_str(str, "</layout_");
		string_append_str(str, element_name);
		string_append_str(str, ">\n");
	}
	else {
		string_append_str(str, " />\n");
	}
	
	return str;
}

void string_append(string_t* dest, string_t* src)
{
	if (dest == NULL || dest->data == NULL) return;
	if (src == NULL || src->data == NULL) return;

	string_append_str(dest, src->data);
}

string_t* ast_string(parser_t* parser, int ident)
{
	string_t* str = string_create(10);

	if (parser == NULL) return str;

	if (parser->layout != NULL) {
		string_append_str(str, "<layout>\n");

		for (size_t i = 0; i < parser->layout->elements->length; i++) {
			ast_layout_node_t* element = (ast_layout_node_t*) parser->layout->elements->data[i];
			
			string_t* buf = ast_layout_string(element, parser, ident + 1);
			string_append(str, buf);
			string_free(buf);
		}

		string_append_str(str, "</layout>\n");
	}
	else {
		string_append_str(str, "<layout />\n");
	}

	return str;
}

void ast_print(parser_t* parser)
{
	string_t* tree = ast_string(parser, 0);

	printf("XML AST Tree:\n");

	string_print(tree);

	string_free(tree);
}

void generate_layout_ident(string_t* str, int ident)
{
	for (int i = 0; i < ident; i++) string_append_char(str, '\t');
}

string_t* generate_layout(ast_layout_node_t* node, parser_t* parser, int ident)
{
	string_t* str = string_create(10);

	return str;
}

bool is_style_attribute(char* attribute_name)
{
	if (strcmp(attribute_name, "رنگ") == 0) return true;
	else if (strcmp(attribute_name, "زمینه") == 0) return true;
	else if (strcmp(attribute_name, "فونت") == 0) return true;
	else if (strcmp(attribute_name, "اندازه") == 0) return true;
	else if (strcmp(attribute_name, "فاصله") == 0) return true;
	else if (strcmp(attribute_name, "فضا") == 0) return true;
	else if (strcmp(attribute_name, "حاشیه") == 0) return true;
	else if (strcmp(attribute_name, "ماوس") == 0) return true;
	else if (strcmp(attribute_name, "گردی") == 0) return true;

	return false;
}

char* attribute_style2css(const char* attribute_name) {
	char* res = malloc(sizeof(char) * 18);
	if (res == NULL) {
		fprintf(stderr, "Memory allocation failed\n");
		exit(1);
	}

	if (strcmp(attribute_name, "رنگ") == 0) strcpy(res, "color");
	else if (strcmp(attribute_name, "زمینه") == 0) strcpy(res, "background-color");
	else if (strcmp(attribute_name, "فونت") == 0) strcpy(res, "font-family");
	else if (strcmp(attribute_name, "اندازه") == 0) strcpy(res, "font-size");
	else if (strcmp(attribute_name, "فاصله") == 0) strcpy(res, "padding");
	else if (strcmp(attribute_name, "فضا") == 0) strcpy(res, "margin");
	else if (strcmp(attribute_name, "حاشیه") == 0) strcpy(res, "border");
	else if (strcmp(attribute_name, "ماوس") == 0) strcpy(res, "cursor");
	else if (strcmp(attribute_name, "گردی") == 0) strcpy(res, "border-radius");
	else strcpy(res, "");

	return res;
}

string_t* generate_layout_element_attribute(parser_t* parser, hashmap_entry_t* entry)
{
	string_t* str = string_create(10);

	string_append_str(str, entry->key);
	string_append_char(str, '=');

	if (strlen(entry->value) == 1) {
		string_append_str(str, entry->value);
	}
	else {
		string_append_char(str, '\"');
		string_append_str(str, entry->value);
		string_append_char(str, '\"');
	}

	return str;
}

string_t* generate_layout_element_attributes(parser_t* parser, ast_layout_node_t* element, char** element_content, int ident)
{
	string_t* str = string_create(10);

	hashmap_t* styles = hashmap_create();
	int html_attrs = 0;

	if (element->attributes != NULL && element->attributes->size > 0) {
		for (size_t i = 0; i < element->attributes->size; i++) {
			hashmap_entry_t *entry = element->attributes->data[i];

			while (entry) {
				if (is_style_attribute(entry->key)) {
					hashmap_put(styles, entry->key, entry->value);
				}
				else {
					if (strcmp(entry->key, "داده") == 0) *element_content = entry->value;
					else {
						string_append_char(str, ' ');
						html_attrs++;

						string_t* buf = generate_layout_element_attribute(parser, entry);

						if (strlen(buf->data) > 0) string_append(str, buf);

						string_free(buf);
					}
				}

				hashmap_entry_t *temp = entry;
				free(temp->key);
				temp->key = NULL;
				free(temp->value);
				temp->value = NULL;
				
				entry = entry->next;

				free(temp);
				temp = NULL;
			}
		}

		hashmap_free(element->attributes);
		// free(element->attributes);
		element->attributes = NULL;
	}

	if (styles->length > 0) {
		if (html_attrs > 0) string_append_char(str, ' ');

		string_append_str(str, "style=\"");

		printf("-->%ld\n", styles->length);

		for (size_t i = 0; i < styles->length; i++) {
			hashmap_entry_t *entry = styles->data[i];

			while (entry) {
				char* buf = attribute_style2css(entry->key);
				string_append_str(str, buf);
				free(buf);
				buf = NULL;

				string_append_char(str, ':');
				string_append_str(str, entry->value);
				string_append_char(str, ';');

				hashmap_entry_t *temp = entry;
				free(temp->key);
				temp->key = NULL;
				free(temp->value);
				temp->value = NULL;
				
				entry = entry->next;

				free(temp);
				temp = NULL;
			}
		}

		string_append_str(str, "\"");
	}

	free(styles->data);
	styles->data = NULL;
	free(styles);
	styles = NULL;

	return str;
}

string_t* generate_layout_element(ast_layout_node_t* element, parser_t* parser, int ident)
{
	string_t* str = string_create(10);

	if (parser == NULL) return str;
	if (element == NULL) return str;

	char* element_name = generate_layout_type_string(element->type);
	char* element_content = NULL;

	generate_layout_ident(str, ident);

	string_append_char(str, '<');
	string_append_str(str, element_name);

	string_t* buf = generate_layout_element_attributes(parser, element, &element_content, ident);
	if (buf->length > 0) {
		string_append_char(str, ' ');
		
		string_append(str, buf);
	}
	string_free(buf);

	string_append_char(str, '>');
	bool needBreak = false;

	if (element->is_mother) {
		if (element->children != NULL && element->children->length > 0) {
			needBreak = true;
			string_append_char(str, '\n');

			for (size_t i = 0; i < element->children->length; i++) {
				ast_layout_node_t *entry = element->children->data[i];

				string_t* buf = generate_layout_element(entry, parser, ident + 1);
				string_append(str, buf);
				string_free(buf);

				ast_layout_node_free(entry);
			}
		}

		if (element_content != NULL && strlen(element_content) > 0) {
			if (needBreak) generate_layout_ident(str, ident + 1);

			string_append_str(str, element_content);
			
			if (needBreak) string_append_char(str, '\n');
		}

		if (needBreak) generate_layout_ident(str, ident);

		string_append_char(str, '<');
		string_append_char(str, '/');
		string_append_str(str, element_name);
		string_append_char(str, '>');
		string_append_char(str, '\n');
	}
	else {
		needBreak = true;
		string_append_char(str, '\n');
	}


	return str;
}

string_t* generate_string(parser_t* parser, int ident)
{
	string_t* str = string_create(10);

	if (parser == NULL) return str;

	if (parser->layout != NULL) {
		generate_layout_ident(str, ident);
		string_append_str(str, "<!doctype html>\n");

		generate_layout_ident(str, ident);
		string_append_str(str, "<html dir=\"rtl\" lang=\"fa_IR\">\n");

		generate_layout_ident(str, ident + 1);
		string_append_str(str, "<head>\n");
		generate_layout_ident(str, ident + 2);
		string_append_str(str, "<meta charset=\"UTF-8\">\n");
		generate_layout_ident(str, ident + 1);
		string_append_str(str, "</head>\n");

		generate_layout_ident(str, ident + 1);
		string_append_str(str, "<body>\n");

		for (size_t i = 0; i < parser->layout->elements->length; i++) {
			ast_layout_node_t* element = (ast_layout_node_t*) parser->layout->elements->data[i];

			string_t* buf = generate_layout_element(element, parser, ident + 2);
			string_append(str, buf);
			string_free(buf);
		}

		generate_layout_ident(str, ident + 1);
		string_append_str(str, "</body>\n");

		generate_layout_ident(str, ident);
		string_append_str(str, "</html>");
	}

	return str;
}

void generate_file(parser_t* parser, char* output_file)
{
	string_t* code = generate_string(parser, 0);

	FILE* file = fopen(output_file, "w");
	if (file == NULL) {
		fprintf(stderr, "Could not open file %s for writing\n", output_file);
		string_free(code);
		return;
	}

	fprintf(file, "%s", code->data);

	fclose(file);
	string_free(code);
}

void generate_print(parser_t* parser)
{
	string_t* code = generate_string(parser, 0);

	printf("Generate Code:\n");

	string_print(code);

	string_free(code);
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

	ast_print(parser);

	generate_print(parser);
	generate_file(parser, "output.html");

	print_message("free parser\n");
	parser_free(parser);
	print_message("end free parser\n");

	print_message("free lexer\n");
	lexer_free(lexer);
	print_message("end lexer free\n");

	return 0;
}
