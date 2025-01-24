#include "token_value.h"

char *token_value_stringify(token_t *token) {
    DEBUG_ME;
    static char buffer[1024];

    switch (token->data_type) {
        case TOKEN_NUMBER_INT:
            snprintf(buffer, sizeof(buffer), "%d", token->data.number_int);

            return buffer;

        case TOKEN_NUMBER_FLOAT:
            snprintf(buffer, sizeof(buffer), "%f", token->data.number_float);

            return buffer;

        case TOKEN_STRING:
        case TOKEN_IDENTIFIER:
            return token->data.string;

        case TOKEN_BOOLEAN:
            return token->data.boolean ? "True" : "False";

        default:
            return "error?";
            // return token_type_keyword(token->type);
    }
}
