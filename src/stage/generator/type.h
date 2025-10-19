#ifndef _SRC_STAGE_GENERATOR_TYPE_H_
#define _SRC_STAGE_GENERATOR_TYPE_H_

#include <stddef.h> // for size_t

// base
#include <base.h>

// ast
#include <stage/ast/type.h>

typedef struct
{
    ast_t* ast;
    ast_package_t* package; // (nullable): TODO
    size_t ident;
} generator_t;

#endif // _SRC_STAGE_GENERATOR_TYPE_H_
