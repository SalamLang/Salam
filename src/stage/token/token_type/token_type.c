#include <stage/token/token_type/token_type.h>

token_type_t token_type(const char* name)
{
    if (string_compare(name, "OPERATOR") == 0)     return TOKEN_TYPE_OPERATOR;

    else if (string_compare(name, "IDENTIFIER") == 0)        return TOKEN_TYPE_IDENTIFIER;

    // keyword
    else if (string_compare(name, "KEYWORD_IF") == 0)        return TOKEN_TYPE_KEYWORD_IF;
    else if (string_compare(name, "KEYWORD_ELSE") == 0)        return TOKEN_TYPE_KEYWORD_ELSE;
    else if (string_compare(name, "KEYWORD_FOR") == 0)        return TOKEN_TYPE_KEYWORD_FOR;
    else if (string_compare(name, "KEYWORD_WHILE") == 0)        return TOKEN_TYPE_KEYWORD_WHILE;
    else if (string_compare(name, "KEYWORD_RET") == 0)        return TOKEN_TYPE_KEYWORD_RET;
    else if (string_compare(name, "KEYWORD_FN") == 0)        return TOKEN_TYPE_KEYWORD_FN;

    // value
    else if (string_compare(name, "STRING") == 0)       return TOKEN_TYPE_VALUE_STRING;
    else if (string_compare(name, "NUMBER_INT") == 0)   return TOKEN_TYPE_VALUE_NUMBER_INT;
    else if (string_compare(name, "NUMBER_FLOAT") == 0) return TOKEN_TYPE_VALUE_NUMBER_FLOAT;
    
    return TOKEN_TYPE_UNKNOWN;
}
