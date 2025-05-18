#include <stage/token/token_keyword_type/token_keyword_type.h>

token_type_t token_keyword_type(char* value)
{
    // keyword
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
    } else if (string_compare(value, "end") == 0) {
        return TOKEN_TYPE_KEYWORD_END;
    } else if (string_compare(value, "print") == 0) {
        return TOKEN_TYPE_KEYWORD_PRINT;
    } else if (string_compare(value, "import") == 0) {
        return TOKEN_TYPE_KEYWORD_IMPORT;
    } else if (string_compare(value, "package") == 0) {
        return TOKEN_TYPE_KEYWORD_PACKAGE;
    } else if (string_compare(value, "struct") == 0) {
        return TOKEN_TYPE_KEYWORD_STRUCT;
    } else if (string_compare(value, "enum") == 0) {
        return TOKEN_TYPE_KEYWORD_ENUM;
    } else if (string_compare(value, "union") == 0) {
        return TOKEN_TYPE_KEYWORD_UNION;
    } else if (string_compare(value, "type") == 0) {
        return TOKEN_TYPE_KEYWORD_TYPE;
    } else if (string_compare(value, "raw") == 0) {
        return TOKEN_TYPE_KEYWORD_RAW;
    }

    // kind
    else if (string_compare(value, "int") == 0) {
        return TOKEN_TYPE_KIND_INT;
    } else if (string_compare(value, "float") == 0) {
        return TOKEN_TYPE_KIND_FLOAT;
    } else if (string_compare(value, "string") == 0) {
        return TOKEN_TYPE_KIND_STRING;
    } else if (string_compare(value, "bool") == 0) {
        return TOKEN_TYPE_KIND_BOOL;
    } else if (string_compare(value, "array") == 0) {
        return TOKEN_TYPE_KIND_ARRAY;
    } else if (string_compare(value, "map") == 0) {
        return TOKEN_TYPE_KIND_MAP;
    } else if (string_compare(value, "set") == 0) {
        return TOKEN_TYPE_KIND_SET;
    } else if (string_compare(value, "void") == 0) {
        return TOKEN_TYPE_KIND_VOID;
    }

    // value
    else if (string_compare(value, "true") == 0) {
        return TOKEN_TYPE_VALUE_TRUE;
    } else if (string_compare(value, "false") == 0) {
        return TOKEN_TYPE_VALUE_FALSE;
    } else if (string_compare(value, "null") == 0) {
        return TOKEN_TYPE_VALUE_NULL;
    }

    return TOKEN_TYPE_IDENTIFIER;
}
