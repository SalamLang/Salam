#include <stage/token/token_operator_keyword_type/token_operator_keyword_type.h>

token_operator_type_t token_operator_keyword_type(char* value)
{
    if (string_compare(value, "and") == 0) {
        return TOKEN_OPERATOR_TYPE_AND;
    } else if (string_compare(value, "or") == 0) {
        return TOKEN_OPERATOR_TYPE_OR;
    }

    return TOKEN_OPERATOR_TYPE_UNKNOWN;
}
