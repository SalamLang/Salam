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

#include "jsgen/jsgen_internal.h"

static void collect_local_names(jg_t *g, ast_node_t *n)
{
    if (!n) return;
    if (n->name &&
        (n->kind == AST_PARAM || n->kind == AST_VAR_DECL || n->kind == AST_CONST_DECL ||
         n->kind == AST_REPEAT || n->kind == AST_EACH))
        vec_push(g->cg.a, &g->taken, CONST_CAST(n->name));
    if (n->kind == AST_EACH && n->c && n->c->name)
        vec_push(g->cg.a, &g->taken, CONST_CAST(n->c->name));
    collect_local_names(g, n->a);
    collect_local_names(g, n->b);
    collect_local_names(g, n->c);
    collect_local_names(g, n->d);
    {
        size_t i = 0;
        for (; i < n->list.len; i++)
            collect_local_names(g, (ast_node_t *)n->list.data[i]);
    }
}

static void census(jg_t *g, ast_node_t *program)
{
    size_t i = 0;
    for (; i < program->list.len; i++) {
        ast_node_t *d = (ast_node_t *)program->list.data[i];
        if (d->kind == AST_FUNC_DEF || d->kind == AST_STRUCT_DEF)
            collect_local_names(g, d);
    }
}

static void jsg_emit_globals(jg_t *g, ast_node_t *program)
{
    cg_t *cg = &g->cg;
    {
        size_t i = 0;
        for (; i < program->list.len; i++) {
            ast_node_t *d = (ast_node_t *)program->list.data[i];
            if (d->kind != AST_CONST_DECL && d->kind != AST_VAR_DECL) continue;
            if (d->is_extern) continue;
            {
                const char *ref = jsg_global_ref(g, cg->pkg, d->name);
                const char *kw = (d->kind == AST_CONST_DECL) ? "const" : "let";
                if (d->a)
                    cg_line(cg, "%s %s = %s;", kw, ref, jsg_expr(g, d->a));
                else
                    cg_line(cg, "%s %s = %s;", d->is_mut ? "let" : "const", ref,
                            jsg_zero(g, d->type_str));
            }
        }
    }
}

static void jsg_emit_one(jg_t *g, jsgen_output_t *out, ast_node_t *fn, symbol_t *owner)
{
    cg_t *cg = &g->cg;
    sb_t *saved = cg->c;
    sb_t body;
    sb_init(&body);
    cg->c = &body;
    g->last_fn_name = NULL;
    jsg_function(g, fn, owner);
    cg->c = saved;
    if (g->last_fn_name && body.len) {
        vec_push(cg->a, &out->fn_names, CONST_CAST(g->last_fn_name));
        vec_push(cg->a, &out->fn_texts, CONST_CAST(arena_strdup(cg->a, sb_cstr(&body))));
    }
    sb_free(&body);
}

static void jsg_emit_functions(jg_t *g, jsgen_output_t *out, ast_node_t *program)
{
    cg_t *cg = &g->cg;
    {
        size_t i = 0;
        for (; i < program->list.len; i++) {
            ast_node_t *d = (ast_node_t *)program->list.data[i];
            if (d->typarams.len > 0) continue;
            if (d->kind == AST_FUNC_DEF) {
                if (d->is_extern && !d->a) continue;
                jsg_emit_one(g, out, d, NULL);
            } else if (d->kind == AST_STRUCT_DEF) {
                symbol_t *ssym = scope_lookup_local(cg->sem->global, d->name);
                {
                    size_t j = 0;
                    for (; j < d->list.len; j++) {
                        ast_node_t *m = (ast_node_t *)d->list.data[j];
                        if (m->kind == AST_FUNC_DEF) jsg_emit_one(g, out, m, ssym);
                    }
                }
            }
        }
    }
}

static void jsg_emit_impls(jg_t *g, jsgen_output_t *out)
{
    cg_t *cg = &g->cg;
    {
        size_t i = 0;
        for (; i < cg->sem->global->symbols.len; i++) {
            symbol_t *owner = (symbol_t *)cg->sem->global->symbols.data[i];
            if (owner->kind != SYM_TYPEIMPL || !owner->members) continue;
            {
                size_t j = 0;
                for (; j < owner->members->symbols.len; j++) {
                    symbol_t *m = (symbol_t *)owner->members->symbols.data[j];
                    if (m->kind != SYM_METHOD) continue;
                    {
                        size_t k = 0;
                        for (; k < m->overloads.len; k++) {
                            func_sig_t *sig = (func_sig_t *)m->overloads.data[k];
                            if (sig && sig->decl) jsg_emit_one(g, out, sig->decl, owner);
                        }
                    }
                }
            }
        }
    }
}

jsgen_output_t *jsgen_run(arena_t *a, logger_t *log, ast_node_t *program,
                          sema_result_t *sem, const char *module, const char *entry)
{
    jg_t g;
    sb_t globals;
    jsgen_output_t *out = (jsgen_output_t *)arena_alloc(a, sizeof(*out));
    memset(&g, 0, sizeof(g));
    g.cg.a = a;
    g.cg.log = log;
    g.cg.sem = sem;
    g.cg.module = module;
    g.cg.pkg = program->name ? program->name : "main";
    g.cg.entry = (entry && entry[0]) ? entry : "main";
    vec_init(&g.cg.locals);
    vec_init(&g.cg.vec_types);
    vec_init(&g.cg.dyn_ifaces);
    vec_init(&g.cg.dyn_impls);
    vec_init(&g.cg.deferred);
    vec_init(&g.cg.fn_defers);
    vec_init(&g.local_muts);
    vec_init(&g.local_emit);
    vec_init(&g.taken);
    vec_init(&g.fn_used_names);
    memset(out, 0, sizeof(*out));
    out->module = module;
    vec_init(&out->fn_names);
    vec_init(&out->fn_texts);
    sb_init(&globals);
    g.cg.c = &globals;
    LOG_I(log, PH_CODEGEN, "generating JavaScript for module '%s'", module);
    census(&g, program);
    jsg_emit_globals(&g, program);
    jsg_emit_functions(&g, out, program);
    jsg_emit_impls(&g, out);
    out->globals_src = arena_strdup(a, sb_cstr(&globals));
    out->entry_mangled = g.entry_mangled;
    sb_free(&globals);
    LOG_I(log, PH_CODEGEN, "generated %zu function(s) for '%s'", out->fn_names.len,
          module);
    return out;
}
