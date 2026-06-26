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

#ifndef SALAM_SEMANTIC_SEMA_H
#define SALAM_SEMANTIC_SEMA_H

#include "core/prelude.h"
#include "core/arena.h"
#include "logger/logger.h"
#include "ast/ast.h"
#include "xml/xml.h"
#include "semantic/types.h"
#include "semantic/symbol.h"

typedef struct {
    scope_t    *global;
    type_ctx_t *tc;
    bool        ok;
    size_t      errors;
    size_t      warnings;
    vec_t       packages;   /* symbol_t* SYM_PACKAGE, one per loaded module (deduped
                             * by path). The tree-walking interpreter executes these
                             * packages' AST directly instead of reimplementing the
                             * stdlib in C; codegen/LLVM reach the same .salam source. */
} sema_result_t;

sema_result_t *sema_run(arena_t *a, logger_t *log, ast_node_t *program, const char *file);

void symbols_to_xml(xml_writer_t *w, const sema_result_t *r);

const char *salam_resolve_import(arena_t *a, const char *dir, const char *spec);

void salam_set_stdlib_root(const char *root);

const char *salam_get_stdlib_root(void);

#endif /* SALAM_SEMANTIC_SEMA_H */
