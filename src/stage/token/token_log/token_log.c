#include <stage/token/token_log/token_log.h>

void token_log(token_t* token)
{
    if (token == NULL) {
        log_info("Token: NULL\n");
        return;
    }

    log_info("Token: Type: %s(%d) (%s), Start: %zu:%zu(%zu), End:%zu:%zu(%zu), Length: %zu\n",
             token_name(token->type),
             token->type,
             token->value ? token->value : "NULL",
             token->location.begin_line, token->location.begin_column, token->location.begin_index,
             token->location.end_line, token->location.end_column, token->location.end_index,
             token->location.length);

    // if (token->type == TOKEN_TYPE_IDENTIFIER) {
    //     log_info("Identifier: '%s'\n", token->value);
    // } else if (token->type == TOKEN_TYPE_VALUE_STRING) {
    //     log_info("String: '%s'\n", token->value);
    // } else if (token->type == TOKEN_TYPE_VALUE_NUMBER_INT) {
    //     log_info("NumberInt: '%s'\n", token->value);
    // } else if (token->type == TOKEN_TYPE_VALUE_NUMBER_FLOAT) {
    //     log_info("NumberFloat: '%s'\n", token->value);
    // }
}
