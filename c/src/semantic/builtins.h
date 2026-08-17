/*
 * Salam Programming Language (2024-2026)
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

/*
 * Argument count for atomic_load/store/add/swap/cas, or 0 for any other
 * name. Shared by the semantic pass and every backend so the set of
 * intrinsics is described in exactly one place.
 */
int salam_atomic_arity(const char *name);

bool salam_builtin_global_const(arena_t *a, const char *name, ast_node_t *n,
                                bool *is_str);

/*
 * Hand the -dNAME=VALUE constants from the command line to the name
 * resolver. Each entry is one raw "NAME=VALUE" (or bare "NAME", meaning
 * true) string, already checked for a SCREAMING_SNAKE_CASE name by the CLI.
 * Values are copied into storage that lives as long as the process, so the
 * caller's strings need not outlive the call. Definitions past the internal
 * capacity are reported and dropped rather than silently ignored.
 */
void salam_builtin_set_consts(const char *const *defs, int ndefs);

#endif /* SALAM_SEMANTIC_BUILTINS_H */
