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

#include "codegen/codegen_internal.h"

void cg_emit_lambda(cg_t *cg, ast_node_t *n)
{
    if (n->name) return;
    const char *name = cg_fmt(cg, "_Salam_lambda_%d", cg->lam_n++);
    n->name = name;
    const char *cret = cg_ctype(cg, func_ret_of(n->type_str));
    size_t ncap = n->captures.len;
    
    sb_t params; sb_init(&params); sb_puts(&params, "void *__envp");
    { size_t i = 0; for (; i < n->list.len; i++) {
        ast_node_t *p = (ast_node_t *)n->list.data[i];
        sb_puts(&params, ", ");
        sb_puts(&params, cg_decl(cg, p->type_str, p->name));
    } }
    const char *ps = arena_strdup(cg->a, sb_cstr(&params)); sb_free(&params);
    
    if (ncap) {
        sb_t st; sb_init(&st);
        sb_puts(&st, cg_fmt(cg, "typedef struct { void *fn;"));
        { size_t i = 0; for (; i < ncap; i++) {
            ast_node_t *c = (ast_node_t *)n->captures.data[i];
            sb_puts(&st, cg_fmt(cg, " %s;", cg_decl(cg, c->type_str, c->name)));
        } }
        sb_puts(&st, cg_fmt(cg, " } %s_env;\n", name));
        sb_puts(cg->lam_decls, sb_cstr(&st)); sb_free(&st);
        sb_puts(cg->lam_decls, cg_fmt(cg, "static void *%s_make(", name));
        { size_t i = 0; for (; i < ncap; i++) {
            ast_node_t *c = (ast_node_t *)n->captures.data[i];
            if (i) sb_puts(cg->lam_decls, ", ");
            sb_puts(cg->lam_decls, cg_decl(cg, c->type_str, c->name));
        } }
        sb_puts(cg->lam_decls, ");\n");
    }
    sb_puts(cg->lam_decls, cg_fmt(cg, "static %s %s(%s);\n", cret, name, ps));
    
    sb_t      *saved_c = cg->c;        int        saved_indent = cg->indent;
    vec_t      saved_locals = cg->locals, saved_defers = cg->fn_defers;
    bool       saved_sret   = cg->cur_sret;
    symbol_t  *saved_struct = cg->cur_struct;
    ast_node_t *saved_lambda = cg->cur_lambda;
    cg->c = cg->lam_defs; cg->indent = 0;
    vec_init(&cg->locals); vec_init(&cg->fn_defers);
    cg->cur_sret = false; cg->cur_struct = NULL; cg->cur_lambda = n;
    { size_t i = 0; for (; i < n->list.len; i++)
        local_add(cg, ((ast_node_t *)n->list.data[i])->name); }
    cg_line(cg, "static %s %s(%s) {", cret, name, ps);
    cg->indent++;
    if (ncap)
        cg_line(cg, "%s_env *__env = (%s_env *)__envp; (void)__env;", name, name);
    else
        cg_line(cg, "(void)__envp;");
    if (n->a) cg_block(cg, n->a);
    cg_emit_defers(cg);
    cg->indent--;
    cg_line(cg, "}");
    sb_putc(cg->c, '\n');
    
    if (ncap) {
        sb_t mp; sb_init(&mp);
        { size_t i = 0; for (; i < ncap; i++) {
            ast_node_t *c = (ast_node_t *)n->captures.data[i];
            if (i) sb_puts(&mp, ", ");
            sb_puts(&mp, cg_decl(cg, c->type_str, c->name));
        } }
        const char *mps = arena_strdup(cg->a, sb_cstr(&mp)); sb_free(&mp);
        cg_line(cg, "static void *%s_make(%s) {", name, mps);
        cg->indent++;
        cg_line(cg, "%s_env *e = (%s_env *)salam_alloc(sizeof(%s_env));", name, name, name);
        cg_line(cg, "e->fn = (void*)&%s;", name);
        { size_t i = 0; for (; i < ncap; i++) {
            ast_node_t *c = (ast_node_t *)n->captures.data[i];
            cg_line(cg, "e->%s = %s;", cg_cident(cg, c->name), cg_cident(cg, c->name));
        } }
        cg_line(cg, "return e;");
        cg->indent--;
        cg_line(cg, "}");
    } else {
        cg_line(cg, "static void *%s_env = (void*)&%s;", name, name);
    }
    sb_putc(cg->c, '\n');
    cg->c = saved_c; cg->indent = saved_indent; cg->locals = saved_locals;
    cg->fn_defers = saved_defers; cg->cur_sret = saved_sret; cg->cur_struct = saved_struct;
    cg->cur_lambda = saved_lambda;
}

const char *cg_lambda_value(cg_t *cg, ast_node_t *n)
{
    if (!n->name) cg_emit_lambda(cg, n);
    if (n->captures.len == 0)
        return cg_fmt(cg, "(void*)&%s_env", n->name);
    sb_t a; sb_init(&a);
    { size_t i = 0; for (; i < n->captures.len; i++) {
        if (i) sb_puts(&a, ", ");
        sb_puts(&a, cg_expr(cg, (ast_node_t *)n->captures.data[i]));
    } }
    const char *args = arena_strdup(cg->a, sb_cstr(&a)); sb_free(&a);
    return cg_fmt(cg, "%s_make(%s)", n->name, args);
}

void cg_lift_walk(cg_t *cg, ast_node_t *node)
{
    if (!node) return;
    if (node->kind == AST_LAMBDA) { cg_lift_walk(cg, node->a); cg_emit_lambda(cg, node); return; }
    cg_lift_walk(cg, node->a); cg_lift_walk(cg, node->b);
    cg_lift_walk(cg, node->c); cg_lift_walk(cg, node->d);
    { size_t i = 0; for (; i < node->list.len; i++)
        cg_lift_walk(cg, (ast_node_t *)node->list.data[i]); }
}
