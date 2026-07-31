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

#ifndef SALAM_JSGEN_JSGEN_H
#define SALAM_JSGEN_JSGEN_H

#include "core/prelude.h"
#include "core/arena.h"
#include "core/vec.h"
#include "logger/logger.h"
#include "ast/ast.h"
#include "semantic/sema.h"

typedef struct {
    const char *module;
    const char *globals_src;
    vec_t fn_names;
    vec_t fn_texts;
    const char *entry_mangled;
} jsgen_output_t;

jsgen_output_t *jsgen_run(arena_t *a, logger_t *log, ast_node_t *program,
                          sema_result_t *sem, const char *module, const char *entry,
                          bool enable_minify, const char **minify_last,
                          vec_t *minify_keys, vec_t *minify_vals);

#endif /* SALAM_JSGEN_JSGEN_H */
