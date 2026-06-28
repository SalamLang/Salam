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

#ifndef SALAM_CODEGEN_PRINT_FMT_H
#define SALAM_CODEGEN_PRINT_FMT_H

#include "core/prelude.h"
#include "core/arena.h"
#include "core/vec.h"
#include "ast/ast.h"

typedef enum {
    PF_LIT = 0,
    PF_STR,
    PF_I32,
    PF_U32,
    PF_I64,
    PF_U64,
    PF_F64,
    PF_BOOL,
    PF_CHAR
} pf_kind_t;

typedef struct {
    pf_kind_t   kind;
    const char *text;
    ast_node_t *expr;
} pf_seg_t;

void pf_build(arena_t *a, ast_node_t *call, bool nl, vec_t *out);

const char *pf_spec(pf_kind_t k);

#endif /* SALAM_CODEGEN_PRINT_FMT_H */
