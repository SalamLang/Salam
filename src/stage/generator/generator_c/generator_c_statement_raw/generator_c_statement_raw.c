#include "generator_c_statement_raw.h"

buffer_t* generator_c_statement_raw(generator_t* generator, ast_t* ast)
{
    DEBUG_ME;
    ast_statement_raw_t raw = ast->raw.statement_raw_value;
    if (raw.value == NULL) return NULL;

    size_t length = string_length(raw.value);
    if (length == 0) return NULL;

    buffer_t* temp = buffer_create(string_length(raw.value));

    buffer_t* ident = generator_c_ident(generator);
    buffer_append(temp, ident);

    buffer_append_str(temp, raw.value);

    buffer_destroy(ident);

    return temp;
}
