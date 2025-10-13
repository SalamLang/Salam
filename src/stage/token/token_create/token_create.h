#ifndef _SRC_STAGE_TOKEN_TOKEN_CREATE_TOKEN_CREATE_H_
#define _SRC_STAGE_TOKEN_TOKEN_CREATE_TOKEN_CREATE_H_

// memory
#include <utility/memory/memory_allocation/memory_allocation.h>

// token
#include <stage/token/type.h>

token_t* token_create(token_type_t type, token_location_t token_location);

#endif // _SRC_STAGE_TOKEN_TOKEN_CREATE_TOKEN_CREATE_H_
