#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <locale.h>
#include <stdbool.h>
#include <string.h>
#include <limits.h>

typedef enum {
	// values
	TOKEN_TYPE_IDENTIFIER,
	TOKEN_TYPE_NUMBER,
	TOKEN_TYPE_STRING,

	// keywords
	TOKEN_TYPE_FUNCTION,
	TOKEN_TYPE_RETURN,
	
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
} lexer_t;


typedef struct {
	lexer_t* lexer;
	size_t token_index;
} parser_t;

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
void parser_function(parser_t* parser);
void parser_block(parser_t* parser);
void parser_statement(parser_t* parser);
void parser_return(parser_t* parser);
void parser_expression(parser_t* parser);

char* token_type2str(token_type_t type)
{
    switch(type) {
        case TOKEN_TYPE_IDENTIFIER: return "IDENTIFIER";
        case TOKEN_TYPE_NUMBER: return "NUMBER";
        case TOKEN_TYPE_STRING: return "STRING";
        case TOKEN_TYPE_FUNCTION: return "FUNCTION";
        case TOKEN_TYPE_RETURN: return "RETURN";
        case TOKEN_TYPE_SECTION_OPEN: return "SECTION_OPEN";
        case TOKEN_TYPE_SECTION_CLOSE: return "SECTION_CLOSE";
        case TOKEN_TYPE_PARENTHESE_OPEN: return "PARENTHESIS_OPEN";
        case TOKEN_TYPE_PARENTHESE_CLOSE: return "PARENTHESIS_CLOSE";
        case TOKEN_TYPE_PLUS: return "PLUS";
        case TOKEN_TYPE_MINUS: return "MINUS";
        case TOKEN_TYPE_EOF: return "EOF";
        case TOKEN_TYPE_ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

char* file_read(char* file_Name)
{
	FILE* file = fopen(file_Name, "r");
	if (file == NULL)
	{
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
	printf("%zu - %s - %s\n", t->location.length, token_type2str(t->type), t->value);
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

	// printf("number = %s\n", number);
}

void read_string(lexer_t* lexer, wchar_t ch)
{
	char* string = (char*)malloc(sizeof(char) * 1024);
	int i = 0;
	while (ch != L'"') {
		string[i++] = ch;
		ch = read_token(lexer);
	}
	string[i] = 0;

	size_t length = strlen(string);
	token_t* t = token_create(TOKEN_TYPE_STRING, string, length, lexer->line, lexer->column - length, lexer->line, lexer->column);
	array_push(lexer->tokens, t);

	// printf("string = %s\n", string);
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
		if (current_char == '{') {
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
			read_string(lexer, current_char);
		} else if (is_number(current_char)) {
			read_number(lexer, current_char);
		} else if (is_alpha(current_char)) {
			read_identifier(lexer, current_char);
		} else {
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

	return parser;
}

void parser_free(parser_t* parser)
{
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

void parser_function(parser_t* parser) {
    printf("Parsing function\n");
	token_t* t;

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

    parser_block(parser);
}


void parser_return(parser_t* parser) {
    printf("Parsing return statement\n");

    parser->token_index++;
    parser_expression(parser);
}

void parser_statement(parser_t* parser) {
    printf("Parsing statement\n");

    if (parser->lexer->tokens->length > parser->token_index && ((token_t*)parser->lexer->tokens->data[parser->token_index])->type == TOKEN_TYPE_RETURN) {
        parser_return(parser);
    } else {
		printf("Error: Unexpected token as statement\n");
		exit(EXIT_FAILURE);
    }
}

void parser_expression(parser_t* parser) {
    printf("Parsing expression\n");

	// ( and ) and + and - operator
	// we have string, number
	// TODO
}

void parser_block(parser_t* parser) {
    printf("Parsing block\n");

	parser_token_eat(parser, TOKEN_TYPE_SECTION_OPEN);

    while (parser->lexer->tokens->length > parser->token_index && ((token_t*) parser->lexer->tokens->data[parser->token_index])->type != TOKEN_TYPE_SECTION_CLOSE) {
        parser_statement(parser);
    }

	parser_token_eat(parser, TOKEN_TYPE_SECTION_CLOSE);
}

void parser_parse(parser_t* parser)
{
    while (parser->token_index < parser->lexer->tokens->length) {
        token_t* current_token = (token_t*)parser->lexer->tokens->data[parser->token_index];
        
        switch (current_token->type) {
            case TOKEN_TYPE_FUNCTION:
                parser_function(parser);
                break;
            case TOKEN_TYPE_SECTION_OPEN:
                parser_block(parser);
                break;
            case TOKEN_TYPE_RETURN:
                parser_return(parser);
                break;
            default:
                parser_statement(parser);
                break;
        }

        parser->token_index++;
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

	exit(EXIT_SUCCESS);
}
