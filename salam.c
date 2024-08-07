//
// THE SALAM PROGRAMMING LANGUAGE
//

#define _SALAM_LANGUAGE_

#include <stdio.h>
#include <wchar.h>
#include <ctype.h>
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

char* read_dynamic_string()
{
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

			if (new_input == NULL) {
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
	
	if (sign == -1) {
		result[0] = '-';
	}

	for (int i = numDigits - 1 + (sign == -1 ? 1 : 0); i >= (sign == -1 ? 1 : 0); i--) {
		result[i] = '0' + abs(value % 10);
		value /= 10;
	}

	result[numDigits + (sign == -1 ? 1 : 0)] = '\0';

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
	CREATE_MEMORY_OBJECT(arr->data, void*, arr->size, "Error: array_create<arr-data> - Memory allocation error in %s:%d\n",  __FILE__, __LINE__);

	if (arr->data == NULL) {
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

array_t* array_copy(array_t* arr)
{
	if (arr == NULL || arr->length == 0) return NULL;

	array_t* copy = array_create(arr->length);
	if (copy == NULL) return NULL;

	for (size_t i = 0; i < arr->length; i++) {
		array_push(copy, strdup(arr->data[i]));
	}

	return copy;

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

void array_token_free(array_t* arr)
{
	if (arr == NULL) return;

	if (arr->data != NULL) {
		for (size_t i = 0; i < arr->length; i++) {
			token_t* token = arr->data[i];

			if (token != NULL) {
				if (token->value != NULL) {
					free(token->value);
					token->value = NULL;
				}
				
				free(arr->data[i]);
				arr->data[i] = NULL;
			}
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
	print_message("%s - ", token_type2str(t->type));
	print_message("%s\n", t->value);
}

char* array_string(array_t* array, char* seperator)
{
	if (array == NULL || array->length == 0) return NULL;

	string_t* buffer = string_create(10);

	for (size_t i = 0; i < array->length; i++) {
		char* t = (char*) array->data[i];
		if (t != NULL) {
			string_append_str(buffer, t);

			if (seperator != NULL && i < array->length - 1) string_append_str(buffer, seperator);
		}
	}

	char* buf = strdup(buffer->data);
	string_free(buffer);

	return buf;
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
	CREATE_MEMORY_OBJECT(map, hashmap_t, 1, "Error: hashmap_create<map> - Memory allocation error in %s:%d\n",  __FILE__, __LINE__);

	map->size = 16;
	map->length = 0;
	map->data = (hashmap_entry_t**) calloc(map->size, sizeof(hashmap_entry_t*));

	if (map->data == NULL) {
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

	hashmap_entry_t *new_entry;
	CREATE_MEMORY_OBJECT(new_entry, hashmap_entry_t, 1, "Error: hashmap_put<new_entry> - Memory allocation error in %s:%d\n",  __FILE__, __LINE__);
	
	new_entry->key = strdup(key);
	new_entry->value = value;
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
		if (strcmp(entry->key, key) == 0) return entry->value;

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
			if (prev == NULL) map->data[index] = entry->next;
			else prev->next = entry->next;

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

void hashmap_string_free(hashmap_t *map)
{
	if (map == NULL) return;

	if (map->data != NULL) {
		for (size_t i = 0; i < map->size; i++) {
			hashmap_entry_t *entry = map->data[i];

			while (entry) {
				hashmap_entry_t *next = entry->next;

				free(entry->key);
				entry->key = NULL;

				free(entry->value);
				entry->value = NULL;
				
				free(entry);

				entry = next;
			}
		}

		free(map->data);
		map->data = NULL;
	}

	free(map);
	map = NULL;
}

void hashmap_array_free(hashmap_t *map)
{
	if (map == NULL) return;

	if (map->data != NULL) {
		for (size_t i = 0; i < map->size; i++) {
			hashmap_entry_t *entry = map->data[i];

			while (entry) {
				hashmap_entry_t *next = entry->next;
				free(entry->key);
				entry->key = NULL;
				array_t* array = entry->value;

				array_free(array);
				entry->value = NULL;
				
				free(entry);
				entry = next;
			}
		}

		free(map->data);
		map->data = NULL;
	}

	free(map);
	map = NULL;
}

void hashmap_free(hashmap_t *map)
{
	if (map == NULL) return;

	if (map->data != NULL) {
		for (size_t i = 0; i < map->size; i++) {
			hashmap_entry_t *entry = map->data[i];

			while (entry) {
				hashmap_entry_t *next = entry->next;

				if (entry->key != NULL) {
					free(entry->key);
					entry->key = NULL;
				}

				if (entry->value != NULL) {
					free(entry->value);
					entry->value = NULL;
				}

				free(entry);
				entry = next;
			}
		}

		free(map->data);
		map->data = NULL;
	}

	free(map);
	map = NULL;
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
	}
	else {
		lexer->data = (char*) data;
		lexer->length = strlen(data);
	}

	return lexer;
}

void lexer_free(lexer_t* lexer)
{
	if (lexer == NULL) return;

	array_token_free(lexer->tokens);

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
	}
	else {
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
		}
		else if (lexer->data[lexer->index - 1] == '*' && lexer->data[lexer->index] == '/') {
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
			}
		else if (ch == L't') {
				string[i++] = '\t';
			}
		else if (ch == L'"') {
				string[i++] = '"';
			}
		else if (ch == L'\'') {
				string[i++] = '\'';
			}
		else if (ch == L'\\') {
				string[i++] = '\\';
			}
			else {
				print_error("%s", messages[language][MESSAGE_LEXER_STRING_UNKNOWN_ESCAPE]);
				free(string);

				exit(EXIT_FAILURE);
			}
		}
		else {
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

	if (wcrtomb(mb_char, wide_char, NULL) == (size_t) -1) {
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
		}
		else if (current_char == '\n') {
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
		}
		else if (current_wchar == '[') {
			token_t* t = token_create(TOKEN_TYPE_BRACKETS_OPEN, "[", 1, lexer->line, lexer->column - 1, lexer->line, lexer->column);
			array_push(lexer->tokens, t);
		}
		else if (current_wchar == ']') {
			token_t* t = token_create(TOKEN_TYPE_BRACKETS_CLOSE, "]", 1, lexer->line, lexer->column - 1, lexer->line, lexer->column);
			array_push(lexer->tokens, t);
		}
		else if (current_wchar == '%' || current_wchar == L'٪') {
			token_t* t = token_create(TOKEN_TYPE_MODULE, "%", 1, lexer->line, lexer->column - 1, lexer->line, lexer->column);
			array_push(lexer->tokens, t);
		}
		else if (current_wchar == '{') {
			token_t* t = token_create(TOKEN_TYPE_SECTION_OPEN, "{", 1, lexer->line, lexer->column - 1, lexer->line, lexer->column);
			array_push(lexer->tokens, t);
		}
		else if (current_wchar == '}') {
			token_t* t = token_create(TOKEN_TYPE_SECTION_CLOSE, "}", 1, lexer->line, lexer->column - 1, lexer->line, lexer->column);
			array_push(lexer->tokens, t);
		}
		else if (current_wchar == '(') {
			token_t* t = token_create(TOKEN_TYPE_PARENTHESE_OPEN, "(", 1, lexer->line, lexer->column - 1, lexer->line, lexer->column);
			array_push(lexer->tokens, t);
		}
		else if (current_wchar == ')') {
			token_t* t = token_create(TOKEN_TYPE_PARENTHESE_CLOSE, ")", 1, lexer->line, lexer->column - 1, lexer->line, lexer->column);
			array_push(lexer->tokens, t);
		}
		else if (current_wchar == '+') {
			token_t* t = token_create(TOKEN_TYPE_PLUS, "+", 1, lexer->line, lexer->column - 1, lexer->line, lexer->column);
			array_push(lexer->tokens, t);
		}
		else if (current_wchar == ':') {
			token_t* t = token_create(TOKEN_TYPE_COLON, ":", 1, lexer->line, lexer->column - 1, lexer->line, lexer->column);
			array_push(lexer->tokens, t);
		}
		else if (current_wchar == '.') {
			token_t* t = token_create(TOKEN_TYPE_DOT, ".", 1, lexer->line, lexer->column - 1, lexer->line, lexer->column);
			array_push(lexer->tokens, t);
		}
		else if (current_wchar == '#') {
			token_t* t = token_create(TOKEN_TYPE_SHARP, "#", 1, lexer->line, lexer->column - 1, lexer->line, lexer->column);
			array_push(lexer->tokens, t);
		}
		else if (current_wchar == '*') {
			token_t* t = token_create(TOKEN_TYPE_MULTIPLY, "*", 1, lexer->line, lexer->column - 1, lexer->line, lexer->column);
			array_push(lexer->tokens, t);
		}
		else if (current_wchar == '/') {
			if (lexer->index < lexer->length && lexer->data[lexer->index] == L'/') {
				lexer->index++;
				lexer->column++;
				read_comment_singleline(lexer);
			}
		else if (lexer->index < lexer->length && lexer->data[lexer->index] == L'*') {
				lexer->index++;
				lexer->column++;
				read_comment_multiline(lexer);
			}
			else {
				token_t* t = token_create(TOKEN_TYPE_DIVIDE, "/", 1, lexer->line, lexer->column - 1, lexer->line, lexer->column);
				array_push(lexer->tokens, t);
			}
		}
		else if (current_wchar == ',') {
			token_t* t = token_create(TOKEN_TYPE_COMMA, ",", 1, lexer->line, lexer->column - 1, lexer->line, lexer->column);
			array_push(lexer->tokens, t);
		}
		else if (current_wchar == '-') {
			token_t* t = token_create(TOKEN_TYPE_MINUS, "-", 1, lexer->line, lexer->column - 1, lexer->line, lexer->column);
			array_push(lexer->tokens, t);
		}
		else if (current_wchar == L'=') {
			if (lexer->index < lexer->length && lexer->data[lexer->index] == L'=') {
				token_t* t = token_create(TOKEN_TYPE_EQUAL_EQUAL, "==", 2, lexer->line, lexer->column - 2, lexer->line, lexer->column);
				array_push(lexer->tokens, t);
				lexer->index++;
				lexer->column++;
			}
			else {
				token_t* t = token_create(TOKEN_TYPE_EQUAL, "=", 1, lexer->line, lexer->column - 1, lexer->line, lexer->column);
				array_push(lexer->tokens, t);
			}
		}
		else if (current_char == '!') {
			if (lexer->index < lexer->length && lexer->data[lexer->index] == '=') {
				token_t* t = token_create(TOKEN_TYPE_NOT_EQUAL, "!=", 2, lexer->line, lexer->column - 2, lexer->line, lexer->column);
				array_push(lexer->tokens, t);
				lexer->index++;
				lexer->column++;
			}
			else {
				token_t* t = token_create(TOKEN_TYPE_NOT, "!", 1, lexer->line, lexer->column - 1, lexer->line, lexer->column);
				array_push(lexer->tokens, t);
			}
		}
		else if (current_char == '>') {
			if (lexer->index < lexer->length && lexer->data[lexer->index] == '=') {
				token_t* t = token_create(TOKEN_TYPE_GREATER_THAN_EQUAL, ">=", 2, lexer->line, lexer->column - 2, lexer->line, lexer->column);
				array_push(lexer->tokens, t);
				lexer->index++;
				lexer->column++;
			}
			else {
				token_t* t = token_create(TOKEN_TYPE_GREATER_THAN, ">", 1, lexer->line, lexer->column - 1, lexer->line, lexer->column);
				array_push(lexer->tokens, t);
			}
		}
		else if (current_char == '<') {
			if (lexer->index < lexer->length && lexer->data[lexer->index] == '=') {
				token_t* t = token_create(TOKEN_TYPE_LESS_THAN_EQUAL, "<=", 2, lexer->line, lexer->column - 2, lexer->line, lexer->column);
				array_push(lexer->tokens, t);
				lexer->index++;
				lexer->column++;
			}
			else {
				token_t* t = token_create(TOKEN_TYPE_LESS_THAN, "<", 1, lexer->line, lexer->column - 1, lexer->line, lexer->column);
				array_push(lexer->tokens, t);
			}
		}
		else if (current_wchar == '\"') {
			current_wchar = read_token(lexer);
			read_string(lexer, current_wchar);
		}
		else if (is_number(current_wchar)) {
			read_number(lexer, current_wchar);
		}
		else if (is_alpha(current_wchar)) {
			read_identifier(lexer, current_wchar);
		}
		else {
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

ast_layout_node_t* parser_layout_element_single(ast_layout_type_t layout_type, parser_t* parser)
{
	ast_layout_node_t* element;
	CREATE_MEMORY_OBJECT(element, ast_layout_node_t, 1, "Error: parser_layout_element_single<element> - Memory allocation error in %s:%d\n",  __FILE__, __LINE__);

	element->type = layout_type;
	element->children = array_create(1);
	element->attributes = hashmap_create();
	element->styles = NULL; // hashmap_create();
	element->hoverStyles = NULL; // hashmap_create();
	element->is_mother = false;

	parser->token_index++; // Eating keyword

	return element;
}

array_t* parser_layout_attribute_array_value(parser_t* parser)
{
	array_t* values = array_create(1);

	while (parser->token_index < parser->lexer->tokens->length) {
		token_t* attr_value = parser_token_get(parser);

		if (attr_value->type == TOKEN_TYPE_END || attr_value->type == TOKEN_TYPE_EOF) break;

		array_push(values, strdup(attr_value->value));
		parser->token_index++;

		if (parser->token_index < parser->lexer->tokens->length) {
			attr_value = parser_token_get(parser);

			if (attr_value->type == TOKEN_TYPE_COMMA) {
				parser->token_index++;
			}
			else {
				break;
			}
		}
		else {
			break;
		}
	}

	return values;
}

void parser_layout_element_attribute(parser_t* parser, ast_layout_type_t layout_type, ast_layout_node_t* element, bool acceptAttrs, bool acceptStyles, bool acceptHoverStyles, bool acceptChildren)
{
	token_t* current_token = parser_token_get(parser);

	ast_layout_type_t type = convert_layout_identifier_token_type(current_token->value);

	if (type == AST_TYPE_LAYOUT_ERROR && (acceptAttrs == true || acceptStyles == true)) {
		parser->token_index++; // Eating the attr name
		parser_token_eat_nodata(parser, TOKEN_TYPE_COLON); // :
		
		array_t* values = parser_layout_attribute_array_value(parser);

		if (acceptStyles == true && is_style_attribute(current_token->value)) hashmap_put(element->styles, current_token->value, values);
		else if (acceptAttrs == true) hashmap_put(element->attributes, current_token->value, values);
	}
	else if (type == AST_TYPE_LAYOUT_HOVER && acceptHoverStyles == true) {
		parser->token_index++; // Eating keyword
		parser_token_eat_nodata(parser, TOKEN_TYPE_COLON);

		parser_layout_element_attributes(parser, layout_type, element, false, true, false, false);

		parser_token_eat_nodata(parser, TOKEN_TYPE_END);
	}
	else if (acceptChildren == true) {
		token_print(current_token);

		ast_layout_node_t* new_child = parser_layout_element(parser);

		array_push(element->children, new_child);
	}
	else {
		printf("Error!");

		exit(1);
	}
}

void parser_layout_element_attributes(parser_t* parser, ast_layout_type_t layout_type, ast_layout_node_t* element, bool acceptAttrs, bool acceptStyles, bool acceptHoverStyles, bool acceptChildren)
{
	while (parser->token_index < parser->lexer->tokens->length) {
		token_t* current_token = parser_token_get(parser);

		if (current_token->type == TOKEN_TYPE_END) break;

		parser_layout_element_attribute(parser, layout_type, element, acceptAttrs, acceptStyles, acceptHoverStyles, acceptChildren);
	}
}

ast_layout_node_t* parser_layout_element_mother(ast_layout_type_t layout_type, parser_t* parser)
{
	printf("parser_layout_element_mother\n");
	ast_layout_node_t* element;
	CREATE_MEMORY_OBJECT(element, ast_layout_node_t, 1, "Error: parser_layout_element_mother<element> - Memory allocation error in %s:%d\n",  __FILE__, __LINE__);

	element->type = layout_type;
	element->children = array_create(1);
	element->attributes = hashmap_create();
	element->styles = hashmap_create();
	element->hoverStyles = hashmap_create();
	element->is_mother = true;

	parser->token_index++; // Eating keyword

	parser_token_eat_nodata(parser, TOKEN_TYPE_COLON);

	parser_layout_element_attributes(parser, layout_type, element, true, true, true, true);

	parser_token_eat_nodata(parser, TOKEN_TYPE_END);

	return element;
}

ast_layout_node_t* parser_layout_element(parser_t* parser)
{
	token_t* current_token = parser_token_get(parser);

	if (current_token->type != TOKEN_TYPE_IDENTIFIER) return NULL;
	
	ast_layout_type_t type = convert_layout_identifier_token_type(current_token->value);

	switch (type) {
		case AST_TYPE_LAYOUT_HOVER:
		case AST_TYPE_LAYOUT_TEXT:
		case AST_TYPE_LAYOUT_INPUT:
		case AST_TYPE_LAYOUT_BUTTON:
		case AST_TYPE_LAYOUT_CENTER:
		case AST_TYPE_LAYOUT_LINK:
		case AST_TYPE_LAYOUT_IMAGE:
		case AST_TYPE_LAYOUT_FORM:
		case AST_TYPE_LAYOUT_DIV:
		case AST_TYPE_LAYOUT_PARAGTAPH:
		case AST_TYPE_LAYOUT_SELECT:
		case AST_TYPE_LAYOUT_SELECT_OPTION:
		case AST_TYPE_LAYOUT_TABLE:
		case AST_TYPE_LAYOUT_TABLE_ROW:
		case AST_TYPE_LAYOUT_TABLE_COLUMN:
		case AST_TYPE_LAYOUT_BOLD:
		case AST_TYPE_LAYOUT_TEXTAREA:
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

// array_t* parser_layout_elements(parser_t* parser)
// {
// 	array_t* elements = array_create(4);

// 	while (parser->token_index < parser->lexer->tokens->length) {
// 		token_t* current_token = parser_token_get(parser);

// 		if (current_token->type == TOKEN_TYPE_IDENTIFIER) {
// 			ast_layout_node_t* element = parser_layout_element(parser);

// 			if (element == NULL) {
// 				token_free(current_token);
// 				array_free(elements);

// 				print_message("Error: invalid element inside layout block!");
				
// 				exit(EXIT_FAILURE);
// 			}

// 			array_push(elements, element);
// 		}
// 		else {
// 			break;
// 		}
// 	}

// 	return elements;
// }


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
		}
		else {
			token_free(token);
			print_message("Error: Expected nodata %s\n", token_type2str(type));

			exit(EXIT_FAILURE);
		}
	}
	else {
		print_message("Error: Unexpected end of file\n");

		exit(EXIT_FAILURE);
	}
}

token_t* parser_token_get(parser_t* parser)
{
	if (parser->lexer->tokens->length > parser->token_index) {
		token_t* token = parser->lexer->tokens->data[parser->token_index];

		return token;
	}

	return NULL;
}

token_t* parser_token_eat(parser_t* parser, token_type_t type)
{
	if (parser->lexer->tokens->length > parser->token_index) {
		token_t* token = parser_token_get(parser);

		if (token->type == type) {
			token_t* token = parser->lexer->tokens->data[parser->token_index];

			return token;
		}
		else {
			token_free(token);
			print_message("Error: Expected data %s\n", token_type2str(type));

			exit(EXIT_FAILURE);
		}
	}
	else {
		print_message("Error: Unexpected end of file\n");

		exit(EXIT_FAILURE);
	}

	return NULL;
}

void parser_parse(parser_t* parser)
{
	if (parser->lexer->tokens->length == 0) return;

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
			// 
			// 	break;

			case TOKEN_TYPE_LAYOUT:
				parser->layout = parser_layout_element_mother(AST_TYPE_LAYOUT, parser);

				break;
			
			case TOKEN_TYPE_IDENTIFIER:

				break;

			default:
				// if (parser_expression_has(parser)) {
				// 	CREATE_MEMORY_OBJECT(expression_node, ast_node_t, 1, "Error: parser_parse<expression_node> - Memory allocation error in %s:%d\n",  __FILE__, __LINE__);
				// 
				// 	expression_node->type = AST_EXPRESSION;
				// 	expression_node->data.expression = parser_expression(parser);
				// 	if (parser->expressions && expression_node != NULL) {
				// 		array_push(parser->expressions, expression_node);
				// 	}
				// }
				// else {
					print_error(messages[language][MESSAGE_PARSER_BAD_TOKEN_AS_STATEMENT], token_type2str(current_token->type));

					exit(EXIT_FAILURE);
				// }

				break;
		}
	}
}

void attributes_free(hashmap_t* attributes)
{
	if (attributes != NULL) {
		if (attributes->data != NULL) {
			for (size_t i = 0; i < attributes->size; i++) {
				hashmap_entry_t *entry = attributes->data[i];

				while (entry) {
					hashmap_entry_t *next = entry->next;

					if (entry->key != NULL) {
						free(entry->key);
						entry->key = NULL;
					}

					if (entry->value != NULL) {
						array_free(entry->value);
						// free(entry->value);
						entry->value = NULL;
					}

					free(entry);
					entry = next;
				}
			}

			free(attributes->data);
			attributes->data = NULL;
		}

		free(attributes);
		attributes = NULL;
	}
}

void children_free(array_t* children)
{
	if (children != NULL) {
		if (children->data != NULL) {
			for (size_t i = 0; i < children->length; i++) {
				if (children->data[i] != NULL) {
					ast_layout_node_free((ast_layout_node_t*) children->data[i]);
				}
			}

			free(children->data);
			children->data = NULL;
		}

		free(children);
		children = NULL;
	}
}

void ast_layout_node_free(ast_layout_node_t* node)
{
	if (node == NULL) return;

	attributes_free(node->attributes);
	attributes_free(node->styles);
	attributes_free(node->hoverStyles);

	children_free(node->children);

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
		case AST_TYPE_LAYOUT_LINK: return "a";
		case AST_TYPE_LAYOUT_CENTER: return "center";
		case AST_TYPE_LAYOUT_IMAGE: return "img";
		case AST_TYPE_LAYOUT_TABLE: return "table";
		case AST_TYPE_LAYOUT_TABLE_ROW: return "tr";
		case AST_TYPE_LAYOUT_TABLE_COLUMN: return "td";
		case AST_TYPE_LAYOUT_BOLD: return "b";
		case AST_TYPE_LAYOUT_DIV: return "div";
		case AST_TYPE_LAYOUT_PARAGTAPH: return "p";
		case AST_TYPE_LAYOUT_TEXTAREA: return "textarea";
		case AST_TYPE_LAYOUT_SELECT: return "select";
		case AST_TYPE_LAYOUT_SELECT_OPTION: return "option";
		case AST_TYPE_LAYOUT_FORM: return "form";

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
		case AST_TYPE_LAYOUT_LINK: return "a";
		case AST_TYPE_LAYOUT_FORM: return "form";
		case AST_TYPE_LAYOUT_BOLD: return "b";
		case AST_TYPE_LAYOUT_CENTER: return "center";
		case AST_TYPE_LAYOUT_IMAGE: return "img";
		case AST_TYPE_LAYOUT_TABLE: return "table";
		case AST_TYPE_LAYOUT_TABLE_ROW: return "tr";
		case AST_TYPE_LAYOUT_TABLE_COLUMN: return "td";
		case AST_TYPE_LAYOUT_DIV: return "div";
		case AST_TYPE_LAYOUT_PARAGTAPH: return "p";
		case AST_TYPE_LAYOUT_TEXTAREA: return "textarea";
		case AST_TYPE_LAYOUT_SELECT: return "select";
		case AST_TYPE_LAYOUT_SELECT_OPTION: return "option";

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
				attributes_free(node->data.layout->attributes);
				attributes_free(node->data.layout->styles);
				attributes_free(node->data.layout->hoverStyles);

				children_free(node->data.layout->children);

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
	if (parser == NULL) return;

	if (parser->layout != NULL) {
		attributes_free(parser->layout->attributes);
		attributes_free(parser->layout->styles);
		attributes_free(parser->layout->hoverStyles);

		children_free(parser->layout->children);

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
	// if (suffix == NULL) {
	// 	printf("==>%s\n", str->data);
	// }
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

				array_t* arr = entry->value;
				char* str_value = array_string(arr, ", ");
				string_append_str(str, str_value == NULL ? "NULL" : str_value);

				if (str_value != NULL) free(str_value);

				string_append_char(str, '\"');

				entry = entry->next;
			}
		}
	}

	if (element->is_mother) {
		string_append_char(str, '>');

		bool hasChild = false;

		if (element->children != NULL) {
			if (element->children->data != NULL) {
				if (element->children->length > 0) {
					hasChild = true;

					string_append_char(str, '\n');

					for (size_t j = 0; j < element->children->length; j++) {
						ast_layout_node_t* child = (ast_layout_node_t*) element->children->data[j];

						string_t* buf = ast_layout_string(child, parser, ident + 1);
						string_append(str, buf);
						string_free(buf);
					}
				}
			}
		}

		if (hasChild) {
			ast_layout_ident(str, ident);
		}

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

		for (size_t i = 0; i < parser->layout->children->length; i++) {
			ast_layout_node_t* element = (ast_layout_node_t*) parser->layout->children->data[i];
			
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

bool is_allowed_general_layout_property(char* attribute_name, char** new_attribute_name)
{
	// *new_attribute_name = NULL;

	const char* attributes[] = {
		"شناسه",
		"کلاس",
		"الویت",
	};
	const char* html_attributes[] = {
		"id",
		"class",
		"z-index",
	};
	int num_attributes = sizeof(attributes) / sizeof(attributes[0]);

	for (int i = 0; i < num_attributes; i++) {
		if (strcmp(attribute_name, attributes[i]) == 0)
		{ strcpy(*new_attribute_name, html_attributes[i]); return true; }
	}

	return false;
}

bool is_allowed_mother_layout_property(ast_layout_type_t type, char* attribute_name, char** new_attribute_name)
{
	// *new_attribute_name = NULL;
	if (is_allowed_general_layout_property(attribute_name, new_attribute_name)) return true;

	switch (type) {
		case AST_TYPE_LAYOUT_BUTTON:
			if (strcmp(attribute_name, "نام") == 0) { strcpy(*new_attribute_name, "name"); return true; }
		break;

		case AST_TYPE_LAYOUT_TEXT:
		break;

		case AST_TYPE_LAYOUT_INPUT:
			if (strcmp(attribute_name, "نام") == 0) { strcpy(*new_attribute_name, "name"); return true; }
			else if (strcmp(attribute_name, "نوع") == 0) { strcpy(*new_attribute_name, "type"); return true; }
		break;

		case AST_TYPE_LAYOUT_LINK:
			if (strcmp(attribute_name, "منبع") == 0) { strcpy(*new_attribute_name, "href"); return true; }
			else if (strcmp(attribute_name, "نوع") == 0) { strcpy(*new_attribute_name, "target"); return true; }
		break;

		case AST_TYPE_LAYOUT_IMAGE:
			if (strcmp(attribute_name, "منبع") == 0) { strcpy(*new_attribute_name, "src"); return true; }
			else if (strcmp(attribute_name, "عرض") == 0) { strcpy(*new_attribute_name, "width"); return true; }
			else if (strcmp(attribute_name, "ارتفاع") == 0) { strcpy(*new_attribute_name, "height"); return true; }
		break;

		case AST_TYPE_LAYOUT_CENTER:
		break;

		case AST_TYPE_LAYOUT_FORM:
			if (strcmp(attribute_name, "نام") == 0) { strcpy(*new_attribute_name, "name"); return true; }
			else if (strcmp(attribute_name, "نوع") == 0) { strcpy(*new_attribute_name, "method"); return true; }
			else if (strcmp(attribute_name, "منبع") == 0) { strcpy(*new_attribute_name, "action"); return true; }
		break;

		case AST_TYPE_LAYOUT_TABLE:
		break;

		case AST_TYPE_LAYOUT_TABLE_ROW:
		break;

		case AST_TYPE_LAYOUT_TABLE_COLUMN:
		break;

		case AST_TYPE_LAYOUT_BOLD:
		break;

		case AST_TYPE_LAYOUT_DIV:
		break;

		case AST_TYPE_LAYOUT_PARAGTAPH:
		break;

		case AST_TYPE_LAYOUT_TEXTAREA:
			if (strcmp(attribute_name, "نام") == 0) { strcpy(*new_attribute_name, "name"); return true; }
		break;

		case AST_TYPE_LAYOUT_SELECT:
			if (strcmp(attribute_name, "نام") == 0) { strcpy(*new_attribute_name, "name"); return true; }
		break;

		case AST_TYPE_LAYOUT_SELECT_OPTION:
			if (strcmp(attribute_name, "نام") == 0) { strcpy(*new_attribute_name, "name"); return true; } // THIS WILL MOVE TO INNER HTML
			else if (strcmp(attribute_name, "مقدار") == 0) { strcpy(*new_attribute_name, "value"); return true; }
		break;
	}

	return false;
}

bool is_allowed_single_layout_property(ast_layout_type_t type, char* attribute_name, char** new_attribute_name)
{
	// *new_attribute_name = NULL;
	// if (is_allowed_general_layout_property(attribute_name, new_attribute_name)) return true;

	switch (type) {
		case AST_TYPE_LAYOUT_LINE:
		break;

		case AST_TYPE_LAYOUT_BREAK:
		break;
	}

	return false;
}

bool is_allowed_layout_property(bool is_mother, ast_layout_type_t type, char* attribute_name, char** new_attribute_name)
{
	if (is_mother) return is_allowed_mother_layout_property(type, attribute_name, new_attribute_name);
	else return is_allowed_single_layout_property(type, attribute_name, new_attribute_name);
}

bool is_style_attribute(char* attribute_name)
{
	if (strcmp(attribute_name, "رنگ") == 0) return true;
	else if (strcmp(attribute_name, "زمینه") == 0) return true;
	else if (strcmp(attribute_name, "فونت") == 0) return true;
	else if (strcmp(attribute_name, "اندازه") == 0) return true;
	else if (strcmp(attribute_name, "فاصله") == 0) return true;
	else if (strcmp(attribute_name, "فضا") == 0) return true;
	else if (strcmp(attribute_name, "طول") == 0) return true;
	else if (strcmp(attribute_name, "ارتفاع") == 0) return true;
	else if (strcmp(attribute_name, "حاشیه") == 0) return true;
	else if (strcmp(attribute_name, "تصویر") == 0) return true;
	else if (strcmp(attribute_name, "ماوس") == 0) return true;
	else if (strcmp(attribute_name, "گردی") == 0) return true;

	return false;
}

char* trim_whitespace(char* str)
{
	char* end;

	while (isspace((unsigned char) *str)) str++;

	if (*str == 0) return str;

	end = str + strlen(str) - 1;
	while (end > str && isspace((unsigned char) *end)) end--;

	end[1] = '\0';

	return str;
}

array_t* split_by_space(const char* str)
{
	array_t* arr = array_create(10);
	char* temp_str = strdup(str);
	char* token = strtok(temp_str, " ");

	while (token != NULL) {
		array_push(arr, strdup(token));
		token = strtok(NULL, " ");
	}

	free(temp_str);
	
	return arr;
}

char* trim_value(char* value)
{
	char* result;
	CREATE_MEMORY_OBJECT(result, char, 256, "Error: trim_value<result> - Memory allocation error in %s:%d\n",  __FILE__, __LINE__);

	int i = 0;
	while (value[i] == ' ' || value[i] == '\t' || value[i] == '\n') i++;

	int j = strlen(value) - 1;
	while (value[j] == ' ' || value[j] == '\t' || value[j] == '\n') j--;

	int k = 0;
	for (int l = i; l <= j; l++) {
		result[k] = value[l];
		k++;
	}

	result[k] = '\0';

	return result;
}

char* replace_substring(const char* str, const char* old_substr, const char* new_substr)
{
	const char *pos = strstr(str, old_substr);
	if (pos == NULL) {
		size_t result_len = strlen(str);

		char *result;
		CREATE_MEMORY_OBJECT(result, char, result_len + 1, "Error: replace_substring<result1> - Memory allocation error in %s:%d\n",  __FILE__, __LINE__);

		strcpy(result, str);

		return result;
	}

	size_t old_len = strlen(old_substr);
	size_t new_len = strlen(new_substr);
	size_t result_len = strlen(str) - old_len + new_len;

	char *result;
	CREATE_MEMORY_OBJECT(result, char, result_len + 1, "Error: replace_substring<result2> - Memory allocation error in %s:%d\n",  __FILE__, __LINE__);

	size_t pos_index = pos - str;
	strncpy(result, str, pos_index);
	result[pos_index] = '\0';

	strcat(result, new_substr);

	strcat(result, pos + old_len);

	return result;
}

char* replace_all_substrings(const char* str, const char* old_substr, const char* new_substr)
{
	if (str == NULL || old_substr == NULL || new_substr == NULL) return NULL;

	size_t str_len = strlen(str);
	size_t old_len = strlen(old_substr);
	size_t new_len = strlen(new_substr);

	if (old_len == 0) {
		char *result;
		CREATE_MEMORY_OBJECT(result, char, str_len + 1, "Error: replace_all_substrings<result1> - Memory allocation error in %s:%d\n",  __FILE__, __LINE__);

		strcpy(result, str);
		return result;
	}

	size_t max_result_len = str_len;
	const char *tmp = str;
	while ((tmp = strstr(tmp, old_substr)) != NULL) {
		max_result_len += (new_len - old_len);
		tmp += old_len;
	}

	char *result;
	CREATE_MEMORY_OBJECT(result, char, max_result_len + 1, "Error: replace_all_substrings<result2> - Memory allocation error in %s:%d\n",  __FILE__, __LINE__);

	char *result_ptr = result;
	const char *search_start = str;
	const char *pos;

	while ((pos = strstr(search_start, old_substr)) != NULL) {
		size_t bytes_to_copy = pos - search_start;

		strncpy(result_ptr, search_start, bytes_to_copy);
		result_ptr += bytes_to_copy;

		strcpy(result_ptr, new_substr);
		result_ptr += new_len;

		search_start = pos + old_len;
	}

	strcpy(result_ptr, search_start);

	return result;
}

bool has_css_size_prefix(char* css_value, char** css_output_value)
{
	const char* prefixes[] = {"px", "em", "rem", "vw", "vh", "%", "cm", "mm", "in", "pt", "pc", "ex", "ch", "vmin", "vmax"};
	int num_prefixes = sizeof(prefixes) / sizeof(prefixes[0]);
	
	const char* persian_prefixes[] = {"پیکسل", "ای ام", "رایم", "ویو ویدث", "ویو هایت", "درصد", "سانتیمتر", "میلیمتر", "اینچ", "پوینت", "پیکا", "اکس", "سی اچ", "وی مین", "وی مکس"};
	int num_persian_prefixes = sizeof(persian_prefixes) / sizeof(persian_prefixes[0]);

	size_t len = strlen(css_value);
	if (len == 0) {
		*css_output_value = NULL;
		return false;
	}

	string_t* buffer = string_create(len);

	size_t i = 0;
	if (css_value[i] == '-' || css_value[i] == '+') {
		if (css_value[i] == '-') string_append_char(buffer, css_value[i]);

		i++;
	}

	if (!isdigit(css_value[i])) {
		*css_output_value = NULL;
		return false;
	}

	bool decimal_point_found = false;
	while (i < len && (isdigit(css_value[i]) || (css_value[i] == '.' && !decimal_point_found))) {
		if (css_value[i] == '.') {
			string_append_char(buffer, css_value[i]);
			decimal_point_found = true;
		}
		else {
			string_append_char(buffer, css_value[i]);
		}

		i++;
	}

	while (i < len && isspace(css_value[i])) i++;

	for (int j = 0; j < num_prefixes; j++) {
		size_t prefix_len = strlen(prefixes[j]);
		if (len - i == prefix_len && strncmp(css_value + i, prefixes[j], prefix_len) == 0) {
			string_append_str(buffer, prefixes[j]);
			*css_output_value = strdup(buffer->data);

			string_free(buffer);

			return true;
		}
	}

	for (int j = 0; j < num_persian_prefixes; j++) {
		size_t prefix_len = strlen(persian_prefixes[j]);
		if (len - i == prefix_len && strncmp(css_value + i, persian_prefixes[j], prefix_len) == 0) {
			string_append_str(buffer, prefixes[j]);
			*css_output_value = strdup(buffer->data);

			string_free(buffer);
			
			return true;
		}
	}
	
	*css_output_value = NULL;

	return false;	
}

char* normalise_css_size(char* attribute_value)
{
	if (!string_is_number(attribute_value)) return attribute_value;

	int value_length = strlen(attribute_value) + 3;

	char* res = realloc(attribute_value, value_length * sizeof(char));
	CHECK_MEMORY_ALLOCATION(res, "Error: normalise_css_size<res> - Memory allocation error in %s:%d\n",  __FILE__, __LINE__);

	strcat(res, "px");

	return res;
}

char* attribute_css_multiple_size_value(char* attribute_name, array_t* attribute_values)
{
	if (attribute_values == NULL || attribute_values->length == 0) return NULL;

	if (attribute_values->length == 1) {
		char* value = strdup(attribute_values->data[0]);
		value = normalise_css_size(value);

		return value;
	}

	string_t* buffer = string_create(10);

	for (size_t i = 0; i < attribute_values->length; i++) {
		char* value = strdup(attribute_values->data[i]);
		value = normalise_css_size(value);
		
		char* out_value;
		if (!has_css_size_prefix(value, &out_value)) return NULL;

		if (value != NULL) free(value);

		if (out_value != NULL) {
			string_append_str(buffer, out_value);

			if (out_value != NULL) free(out_value);

			if (i != attribute_values->length - 1) string_append_char(buffer, ' ');
		}
	}

	char* buf = strdup(buffer->data);
	string_free(buffer);

	return buf;
}

char* attribute_css_size_value(char* attribute_name, char* attribute_value)
{
	char* res;
	int attribute_value_length = strlen(attribute_value) + 3;
	CREATE_MEMORY_OBJECT(res, char, attribute_value_length, "Error: attribute_css_size_value<res> - Memory allocation error in %s:%d\n",  __FILE__, __LINE__);

	strcpy(res, attribute_value);

	if (string_is_number(attribute_value)) strcat(res, "px");

	return res;
}

bool is_english_digit(wchar_t ch)
{
	// 0123456789
	return ch >= L'0' && ch <= L'9';
}

bool is_persian_digit(wchar_t ch)
{
	// ۰۱۲۳۴۵۶۷۸۹
	return ch >= 0x06F0 && ch <= 0x06F9;
	// return ch >= '۰' && ch <= '۹';
}

bool is_arabic_digit(wchar_t ch)
{
	// ٠١٢٣٤٥٦٧٨٩
	return ch >= 0x0660 && ch <= 0x0669;
}

bool string_is_number(const char* value)
{
	size_t len = mbstowcs(NULL, value, 0);
	if (len == (size_t)-1) return false;

	wchar_t* wvalue;
	CREATE_MEMORY_OBJECT(wvalue, wchar_t, (len + 1), "Error: string_is_number<wvalue> - Memory allocation error in %s:%d\n",  __FILE__, __LINE__);

	mbstowcs(wvalue, value, len + 1);

	if (wvalue[0] == L'\0') {
		free(wvalue);

		return false;
	}

	size_t start = 0;
	if (wvalue[0] == L'+' || wvalue[0] == L'-') start = 1;

	if (start == 1 && wvalue[1] == L'\0') {
		free(wvalue);

		return false;
	}

	for (size_t i = start; wvalue[i] != L'\0'; i++) {
		if (!(is_english_digit(wvalue[i]) || is_persian_digit(wvalue[i]) || is_arabic_digit(wvalue[i]))) {
			free(wvalue);

			return false;
		}
	}

	free(wvalue);

	return true;
}

char* attribute_css_values(char* attribute_name, array_t* attribute_values)
{
	if (attribute_name == NULL) return NULL;
	else if (attribute_values == NULL || attribute_values->length == 0) return NULL;

	char* attribute_value = (char*) attribute_values->data[0];

	if (strcmp(attribute_name, "cursor") == 0) {
		if (attribute_values->length == 1 && strcmp(attribute_value, "دست") == 0) return "pointer";
	}
	else if (strcmp(attribute_name, "color") == 0 || strcmp(attribute_name, "background-color") == 0) {
		if (attribute_values->length == 1 && strcmp(attribute_value, "سیاه") == 0) return "black";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "سفید") == 0) return "white";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "صورتی") == 0) return "pink";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "بنفش") == 0) return "purple";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "قرمز") == 0) return "red";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "سبز") == 0) return "green";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "زرد") == 0) return "yellow";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "ابی") == 0 || strcmp(attribute_value, "آبی") == 0) return "blue";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "قهوه‌ای") == 0 || strcmp(attribute_value, "قهوه ای") == 0) return "brown";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "نارنجی") == 0) return "orange";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "خاکستری") == 0) return "gray";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "طوسی") == 0) return "silver";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "طلایی") == 0) return "gold";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "بژ") == 0) return "beige";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "زیتونی") == 0) return "olive";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "لاجوردی") == 0) return "navy";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "فیروزه‌ای") == 0 || strcmp(attribute_value, "فیروزه ای") == 0) return "turquoise";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "نیلی") == 0) return "indigo";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "خرمایی") == 0) return "crimson";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "قرمز تیره") == 0) return "darkred";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "صورتی تیره") == 0) return "deeppink";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "آتشین") == 0) return "firebrick";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "صورتی داغ") == 0) return "hotpink";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "قرمز هندی") == 0) return "indianred";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "اسطوخودوسی") == 0) return "lavenderblush";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "مرجانی روشن") == 0) return "lightcoral";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "صورتی روشن") == 0) return "lightpink";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "زرشکی") == 0) return "maroon";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "رز مه آلود") == 0) return "mistyrose";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "قرمز بنفش کم رنگ") == 0) return "palevioletred";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "رنگ پوست") == 0) return "bisque";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "مرجانی") == 0) return "coral";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "نارنجی تیره") == 0) return "darkorange";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "سالمون تیره") == 0) return "darksalmon";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "سالمون روشن") == 0) return "lightsalmon";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "نارنجی قرمز") == 0) return "orangered";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "خامه پاپایا") == 0) return "papayawhip";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "هلو") == 0) return "peachpuff";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "سالمون") == 0) return "salmon";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "گوجه فرنگی") == 0) return "tomato";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "خمیر ذرت") == 0) return "cornsilk";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "طلایی تیره") == 0) return "darkgoldenrod";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "خاکی تیره") == 0) return "darkkhaki";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "طلایی روشن") == 0) return "lightgoldenrodyellow";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "زرد روشن") == 0) return "lightyellow";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "خاکی") == 0) return "khaki";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "پرتقالی کم رنگ") == 0) return "palegoldenrod";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "زرد کم رنگ") == 0) return "palegoldenrod";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "سبز دریایی") == 0) return "aquamarine";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "چارتوز") == 0) return "chartreuse";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "سبز تیره") == 0) return "darkgreen";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "زیتونی تیره") == 0) return "darkolivegreen";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "سبز دریایی تیره") == 0) return "darkseagreen";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "خاکستری تیره") == 0) return "darkslategray";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "خاکستری تیره") == 0) return "darkslategrey";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "سبز جنگلی") == 0) return "forestgreen";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "زرد سبز") == 0) return "greenyellow";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "شهد") == 0) return "honeydew";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "سبز چمنی") == 0) return "lawngreen";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "سبز روشن") == 0) return "lightgreen";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "سبز دریایی روشن") == 0) return "lightseagreen";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "لیمو سبز") == 0) return "lime";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "لیمو سبز روشن") == 0) return "limegreen";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "آب دریایی میانه") == 0) return "mediumaquamarine";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "سبز دریایی میانه") == 0) return "mediumseagreen";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "سبز بهاری میانه") == 0) return "mediumspringgreen";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "سبز زیتونی تیره") == 0) return "olivedrab";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "سبز کم رنگ") == 0) return "palegreen";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "سبز دریایی") == 0) return "seagreen";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "سبز بهاری") == 0) return "springgreen";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "فیروزه ای") == 0) return "teal";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "سبز زرد") == 0) return "yellowgreen";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "آبی آسمانی روشن") == 0) return "aliceblue";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "آبی") == 0) return "aqua";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "آبی روشن") == 0) return "azure";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "آبی") == 0) return "blue";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "آبی کاپیتان") == 0) return "cadetblue";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "آبی گل گندم") == 0) return "cornflowerblue";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "آبی آسمانی تیره") == 0) return "darkcyan";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "فیروزه ای تیره") == 0) return "darkturquoise";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "آبی آسمانی عمیق") == 0) return "deepskyblue";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "آبی آسمانی داج") == 0) return "dodgerblue";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "آبی روشن") == 0) return "lightblue";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "فیروزه ای روشن") == 0) return "lightcyan";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "آبی آسمانی روشن") == 0) return "lightskyblue";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "آبی فولادی روشن") == 0) return "lightsteelblue";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "آبی میانه") == 0) return "mediumblue";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "فیروزه ای میانه") == 0) return "mediumturquoise";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "آبی نیمه شب") == 0) return "midnightblue";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "نیلی") == 0) return "navy";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "آبی روشن") == 0) return "paleturquoise";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "آبی پودری") == 0) return "powderblue";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "آبی سلطنتی") == 0) return "royalblue";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "آبی آسمانی") == 0) return "skyblue";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "آبی فولادی") == 0) return "steelblue";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "آبی بنفش") == 0) return "blueviolet";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "ارغوانی تیره") == 0) return "darkmagenta";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "بنفش ارکیده تیره") == 0) return "darkorchid";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "آبی ارغوانی تیره") == 0) return "darkslateblue";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "بنفش تیره") == 0) return "darkviolet";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "سرخابی") == 0) return "fuchsia";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "نیلی") == 0) return "indigo";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "اسطوخودوس") == 0) return "lavender";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "ارکیده میانه") == 0) return "mediumorchid";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "ارغوانی میانه") == 0) return "mediumpurple";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "آبی ارغوانی میانه") == 0) return "mediumslateblue";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "قرمز بنفش میانه") == 0) return "mediumvioletred";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "ارکیده") == 0) return "orchid";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "آلو") == 0) return "plum";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "بنفش") == 0) return "purple";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "بنفش ربکا") == 0) return "rebeccapurple";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "آبی تخته سنگ") == 0) return "slateblue";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "خار مریم") == 0) return "thistle";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "بنفش") == 0) return "violet";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "سفید قدیمی") == 0) return "antiquewhite";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "بادام سفید شده") == 0) return "blanchedalmond";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "چوب بری") == 0) return "burlywood";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "شکلاتی") == 0) return "chocolate";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "کتان") == 0) return "linen";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "موکاسین") == 0) return "moccasin";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "سفید ناواجو") == 0) return "navajowhite";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "توری قدیمی") == 0) return "oldlace";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "پرو") == 0) return "peru";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "قهوه ای گل رز") == 0) return "rosybrown";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "قهوه ای زین اسب") == 0) return "saddlebrown";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "قهوه ای شنی") == 0) return "sandybrown";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "سیه نا") == 0) return "sienna";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "حنا") == 0) return "tan";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "گندمی") == 0) return "wheat";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "خاکستری تیره") == 0) return "darkgray";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "خاکستری کم رنگ") == 0) return "darkgrey";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "خاکستری کم رنگ") == 0) return "dimgray";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "خاکستری کم رنگ") == 0) return "dimgrey";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "گل سفید") == 0) return "floralwhite";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "خاکستری روشن") == 0) return "gainsboro";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "خاکستری سفید") == 0) return "ghostwhite";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "خاکستری") == 0) return "gray";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "خاکستری") == 0) return "grey";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "خاکستری کم رنگ") == 0) return "lightgray";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "خاکستری کم رنگ") == 0) return "lightgrey";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "خاکستری تخته سنگی روشن") == 0) return "lightslategray";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "خاکستری تخته سنگی روشن") == 0) return "lightslategrey";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "نعنایی") == 0) return "mintcream";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "صدف") == 0) return "seashell";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "خاکستری نقره ای") == 0) return "silver";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "خاکستری تخته سنگی") == 0) return "slategray";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "خاکستری تخته سنگی") == 0) return "slategrey";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "برفی") == 0) return "snow";
		else if (attribute_values->length == 1 && strcmp(attribute_value, "سفید دودی") == 0) return "whitesmoke";
	}
	else if (strcmp(attribute_name, "font-family") == 0) {
		return attribute_value;
	}
	else if (strcmp(attribute_name, "font-size") == 0) {
		if (attribute_values->length == 1) {
			char* size_value = attribute_css_size_value(attribute_name, attribute_value);

			if (size_value) {
				return size_value;
			}
		}
	}
	else if (strcmp(attribute_name, "padding-top") == 0 || strcmp(attribute_name, "padding-left") == 0 || strcmp(attribute_name, "padding-right") == 0 || strcmp(attribute_name, "padding-bottom") == 0) {
		if (attribute_values->length == 1) {
			char* size_value = attribute_css_size_value(attribute_name, attribute_value);

			if (size_value) {
				return size_value;
			}
		}
	}
	else if (strcmp(attribute_name, "margin-top") == 0 || strcmp(attribute_name, "margin-left") == 0 || strcmp(attribute_name, "margin-right") == 0 || strcmp(attribute_name, "margin-bottom") == 0) {
		if (attribute_values->length == 1) {
			char* size_value = attribute_css_size_value(attribute_name, attribute_value);

			if (size_value) {
				return size_value;
			}
		}
	}
	else if (strcmp(attribute_name, "width") == 0) {
		if (attribute_values->length == 1) {
			char* size_value = attribute_css_size_value(attribute_name, attribute_value);

			if (size_value) {
				return size_value;
			}
		}
	}
	else if (strcmp(attribute_name, "height") == 0) {
		if (attribute_values->length == 1) {
			char* size_value = attribute_css_size_value(attribute_name, attribute_value);

			if (size_value) {
				return size_value;
			}
		}
	}
	else if (strcmp(attribute_name, "border-radius") == 0 || strcmp(attribute_name, "padding") == 0 || strcmp(attribute_name, "margin") == 0) {
		char* size_value = attribute_css_multiple_size_value(attribute_name, attribute_values);

		if (size_value) {
			return size_value;
		}
	}
	else if (strcmp(attribute_name, "border-size") == 0) {
		if (attribute_values->length == 1) {
			char* size_value = attribute_css_size_value(attribute_name, attribute_value);

			if (size_value) {
				return size_value;
			}
		}
	}
	else if (strcmp(attribute_name, "border") == 0) {
		if (attribute_values->length == 1) {
			char* size_value = attribute_css_size_value(attribute_name, attribute_value);

			if (size_value) {
				return size_value;
			}
		}
	}
	else if (strcmp(attribute_name, "background-image") == 0) {
		if (attribute_values->length == 1) {
			return attribute_value;
		}
	}
	else {
		printf("This is not a attribute valid value, error? %s => %s\n", attribute_name, attribute_value);
	}

	return NULL;
}

char* attribute_css_name(const char* attribute_name)
{
	if (strcmp(attribute_name, "رنگ") == 0) return "color";
	else if (strcmp(attribute_name, "زمینه") == 0) return "background-color";
	else if (strcmp(attribute_name, "فونت") == 0) return "font-family";
	else if (strcmp(attribute_name, "اندازه") == 0) return "font-size";
	else if (strcmp(attribute_name, "فاصله") == 0) return "padding";
	else if (strcmp(attribute_name, "طول") == 0) return "width";
	else if (strcmp(attribute_name, "ارتفاع") == 0) return "height";
	else if (strcmp(attribute_name, "فضا") == 0) return "margin";
	else if (strcmp(attribute_name, "حاشیه") == 0) return  "border";
	else if (strcmp(attribute_name, "تصویر") == 0) return "background-image";
	else if (strcmp(attribute_name, "ماوس") == 0) return "cursor";
	else if (strcmp(attribute_name, "گردی") == 0) return "border-radius";
	else {
		return NULL;
	}
}

string_t* generate_layout_element_attribute(parser_t* parser, char* key, array_t* values)
{
	string_t* buf = string_create(10);

	char* str_values = array_string(values, " ");

	if (str_values != NULL) {
		string_append_str(buf, key);
		string_append_char(buf, '=');

		if (strlen(str_values) == 1) {
			string_append_str(buf, str_values);
		}
		else {
			string_append_char(buf, '\"');
			string_append_str(buf, str_values);
			string_append_char(buf, '\"');
		}

		free(str_values);
	}

	return buf;
}

string_t* generate_layout_element_attributes(parser_t* parser, ast_layout_node_t* element, array_t** element_content, int ident)
{
	string_t* str = string_create(10);

	int css_attrs = 0;
	int css_hover_attrs = 0;
	int html_attrs = 0;

	if (element->attributes != NULL && element->attributes->length > 0) {
		printf("has attrs\n");
		for (size_t i = 0; i < element->attributes->size; i++) {
			hashmap_entry_t *entry = element->attributes->data[i];
			printf("for iterate\n");

			while (entry) {
				printf("-->%s\n", entry->key);

				if (strcmp(entry->key, "محتوا") == 0) {
					printf("is content\n");
					*element_content = array_copy(entry->value);
				}
				else {
					char* newKey;
					CREATE_MEMORY_OBJECT(newKey, char, 100, "Error: generate_layout_element_attributes<newKey> - Memory allocation error in %s:%d\n",  __FILE__, __LINE__);

					if ((is_allowed_layout_property(element->is_mother, element->type, entry->key, &newKey) == true) && newKey != NULL) {
						if (html_attrs != 0) string_append_char(str, ' ');
						html_attrs++;

						string_t* buf = generate_layout_element_attribute(parser, newKey, entry->value);
						string_append(str, buf);

						string_free(buf);
					}
					
					free(newKey);
				}				
				
				entry = entry->next;
			}
		}
	}

	if (element->styles != NULL && element->styles->length > 0) {
		string_t* css_buffer = string_create(10);

		for (size_t i = 0; i < element->styles->size; i++) {
			hashmap_entry_t *entry = element->styles->data[i];

			while (entry) {
				css_attrs++;

				char* buf1 = attribute_css_name(entry->key);

				if (buf1 != NULL) {
					array_t* values = entry->value;

					if (values != NULL && values->length > 0) {
						char* buf2 = attribute_css_values(buf1, values);

						if (buf2 != NULL) {
							string_append_str(css_buffer, buf1);

							string_append_char(css_buffer, ':');

							string_append_str(css_buffer, buf2);

							if (element->styles->length != css_attrs) string_append_char(css_buffer, ';');

							free(buf2);
							buf2 = NULL;
						}
					}

					free(buf1);
					buf1 = NULL;
				}

				entry = entry->next;
			}
		}

		if (css_buffer != NULL && css_buffer->length > 0) {
			if (html_attrs > 0) string_append_char(str, ' ');

			string_append_str(str, "style=\"");
			string_append(str, css_buffer);
			string_append_str(str, "\"");
		}

		if (css_buffer != NULL) string_free(css_buffer);
	}

	if (element->hoverStyles != NULL && element->hoverStyles->length > 0) {
		string_t* css_hover_buffer = string_create(10);

		for (size_t i = 0; i < element->hoverStyles->size; i++) {
			hashmap_entry_t *entry = element->hoverStyles->data[i];

			while (entry) {
				css_hover_attrs++;

				char* buf1 = attribute_css_name(entry->key);

				if (buf1 != NULL) {
					array_t* values = entry->value;

					if (values != NULL && values->length > 0) {
						char* buf2 = attribute_css_values(buf1, values);

						if (buf2 != NULL) {
							string_append_str(css_hover_buffer, buf1);

							string_append_char(css_hover_buffer, ':');

							string_append_str(css_hover_buffer, buf2);

							if (element->hoverStyles->length != css_hover_attrs) string_append_char(css_hover_buffer, ';');

							free(buf2);
							buf2 = NULL;
						}
					}

					free(buf1);
					buf1 = NULL;
				}

				entry = entry->next;
			}
		}

		if (css_hover_buffer != NULL && css_hover_buffer->length > 0) {
			if (html_attrs > 0 || css_attrs > 0) string_append_char(str, ' ');

			string_append_str(str, "hover_style=\"");
			string_append(str, css_hover_buffer);
			string_append_str(str, "\"");
		}

		if (css_hover_buffer != NULL) string_free(css_hover_buffer);
	}
	
	return str;
}

string_t* generate_layout_element(ast_layout_node_t* element, parser_t* parser, int ident)
{
	string_t* str = string_create(10);

	if (parser == NULL) return NULL;
	else if (element == NULL) return NULL;

	char* element_name = generate_layout_type_string(element->type);
	array_t* element_content = NULL;

	generate_layout_ident(str, ident);

	string_append_char(str, '<');
	string_append_str(str, element_name);

	string_t* buf = generate_layout_element_attributes(parser, element, &element_content, ident);
	if (buf != NULL && buf->length > 0) {
		string_append_char(str, ' ');
		
		string_append(str, buf);
	}
	string_free(buf);

	string_append_char(str, '>');
	bool needBreak = false;

	if (element->is_mother) {
		if (element->children != NULL) {
			if (element->children->length > 0) {
				needBreak = true;
				string_append_char(str, '\n');

				for (size_t i = 0; i < element->children->length; i++) {
					ast_layout_node_t *entry = element->children->data[i];

					string_t* buf = generate_layout_element(entry, parser, ident + 1);
					if (buf != NULL && buf->length > 0) {
						string_append(str, buf);
						string_free(buf);
					}
				}
			}
		}

		if (element_content != NULL && element_content->length > 0) {
			if (needBreak) generate_layout_ident(str, ident + 1);

			char* value = array_string(element_content, "\n");

			string_append_str(str, value == NULL ? "NULL" : value);
			if (value != NULL) free(value);
			
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

	if (element_content != NULL) array_free(element_content);

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

		generate_layout_ident(str, ident + 2);
		string_append_str(str, "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n");

		generate_layout_ident(str, ident + 2);
		string_append_str(str, "<meta http-equiv=\"X-UA-Compatible\" content=\"ie=edge\">\n");

		if (parser->layout->attributes != NULL) {
			if (parser->layout->attributes->data != NULL) {
				if (parser->layout->attributes->length > 0) {
					array_t* title_values = hashmap_get(parser->layout->attributes, "عنوان");
					if (title_values != NULL && title_values->length > 0) {
						char* titles = array_string(title_values, ", ");

						if (titles != NULL) {
							generate_layout_ident(str, ident + 2);

							string_append_str(str, "<title>");
							string_append_str(str, titles);
							string_append_str(str, "</title>\n");

							free(titles);
						}
					}
				}
			}
		}

		generate_layout_ident(str, ident + 2);
		string_append_str(str, "<style type=\"text/css\">\n");
		generate_layout_ident(str, ident + 3);
		string_append_str(str, "html,body,div,span,applet,object,iframe,h1,h2,h3,h4,h5,h6,p,blockquote,pre,a,abbr,acronym,address,big,cite,code,del,dfn,em,img,ins,kbd,q,s,samp,small,strike,strong,sub,sup,tt,var,b,u,i,center,dl,dt,dd,ol,ul,li,fieldset,form,label,legend,table,caption,tbody,tfoot,thead,tr,th,td,article,aside,canvas,details,embed,figure,figcaption,footer,header,hgroup,menu,nav,output,ruby,section,summary,time,mark,audio,video{margin:0;padding:0;border:0;font-size:100%;font:inherit;vertical-align:baseline}article,aside,details,figcaption,figure,footer,header,hgroup,menu,nav,section{display:block}body{line-height:1}ol,ul{list-style:none}blockquote,q{quotes:none}blockquote:before,blockquote:after,q:before,q:after{content:'';content:none}table{border-collapse:collapse;border-spacing:0}\n");
		generate_layout_ident(str, ident + 2);
		string_append_str(str, "</style>\n");

		generate_layout_ident(str, ident + 1);
		string_append_str(str, "</head>\n");

		generate_layout_ident(str, ident + 1);
		string_append_str(str, "<body>\n");

		for (size_t i = 0; i < parser->layout->children->length; i++) {
			ast_layout_node_t* element = (ast_layout_node_t*) parser->layout->children->data[i];

			string_t* buf = generate_layout_element(element, parser, ident + 2);
			if (buf) {
				string_append(str, buf);
				string_free(buf);
			}
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
	FILE* file = fopen(output_file, "w");
	if (file == NULL) {
		fprintf(stderr, "Could not open file %s for writing\n", output_file);
		
		return;
	}

	string_t* code = generate_string(parser, 0);

	fprintf(file, "%s", code->data);

	printf("File saved:\n%s\n", code->data);

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

	// char* test1 = "1px";
	// char* out1;
	// printf("%s\n", has_css_size_prefix(test1, &out1) == true ? "true" : "false");
	// printf("%s\n----\n", out1);


	// char* test2 = "1234 px";
	// char* out2;
	// printf("%s\n", has_css_size_prefix(test2, &out2) == true ? "true" : "false");
	// printf("%s\n----\n", out2);


	// char* test3 = "3.14 px";
	// char* out3;
	// printf("%s\n", has_css_size_prefix(test3, &out3) == true ? "true" : "false");
	// printf("%s\n----\n", out3);

	// char* test4 = "-3.14پیکسل";
	// char* out4;
	// printf("%s\n", has_css_size_prefix(test4, &out4) == true ? "true" : "false");
	// printf("%s\n----\n", out4);

	// char* test5 = "-3.14";
	// char* out5;
	// printf("%s\n", has_css_size_prefix(test5, &out5) == true ? "true" : "false");
	// printf("%s\n----\n", out5);

	// char* test6 = "px";
	// char* out6;
	// printf("%s\n", has_css_size_prefix(test6, &out6) == true ? "true" : "false");
	// printf("%s\n----\n", out6);

	// return 0;

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
	}
	else {
		if (strcmp(argv[1], "--ast") == 0) {
			// isAst = true;
		}
		else if (strcmp(argv[1], "--code") == 0) {
			// isAst = false;
		}
		else {
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
