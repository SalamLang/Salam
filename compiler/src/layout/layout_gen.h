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

#ifndef SALAM_LAYOUT_LAYOUT_GEN_H
#define SALAM_LAYOUT_LAYOUT_GEN_H

#include "core/prelude.h"
#include "core/arena.h"
#include "logger/logger.h"
#include "diag/diag.h"
#include "ast/ast.h"

typedef struct {
    const char *html;
    const char *css;
    const char *js;
    const char *title;
    const char *lang;
    const char *dir;
} layout_result_t;

void layout_localize_names(ast_node_t *node);

layout_result_t *layout_generate(arena_t *a, logger_t *log, diag_engine_t *diag,
                                 const char *file, ast_node_t *layout_block);

const char *layout_document(arena_t *a, const layout_result_t *r, bool inl,
                            const char *css_href, const char *js_href);

#endif /* SALAM_LAYOUT_LAYOUT_GEN_H */
