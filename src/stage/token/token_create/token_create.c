#include <stage/token/token_create/token_create.h>

token_t* token_create(token_type_t type, token_location_t token_location)
{
    token_t* token = memory_allocation(sizeof(token_t));
    token->location = token_location;
    token->type = type;
    token->source = NULL;
    token->value = NULL;
    return token;
}
