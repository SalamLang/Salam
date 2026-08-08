/*
 * Salam Programming Language (2024-2026)
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

/* Definite-assignment analysis.
 *
 * `x: int` declares a variable without giving it a value. In C, reading such a
 * variable before something assigns to it is undefined behaviour:
 *
 *     int x;
 *     int y = x;   // UB
 *
 * Salam rejects that shape outright. This pass walks every function body in
 * flow order and reports any read of a local that is not preceded, on every
 * path reaching it, by an assignment.
 *
 * The walk is structural rather than a real CFG:
 *   - each arm of an `if` / `match` is analyzed from the state at the join's
 *     entry, and the arm states are intersected afterwards;
 *   - an arm that cannot fall through (it ends in `ret`/`break`/`continue`)
 *     contributes nothing to the intersection;
 *   - a loop body is analyzed from the pre-loop state and its assignments are
 *     discarded afterwards, because the body may run zero times;
 *   - `defer` and lambda bodies are skipped: they run later, by which point
 *     the variable may well have been assigned.
 *
 * A variable handed to a `&:` reference parameter counts as assigned - the
 * callee writes through it. sema_call.c's mark_ref_args() stamps `ref_arg` on
 * those argument nodes so this pass can see them without redoing overload
 * resolution.
 */

#include "core/prelude.h"
#include "semantic/sema_internal.h"

typedef struct {
    const char *name;
    ast_node_t *decl;
    bool inited;
} di_var_t;

typedef struct {
    sema_t *s;
    vec_t vars; /* di_var_t*, innermost last; a scope pops back to its mark */
} di_t;

static void di_stmt(di_t *d, ast_node_t *n);

static void di_expr(di_t *d, ast_node_t *n);

static di_var_t *di_at(di_t *d, size_t i)
{
    return (di_var_t *)d->vars.data[i];
}

static di_var_t *di_find(di_t *d, const char *name)
{
    size_t i = d->vars.len;
    if (!name) return NULL;
    for (; i > 0; i--) {
        di_var_t *v = di_at(d, i - 1);
        if (v->name && strcmp(v->name, name) == 0) return v;
    }
    return NULL;
}

/* Every declaration is tracked, initialized or not, so that an inner binding
 * shadows an outer one of the same name instead of aliasing it. */
static void di_declare(di_t *d, ast_node_t *decl)
{
    di_var_t *v = (di_var_t *)arena_alloc(d->s->a, sizeof(*v));
    v->name = decl->name;
    v->decl = decl;
    v->inited = decl->a != NULL;
    vec_push(d->s->a, &d->vars, v);
}

static void di_mark(di_t *d, const char *name)
{
    di_var_t *v = di_find(d, name);
    if (v) v->inited = true;
}

static void di_read(di_t *d, ast_node_t *id)
{
    di_var_t *v = di_find(d, id->name);
    if (!v || v->inited) return;
    SERR(d->s, 89, &id->span,
         "'%s' is read before it is given a value (declared on line %u); give it a "
         "value with ':=', or assign to it on every path that reaches here",
         v->name, v->decl ? v->decl->span.begin.line : 0);
    v->inited = true; /* one report per variable, not one per read */
}

static unsigned char *di_snap(di_t *d, size_t n)
{
    unsigned char *buf = (unsigned char *)arena_alloc(d->s->a, n ? n : 1);
    size_t i = 0;
    for (; i < n; i++) buf[i] = di_at(d, i)->inited ? 1 : 0;
    return buf;
}

static void di_load(di_t *d, const unsigned char *buf, size_t n)
{
    size_t i = 0;
    for (; i < n; i++) di_at(d, i)->inited = buf[i] != 0;
}

/* Analyze one branch from `base`, restore the scope depth, and report the
 * state it ends in through `out`. */
static void di_arm(di_t *d, ast_node_t *stmt, const unsigned char *base,
                   unsigned char *out, size_t n)
{
    size_t i = 0;
    di_load(d, base, n);
    di_stmt(d, stmt);
    d->vars.len = n;
    for (; i < n; i++) out[i] = di_at(d, i)->inited ? 1 : 0;
}

static void di_commit(di_t *d, const unsigned char *base, const unsigned char *acc,
                      size_t n, bool any_live)
{
    size_t i = 0;
    di_load(d, base, n);
    if (!any_live) return;
    for (; i < n; i++)
        if (acc[i]) di_at(d, i)->inited = true;
}

static void di_if(di_t *d, ast_node_t *n)
{
    size_t nv, i = 0;
    unsigned char *base, *th, *el;
    bool th_live, el_live;

    di_expr(d, n->a);
    nv = d->vars.len;
    base = di_snap(d, nv);
    th = di_snap(d, nv);
    el = di_snap(d, nv);

    di_arm(d, n->b, base, th, nv);
    th_live = !sema_stmt_terminates(d->s, n->b);
    if (n->c) {
        di_arm(d, n->c, base, el, nv);
        el_live = !sema_stmt_terminates(d->s, n->c);
    } else {
        el_live = true; /* the implicit empty else assigns nothing */
        for (; i < nv; i++) el[i] = base[i];
    }

    di_load(d, base, nv);
    for (i = 0; i < nv; i++) {
        bool got = th_live && el_live ? (th[i] && el[i])
                   : th_live          ? th[i] != 0
                   : el_live          ? el[i] != 0
                                      : false;
        if (got) di_at(d, i)->inited = true;
    }
}

static void di_match(di_t *d, ast_node_t *n)
{
    size_t nv, i;
    unsigned char *base, *acc, *st;
    bool has_else = false, any_live = false;

    di_expr(d, n->a);
    nv = d->vars.len;
    base = di_snap(d, nv);
    acc = di_snap(d, nv);
    st = di_snap(d, nv);
    for (i = 0; i < nv; i++) acc[i] = 1;

    for (i = 0; i < n->list.len; i++) {
        ast_node_t *arm = (ast_node_t *)n->list.data[i];
        size_t j = 0;
        if (arm->op == TK_KW_ELSE) has_else = true;
        di_arm(d, arm->b, base, st, nv);
        if (sema_stmt_terminates(d->s, arm->b)) continue;
        any_live = true;
        for (; j < nv; j++) acc[j] = (unsigned char)(acc[j] && st[j]);
    }
    /* Without an else arm the match may match nothing, so nothing it assigns
     * is guaranteed. */
    di_commit(d, base, acc, nv, has_else && any_live);
}

/* The body of a loop may run zero times, so whatever it assigns is not
 * guaranteed afterwards. Reads inside it still see the pre-loop state, which
 * is what a first iteration would see. */
static void di_loop(di_t *d, ast_node_t *cond, ast_node_t *body, ast_node_t *post)
{
    size_t nv;
    unsigned char *base;
    di_expr(d, cond);
    nv = d->vars.len;
    base = di_snap(d, nv);
    di_stmt(d, body);
    d->vars.len = nv;
    di_stmt(d, post);
    d->vars.len = nv;
    di_load(d, base, nv);
}

static void di_for(di_t *d, ast_node_t *n)
{
    size_t mark = d->vars.len;
    di_stmt(d, n->a); /* the init runs exactly once, so it does carry over */
    di_loop(d, n->b, n->d, n->c);
    d->vars.len = mark;
}

static void di_repeat(di_t *d, ast_node_t *n)
{
    di_expr(d, n->a);
    di_expr(d, n->c);
    di_expr(d, n->d);
    di_loop(d, NULL, n->b, NULL);
}

static void di_call(di_t *d, ast_node_t *n)
{
    size_t i = 0;
    di_expr(d, n->a);
    for (; i < n->list.len; i++) {
        ast_node_t *arg = (ast_node_t *)n->list.data[i];
        if (arg && arg->kind == AST_IDENTIFIER && arg->ref_arg) {
            di_mark(d, arg->name); /* a `&:` parameter: the callee writes it */
            continue;
        }
        di_expr(d, arg);
    }
}

static void di_assign(di_t *d, ast_node_t *n)
{
    ast_node_t *t = n->a;
    di_expr(d, n->b);
    if (t && t->kind == AST_IDENTIFIER) {
        if (n->op != TK_ASSIGN) di_read(d, t); /* `x += 1` reads x first */
        di_mark(d, t->name);
        return;
    }
    /* `x.f = v` / `x[i] = v` read x to find the slot, so x must already hold a
     * value; assigning one field does not make the rest of it defined. */
    di_expr(d, t);
}

static void di_block(di_t *d, ast_node_t *n)
{
    size_t mark = d->vars.len, i = 0;
    for (; i < n->list.len; i++) di_stmt(d, (ast_node_t *)n->list.data[i]);
    d->vars.len = mark;
}

static void di_expr(di_t *d, ast_node_t *n)
{
    size_t i = 0;
    if (!n) return;
    switch (n->kind) {
    case AST_IDENTIFIER:
        di_read(d, n);
        return;
    case AST_LAMBDA:
    case AST_FUNC_ADDR:
        return; /* the body runs later, possibly after the assignment */
    case AST_CALL:
        di_call(d, n);
        return;
    case AST_MATCH:
        di_match(d, n);
        return;
    case AST_BLOCK:
        di_block(d, n);
        return;
    default:
        break;
    }
    di_expr(d, n->a);
    di_expr(d, n->b);
    di_expr(d, n->c);
    di_expr(d, n->d);
    for (; i < n->list.len; i++) di_expr(d, (ast_node_t *)n->list.data[i]);
}

static void di_stmt(di_t *d, ast_node_t *n)
{
    if (!n) return;
    switch (n->kind) {
    case AST_BLOCK:
        di_block(d, n);
        break;
    case AST_VAR_DECL:
    case AST_CONST_DECL:
        di_expr(d, n->a);
        di_declare(d, n);
        break;
    case AST_ASSIGN:
        di_assign(d, n);
        break;
    case AST_IF:
        di_if(d, n);
        break;
    case AST_MATCH:
        di_match(d, n);
        break;
    case AST_UNTIL:
        di_loop(d, n->a, n->b, NULL);
        break;
    case AST_FOR:
        di_for(d, n);
        break;
    case AST_REPEAT:
        di_repeat(d, n);
        break;
    case AST_EACH:
        di_expr(d, n->a);
        di_loop(d, NULL, n->b, NULL);
        break;
    case AST_DEFER: /* runs at function exit, after any later assignment */
    case AST_BREAK:
    case AST_CONTINUE:
        break;
    default:
        di_expr(d, n);
        break;
    }
}

void sema_check_definite_init(sema_t *s, ast_node_t *fn)
{
    di_t d;
    if (!fn || !fn->a) return;
    d.s = s;
    vec_init(&d.vars);
    di_stmt(&d, fn->a);
}
