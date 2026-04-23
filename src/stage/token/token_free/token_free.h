#ifndef _STAGE_TOKEN_TOKEN_FREE_TOKEN_FREE_H_
#define _STAGE_TOKEN_TOKEN_FREE_TOKEN_FREE_H_

#include <stddef.h>  // for NULL

// base
#include <base.h>

// memory
#include <utility/memory/memory_destroy/memory_destroy.h>

// token
#include <stage/token/type.h>

// value
#include <stage/value/type.h>
#include <stage/value/value_destroy/value_destroy.h>

void token_free(token_t* token);

#endif  // _STAGE_TOKEN_TOKEN_FREE_TOKEN_FREE_H_
