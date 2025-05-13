#include <stage/token/token_char_type/token_char_type.h>

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
            return TOKEN_TYPE_OPERATOR;
        
        default:
            return TOKEN_TYPE_UNKNOWN;
    }
}
