#include <stage/token/token_operator_name/token_operator_name.h>

const char* token_operator_name(token_operator_type_t operator_type)
{
    switch (operator_type) {
        case TOKEN_OPERATOR_TYPE_UNKNOWN:              return "OPERATOR_UNKNOWN";
        case TOKEN_OPERATOR_TYPE_BRACE_OPEN:           return "{";
        case TOKEN_OPERATOR_TYPE_BRACE_CLOSE:          return "}";
        case TOKEN_OPERATOR_TYPE_PAREN_OPEN:           return "(";
        case TOKEN_OPERATOR_TYPE_PAREN_CLOSE:          return ")";
        case TOKEN_OPERATOR_TYPE_BRACKET_OPEN:         return "[";
        case TOKEN_OPERATOR_TYPE_BRACKET_CLOSE:        return "]";
        case TOKEN_OPERATOR_TYPE_COMMA:                return ",";
        case TOKEN_OPERATOR_TYPE_SEMICOLON:            return ";";
        case TOKEN_OPERATOR_TYPE_COLON:                return ":";
        case TOKEN_OPERATOR_TYPE_DOT:                  return ".";
        case TOKEN_OPERATOR_TYPE_DOT_DOT:              return "..";
        case TOKEN_OPERATOR_TYPE_DOT_DOT_DOT:          return "...";
        case TOKEN_OPERATOR_TYPE_ARROW:                return "->";
        case TOKEN_OPERATOR_TYPE_HASH:                 return "#";
        case TOKEN_OPERATOR_TYPE_ASSIGN:               return "=";
        case TOKEN_OPERATOR_TYPE_PLUS:                 return "+";
        case TOKEN_OPERATOR_TYPE_MINUS:                return "-";
        case TOKEN_OPERATOR_TYPE_ASTERISK:             return "*";
        case TOKEN_OPERATOR_TYPE_SLASH:                return "/";
        case TOKEN_OPERATOR_TYPE_PERCENT:              return "%";
        case TOKEN_OPERATOR_TYPE_CARET:                return "^";
        case TOKEN_OPERATOR_TYPE_AMPERSAND:            return "&";
        case TOKEN_OPERATOR_TYPE_PIPE:                 return "|";
        case TOKEN_OPERATOR_TYPE_TILDE:                return "~";
        case TOKEN_OPERATOR_TYPE_EXCLAMATION:          return "!";
        case TOKEN_OPERATOR_TYPE_QUESTION:             return "?";
        case TOKEN_OPERATOR_TYPE_NULL_COALESCING:      return "??";
        case TOKEN_OPERATOR_TYPE_NULLISH_COALESCING:   return "?:";
        case TOKEN_OPERATOR_TYPE_LT:                   return "<";
        case TOKEN_OPERATOR_TYPE_GT:                   return ">";
        case TOKEN_OPERATOR_TYPE_LT_EQ:                return "<=";
        case TOKEN_OPERATOR_TYPE_GT_EQ:                return ">=";
        case TOKEN_OPERATOR_TYPE_EQ:                   return "==";
        case TOKEN_OPERATOR_TYPE_NOT_EQ:               return "!=";
        case TOKEN_OPERATOR_TYPE_AND:                  return "&&";
        case TOKEN_OPERATOR_TYPE_OR:                   return "||";
        case TOKEN_OPERATOR_TYPE_INC:                  return "++";
        case TOKEN_OPERATOR_TYPE_DEC:                  return "--";
    }
    return "UNKNOWN_ERROR";
}
