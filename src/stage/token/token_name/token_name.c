#include <stage/token/token_name/token_name.h>

const char* token_name(token_type_t type)
{
    switch (type) {
        case TOKEN_TYPE_IDENTIFIER:   return "IDENTIFIER";
        case TOKEN_TYPE_KEYWORD:      return "KEYWORD";
        case TOKEN_TYPE_STRING:       return "STRING";
        case TOKEN_TYPE_NUMBER_INT:   return "NUMBER_INT";
        case TOKEN_TYPE_NUMBER_FLOAT: return "NUMBER_FLOAT";
        case TOKEN_TYPE_OPERATOR:     return "OPERATOR";
        case TOKEN_TYPE_UNKNOWN:
        default:                      return "UNKNOWN";
    }
}
