#include "lexer.h"

#define LEXER_CURRENT (lexer->source[lexer->index])
#define LEXER_CURRENT_PREV (lexer->source[lexer->index - 1])
#define LEXER_CURRENT_NEXT (lexer->source[lexer->index + 1])

#define LEXER_NEXT lexer->index++
#define LEXER_PREV lexer->index--

#define LEXER_NEXT_LINE lexer->line++
#define LEXER_NEXT_COLUMN lexer->column++

#define LEXER_ZERO_COLUMN lexer->column = 0

#define LEXER_CURRENT_UTF8 (utf8_decode(lexer->source, &lexer->index))
#define LEXER_CURRENT_PREV_UTF8 (utf8_decode(lexer->source, &lexer->index - 1))
#define LEXER_CURRENT_NEXT_UTF8 (utf8_decode(lexer->source, &lexer->index + 1))

#define LEXER_NEXT_UTF8 lexer->index += utf8_char_length(lexer->source[lexer->index])
#define LEXER_PREV_UTF8 lexer->index -= utf8_char_length(lexer->source[lexer->index])

#define LEXER_PUSH_TOKEN(TOKEN) array_push(lexer->tokens, TOKEN)

/**
 *
 * @variable token_names
 * @brief Token names
 * @type {token_name_t[]}
 *
 */
token_name_t token_names[] = {
	#undef ADD_TOKEN
	#undef ADD_CHAR_TOKEN
	#undef ADD_KEYWORD
	#undef ADD_KEYWORD_REPEAT
	#undef ADD_KEYWORD_HIDE

	#define ADD_TOKEN(TOKEN_TYPE, TOKEN_NAME, TOKEN_VALUE) {TOKEN_TYPE, TOKEN_NAME, TOKEN_VALUE, -1},
	#define ADD_CHAR_TOKEN(TOKEN_TYPE, TOKEN_NAME, TOKEN_VALUE, TOKEN_CHAR) {TOKEN_TYPE, TOKEN_NAME, TOKEN_VALUE, TOKEN_CHAR},
	#define ADD_KEYWORD(TOKEN_TYPE, TOKEN_NAME, TOKEN_VALUE, TOKEN_VALUE_LENGTH) {TOKEN_TYPE, TOKEN_NAME, TOKEN_VALUE, -1},
	#define ADD_KEYWORD_REPEAT(TOKEN_TYPE, TOKEN_NAME, TOKEN_VALUE, TOKEN_VALUE_LENGTH)
	#define ADD_KEYWORD_HIDE(TOKEN_TYPE, TOKEN_NAME, TOKEN_VALUE, TOKEN_VALUE_LENGTH) {TOKEN_TYPE, TOKEN_NAME, TOKEN_VALUE, -1},

	#include "token.h"
};

/**
 *
 * @variable keywords
 * @brief Keywords
 * @type {keyword_t[]}
 *
 */
const keyword_t keywords[] = {
	#undef ADD_TOKEN
	#undef ADD_CHAR_TOKEN
	#undef ADD_KEYWORD
	#undef ADD_KEYWORD_REPEAT
	#undef ADD_KEYWORD_HIDE

	#define ADD_TOKEN(TOKEN_TYPE, TOKEN_NAME, TOKEN_VALUE)
	#define ADD_CHAR_TOKEN(TOKEN_TYPE, TOKEN_NAME, TOKEN_VALUE, TOKEN_CHAR)
	#define ADD_KEYWORD(TOKEN_TYPE, TOKEN_NAME, TOKEN_VALUE, TOKEN_VALUE_LENGTH) {.length=TOKEN_VALUE_LENGTH, .type=TOKEN_TYPE, .name=TOKEN_NAME, .keyword=TOKEN_VALUE},
	#define ADD_KEYWORD_REPEAT(TOKEN_TYPE, TOKEN_NAME, TOKEN_VALUE, TOKEN_VALUE_LENGTH) {.length=TOKEN_VALUE_LENGTH, .type=TOKEN_TYPE, .name=TOKEN_NAME, .keyword=TOKEN_VALUE},
	#define ADD_KEYWORD_HIDE(TOKEN_TYPE, TOKEN_NAME, TOKEN_VALUE, TOKEN_VALUE_LENGTH)

	#include "token.h"
};

wchar_t read_token(lexer_t* lexer, int* wcl)
{
	if (lexer->source[lexer->index] == '\0') {
		return L'\0';
	}

	wchar_t current_char;
	int char_size = mbtowc(&current_char, &lexer->source[lexer->index], MB_CUR_MAX);

	if (char_size < 0) {
		printf("%s\n", "MESSAGE_LEXER_TOKEN_READ_UNICODE");

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
	*wcl = char_size;

	return current_char;
}

/**
 *
 * @function is_english_digit
 * @brief Check if the character is an English digit
 * @params {wchar_t} ch - Character
 * @returns {bool} - True if the character is an English digit, false otherwise
 *
 */
bool is_english_digit(wchar_t ch)
{
	DEBUG_ME;
	// 0123456789
	return ch >= L'0' && ch <= L'9';
}

/**
 *
 * @function is_persian_digit
 * @brief Check if the character is a Persian digit
 * @params {wchar_t} ch - Character
 * @returns {bool} - True if the character is a Persian digit, false otherwise
 *
 */
bool is_persian_digit(wchar_t ch)
{
	DEBUG_ME;
	// ۰۱۲۳۴۵۶۷۸۹
	return ch >= 0x06F0 && ch <= 0x06F9;
	// return ch >= '۰' && ch <= '۹';
}

/**
 *
 * @function is_arabic_digit
 * @brief Check if the character is an Arabic digit
 * @params {wchar_t} ch - Character
 * @returns {bool} - True if the character is an Arabic digit, false otherwise
 *
 */
bool is_arabic_digit(wchar_t ch)
{
	DEBUG_ME;
	// ٠١٢٣٤٥٦٧٨٩
	return ch >= 0x0660 && ch <= 0x0669;
}

/**
 *
 * @function string_is_percentage
 * @brief Check if the string is a percentage
 * @params {const char*} value - Value
 * @params {bool} acceptSign - Accept sign
 * @returns {bool} - True if the string is a percentage
 *
 */
bool string_is_percentage(const char* value, bool acceptSign)
{
	DEBUG_ME;
	size_t len = mbstowcs(NULL, value, 0);
	if (len == (size_t) -1) {
		return false;
	}

	wchar_t* wvalue = memory_allocate(sizeof(wchar_t) * (len + 1));
	mbstowcs(wvalue, value, len + 1);

	if (wvalue[0] == L'\0') {
		memory_destroy(wvalue);
		return false;
	}

	size_t start = 0;
	if (acceptSign == true) {
		if (wvalue[0] == L'+' || wvalue[0] == L'-') {
			start = 1;
		}
	}

	if (start == 1 && wvalue[1] == L'\0') {
		memory_destroy(wvalue);
		return false;
	}

	size_t len_wvalue = wcslen(wvalue);

	if (wvalue[len_wvalue - 1] != L'%') {
		memory_destroy(wvalue);
		return false;
	}

	for (size_t i = start; i < len_wvalue - 1; i++) {
		if (!(is_english_digit(wvalue[i]) || is_persian_digit(wvalue[i]) || is_arabic_digit(wvalue[i]))) {
			memory_destroy(wvalue);
			return false;
		}
	}

	memory_destroy(wvalue);
	return true;
}

/**
 *
 * @function string_is_integer
 * @brief Check if the string is a integer
 * @params {const char*} value - Value
 * @returns {bool} - True if the string is a integer, false otherwise
 *
 */
bool string_is_integer(const char* value)
{
	DEBUG_ME;
	size_t len = mbstowcs(NULL, value, 0);
	if (len == (size_t) -1) {
		return false;
	}

	wchar_t* wvalue = memory_allocate(sizeof(wchar_t) * (len + 1));
	mbstowcs(wvalue, value, len + 1);

	if (wvalue[0] == L'\0') {
		memory_destroy(wvalue);

		return false;
	}

	size_t start = 0;
	if (wvalue[0] == L'+' || wvalue[0] == L'-') {
		start = 1;
	}

	if (start == 1 && wvalue[1] == L'\0') {
		memory_destroy(wvalue);

		return false;
	}

	size_t i;

	for (i = start; wvalue[i] != L'\0'; i++) {
		if (!(is_english_digit(wvalue[i]) || is_persian_digit(wvalue[i]) || is_arabic_digit(wvalue[i]))) {
			memory_destroy(wvalue);

			return false;
		}
	}

	memory_destroy(wvalue);

	if (i == start) {
		return false;
	}

	return true;
}

/**
 *
 * @function string_is_float
 * @brief Check if the string is a float
 * @params {const char*} value - Value
 * @returns {bool} - True if the string is a float, false otherwise
 *
 */
bool string_is_float(const char* value)
{
	DEBUG_ME;
	size_t len = mbstowcs(NULL, value, 0);
	if (len == (size_t) -1) {
		return false;
	}

	wchar_t* wvalue = memory_allocate(sizeof(wchar_t) * (len + 1));
	mbstowcs(wvalue, value, len + 1);

	if (wvalue[0] == L'\0') {
		memory_destroy(wvalue);

		return false;
	}

	size_t start = 0;
	if (wvalue[0] == L'+' || wvalue[0] == L'-') {
		start = 1;
	}

	if (start == 1 && wvalue[1] == L'\0') {
		memory_destroy(wvalue);

		return false;
	}

	bool hasDot = false;
	size_t i;

	for (i = start; wvalue[i] != L'\0'; i++) {
		if (wvalue[i] == L'.') {
			if (hasDot) {
				memory_destroy(wvalue);

				return false;
			}

			hasDot = true;
		}
		else if (!(is_english_digit(wvalue[i]) || is_persian_digit(wvalue[i]) || is_arabic_digit(wvalue[i]))) {
			memory_destroy(wvalue);

			return false;
		}
	}

	memory_destroy(wvalue);

	if (hasDot == false) {
		return false;
	}
	else if (i == start) {
		return false;
	}

	return true;
}

/**
 *
 * @function string_is_number
 * @brief Check if the string is a number
 * @params {const char*} value - Value
 * @returns {bool} - True if the string is a number, false otherwise
 *
 */
bool string_is_number(const char* value)
{
	DEBUG_ME;
	size_t len = mbstowcs(NULL, value, 0);
	if (len == (size_t) -1) {
		return false;
	}

	wchar_t* wvalue = memory_allocate(sizeof(wchar_t) * (len + 1));
	mbstowcs(wvalue, value, len + 1);

	if (wvalue[0] == L'\0') {
		memory_destroy(wvalue);

		return false;
	}

	size_t start = 0;
	if (wvalue[0] == L'+' || wvalue[0] == L'-') {
		start = 1;
	}

	if (start == 1 && wvalue[1] == L'\0') {
		memory_destroy(wvalue);

		return false;
	}

	size_t i;

	for (i = start; wvalue[i] != L'\0'; i++) {
		if (!(is_english_digit(wvalue[i]) || is_persian_digit(wvalue[i]) || is_arabic_digit(wvalue[i]))) {
			memory_destroy(wvalue);

			return false;
		}
	}

	memory_destroy(wvalue);

	if (i == start) {
		return false;
	}

	return true;
}

/**
 *
 * @function token_create
 * @brief Creating a new token
 * @params {token_type_t} type - Token type
 * @params {location_t} location - Token location
 * @returns {token_t*}
 *
 */
token_t* token_create(token_type_t type, location_t location)
{
	DEBUG_ME;
	token_t* token = memory_allocate(sizeof(token_t));
	token->type = type;
	token->location = location;
	token->data_type = TOKEN_ERROR;

	token->name = token_name;
	token->value_stringify = token_value_stringify;

	token->print = token_print;
	token->stringify = token_stringify;
	token->destroy = token_destroy;

	return token;
}

/**
 *
 * @function token_copy
 * @brief Copying a token
 * @params {token_t*} token - Token
 * @returns {token_t*}
 *
 */
token_t* token_copy(token_t* token)
{
	DEBUG_ME;
	token_t* copy = token_create(token->type, token->location);
	copy->data_type = token->data_type;

	switch (token->data_type) {
		case TOKEN_NUMBER_INT:
			copy->data.number_int = token->data.number_int;
			break;

		case TOKEN_NUMBER_FLOAT:
			copy->data.number_float = token->data.number_float;
			break;

		case TOKEN_STRING:
		case TOKEN_IDENTIFIER:
			if (token->data.string != NULL) {
				copy->data.string = strdup(token->data.string);
			}
			break;

		case TOKEN_BOOLEAN:
			copy->data.boolean = token->data.boolean;
			break;

		default:
			break;
	}

	return copy;
}

/**
 *
 * @function token_destroy
 * @brief Destroying a token
 * @params {token_t*} token - Token
 * @returns {void}
 *
 */
void token_destroy(token_t* token)
{
	DEBUG_ME;
	if (token != NULL) {
		if (token->data_type == TOKEN_STRING || token->data_type == TOKEN_IDENTIFIER) {
			if (token->data.string != NULL) {
				memory_destroy(token->data.string);
			}
		}

		memory_destroy(token);
	}
}

/**
 *
 * @function token_type_stringify
 * @brief Get the name of a token type
 * @params {token_type_t} type - Token type
 * @returns {char*}
 *
 */
const char* token_type_stringify(token_type_t type)
{
	DEBUG_ME;
	switch (type) {
		#undef ADD_TOKEN
		#undef ADD_CHAR_TOKEN
		#undef ADD_KEYWORD
		#undef ADD_KEYWORD_REPEAT
		#undef ADD_KEYWORD_HIDE

		#define ADD_TOKEN(TOKEN_TYPE, TOKEN_NAME, TOKEN_VALUE) case TOKEN_TYPE: return TOKEN_NAME;
		#define ADD_CHAR_TOKEN(TOKEN_TYPE, TOKEN_NAME, TOKEN_VALUE, TOKEN_CHAR) case TOKEN_TYPE: return TOKEN_NAME;
		#define ADD_KEYWORD(TOKEN_TYPE, TOKEN_NAME, TOKEN_VALUE, TOKEN_VALUE_LENGTH) case TOKEN_TYPE: return TOKEN_NAME;
		#define ADD_KEYWORD_REPEAT(TOKEN_TYPE, TOKEN_NAME, TOKEN_VALUE, TOKEN_VALUE_LENGTH)
		#define ADD_KEYWORD_HIDE(TOKEN_TYPE, TOKEN_NAME, TOKEN_VALUE, TOKEN_VALUE_LENGTH)

		#include "token.h"
	}

	return TOKEN_NAME_UNKNOWN;
}

/**
 *
 * @function token_char_type
 * @brief Get the type of a character
 * @params {char} c - Character
 * @returns {token_type_t}
 *
 */
token_type_t token_char_type(char c)
{
	DEBUG_ME;
	switch (c) {
		#undef ADD_TOKEN
		#undef ADD_CHAR_TOKEN
		#undef ADD_KEYWORD
		#undef ADD_KEYWORD_REPEAT
		#undef ADD_KEYWORD_HIDE

		#define ADD_TOKEN(TOKEN_TYPE, TOKEN_NAME, TOKEN_VALUE)
		#define ADD_CHAR_TOKEN(TOKEN_TYPE, TOKEN_NAME, TOKEN_VALUE, TOKEN_CHAR) case TOKEN_CHAR: return TOKEN_TYPE;
		#define ADD_KEYWORD(TOKEN_TYPE, TOKEN_NAME, TOKEN_VALUE, TOKEN_VALUE_LENGTH)
		#define ADD_KEYWORD_REPEAT(TOKEN_TYPE, TOKEN_NAME, TOKEN_VALUE, TOKEN_VALUE_LENGTH)
		#define ADD_KEYWORD_HIDE(TOKEN_TYPE, TOKEN_NAME, TOKEN_VALUE, TOKEN_VALUE_LENGTH)

		#include "token.h"
	}

	return TOKEN_ERROR;
}

/**
 *
 * @function token_stringify
 * @brief Get the name & value & location of a token as a string
 * @params {token_t*} token - Token
 * @returns {char*} - String representation of the token
 *
 */
char* token_stringify(token_t* token)
{
	DEBUG_ME;
	static char buffer[1024];
	const char* type = token_type_stringify(token->type);
	const char* value = token_value_stringify(token);
	const char* location = location_stringify(token->location);

	snprintf(buffer, sizeof(buffer), "%s: %s at %s", type, value, location);

	return buffer;
}

/**
 *
 * @function token_print
 * @brief Print a token
 * @params {token_t*} token - Token
 * @returns {void}
 *
 */
void token_print(token_t* token)
{
	DEBUG_ME;
	printf("Token: ");
	printf("%s\n", token_stringify(token));
}

/**
 *
 * @function token_name
 * @brief Get the name of a token
 * @params {token_type_t} Token type
 * @returns {char*}
 *
 */
char* token_name(token_type_t type)
{
	DEBUG_ME;
	switch (type) {
		#undef ADD_TOKEN
		#undef ADD_CHAR_TOKEN
		#undef ADD_KEYWORD
		#undef ADD_KEYWORD_REPEAT
		#undef ADD_KEYWORD_HIDE

		#define ADD_TOKEN(TOKEN_TYPE, TOKEN_NAME, TOKEN_VALUE) case TOKEN_TYPE: return TOKEN_NAME;
		#define ADD_CHAR_TOKEN(TOKEN_TYPE, TOKEN_NAME, TOKEN_VALUE, TOKEN_CHAR) case TOKEN_TYPE: return TOKEN_NAME;
		#define ADD_KEYWORD(TOKEN_TYPE, TOKEN_NAME, TOKEN_VALUE, TOKEN_VALUE_LENGTH) case TOKEN_TYPE: return TOKEN_NAME;
		#define ADD_KEYWORD_REPEAT(TOKEN_TYPE, TOKEN_NAME, TOKEN_VALUE, TOKEN_VALUE_LENGTH)
		#define ADD_KEYWORD_HIDE(TOKEN_TYPE, TOKEN_NAME, TOKEN_VALUE, TOKEN_VALUE_LENGTH)

		#include "token.h"
	}

	return TOKEN_NAME_UNKNOWN;
}

/**
 *
 * @function token_type_keyword
 * @brief Get the keyword of a token type
 * @params {token_type_t} Token type
 * @returns {char*}
 *
 */
char* token_type_keyword(token_type_t type)
{
	DEBUG_ME;
	switch (type) {
		#undef ADD_TOKEN
		#undef ADD_CHAR_TOKEN
		#undef ADD_KEYWORD
		#undef ADD_KEYWORD_REPEAT
		#undef ADD_KEYWORD_HIDE

		#define ADD_TOKEN(TOKEN_TYPE, TOKEN_NAME, TOKEN_VALUE) case TOKEN_TYPE: return TOKEN_VALUE;
		#define ADD_CHAR_TOKEN(TOKEN_TYPE, TOKEN_NAME, TOKEN_VALUE, TOKEN_CHAR) case TOKEN_TYPE: return TOKEN_VALUE;
		#define ADD_KEYWORD(TOKEN_TYPE, TOKEN_NAME, TOKEN_VALUE, TOKEN_VALUE_LENGTH) case TOKEN_TYPE: return TOKEN_VALUE;
		#define ADD_KEYWORD_REPEAT(TOKEN_TYPE, TOKEN_NAME, TOKEN_VALUE, TOKEN_VALUE_LENGTH)
		#define ADD_KEYWORD_HIDE(TOKEN_TYPE, TOKEN_NAME, TOKEN_VALUE, TOKEN_VALUE_LENGTH)

		#include "token.h"
	}

	return TOKEN_NAME_UNKNOWN;
}

/**
 *
 * @function token_value_stringify
 * @brief Get the value of a token
 * @params {token_t*} Token
 * @returns {char*}
 *
 */
char* token_value_stringify(token_t* token)
{
	DEBUG_ME;
	static char buffer[1024];

	switch (token->data_type) {
		case TOKEN_NUMBER_INT:
			snprintf(buffer, sizeof(buffer), "%d", token->data.number_int);

			return buffer;

		case TOKEN_NUMBER_FLOAT:
			snprintf(buffer, sizeof(buffer), "%f", token->data.number_float);

			return buffer;

		case TOKEN_STRING:
		case TOKEN_IDENTIFIER:
			return token->data.string;

		case TOKEN_BOOLEAN:
			return token->data.boolean ? TOKEN_BOOL_TRUE : TOKEN_BOOL_FALSE;

		default:
			return token_type_keyword(token->type);
	}
}

/**
 *
 * @function lexer_create
 * @brief Creating a new lexer state
 * @params {char*} file_path - File path
 * @params {char*} source - Source code
 * @returns {lexer_t*}
 *
 */
lexer_t* lexer_create(const char* file_path, char* source)
{
	DEBUG_ME;
	lexer_t* lexer = memory_allocate(sizeof(lexer_t));

	lexer->file_path = file_path;
	lexer->source = source;
	lexer->source_size = 0;
	// lexer->source_length = mb2strlen(lexer->source);
	lexer->source_length = strlen(lexer->source);
	lexer->index = 0;
	lexer->line = 1;
	lexer->column = 1;

	lexer->tokens = array_create(sizeof(token_t*), 10);

	lexer->tokens->print = cast(void (*)(void*), array_token_print);
	lexer->tokens->stringify = cast(char* (*)(void*), array_token_stringify);
	lexer->tokens->destroy = cast(void (*)(void*), array_token_destroy);

	lexer->token_index = 0;

	return lexer;
}

/**
 *
 * @function lexer_destroy
 * @brief Destroying a lexer state
 * @params {lexer_t*} lexer - Lexer state
 * @returns {void}
 *
 */
void lexer_destroy(lexer_t* lexer)
{
	DEBUG_ME;
	if (lexer != NULL) {
		array_destroy_custom(lexer->tokens, cast(void (*)(void*), token_destroy));

		memory_destroy(lexer);
	}
}

/**
 *
 * @function lexer_save
 * @brief Saving the lexer state
 * @params {lexer_t*} lexer - Lexer state
 * @params {const char*} tokens_output - Tokens output file
 * @returns {void}
 *
 */
void lexer_save(lexer_t* lexer, const char* tokens_output)
{
	DEBUG_ME;
	file_writes(tokens_output, "");

	file_appends(tokens_output, "Tokens:\n");
	file_appends(tokens_output, "Lexer source: ");
	file_appends(tokens_output, lexer->source == NULL ? "REPL" : lexer->source);

	file_appends(tokens_output, "\n");
	file_appends(tokens_output, "\n");

	file_appends(tokens_output, "Lexer index: ");
	file_appends(tokens_output, int2string(lexer->index));
	file_appends(tokens_output, "\n");
	file_appends(tokens_output, "Lexer line: ");
	file_appends(tokens_output, int2string(lexer->line));
	file_appends(tokens_output, "\n");
	file_appends(tokens_output, "Lexer column: ");
	file_appends(tokens_output, int2string(lexer->column));

	file_appends(tokens_output, "\n");
	file_appends(tokens_output, "\n");

	for (size_t i = 0; i < lexer->tokens->length; i++) {
		token_t* token = array_get(lexer->tokens, i);

		file_appends(tokens_output, token_stringify(token));
		file_appends(tokens_output, "\n");
	}
}

/**
 *
 * @function lexer_debug
 * @brief Debugging the lexer state
 * @params {lexer_t*} lexer - Lexer state
 * @returns {void}
 *
 */
void lexer_debug(lexer_t* lexer)
{
	DEBUG_ME;
	printf("============= START LEXER DEBUG =============\n");

	printf("Lexer source: %s\n", lexer->source == NULL ? "REPL" : lexer->source);
	printf("Lexer index: %zu\n", lexer->index);
	printf("Lexer line: %zu\n", lexer->line);
	printf("Lexer column: %zu\n", lexer->column);

	lexer->tokens->print(lexer->tokens);

	printf("============= END LEXER DEBUG =============\n");
}

/**
 *
 * @function location_string
 * @brief Get the string representation of a location
 * @params {location_t} location - Location
 * @returns {char*}
 *
 */
char* location_stringify(location_t location)
{
	DEBUG_ME;
	static char buffer[256];
	snprintf(buffer, sizeof(buffer), "%zu:%zu - %zu:%zu", location.start_line, location.start_column, location.end_line, location.end_column);

	return buffer;
}

/**
 *
 * @function location_print
 * @brief Print a location
 * @params {location_t} location - Location
 * @returns {void}
 *
 */
void location_print(location_t location)
{
	DEBUG_ME;
	printf("Location: %s\n", location_stringify(location));
}

/**
 *
 * @function lexer_lex_number
 * @brief Lexing a number
 * @params {lexer_t*} lexer - Lexer state
 * @params {wchar_t} wc - wide character
 * @params {int} char_size - Character size
 * @returns {void}
 *
 */
void lexer_lex_number(lexer_t* lexer, wchar_t wc, int wcl)
{
	DEBUG_ME;
	string_t* value = string_create(25);
	string_append_char(value, convert_to_english_digit(wc));

	bool is_float = false;

	while (LEXER_CURRENT != '\0') {
		wc = read_token(lexer, &wcl);
		if (wc == L'\0') {
			break;
		}

		if (!is_wchar_digit(wc) && wc != L'.') {
			break;
		}

		if (wc == L'.') {
			if (is_float) {
				break;
			}

			is_float = true;
		}

		string_append_char(value, convert_to_english_digit(wc));
	}

	LEXER_PREV;

	token_type_t type = is_float ? TOKEN_NUMBER_FLOAT : TOKEN_NUMBER_INT;
	token_t* token = token_create(type, (location_t) {lexer->index, 1, lexer->line, lexer->column, lexer->line, lexer->column});
	token->data_type = type;

	if (is_float) {
		token->data.number_float = atof(value->data);
	}
	else {
		token->data.number_int = atoi(value->data);
	}

	string_destroy(value);

	LEXER_PUSH_TOKEN(token);
}

/**
 *
 * @function type_keyword
 * @brief Check if a string is a keyword then return the token type
 * @params {const char*} string - String
 * @returns {bool}
 *
 */
token_type_t type_keyword(const char* string)
{
	DEBUG_ME;

	for (size_t i = 0; i < sizeof(keywords) / sizeof(keywords[0]); i++) {
		// TODO: keywords[i].length == length
		if (strcmp(string, keywords[i].keyword) == 0) {
			return keywords[i].type;
		}
	}

	return TOKEN_IDENTIFIER;
}

bool is_ident(wchar_t ch)
{
	return is_alpha(ch) || is_number(ch);
}

bool is_number(wchar_t ch)
{
	return (ch >= L'۰' && ch <= L'۹') || (ch >= '0' && ch <= '9');
}

bool is_alpha(wchar_t ch)
{
	return (
		(
			(ch >= L'آ' || ch >= L'ا') && ch <= L'ی'
		) ||
		ch == L'ـ' ||
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

/**
 *
 * @function lexer_lex_identifier
 * @brief Lexing an identifier
 * @params {lexer_t*} lexer - Lexer state
 * @params {wchar_t} wc - wide character
 * @returns {void}
 *
 */
void lexer_lex_identifier(lexer_t* lexer, wchar_t* wc)
{
	DEBUG_ME;
	string_t* value = string_create(25);

	string_append_wchar(value, *wc);

	wchar_t wcn;
	int wcln;

	while (LEXER_CURRENT != '\0') {
		wcn = read_token(lexer, &wcln);

		if (!is_wchar_alpha(wcn)) {
			lexer->index -= wcln;

			break;
		}

		string_append_wchar(value, wcn);
	}

	token_type_t type = type_keyword(value->data);
	token_t* token = token_create(type, (location_t) {lexer->index, 1, lexer->line, lexer->column, lexer->line, lexer->column});
	token->data_type = TOKEN_IDENTIFIER;
	token->data.string = strdup(value->data);

	string_destroy(value);

	LEXER_PUSH_TOKEN(token);
}

/**
 *
 * @function lexer_lex_string
 * @brief Lexing a string
 * @params {lexer_t*} lexer - Lexer state
 * @params {int} type - String type (0 = english strings, 1= persian strings, 2= second persian strings)
 * @returns {void}
 *
 */
void lexer_lex_string(lexer_t* lexer, int type)
{
	DEBUG_ME;
	// Opening quote is already consumed
	string_t* value = string_create(25);

	int wcl = 0;
	wchar_t wc = read_token(lexer, &wcl);

	while (wc != L'\0' && ((type == 0 && wc != L'"') || (type == 1 && wc != L'»') || (type == 2 && wc == L'”'))) {
		string_append_wchar(value, wc);

		wc = read_token(lexer, &wcl);
	}

	if ((type == 0 && wc != L'"') || (type == 1 && wc != L'»') || (type == 2 && wc != L'”')) {
		string_destroy(value);

		error_lexer(2, "Unterminated string value at line %zu, column %zu", lexer->line, lexer->column);

		return;
	}

	token_t* token = token_create(TOKEN_STRING, (location_t){lexer->index, 1, lexer->line, lexer->column, lexer->line, lexer->column});
	token->data_type = TOKEN_STRING;
	token->data.string = strdup(value->data);

	string_destroy(value);

	LEXER_PUSH_TOKEN(token);
}

/**
 *
 * @function lexer_lex
 * @brief Lexing the source code
 * @params {lexer_t*} lexer - Lexer state
 * @returns {void}
 *
 */
void lexer_lex(lexer_t* lexer)
{
	DEBUG_ME;
	char c;
	wchar_t wc;
	int wcl = 0;

	while ((c = LEXER_CURRENT) && c != '\0' && lexer->index < lexer->source_length) {
		wc = read_token(lexer, &wcl);

		switch (wc) {
			// End of file
			case '\0':
				break;

			// New line
			case '\n':
				LEXER_NEXT_LINE;
				LEXER_ZERO_COLUMN;
				continue;

			case '\a': // Alert
			case '\b': // Backspace
			case '\f': // Form feed
			case '\r': // Carriage return
			case '\t': // Horizontal tab
			case '\v': // Vertical tab
			case ' ': // Space
				continue;

			case '{':
			case '}':
			case '[':
			case ']':
			case ':':
			case ',':
			case '(':
			case ')':
			case '+':
			case '-':
			case '*':
			case '/':
			case '%':
			case '^':
			case '=':
			case '<':
			case '>':
			case '!':
				if (LEXER_CURRENT == '/') {
					LEXER_NEXT;
					LEXER_NEXT_COLUMN;

					while (LEXER_CURRENT != '\n' && LEXER_CURRENT != '\0') {
						LEXER_NEXT;
						LEXER_NEXT_COLUMN;
					}
				}
				else {
					token_type_t type = token_char_type(c);
					token_t* token = token_create(type, (location_t) {lexer->index, 1, lexer->line, lexer->column, lexer->line, lexer->column});

					LEXER_PUSH_TOKEN(token);
				}
				continue;

			case '"':
				lexer_lex_string(lexer, 0);
				continue;

			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':
				lexer_lex_number(lexer, wc, wcl);
				continue;

			default:
				if (wc == L'«') {
					lexer_lex_string(lexer, 1);
				}
				else if (wc == L'“') {
					lexer_lex_string(lexer, 2);
				}
				else if (is_wchar_digit(wc)) {
					lexer_lex_number(lexer, wc, wcl);
				}
				else if (c == '_' || is_wchar_alpha(wc) || is_char_alpha(c)) {
					lexer_lex_identifier(lexer, &wc);
				}
				else {
					file_appends_wchar("windows-logs.txt", wc);
					printf("Invalid character encountered: %02X\n", (unsigned char)wc);
					error_lexer(1, "Unknown character '%lc' at line %zu, column %zu", wc, lexer->line, lexer->column);
				}
				continue;
		}
	}

	token_t* token = token_create(TOKEN_EOF, (location_t) {lexer->index, 1, lexer->line, lexer->column, lexer->line, lexer->column});

	LEXER_PUSH_TOKEN(token);
}
