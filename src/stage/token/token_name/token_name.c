#include <stage/token/token_name/token_name.h>

const char* token_name(token_type_t type)
{
    switch (type) {
        case TOKEN_TYPE_UNKNOWN:            return "TOKEN_UNKNOWN";
        case TOKEN_TYPE_EOF:                return "EOF";

        case TOKEN_TYPE_OPERATOR:           return "OPERATOR";

        case TOKEN_TYPE_IDENTIFIER:         return "IDENTIFIER";

        // keyword
        case TOKEN_TYPE_KEYWORD_IF:         return "KEYWORD_IF";
        case TOKEN_TYPE_KEYWORD_ELSE:       return "KEYWORD_ELSE";
        case TOKEN_TYPE_KEYWORD_FOR:        return "KEYWORD_FOR";
        case TOKEN_TYPE_KEYWORD_WHILE:      return "KEYWORD_WHILE";
        case TOKEN_TYPE_KEYWORD_RET:        return "KEYWORD_RET";
        case TOKEN_TYPE_KEYWORD_FN:         return "KEYWORD_FN";
        case TOKEN_TYPE_KEYWORD_END:        return "KEYWORD_END";
        case TOKEN_TYPE_KEYWORD_IMPORT:     return "KEYWORD_IMPORT";
        case TOKEN_TYPE_KEYWORD_PACKAGE:    return "KEYWORD_PACKAGE";
        case TOKEN_TYPE_KEYWORD_STRUCT:     return "KEYWORD_STRUCT";
        case TOKEN_TYPE_KEYWORD_ENUM:       return "KEYWORD_ENUM";
        case TOKEN_TYPE_KEYWORD_UNION:      return "KEYWORD_UNION";
        case TOKEN_TYPE_KEYWORD_TYPE:       return "KEYWORD_TYPE";
        case TOKEN_TYPE_KEYWORD_RAW:        return "KEYWORD_RAW";

        // kind
        case TOKEN_TYPE_KIND_INT:           return "KIND_INT";
        case TOKEN_TYPE_KIND_FLOAT:         return "KIND_FLOAT";
        case TOKEN_TYPE_KIND_STRING:        return "KIND_STRING";
        case TOKEN_TYPE_KIND_BOOL:          return "KIND_BOOL";
        case TOKEN_TYPE_KIND_ARRAY:         return "KIND_ARRAY";
        case TOKEN_TYPE_KIND_MAP:           return "KIND_MAP";
        case TOKEN_TYPE_KIND_SET:           return "KIND_SET";
        case TOKEN_TYPE_KIND_TUPLE:         return "KIND_TUPLE";
        case TOKEN_TYPE_KIND_VOID:          return "KIND_VOID";
        case TOKEN_TYPE_KIND_ANY:           return "KIND_ANY";

        // value
        case TOKEN_TYPE_VALUE_STRING:       return "STRING";
        case TOKEN_TYPE_VALUE_NUMBER_INT:   return "NUMBER_INT";
        case TOKEN_TYPE_VALUE_NUMBER_FLOAT: return "NUMBER_FLOAT";
        case TOKEN_TYPE_VALUE_TRUE:         return "TRUE";
        case TOKEN_TYPE_VALUE_FALSE:        return "FALSE";
        case TOKEN_TYPE_VALUE_NULL:         return "NULL";
    }
    return "UNKNOWN_ERROR";
}
