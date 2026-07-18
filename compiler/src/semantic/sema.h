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
#include "condcomp/condcomp.h"

typedef struct {
    scope_t *global;
    type_ctx_t *tc;
    bool ok;
    size_t errors;
    size_t warnings;
    vec_t diagnostics; /* diag_t* items from the semantic diag engine */
    vec_t packages;
} sema_result_t;

sema_result_t *sema_run(arena_t *a, logger_t *log, ast_node_t *program, const char *file,
                        const char *lang, const cc_table_t *cc);

/* Same as sema_run(), but reuses/populates a caller-owned package cache
 * (path -> symbol_t* pairs) across multiple calls instead of reloading and
 * re-analyzing shared stdlib imports from scratch each time. All calls that
 * share a cache must use the same arena `a` for the lifetime of that cache.
 * Pass NULL for the same behavior as sema_run(). */
sema_result_t *sema_run_cached(arena_t *a, logger_t *log, ast_node_t *program,
                               const char *file, const char *lang, const cc_table_t *cc,
                               vec_t *shared_pkg_cache);

void symbols_to_xml(xml_writer_t *w, const sema_result_t *r);

const char *salam_resolve_import(arena_t *a, const char *dir, const char *spec);

const char *salam_resolve_import_node(arena_t *a, const char *dir, const ast_node_t *imp,
                                      const char *lang);

int salam_package_files(arena_t *a, const char *main_path, const char **out, int max);

void salam_merge_program(arena_t *a, ast_node_t *dst, ast_node_t *src);

void salam_set_stdlib_root(const char *root);

const char *salam_get_stdlib_root(void);

bool salam_find_bundled_tool(const char *name, char *out, size_t n);

bool salam_find_sysroot(const char *triple, char *out, size_t n);

#endif /* SALAM_SEMANTIC_SEMA_H */
