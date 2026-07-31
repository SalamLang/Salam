/*
 * Salam Programming Language (2024–2026)
 *
 *   +-------------------+
 *   |     S A L A M     |
 *   +-------------------+
 *
 * Designed by Seyyed Ali Mohammadiyeh and the Salam Team
 * Born from a decade of language design experience (since 2018)
 *
 * Repository: https://github.com/SalamLang/Salam
 *
 */

#ifndef SALAM_SEMANTIC_BUILTINS_H
#define SALAM_SEMANTIC_BUILTINS_H

#include "core/prelude.h"
#include "ast/ast.h"

typedef struct {
    const char *name;
    const char *runtime;
    int nargs;
    const char *ret;
    const char *arg;
} salam_builtin_t;

const salam_builtin_t *salam_builtin_lookup(const char *name);

bool salam_builtin_global_const(const char *name, ast_node_t *n, bool *is_str);

#endif /* SALAM_SEMANTIC_BUILTINS_H */
