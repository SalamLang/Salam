#ifndef SALAM_CODEGEN_PRINT_FMT_H
#define SALAM_CODEGEN_PRINT_FMT_H
#include "core/prelude.h"
#include "core/arena.h"
#include "core/vec.h"
#include "ast/ast.h"
typedef enum {
    PF_LIT = 0,  /* literal text: separators, newline, and folded constants    */
    PF_STR,      /* runtime str        -> %s                                    */
    PF_I32,      /* signed   <= 32-bit -> %d                                    */
    PF_U32,      /* unsigned <= 32-bit -> %u                                    */
    PF_I64,      /* signed   64-bit    -> %lld                                  */
    PF_U64,      /* unsigned 64-bit    -> %llu                                  */
    PF_F64,      /* f32/f64            -> %g                                    */
    PF_BOOL,     /* bool               -> %s with (cond ? "true" : "false")     */
    PF_CHAR      /* char (codepoint)   -> %c (backend-specific; see notes)      */
} pf_kind_t;
typedef struct {
    pf_kind_t   kind;
    const char *text;   /* PF_LIT only: the raw (un-escaped) literal bytes      */
    ast_node_t *expr;   /* runtime kinds: the argument expression to evaluate   */
} pf_seg_t;
void pf_build(arena_t *a, ast_node_t *call, bool nl, vec_t *out);
const char *pf_spec(pf_kind_t k);
#endif /* SALAM_CODEGEN_PRINT_FMT_H */
