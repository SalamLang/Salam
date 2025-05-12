#include <stage/token/token_type/token_type.h>

token_type_t token_type(const char* name)
{
    if (strcmp(name, "IDENTIFIER") == 0)        return TOKEN_TYPE_IDENTIFIER;
    else if (strcmp(name, "KEYWORD") == 0)      return TOKEN_TYPE_KEYWORD;
    else if (strcmp(name, "STRING") == 0)       return TOKEN_TYPE_STRING;
    else if (strcmp(name, "NUMBER_INT") == 0)   return TOKEN_TYPE_NUMBER_INT;
    else if (strcmp(name, "NUMBER_FLOAT") == 0) return TOKEN_TYPE_NUMBER_FLOAT;
    else if (strcmp(name, "OPERATOR") == 0)     return TOKEN_TYPE_OPERATOR;
    return TOKEN_TYPE_UNKNOWN;
}
