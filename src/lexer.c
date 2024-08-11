#include "base.h"
#include "lexer.h"

#define LEXER_CURRENT (lexer->source[lexer->index])
#define LEXER_NEXT lexer->index++
#define LEXER_PREV lexer->index--
#define LEXER_NEXT_LINE lexer->line++
#define LEXER_NEXT_COLUMN lexer->column++
#define LEXER_ZERO_COLUMN lexer->column = 0;
#define LEXER_PUSH_TOKEN(TOKEN) array_push(lexer->tokens, TOKEN)

/**
 * 
 * @function location_print
 * @brief Print a location
 * @param {location_t} location - Location
 * @returns {void}
 * 
 */
void location_print(location_t location)
{
    printf("Location: %zu:%zu - %zu:%zu\n", location.start_line, location.start_column, location.end_line, location.end_column);
}
/**
 * 
 * @function token_create
 * @brief Creating a new token
 * @param {token_type_t} type - Token type
 * @param {location_t} location - Token location
 * @returns {token_t*}
 * 
 */
token_t* token_create(token_type_t type, location_t location)
{
    token_t* token = memory_allocate(sizeof(token_t));
    token->type = type;
    token->location = location;
    token->data_type = TOKEN_ERROR;
    token->name = cast(char* (*)(void*), token_name);
    token->value = cast(char* (*)(void*), token_value);
    token->print = cast(void (*)(void*), token_print);
    token->destroy = cast(void (*)(void*), token_destroy);
    return token;
}

/**
 * 
 * @function token_destroy
 * @brief Destroying a token
 * @param {token_t*} token - Token
 * @returns {void}
 * 
 */
void token_destroy(token_t* token)
{
    if (token->type == TOKEN_IDENTIFIER || token->type == TOKEN_STRING) {
        memory_free(token->data.string);
    }

    memory_free(token);
}

/**
 * 
 * @function token_print
 * @brief Print a token
 * @param {token_t*} token - Token
 * @returns {void}
 * 
 */
void token_print(token_t* token)
{
    printf("Token: ");
    switch (token->type) {
        case TOKEN_EOF:
            printf("EOF\n");
            break;
        case TOKEN_IDENTIFIER:
            printf("IDENTIFIER: %s\n", token->data.string);
            break;
        case TOKEN_NUMBER_FLOAT:
            printf("NUMBER_FLOAT: %f\n", token->data.number_float);
            break;
        case TOKEN_NUMBER_INT:
            printf("NUMBER_INT: %d\n", token->data.number_int);
            break;
        case TOKEN_STRING:
            printf("STRING: %s\n", token->data.string);
            break;
        case TOKEN_BOOLEAN:
            printf("BOOLEAN: %s\n", token->data.boolean ? "true" : "false");
            break;
        case TOKEN_ERROR:
            printf("ERROR\n");
            break;
    }
}

/**
 * 
 * @function token_name
 * @brief Get the name of a token
 * @param {token_t*} Token
 * @returns {char*}
 * 
 */
char* token_name(token_t* token)
{
    switch (token->type) {
        case TOKEN_EOF:
            return "EOF";
        case TOKEN_IDENTIFIER:
            return "IDENTIFIER";
        case TOKEN_STRING:
            return "STRING";
        case TOKEN_NUMBER_INT:
            return "NUMBER_INT";
        case TOKEN_NUMBER_FLOAT:
            return "NUMBER_FLOAT";
        case TOKEN_BOOLEAN:
            return "BOOLEAN";
        case TOKEN_ERROR:
            return "ERROR";
    }
    return "UNKNOWN";
}

/**
 * 
 * @function token_value
 * @brief Get the value of a token 
 * @param {token_t*} Token
 * @returns {char*}
 * 
 */
char* token_value(token_t* token)
{
    static char buffer[256];

    switch (token->data_type) {
        case TOKEN_NUMBER_INT:
            snprintf(buffer, sizeof(buffer), "%d", token->data.number_int);
            return buffer;

        case TOKEN_NUMBER_FLOAT:
            snprintf(buffer, sizeof(buffer), "%f", token->data.number_float);
            return buffer;

        case TOKEN_STRING:
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
 * @param {char*} file_path - File path
 * @param {char*} source - Source code
 * @returns {lexer_t*}
 * 
 */
lexer_t* lexer_create(const char* file_path, char* source)
{
    lexer_t* lexer = memory_allocate(sizeof(lexer_t));
    lexer->file_path = file_path;
    lexer->source = source;
    lexer->index = 0;
    lexer->line = 1;
    lexer->column = 1;
    lexer->tokens = array_create(sizeof(token_t*), 10);
    return lexer;
}

/**
 * 
 * @function lexer_destroy
 * @brief Destroying a lexer state
 * @param {lexer_t*} lexer - Lexer state
 * @returns {void}
 * 
 */
void lexer_destroy(lexer_t* lexer)
{
    array_token_free(lexer->tokens);
    memory_free(lexer);
}

/**
 * 
 * @function lexer_lex_number
 * @brief Lexing a number
 * @param {lexer_t*} lexer - Lexer state
 * @returns {void}
 * 
 */
void lexer_lex_number(lexer_t* lexer)
{
    int number = 0;
    while (isdigit(LEXER_CURRENT)) {
        number = number * 10 + (LEXER_CURRENT - '0');
        LEXER_NEXT;
        LEXER_NEXT_COLUMN;
    }

    if (LEXER_CURRENT == '.') {
        LEXER_NEXT;
        LEXER_NEXT_COLUMN;

        float decimal = 0.0;
        float factor = 0.1;
        while (isdigit(LEXER_CURRENT)) {
            decimal += factor * (LEXER_CURRENT - '0');
            factor /= 10;
            LEXER_NEXT;
            LEXER_NEXT_COLUMN;
        }

        token_t* token = token_create(TOKEN_NUMBER_FLOAT, (location_t) {lexer->index, 1, lexer->line, lexer->column, lexer->line, lexer->column});
        token->data.number_float = number + decimal;
        LEXER_PUSH_TOKEN(token);
    } else {
        token_t* token = token_create(TOKEN_NUMBER_INT, (location_t) {lexer->index, 1, lexer->line, lexer->column, lexer->line, lexer->column});
        token->data.number_int = number;
        LEXER_PUSH_TOKEN(token);
    }
}

/**
 * 
 * @function lexer_lex
 * @brief Lexing the source code
 * @param {lexer_t*} lexer - Lexer state
 * @returns {void}
 * 
 */
void lexer_lex(lexer_t* lexer)
{
    // while (LEXER_CURRENT != '\0') {
    //     LEXER_NEXT;
    //     LEXER_NEXT_COLUMN;

    //     switch (LEXER_CURRENT) {
    //         case '\0': // End of file
    //         case -1: // End of file
    //             break;
            
    //         case '\n': // New line
    //             LEXER_NEXT_LINE;
    //             LEXER_ZERO_COLUMN;
    //             break;
            
    //         case '\a': // Alert
    //         case '\b': // Backspace
    //         case '\f': // Form feed
    //         case '\r': // Carriage return
    //         case '\t': // Horizontal tab
    //         case '\v': // Vertical tab
    //         case ' ': // Space
    //             continue;
            
    //         case '0': case '1': case '2': case '3': case '4':
    //         case '5': case '6': case '7': case '8': case '9': {
    //             lexer_lex_number(lexer);
    //         }

    //         default:
    //             continue;
    //     }
    // }
    
    token_t* token = token_create(TOKEN_EOF, (location_t) {lexer->index, 1, lexer->line, lexer->column, lexer->line, lexer->column});
    token->print(token);
    LEXER_PUSH_TOKEN(token);
}
