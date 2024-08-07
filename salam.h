bool debug_enabled = true;

#ifndef MB_CUR_MAX
	#define MB_CUR_MAX (MB_LEN_MAX)
#endif

#define print_error(fmt, ...) fprintf(stderr, fmt, ##__VA_ARGS__)

#define print_message(fmt, ...) if (debug_enabled) fprintf(stdout, fmt, ##__VA_ARGS__)

#define CREATE_MEMORY_OBJECT(result, type, length, fmt, ...) \
	result = (type *) malloc(sizeof(type) * (length));  \
	if (!result) { \
		fprintf(stderr, fmt,  __VA_ARGS__);\
		exit(EXIT_FAILURE); \
	}

#define CHECK_MEMORY_ALLOCATION(ptr, errorMsg, ...)             \
	do {                                                       \
		if (ptr == NULL) {                                     \
			fprintf(stderr, errorMsg, ##__VA_ARGS__);          \
			exit(EXIT_FAILURE);                                \
		}                                                      \
	} while (0)

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

typedef struct {
	size_t size;
	size_t length;
	void** data;
} array_t;

typedef struct hashmap_entry {
	char *key;
	void *value;
	struct hashmap_entry *next;
} hashmap_entry_t;

typedef struct {
	hashmap_entry_t **data;
	size_t size;
	size_t length;
} hashmap_t;

typedef struct {
	size_t size;
	size_t length;
	char* data;
} string_t;

typedef enum {
	// Values
	TOKEN_TYPE_IDENTIFIER,
	TOKEN_TYPE_INT,
	TOKEN_TYPE_FLOAT,
	TOKEN_TYPE_STRING,

	// Keywords
	TOKEN_TYPE_LAYOUT, // صفحه
	TOKEN_TYPE_END, // پایان
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
	TOKEN_TYPE_SHARP, // #
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
	char* data;
	size_t length;

	size_t index;
	size_t line;
	size_t column;

	array_t* tokens;

	size_t last_index;
	size_t last_line;
	size_t last_column;
	size_t last_length;
} lexer_t;

struct ast_node;

typedef enum {
	AST_TYPE_FUNCTION,
	AST_TYPE_LAYOUT,
} ast_node_type_t;

typedef enum {
	AST_TYPE_LAYOUT_ERROR,

	AST_TYPE_LAYOUT_BODY,

	AST_TYPE_LAYOUT_BUTTON,
	AST_TYPE_LAYOUT_TEXT,
	AST_TYPE_LAYOUT_INPUT,
	AST_TYPE_LAYOUT_LINE,
	AST_TYPE_LAYOUT_LINK,
	AST_TYPE_LAYOUT_IMAGE,
	AST_TYPE_LAYOUT_CENTER,
	AST_TYPE_LAYOUT_FORM,
	AST_TYPE_LAYOUT_BREAK,
	AST_TYPE_LAYOUT_TABLE,
	AST_TYPE_LAYOUT_TABLE_ROW,
	AST_TYPE_LAYOUT_TABLE_COLUMN,
	AST_TYPE_LAYOUT_BOLD,
	AST_TYPE_LAYOUT_DIV,
	AST_TYPE_LAYOUT_PARAGTAPH,
	AST_TYPE_LAYOUT_TEXTAREA,
	AST_TYPE_LAYOUT_SELECT,
	AST_TYPE_LAYOUT_SELECT_OPTION,
} ast_layout_type_t;

typedef struct {
	const char* keyword;
	token_type_t token_type;
} keyword_mapping_t;

typedef struct {
	const char* keyword;
	ast_layout_type_t layout_type;
} layout_keyword_mapping_t;

typedef struct {
	ast_layout_type_t type;

	bool is_mother;

	hashmap_t* attributes;
	array_t* children;
} ast_layout_node_t;

typedef struct {
	hashmap_t* attributes;
	array_t* elements;
} ast_layout_t;

typedef struct {
	char* name;
	array_t* arguments;
} ast_function_declaration_t;

typedef struct ast_node {
	ast_node_type_t type;
	union {
		ast_function_declaration_t* function_declaration;
		ast_layout_node_t* layout;
	} data;
} ast_node_t;

typedef struct {
	lexer_t* lexer;
	size_t token_index;
	array_t* functions;
	array_t* expressions;

	ast_layout_node_t* layout;
} parser_t;

// Function Declarations
char* read_dynamic_string();
char* intToString(int value);
char* file_read(char* file_Name);
char* trim_value(char* value);
char* attribute_css_multiple_size_value(char* attribute_name, array_t* attribute_values);
char* attribute_css_size_value(char* attribute_name, char* attribute_value);
bool is_english_digit(wchar_t ch);
bool is_persian_digit(wchar_t ch);
bool is_arabic_digit(wchar_t ch);
bool string_is_number(const char* value);
char* attribute_css_values(char* attribute_name, array_t* attribute_values);
char* normalise_css_size(char* attribute_value);
bool is_allowed_single_layout_property(ast_layout_type_t type, char* attribute_name, char** new_attribute_name);
bool is_allowed_mother_layout_property(ast_layout_type_t type, char* attribute_name, char** new_attribute_name);
bool is_allowed_general_layout_property(char* attribute_name, char** new_attribute_name);
bool is_allowed_layout_property(bool is_mother, ast_layout_type_t type, char* attribute_name, char** new_attribute_name);

// String
string_t* string_create(size_t initial_size);
void string_append_char(string_t* str, char c);
void string_append_str(string_t* str, const char* suffix);
void string_free(string_t* str);
void string_print(string_t* str);
void string_append(string_t* dest, string_t* src);

// AST
string_t* ast_string(parser_t* parser, int ident);
void ast_print(parser_t* parser);
char* ast_layout_type_string(ast_layout_type_t type);
char* ast_node_type_string(ast_node_type_t type);
void ast_layout_ident(string_t* str, int ident);

// Generate
string_t* generate_layout(ast_layout_node_t* node, parser_t* parser, int ident);
void generate_layout_ident(string_t* str, int ident);
string_t* generate_string(parser_t* parser, int ident);
void generate_print(parser_t* parser);
char* generate_layout_type_string(ast_layout_type_t type);

// Hashmap
hashmap_t* hashmap_create();
void hashmap_put(hashmap_t *map, const char *key, void *value);
void* hashmap_get(hashmap_t *map, const char *key);
void* hashmap_remove(hashmap_t *map, const char *key);
void hashmap_free(hashmap_t *map);
void hashmap_print(hashmap_t *map);
void hashmap_array_free(hashmap_t *map);

// Array
array_t* array_create(size_t size);
void* array_pop(array_t* arr);
void array_push(array_t* arr, void* data);
void array_free(array_t* arr);
void array_print(array_t* arr);
array_t* array_copy(array_t* arr);
void array_token_free(array_t* arr);

// Token
token_t* token_create(token_type_t type, const char* value, int a, int b, int c, int b2, int c2);
void token_print(token_t* t);
void token_free(token_t* token);

// Token Utils
char* token_type2str(token_type_t type);
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

// Lexer
lexer_t* lexer_create(const char* data);
void lexer_free(lexer_t* lexer);
void lexer_lex(lexer_t* lexer);

// Parser
parser_t* parser_create(lexer_t* lexer);
void parser_parse(parser_t* parser);
void parser_free(parser_t* parser);
token_t* parser_token_get(parser_t* parser);
void parser_token_eat_nodata(parser_t* parser, token_type_t type);
token_t* parser_token_eat(parser_t* parser, token_type_t type);
array_t* parser_layout_elements(parser_t* parser);
ast_layout_node_t* parser_layout_element(parser_t* parser);
ast_layout_node_t* parser_layout_element_single(ast_layout_type_t type, parser_t* parser);
ast_layout_node_t* parser_layout_element_mother(ast_layout_type_t type, parser_t* parser);
ast_layout_node_t* parser_layout(parser_t* parser);

void help();
int main(int argc, char** argv);

void attributes_free(hashmap_t* attributes);
void children_free(array_t* children);
void ast_layout_node_free(ast_layout_node_t* node);
