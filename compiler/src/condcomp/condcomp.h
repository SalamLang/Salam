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

#ifndef SALAM_CONDCOMP_CONDCOMP_H
#define SALAM_CONDCOMP_CONDCOMP_H

#include "core/prelude.h"
#include "core/arena.h"
#include "logger/logger.h"
#include "ast/ast.h"

typedef enum { CC_BOOL, CC_STR } cc_val_kind_t;

typedef struct {
    const char *name;
    cc_val_kind_t kind;
    bool bval;
    const char *sval;
} cc_const_t;

typedef struct cc_table_t cc_table_t;

/* Builds the compile-time-constant table for one compile: host-or-target
 * OS/arch flags, then each entry of `defines` (bare "NAME" -> CC_BOOL true,
 * "NAME=value" -> CC_STR). target_triple may be NULL to use the host
 * compiler's predefined macros (non-cross-compile). */
cc_table_t *cc_table_build(arena_t *a, const char *target_triple,
                           const char *const *defines, int ndefines);

const cc_const_t *cc_table_lookup(const cc_table_t *tbl, const char *name);

/* Walks `program` in place, folding any `if`/`else`/`end` whose condition is
 * compile-time-constant and splicing the taken branch's contents over the
 * node, so the untaken branch never reaches semantic analysis. A top-level
 * if (one appearing outside a function body) whose condition cannot be
 * folded is a hard error. Returns false iff such an error occurred. */
bool cc_prune_program(arena_t *a, logger_t *log, const char *file, const cc_table_t *tbl,
                      ast_node_t *program);

#endif /* SALAM_CONDCOMP_CONDCOMP_H */
