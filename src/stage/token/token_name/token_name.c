#include <stage/token/token_name/token_name.h>

const char* token_name(token_type_t type)
{
    switch (type) {
        case TOKEN_TYPE_EOF: return "EOF";
        
        case TOKEN_TYPE_OPERATOR:     return "OPERATOR";

        case TOKEN_TYPE_IDENTIFIER:   return "IDENTIFIER";

        // keyword
        case TOKEN_TYPE_KEYWORD_IF:      return "KEYWORD_IF";
        case TOKEN_TYPE_KEYWORD_ELSE:      return "KEYWORD_ELSE";
        case TOKEN_TYPE_KEYWORD_FOR:      return "KEYWORD_FOR";
        case TOKEN_TYPE_KEYWORD_WHILE:      return "KEYWORD_WHILE";
        case TOKEN_TYPE_KEYWORD_RET:      return "KEYWORD_RET";
        case TOKEN_TYPE_KEYWORD_FN:      return "KEYWORD_FN";

        // value
        case TOKEN_TYPE_VALUE_STRING:       return "STRING";
        case TOKEN_TYPE_VALUE_NUMBER_INT:   return "NUMBER_INT";
        case TOKEN_TYPE_VALUE_NUMBER_FLOAT: return "NUMBER_FLOAT";

        case TOKEN_TYPE_UNKNOWN:
        default:                      return "UNKNOWN";
    }
}
