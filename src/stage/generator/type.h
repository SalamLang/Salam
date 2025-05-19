#ifndef _STAGE_GENERATOR_TYPE_H_
#define _STAGE_GENERATOR_TYPE_H_

// base
#include <base.h>

// ast
#include <stage/ast/type.h>

typedef struct
{
    ast_t* ast;
    ast_package_t* package; // (nullable): TODO
} generator_t;

#endif
