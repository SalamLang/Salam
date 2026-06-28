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

#ifndef SALAM_CODEGEN_CODEGEN_H
#define SALAM_CODEGEN_CODEGEN_H

#include "core/prelude.h"
#include "core/arena.h"
#include "logger/logger.h"
#include "ast/ast.h"
#include "semantic/sema.h"

#define SALAM_MOD_PREFIX "salam_mod_"

typedef struct {
    const char *module;
    const char *h_src;
    const char *c_src;
} codegen_output_t;

codegen_output_t *codegen_run(arena_t *a, logger_t *log, ast_node_t *program,
                              sema_result_t *sem, const char *module, bool safe,
                              bool debug_info, const char *src_path,
                              const char *entry);

#endif /* SALAM_CODEGEN_CODEGEN_H */
