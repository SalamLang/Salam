#include "lexer.h"

#define LEXER_CURRENT (lexer->source[lexer->index])
#define LEXER_CURRENT_PREV (lexer->source[lexer->index - 1])
#define LEXER_CURRENT_NEXT (lexer->source[lexer->index + 1])
#define LEXER_NEXT lexer->index++
#define LEXER_PREV lexer->index--
#define LEXER_NEXT_LINE lexer->line++
#define LEXER_NEXT_COLUMN lexer->column++
#define LEXER_ZERO_COLUMN lexer->column = 0;
#define LEXER_PUSH_TOKEN(TOKEN) array_push(lexer->tokens, TOKEN)

/**
 * 
 * @function is_char_digit
 * @brief Check if a character is a digit
 * @params {char} c - Character
 * @returns {bool}
 * 
 */
bool is_char_digit(char c)
{
    DEBUG_ME;
	return c >= '0' && c <= '9';
}

/**
 * 
 * @function is_char_alpha
 * @brief Check if a character is an alphabet
 * @params {char} c - Character
 * @returns {bool}
 * 
 */
bool is_char_alpha(char c)
{
    DEBUG_ME;
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

/**
 * 
 * @function is_char_alnum
 * @brief Check if a character is an alphabet or a digit
 * @params {char} c - Character
 * @returns {bool}
 * 
 */
bool is_char_alnum(char c)
{
    DEBUG_ME;
	return is_char_alpha(c) || is_char_digit(c);
}

/**
 * 
 * @function is_char_whitespace
 * @brief Check if a character is a whitespace
 * @params {char} c - Character
 * @returns {bool}
 * 
 */
bool is_char_whitespace(char c)
{
    DEBUG_ME;
	return c == ' ' || c == '\t' || c == '\n' || c == '\r';
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
	printf("Location: %zu:%zu - %zu:%zu\n", location.start_line, location.start_column, location.end_line, location.end_column);
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

	token->name = cast(char* (*)(token_type_t), token_name);
	token->value = cast(char* (*)(void*), token_value);

	token->print = cast(void (*)(void*), token_print);
	token->destroy = cast(void (*)(void*), token_destroy);
		
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
 * @function token_print
 * @brief Print a token
 * @params {token_t*} token - Token
 * @returns {void}
 * 
 */
void token_print(token_t* token)
{
    DEBUG_ME;
	printf("Token ");
	switch (token->type) {
		case TOKEN_EOF:
			printf("EOF");
			break;
		
		case TOKEN_IDENTIFIER:
			printf("IDENTIFIER: '%s' (%zu)", token->data.string, strlen(token->data.string));
			break;
		
		case TOKEN_NUMBER_FLOAT:
			printf("NUMBER_FLOAT: %f", token->data.number_float);
			break;
		
		case TOKEN_NUMBER_INT:
			printf("NUMBER_INT: %d", token->data.number_int);
			break;
		
		case TOKEN_STRING:
			printf("STRING: '%s' (%zu)", token->data.string, strlen(token->data.string));
			break;
		
		case TOKEN_LEFT_BRACE:
		case TOKEN_RIGHT_BRACE:
		case TOKEN_LEFT_BRACKET:
		case TOKEN_RIGHT_BRACKET:
		case TOKEN_COLON:
		case TOKEN_COMMA:
		case TOKEN_LEFT_PAREN:
		case TOKEN_RIGHT_PAREN:
		case TOKEN_PLUS:
		case TOKEN_MINUS:
		case TOKEN_MULTIPLY:
		case TOKEN_DIVIDE:
		case TOKEN_MOD:
		case TOKEN_POWER:
		case TOKEN_ASSIGN:
		case TOKEN_LESS:
		case TOKEN_GREATER:
		case TOKEN_NOT:
			printf("SYMBOL: '%c'", token->type);
			break;
		
		case TOKEN_NOT_EQUAL: printf("SYMBOL: '!='"); break;
		case TOKEN_EQUAL: printf("SYMBOL: '=='"); break;
		case TOKEN_AND_AND: printf("SYMBOL: '&&'"); break;
		case TOKEN_OR_OR: printf("SYMBOL: '||'"); break;
		case TOKEN_AND_BIT: printf("SYMBOL: '&'"); break;
		case TOKEN_OR_BIT: printf("SYMBOL: '|'"); break;
		case TOKEN_LESS_EQUAL: printf("SYMBOL: '<='"); break;
		case TOKEN_GREATER_EQUAL: printf("SYMBOL: '>='"); break;

		case TOKEN_INCREMENT: printf("SYMBOL: '++"); break;
		case TOKEN_DECREMENT: printf("SYMBOL: '--'"); break;

		case TOKEN_SHIFT_LEFT: printf("SYMBOL: '>>'"); break;
		case TOKEN_SHIFT_RIGHT: printf("SYMBOL: '<<'"); break;
		case TOKEN_SHIFT_LEFT_ASSIGN: printf("SYMBOL: '>>='"); break;
		case TOKEN_SHIFT_RIGHT_ASSIGN: printf("SYMBOL: '<<='"); break;

		case TOKEN_BOOLEAN:
			printf("BOOLEAN: %s", token->data.boolean ? "true" : "false");
			break;

		case TOKEN_ERROR:
			printf("ERROR");
			break;

		case TOKEN_LAYOUT:
			printf("LAYOUT");
			break;

		case TOKEN_IMPORT:
			printf("IMPORT");
			break;

		case TOKEN_FUNCTION:
			printf("FUNCTION");
			break;
			
		case TOKEN_RETURN:
			printf("RETURN");
			break;

		case TOKEN_IF:
			printf("IF");
			break;

		case TOKEN_ELSE:
			printf("ELSE");
			break;

		case TOKEN_PRINT:
			printf("PRINT");
			break;
		
		case TOKEN_WHILE:
			printf("WHILE");
			break;

		case TOKEN_FOR:
			printf("FOR");
			break;

		case TOKEN_BREAK:
			printf("BREAK");
			break;

		case TOKEN_CONTINUE:
			printf("CONTINUE");
			break;

		default:
			printf("UNKNOWN");
			break;
	}

	printf(" at ");
	location_print(token->location);
}

/**
 * 
 * @function token_string
 * @brief Get the name & value of a token as a string
 * @params {token_t*} token - Token
 * @returns {char*} - String representation of the token
 * 
 */
char* token_string(token_t* token)
{
    static char buffer[256];

    switch (token->type) {
        case TOKEN_EOF:
            sprintf(buffer, "EOF");
            break;

        case TOKEN_IDENTIFIER:
            sprintf(buffer, "IDENTIFIER: '%s' (%zu)", token->data.string, strlen(token->data.string));
            break;

        case TOKEN_NUMBER_FLOAT:
            sprintf(buffer, "NUMBER_FLOAT: %f", token->data.number_float);
            break;

        case TOKEN_NUMBER_INT:
            sprintf(buffer, "NUMBER_INT: %d", token->data.number_int);
            break;

        case TOKEN_STRING:
            sprintf(buffer, "STRING: '%s' (%zu)", token->data.string, strlen(token->data.string));
            break;

        case TOKEN_LEFT_BRACE:
        case TOKEN_RIGHT_BRACE:
        case TOKEN_LEFT_BRACKET:
        case TOKEN_RIGHT_BRACKET:
        case TOKEN_COLON:
        case TOKEN_COMMA:
        case TOKEN_LEFT_PAREN:
        case TOKEN_RIGHT_PAREN:
        case TOKEN_PLUS:
        case TOKEN_MINUS:
        case TOKEN_MULTIPLY:
        case TOKEN_DIVIDE:
        case TOKEN_MOD:
        case TOKEN_POWER:
        case TOKEN_ASSIGN:
        case TOKEN_LESS:
        case TOKEN_GREATER:
        case TOKEN_NOT:
            sprintf(buffer, "SYMBOL: '%c'", token->type);
            break;

        case TOKEN_NOT_EQUAL: sprintf(buffer, "SYMBOL: '!='"); break;
        case TOKEN_EQUAL: sprintf(buffer, "SYMBOL: '=='"); break;
        case TOKEN_AND_AND: sprintf(buffer, "SYMBOL: '&&'"); break;
        case TOKEN_OR_OR: sprintf(buffer, "SYMBOL: '||'"); break;
        case TOKEN_AND_BIT: sprintf(buffer, "SYMBOL: '&'"); break;
        case TOKEN_OR_BIT: sprintf(buffer, "SYMBOL: '|'"); break;
        case TOKEN_LESS_EQUAL: sprintf(buffer, "SYMBOL: '<='"); break;
        case TOKEN_GREATER_EQUAL: sprintf(buffer, "SYMBOL: '>='"); break;

        case TOKEN_INCREMENT: sprintf(buffer, "SYMBOL: '++'"); break;
        case TOKEN_DECREMENT: sprintf(buffer, "SYMBOL: '--'"); break;

        case TOKEN_SHIFT_LEFT: sprintf(buffer, "SYMBOL: '>>'"); break;
        case TOKEN_SHIFT_RIGHT: sprintf(buffer, "SYMBOL: '<<'"); break;
        case TOKEN_SHIFT_LEFT_ASSIGN: sprintf(buffer, "SYMBOL: '>>='"); break;
        case TOKEN_SHIFT_RIGHT_ASSIGN: sprintf(buffer, "SYMBOL: '<<='"); break;

        case TOKEN_BOOLEAN:
            sprintf(buffer, "BOOLEAN: %s", token->data.boolean ? "true" : "false");
            break;

        case TOKEN_ERROR:
            sprintf(buffer, "ERROR");
            break;

        case TOKEN_LAYOUT:
            sprintf(buffer, "LAYOUT");
            break;

        case TOKEN_IMPORT:
            sprintf(buffer, "IMPORT");
            break;

        case TOKEN_FUNCTION:
            sprintf(buffer, "FUNCTION");
            break;

        case TOKEN_RETURN:
            sprintf(buffer, "RETURN");
            break;

        case TOKEN_IF:
            sprintf(buffer, "IF");
            break;

        case TOKEN_ELSE:
            sprintf(buffer, "ELSE");
            break;

        case TOKEN_PRINT:
            sprintf(buffer, "PRINT");
            break;

        case TOKEN_WHILE:
            sprintf(buffer, "WHILE");
            break;

        case TOKEN_FOR:
            sprintf(buffer, "FOR");
            break;

        case TOKEN_BREAK:
            sprintf(buffer, "BREAK");
            break;

        case TOKEN_CONTINUE:
            sprintf(buffer, "CONTINUE");
            break;

        default:
            sprintf(buffer, "UNKNOWN");
            break;
    }

    return buffer;
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
		case TOKEN_EOF:
			return "EOF";

		case TOKEN_IDENTIFIER:
			return "IDENTIFIER";

		case TOKEN_STRING:
			return "STRING";
			
		case TOKEN_NUMBER_INT:
			return "NUMBER_INT";

		case TOKEN_LEFT_BRACE:
			return "LEFT_BRACE";

		case TOKEN_RIGHT_BRACE:
			return "RIGHT_BRACE";

		case TOKEN_LEFT_BRACKET:
			return "LEFT_BRACKET";

		case TOKEN_RIGHT_BRACKET:
			return "RIGHT_BRACKET";

		case TOKEN_COLON:
			return "COLON";

		case TOKEN_COMMA:
			return "COMMA";

		case TOKEN_LEFT_PAREN:
			return "LEFT_PAREN";

		case TOKEN_RIGHT_PAREN:
			return "RIGHT_PAREN";

		case TOKEN_NUMBER_FLOAT:
			return "NUMBER_FLOAT";

		case TOKEN_BOOLEAN:
			return "BOOLEAN";

		case TOKEN_PLUS:
			return "PLUS";

		case TOKEN_MINUS:
			return "MINUS";
			
		case TOKEN_MULTIPLY:
			return "MULTIPLY";

		case TOKEN_DIVIDE:
			return "DIVIDE";

		case TOKEN_MOD:
			return "MOD";

		case TOKEN_POWER:
			return "POWER";

		case TOKEN_ASSIGN:
			return "ASSIGN";

		case TOKEN_LESS:
			return "LESS";
			
		case TOKEN_GREATER:
			return "GREATER";

		case TOKEN_NOT:
			return "NOT";

		case TOKEN_NOT_EQUAL:
			return "NOT_EQUAL";

		case TOKEN_EQUAL:
			return "EQUAL";

		case TOKEN_AND_AND:
			return "AND_AND";

		case TOKEN_OR_OR:
			return "OR_OR";

		case TOKEN_AND_BIT:
			return "AND_BIT";

		case TOKEN_OR_BIT:
			return "OR_BIT";

		case TOKEN_LESS_EQUAL:
			return "LESS_EQUAL";

		case TOKEN_GREATER_EQUAL:
			return "GREATER_EQUAL";

		case TOKEN_INCREMENT:
			return "INCREMENT";

		case TOKEN_DECREMENT:
			return "DECREMENT";

		case TOKEN_SHIFT_LEFT:
			return "SHIFT_LEFT";

		case TOKEN_SHIFT_RIGHT:
			return "SHIFT_RIGHT";

		case TOKEN_SHIFT_LEFT_ASSIGN:
			return "SHIFT_LEFT_ASSIGN";

		case TOKEN_SHIFT_RIGHT_ASSIGN:
			return "SHIFT_RIGHT_ASSIGN";

		// Keywords
		case TOKEN_LAYOUT:
			return "LAYOUT";

		case TOKEN_IMPORT:
			return "IMPORT";

		case TOKEN_FUNCTION:
			return "FUNCTION";

		case TOKEN_RETURN:
			return "RETURN";

		case TOKEN_IF:
			return "IF";

		case TOKEN_ELSE:
			return "ELSE";

		case TOKEN_PRINT:
			return "PRINT";

		case TOKEN_WHILE:
			return "WHILE";

		case TOKEN_FOR:
			return "FOR";

		case TOKEN_BREAK:
			return "BREAK";

		case TOKEN_CONTINUE:
			return "CONTINUE";

		case TOKEN_ERROR:
			return "ERROR";
	}

	return "UNKNOWN";
}

/**
 * 
 * @function int2string
 * @brief Convert an integer to a string
 * @params {int} value - Integer value
 * @returns {char*}
 * 
 */
char* int2string(int value)
{
	DEBUG_ME;
	static char buffer[256];
	snprintf(buffer, sizeof(buffer), "%d", value);
	return buffer;
}

/**
 * 
 * @function float2string
 * @brief Convert a float to a string
 * @params {float} value - Float value
 * @returns {char*}
 * 
 */
char* float2string(float value)
{
	DEBUG_ME;
	static char buffer[256];
	snprintf(buffer, sizeof(buffer), "%f", value);
	return buffer;
}

/**
 * 
 * @function double2string
 * @brief Convert a double to a string
 * @params {double} value - Double value
 * @returns {char*}
 * 
 */
char* double2string(double value)
{
	DEBUG_ME;
	static char buffer[256];
	snprintf(buffer, sizeof(buffer), "%f", value);
	return buffer;
}

/**
 * 
 * @function token_value
 * @brief Get the value of a token 
 * @params {token_t*} Token
 * @returns {char*}
 * 
 */
char* token_value(token_t* token)
{
    DEBUG_ME;
	static char buffer[256];

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
			return token->data.boolean ? "true" : "false";

		default:
			return "UNKNOWN";
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
	lexer->index = 0;
	lexer->line = 1;
	lexer->column = 1;
	lexer->tokens = array_create(sizeof(token_t*), 10);
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
        printf("\t");
        token_t* token = array_get(lexer->tokens, i);

		file_appends(tokens_output, token_string(token));
		file_appends(tokens_output, " at ");
		file_appends(tokens_output, location_string(token->location));
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

	array_token_print(lexer->tokens);

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
char* location_string(location_t location)
{
	DEBUG_ME;
	static char buffer[256];
	snprintf(buffer, sizeof(buffer), "%zu:%zu - %zu:%zu", location.start_line, location.start_column, location.end_line, location.end_column);
	return buffer;
}

/**
 * 
 * @function lexer_lex_number
 * @brief Lexing a number
 * @params {lexer_t*} lexer - Lexer state
 * @returns {void}
 * 
 */
void lexer_lex_number(lexer_t* lexer)
{
    DEBUG_ME;
	char* buffer = memory_allocate(256);
	size_t index = 0;
	buffer[index++] = LEXER_CURRENT_PREV;

	while (is_char_digit(LEXER_CURRENT)) {
		buffer[index++] = LEXER_CURRENT;
		LEXER_NEXT;
		LEXER_NEXT_COLUMN;
	}

	if (LEXER_CURRENT == '.') {
		buffer[index++] = LEXER_CURRENT;
		LEXER_NEXT;
		LEXER_NEXT_COLUMN;

		while (is_char_digit(LEXER_CURRENT)) {
			buffer[index++] = LEXER_CURRENT;
			LEXER_NEXT;
			LEXER_NEXT_COLUMN;
		}

		buffer[index] = '\0';

		token_t* token = token_create(TOKEN_NUMBER_FLOAT, (location_t) {lexer->index, 1, lexer->line, lexer->column, lexer->line, lexer->column});
		token->data_type = TOKEN_NUMBER_FLOAT;
		token->data.number_float = atof(buffer);
		LEXER_PUSH_TOKEN(token);

		if (buffer != NULL) {
			memory_destroy(buffer);
		}
	} else {
		buffer[index] = '\0';

		token_t* token = token_create(TOKEN_NUMBER_INT, (location_t) {lexer->index, 1, lexer->line, lexer->column, lexer->line, lexer->column});
		token->data_type = TOKEN_NUMBER_INT;
		token->data.number_int = atoi(buffer);
		LEXER_PUSH_TOKEN(token);

		if (buffer != NULL) {
			memory_destroy(buffer);
		}
	}
}

/**
 * 
 * @function is_keyword
 * @brief Check if a string is a keyword
 * @params {const char*} string - String
 * @returns {bool}
 * 
 */
bool is_keyword(const char* string)
{
    DEBUG_ME;
	for (size_t i = 0; i < sizeof(keywords) / sizeof(keywords[0]); i++) {
		if (strcmp(string, keywords[i].name) == 0) {
			return true;
		}
	}

	return false;
}

/**
 * 
 * @function is_keyword
 * @brief Check if a string is a keyword
 * @params {const char*} string - String
 * @returns {bool}
 * 
 */
token_type_t type_keyword(const char* string)
{
    DEBUG_ME;
	for (size_t i = 0; i < sizeof(keywords) / sizeof(keywords[0]); i++) {
		if (strcmp(string, keywords[i].name) == 0) {
			return keywords[i].type;
		}
	}

	return TOKEN_IDENTIFIER;
}

/**
 * 
 * @function lexer_lex_identifier
 * @brief Lexing an identifier
 * @params {lexer_t*} lexer - Lexer state
 * @returns {void}
 * 
 */
void lexer_lex_identifier(lexer_t* lexer)
{
    DEBUG_ME;
	char* buffer = memory_allocate(256);
	size_t index = 0;
	buffer[index++] = LEXER_CURRENT_PREV;

	while (isalnum(LEXER_CURRENT) || LEXER_CURRENT == '_') {
		buffer[index++] = LEXER_CURRENT;
		LEXER_NEXT;
		LEXER_NEXT_COLUMN;
	}

	buffer[index] = '\0';

	token_type_t type = type_keyword(buffer);

	token_t* token = token_create(type, (location_t) {lexer->index, 1, lexer->line, lexer->column, lexer->line, lexer->column});
	token->data_type = TOKEN_IDENTIFIER;
	token->data.string = buffer;
	LEXER_PUSH_TOKEN(token);
}

/**
 * 
 * @function lexer_lex_string
 * @brief Lexing a string
 * @params {lexer_t*} lexer - Lexer state
 * @returns {void}
 * 
 */
void lexer_lex_string(lexer_t* lexer)
{
    DEBUG_ME;
	// Opening quote is already consumed
	char* buffer = memory_allocate(256);
	size_t index = 0;

	while (LEXER_CURRENT != '"' && LEXER_CURRENT != '\0') {
		buffer[index++] = LEXER_CURRENT;
		LEXER_NEXT;
		LEXER_NEXT_COLUMN;
	}

	buffer[index] = '\0';

	if (LEXER_CURRENT != '\"') {
		error(2, "Unterminated string value at line %zu, column %zu", lexer->line, lexer->column);
	}

	LEXER_NEXT;

	token_t* token = token_create(TOKEN_STRING, (location_t) {lexer->index, 1, lexer->line, lexer->column, lexer->line, lexer->column});
	token->data_type = TOKEN_STRING;
	token->data.string = buffer;
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
	while (LEXER_CURRENT != '\0') {
		LEXER_NEXT;
		LEXER_NEXT_COLUMN;

		char c = LEXER_CURRENT_PREV;

		switch (c) {
			case '\0': // End of file
			case -1: // End of file
				goto gotoend;
				break;
			
			case '\n': // New line
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
					token_t* token = token_create(c, (location_t) {lexer->index, 1, lexer->line, lexer->column, lexer->line, lexer->column});
					LEXER_PUSH_TOKEN(token);
				}
				continue;
			case '"':
				lexer_lex_string(lexer);
				continue;
			
			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':
				lexer_lex_number(lexer);
				continue;

			default:
				if (is_char_alpha(LEXER_CURRENT_PREV) || LEXER_CURRENT_PREV == '_') {
					lexer_lex_identifier(lexer);
				} else {
					printf("--->1 %c\n", LEXER_CURRENT_PREV);
					printf("--->2 %c\n", LEXER_CURRENT);

					printf("--->3 %d\n", LEXER_CURRENT_PREV);
					printf("--->4 %d\n", LEXER_CURRENT);
					token_t* token = token_create(TOKEN_ERROR, (location_t) {lexer->index, 1, lexer->line, lexer->column, lexer->line, lexer->column});
					LEXER_PUSH_TOKEN(token);
				}
				continue;
		}
	}
	
	gotoend:
	token_t* token = token_create(TOKEN_EOF, (location_t) {lexer->index, 1, lexer->line, lexer->column, lexer->line, lexer->column});
	LEXER_PUSH_TOKEN(token);
}
