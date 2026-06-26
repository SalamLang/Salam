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

typedef struct {
    const char *name;     /* Salam name      */
    const char *runtime;  /* C runtime symbol */
    int         nargs;
    const char *ret;      /* return type     */
    const char *arg;      /* each arg's type */
} salam_builtin_t;

const salam_builtin_t *salam_builtin_lookup(const char *name);

#endif /* SALAM_SEMANTIC_BUILTINS_H */
