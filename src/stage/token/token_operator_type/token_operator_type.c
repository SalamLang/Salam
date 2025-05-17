#include <stage/token/token_operator_type/token_operator_type.h>

token_operator_type_t token_operator_type(char* operator) {
    if (string_compare(operator, "{") == 0) return TOKEN_OPERATOR_TYPE_BRACE_OPEN;
    else if (string_compare(operator, "}") == 0) return TOKEN_OPERATOR_TYPE_BRACE_CLOSE;
    else if (string_compare(operator, "(") == 0) return TOKEN_OPERATOR_TYPE_PAREN_OPEN;
    else if (string_compare(operator, ")") == 0) return TOKEN_OPERATOR_TYPE_PAREN_CLOSE;
    else if (string_compare(operator, "[") == 0) return TOKEN_OPERATOR_TYPE_BRACKET_OPEN;
    else if (string_compare(operator, "]") == 0) return TOKEN_OPERATOR_TYPE_BRACKET_CLOSE;
    else if (string_compare(operator, ",") == 0) return TOKEN_OPERATOR_TYPE_COMMA;
    else if (string_compare(operator, ";") == 0) return TOKEN_OPERATOR_TYPE_SEMICOLON;
    else if (string_compare(operator, ":") == 0) return TOKEN_OPERATOR_TYPE_COLON;
    else if (string_compare(operator, ".") == 0) return TOKEN_OPERATOR_TYPE_DOT;
    else if (string_compare(operator, "..") == 0) return TOKEN_OPERATOR_TYPE_DOT_DOT;
    else if (string_compare(operator, "...") == 0) return TOKEN_OPERATOR_TYPE_DOT_DOT_DOT;
    else if (string_compare(operator, "->") == 0) return TOKEN_OPERATOR_TYPE_ARROW;
    else if (string_compare(operator, "#") == 0) return TOKEN_OPERATOR_TYPE_HASH;
    else if (string_compare(operator, "=") == 0) return TOKEN_OPERATOR_TYPE_ASSIGN;
    else if (string_compare(operator, "+") == 0) return TOKEN_OPERATOR_TYPE_PLUS;
    else if (string_compare(operator, "-") == 0) return TOKEN_OPERATOR_TYPE_MINUS;
    else if (string_compare(operator, "*") == 0) return TOKEN_OPERATOR_TYPE_ASTERISK;
    else if (string_compare(operator, "/") == 0) return TOKEN_OPERATOR_TYPE_SLASH;
    else if (string_compare(operator, "%") == 0) return TOKEN_OPERATOR_TYPE_PERCENT;
    else if (string_compare(operator, "^") == 0) return TOKEN_OPERATOR_TYPE_CARET;
    else if (string_compare(operator, "&") == 0) return TOKEN_OPERATOR_TYPE_AMPERSAND;
    else if (string_compare(operator, "|") == 0) return TOKEN_OPERATOR_TYPE_PIPE;
    else if (string_compare(operator, "~") == 0) return TOKEN_OPERATOR_TYPE_TILDE;
    else if (string_compare(operator, "!") == 0) return TOKEN_OPERATOR_TYPE_EXCLAMATION;
    else if (string_compare(operator, "?") == 0) return TOKEN_OPERATOR_TYPE_QUESTION;
    else if (string_compare(operator, "??") == 0) return TOKEN_OPERATOR_TYPE_NULL_COALESCING;
    else if (string_compare(operator, "?:") == 0) return TOKEN_OPERATOR_TYPE_NULLISH_COALESCING;
    else if (string_compare(operator, "<") == 0) return TOKEN_OPERATOR_TYPE_LT;
    else if (string_compare(operator, ">") == 0) return TOKEN_OPERATOR_TYPE_GT;
    else if (string_compare(operator, "<=") == 0) return TOKEN_OPERATOR_TYPE_LT_EQ;
    else if (string_compare(operator, ">=") == 0) return TOKEN_OPERATOR_TYPE_GT_EQ;
    else if (string_compare(operator, "==") == 0) return TOKEN_OPERATOR_TYPE_EQ;
    else if (string_compare(operator, "!=") == 0) return TOKEN_OPERATOR_TYPE_NOT_EQ;
    else if (string_compare(operator, "&&") == 0) return TOKEN_OPERATOR_TYPE_AND;
    else if (string_compare(operator, "||") == 0) return TOKEN_OPERATOR_TYPE_OR;
    else if (string_compare(operator, "++") == 0) return TOKEN_OPERATOR_TYPE_INC;
    else if (string_compare(operator, "--") == 0) return TOKEN_OPERATOR_TYPE_DEC;

    return TOKEN_OPERATOR_TYPE_UNKNOWN;
}
