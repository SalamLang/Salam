#ifndef SALAM_SEMANTIC_BUILTINS_H
#define SALAM_SEMANTIC_BUILTINS_H

#include "core/prelude.h"

typedef struct {
    const char *name;     /* Salam name      */
    const char *runtime;  /* C runtime symbol */
    int         nargs;
    const char *ret;      /* return type     */
    const char *arg;      /* each arg's type */
} salam_builtin_t;

const salam_builtin_t *salam_builtin_lookup(const char *name);

#endif /* SALAM_SEMANTIC_BUILTINS_H */
