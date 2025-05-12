#include <stage/token/token_create/token_create.h>

token_t* token_create(token_type_t type)
{
    token_t* token = memory_allocation(sizeof(token_t));
    token->type = type;
    token->value = NULL;
    return token;
}
