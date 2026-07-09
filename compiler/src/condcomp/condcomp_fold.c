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

#include "core/prelude.h"
#include "condcomp/condcomp.h"
#include "token/token.h"

typedef struct {
    arena_t *a;
    logger_t *log;
    const char *file;
    const cc_table_t *tbl;
} cc_ctx_t;

typedef struct {
    bool ok;
    bool bval;
} cc_bres_t;

typedef struct {
    bool ok;
    bool is_str;
    bool bval;
    const char *sval;
} cc_scalar_t;

/* `lenient`: when true, a bare identifier that is not in the compile-time
 * table folds to false instead of failing to fold. Safe ONLY for top-level
 * if conditions, which can never reference a real runtime symbol (top-level
 * code isn't statements), so there is no risk of masking a typo'd variable.
 * Function-body ifs are never lenient: an unresolvable identifier there must
 * be left unfolded so ordinary semantic analysis reports a real undefined-
 * symbol error instead of silently pruning a branch. */
static cc_bres_t cc_fold_bool(const cc_table_t *tbl, const ast_node_t *n, bool lenient);

static cc_bres_t cc_fold_defined_call(const cc_table_t *tbl, const ast_node_t *n)
{
    cc_bres_t fail = {false, false};
    if (!n->a || n->a->kind != AST_IDENTIFIER || !n->a->name ||
        strcmp(n->a->name, "defined") != 0)
        return fail;
    if (n->list.len != 1) return fail;
    {
        const ast_node_t *arg = (const ast_node_t *)n->list.data[0];
        if (arg->kind != AST_IDENTIFIER || !arg->name) return fail;
        {
            cc_bres_t r;
            r.ok = true;
            r.bval = cc_table_lookup(tbl, arg->name) != NULL;
            return r;
        }
    }
}

static cc_scalar_t cc_fold_scalar(const cc_table_t *tbl, const ast_node_t *n,
                                  bool lenient)
{
    cc_scalar_t fail = {false, false, false, NULL};
    if (!n) return fail;
    if (n->kind == AST_LITERAL) {
        if (n->value.kind == TV_BOOL) {
            cc_scalar_t r = {true, false, n->value.as.b, NULL};
            return r;
        }
        if (n->value.kind == TV_STRING) {
            cc_scalar_t r = {true, true, false, n->value.as.s};
            return r;
        }
        return fail;
    }
    if (n->kind == AST_IDENTIFIER) {
        const cc_const_t *c = cc_table_lookup(tbl, n->name);
        if (!c) {
            if (lenient) {
                cc_scalar_t r = {true, false, false, NULL};
                return r;
            }
            return fail;
        }
        if (c->kind == CC_BOOL) {
            cc_scalar_t r = {true, false, c->bval, NULL};
            return r;
        }
        {
            cc_scalar_t r = {true, true, false, c->sval};
            return r;
        }
    }
    if ((n->kind == AST_UNARY && n->op == TK_NOT) ||
        (n->kind == AST_BINARY && (n->op == TK_AND || n->op == TK_OR)) ||
        n->kind == AST_CALL) {
        cc_bres_t r = cc_fold_bool(tbl, n, lenient);
        if (!r.ok) return fail;
        {
            cc_scalar_t out = {true, false, r.bval, NULL};
            return out;
        }
    }
    return fail;
}

static cc_bres_t cc_fold_bool(const cc_table_t *tbl, const ast_node_t *n, bool lenient)
{
    cc_bres_t fail = {false, false};
    if (!n) return fail;
    if (n->kind == AST_UNARY && n->op == TK_NOT) {
        cc_bres_t r = cc_fold_bool(tbl, n->a, lenient);
        if (!r.ok) return fail;
        {
            cc_bres_t out = {true, !r.bval};
            return out;
        }
    }
    if (n->kind == AST_BINARY && (n->op == TK_AND || n->op == TK_OR)) {
        cc_bres_t l = cc_fold_bool(tbl, n->a, lenient);
        if (!l.ok) return fail;
        {
            cc_bres_t r = cc_fold_bool(tbl, n->b, lenient);
            if (!r.ok) return fail;
            {
                cc_bres_t out;
                out.ok = true;
                out.bval = (n->op == TK_AND) ? (l.bval && r.bval) : (l.bval || r.bval);
                return out;
            }
        }
    }
    if (n->kind == AST_BINARY && (n->op == TK_EQ || n->op == TK_NE)) {
        cc_scalar_t l = cc_fold_scalar(tbl, n->a, lenient);
        cc_scalar_t r = cc_fold_scalar(tbl, n->b, lenient);
        if (!l.ok || !r.ok || l.is_str != r.is_str) return fail;
        {
            bool eq = l.is_str ? (strcmp(l.sval, r.sval) == 0) : (l.bval == r.bval);
            cc_bres_t out;
            out.ok = true;
            out.bval = (n->op == TK_EQ) ? eq : !eq;
            return out;
        }
    }
    if (n->kind == AST_CALL) return cc_fold_defined_call(tbl, n);
    {
        cc_scalar_t s = cc_fold_scalar(tbl, n, lenient);
        if (!s.ok || s.is_str) return fail;
        {
            cc_bres_t out = {true, s.bval};
            return out;
        }
    }
}

static void cc_process_list(cc_ctx_t *ctx, vec_t *list, bool top_level, bool *ok);
static ast_node_t *cc_resolve_slot(cc_ctx_t *ctx, ast_node_t *n, bool *ok);

static bool cc_cond_has_ref(const ast_node_t *n)
{
    if (!n) return false;
    if (n->kind == AST_IDENTIFIER || n->kind == AST_CALL) return true;
    if (n->kind == AST_UNARY && n->op == TK_NOT) return cc_cond_has_ref(n->a);
    if (n->kind == AST_BINARY &&
        (n->op == TK_AND || n->op == TK_OR || n->op == TK_EQ || n->op == TK_NE))
        return cc_cond_has_ref(n->a) || cc_cond_has_ref(n->b);
    return false;
}

static ast_node_t *cc_resolve_if(cc_ctx_t *ctx, ast_node_t *n, bool top_level, bool *ok)
{
    cc_bres_t r = {false, false};
    if (top_level || cc_cond_has_ref(n->a)) r = cc_fold_bool(ctx->tbl, n->a, top_level);
    if (r.ok) {
        ast_node_t *taken = r.bval ? n->b : n->c;
        if (!taken) return NULL;
        if (taken->kind == AST_IF) return cc_resolve_if(ctx, taken, top_level, ok);
        cc_process_list(ctx, &taken->list, top_level, ok);
        return taken;
    }
    if (top_level) {
        LOG_E_AT(ctx->log, PH_PARSER, ctx->file, n->a->span,
                 "top-level if condition must be a compile-time constant");
        *ok = false;
        return NULL;
    }
    cc_process_list(ctx, &n->b->list, false, ok);
    if (n->c) {
        if (n->c->kind == AST_IF)
            n->c = cc_resolve_if(ctx, n->c, false, ok);
        else
            cc_process_list(ctx, &n->c->list, false, ok);
    }
    return n;
}

static ast_node_t *cc_resolve_slot(cc_ctx_t *ctx, ast_node_t *n, bool *ok)
{
    if (!n) return NULL;
    if (n->kind == AST_IF) return cc_resolve_if(ctx, n, false, ok);
    if (n->kind == AST_BLOCK || n->kind == AST_PROGRAM) {
        cc_process_list(ctx, &n->list, false, ok);
        return n;
    }
    n->type = cc_resolve_slot(ctx, n->type, ok);
    n->a = cc_resolve_slot(ctx, n->a, ok);
    n->b = cc_resolve_slot(ctx, n->b, ok);
    n->c = cc_resolve_slot(ctx, n->c, ok);
    n->d = cc_resolve_slot(ctx, n->d, ok);
    {
        size_t i = 0;
        for (; i < n->list.len; i++)
            n->list.data[i] = cc_resolve_slot(ctx, (ast_node_t *)n->list.data[i], ok);
    }
    {
        size_t i = 0;
        for (; i < n->dims.len; i++)
            n->dims.data[i] = cc_resolve_slot(ctx, (ast_node_t *)n->dims.data[i], ok);
    }
    {
        size_t i = 0;
        for (; i < n->captures.len; i++)
            n->captures.data[i] =
                cc_resolve_slot(ctx, (ast_node_t *)n->captures.data[i], ok);
    }
    return n;
}

static void cc_process_list(cc_ctx_t *ctx, vec_t *list, bool top_level, bool *ok)
{
    vec_t out;
    vec_init(&out);
    {
        size_t i = 0;
        for (; i < list->len; i++) {
            ast_node_t *it = (ast_node_t *)list->data[i];
            if (it->kind == AST_IF) {
                ast_node_t *resolved = cc_resolve_if(ctx, it, top_level, ok);
                if (resolved) {
                    if (resolved->kind == AST_BLOCK) {
                        size_t j = 0;
                        for (; j < resolved->list.len; j++)
                            vec_push(ctx->a, &out, resolved->list.data[j]);
                    } else {
                        vec_push(ctx->a, &out, resolved);
                    }
                }
            } else {
                vec_push(ctx->a, &out, cc_resolve_slot(ctx, it, ok));
            }
        }
    }
    *list = out;
}

bool cc_prune_program(arena_t *a, logger_t *log, const char *file, const cc_table_t *tbl,
                      ast_node_t *program)
{
    cc_ctx_t ctx;
    bool ok = true;
    ctx.a = a;
    ctx.log = log;
    ctx.file = file;
    ctx.tbl = tbl;
    cc_process_list(&ctx, &program->list, true, &ok);
    return ok;
}
