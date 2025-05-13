#include <stage/token/token_keyword_type/token_keyword_type.h>

token_type_t token_keyword_type(char* value)
{
    if (string_compare(value, "if") == 0) {
        return TOKEN_TYPE_KEYWORD_IF;
    } else if (string_compare(value, "else") == 0) {
        return TOKEN_TYPE_KEYWORD_ELSE;
    } else if (string_compare(value, "for") == 0) {
        return TOKEN_TYPE_KEYWORD_FOR;
    } else if (string_compare(value, "while") == 0) {
        return TOKEN_TYPE_KEYWORD_WHILE;
    } else if (string_compare(value, "ret") == 0) {
        return TOKEN_TYPE_KEYWORD_RET;
    } else if (string_compare(value, "fn") == 0) {
        return TOKEN_TYPE_KEYWORD_FN;
    }
    return TOKEN_TYPE_UNKNOWN;
}
