#include <stage/token/token_log/token_log.h>

void token_log(token_t* token)
{
    if (token == NULL) {
        log_info("Token: NULL\n");
        return;
    }

    log_info("Token: Type: %s(%d) (%s), Start: %zu:%zu(%zu), End:%zu:%zu(%zu), Length: %zu",
             token_name(token->type),
             token->type,
             token->source ? token->source : "NULL",
             token->location.begin_line, token->location.begin_column, token->location.begin_index,
             token->location.end_line, token->location.end_column, token->location.end_index,
             token->location.length);

    if (token->value == NULL) {
        log_info(", Value: NULL\n");
    } else {
        log_info(", ");
        value_log(token->value);
    }
}
