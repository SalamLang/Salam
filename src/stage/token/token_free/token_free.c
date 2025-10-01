#include <stage/token/token_free/token_free.h>

void token_free(token_t* token) {
    // if (! token) return;

    if (token->source != NULL) {
        memory_destroy(token->source);
    }

    token->location.begin_line = 0;
    token->location.begin_column = 0;
    token->location.begin_index = 0;

    token->location.end_line = 0;
    token->location.end_column = 0;
    token->location.end_index = 0;

    token->location.length = 0;

    token->type = TOKEN_TYPE_UNKNOWN;

    if (token->value != NULL) {
        value_destroy(token->value);
    }

    memory_destroy(token);
}
