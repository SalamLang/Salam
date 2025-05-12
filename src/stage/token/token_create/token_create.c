#include <stage/token/token_create/token_create.h>

token_t* token_create()
{
    token_t* token = memory_allocation(sizeof(token_t));
    return token;
}
