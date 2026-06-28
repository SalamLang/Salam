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

#ifndef SALAM_INTERP_INTERP_H
#define SALAM_INTERP_INTERP_H

#include "core/prelude.h"
#include "core/arena.h"
#include "logger/logger.h"
#include "ast/ast.h"
#include "semantic/sema.h"

typedef struct {
    FILE       *out;
    FILE       *err;
    const char *input_data;
    const char *lang;
    int         timeout_ms;
} interp_options_t;

int interp_run(arena_t *a, logger_t *log, ast_node_t *program,
               sema_result_t *sem, const char *entry,
               const interp_options_t *opts);

#endif /* SALAM_INTERP_INTERP_H */
