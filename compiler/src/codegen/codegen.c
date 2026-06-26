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

static void emit_private_protos(cg_t *cg, ast_node_t *program)
{
    { size_t i = 0; for (; i < program->list.len; i++) {
        ast_node_t *d = (ast_node_t *)program->list.data[i];
        if (d->typarams.len > 0) continue;
        if (d->synthetic) continue;   
        if (d->kind == AST_FUNC_DEF && !d->is_extern && strcmp(d->name, cg->entry) != 0 &&
            !d->is_pub) {
            symbol_t *fsym = scope_lookup_local(cg->sem->global, d->name);
            func_sig_t *sig = sig_of_decl(fsym, d);
            if (sig) sb_puts(cg->c, cg_fmt(cg, "%s;\n", func_signature(cg, d, NULL, sig, false)));
        }
    } }
    sb_puts(cg->c, "\n");
}

static void emit_globals(cg_t *cg, ast_node_t *program)
{
    { size_t i = 0; for (; i < program->list.len; i++) {
        ast_node_t *d = (ast_node_t *)program->list.data[i];
        if (d->kind != AST_CONST_DECL && d->kind != AST_VAR_DECL) continue;
        if (d->is_extern) continue;   
        const char *ts = d->type_str ? d->type_str : "int32_t";
        const char *decl = cg_decl(cg, ts, d->name);
        bool is_array = ts && strchr(ts, '[');
        bool can_defer = d->kind == AST_VAR_DECL && d->a && d->a->kind != AST_LITERAL && !is_array;
        if (can_defer) {
            sb_puts(cg->c, cg_fmt(cg, "%s;\n", decl));   
            vec_push(cg->a, &cg->deferred, CONST_CAST(cg_fmt(cg, "%s = %s;",
                     cg_cident(cg, d->name), cg_expr(cg, d->a))));
        } else {
            bool want_const = (d->kind == AST_CONST_DECL || !d->is_mut);
            bool gct_const  = want_const &&
                              (strncmp(cg_ctype(cg, ts), "const ", 6) == 0);
            const char *pfx = (want_const && !gct_const) ? "const " : "";
            if (d->a) sb_puts(cg->c, cg_fmt(cg, "%s%s = %s;\n", pfx, decl, cg_expr(cg, d->a)));
            else      sb_puts(cg->c, cg_fmt(cg, "%s%s;\n", pfx, decl));
        }
    } }
    sb_puts(cg->c, "\n");
}

static void emit_impl_protos(cg_t *cg)
{
    { size_t i = 0; for (; i < cg->sem->global->symbols.len; i++) {
        symbol_t *owner = (symbol_t *)cg->sem->global->symbols.data[i];
        if (owner->kind != SYM_TYPEIMPL || !owner->members) continue;
        { size_t j = 0; for (; j < owner->members->symbols.len; j++) {
            symbol_t *m = (symbol_t *)owner->members->symbols.data[j];
            if (m->kind != SYM_METHOD) continue;
            { size_t k = 0; for (; k < m->overloads.len; k++) {
                func_sig_t *sig = (func_sig_t *)m->overloads.data[k];
                if (sig && sig->decl)
                    sb_puts(cg->c, cg_fmt(cg, "%s;\n", func_signature(cg, sig->decl, owner, sig, false)));
            } }
        } }
    } }
}

static void emit_impl_bodies(cg_t *cg)
{
    { size_t i = 0; for (; i < cg->sem->global->symbols.len; i++) {
        symbol_t *owner = (symbol_t *)cg->sem->global->symbols.data[i];
        if (owner->kind != SYM_TYPEIMPL || !owner->members) continue;
        { size_t j = 0; for (; j < owner->members->symbols.len; j++) {
            symbol_t *m = (symbol_t *)owner->members->symbols.data[j];
            if (m->kind != SYM_METHOD) continue;
            { size_t k = 0; for (; k < m->overloads.len; k++) {
                func_sig_t *sig = (func_sig_t *)m->overloads.data[k];
                if (sig && sig->decl) cg_function(cg, sig->decl, owner);
            } }
        } }
    } }
}

static void emit_function_bodies(cg_t *cg, ast_node_t *program)
{
    { size_t i = 0; for (; i < program->list.len; i++) {
        ast_node_t *d = (ast_node_t *)program->list.data[i];
        if (d->typarams.len > 0) continue;
        if (d->synthetic) continue;   
        if (d->kind == AST_FUNC_DEF) {
            if (d->is_extern && !d->a) continue;
            cg_function(cg, d, NULL);
        } else if (d->kind == AST_STRUCT_DEF) {
            symbol_t *ssym = scope_lookup_local(cg->sem->global, d->name);
            { size_t j = 0; for (; j < d->list.len; j++) {
                ast_node_t *m = (ast_node_t *)d->list.data[j];
                if (m->kind == AST_FUNC_DEF) cg_function(cg, m, ssym);
            } }
        } else if (d->kind == AST_LAYOUT_BLOCK) {
            sb_puts(cg->c, "/* layout block omitted in general (C) mode */\n\n");
        }
    } }
}

static const char *inst_fn_name(cg_t *cg, symbol_t *owner, ast_node_t *fn, func_sig_t *sig)
{
    return cg_mangle_in(cg, "g", owner ? owner->name : NULL, fn->name, &sig->params);
}

static void emit_instances_header(cg_t *cg, ast_node_t *program)
{
    sb_t *h = cg->h;
    
    { size_t i = 0; for (; i < program->list.len; i++) {
        ast_node_t *d = (ast_node_t *)program->list.data[i];
        if (!d->synthetic) continue;
        if (d->kind == AST_FUNC_DEF && !d->is_extern) {
            symbol_t *fsym = scope_lookup_local(cg->sem->global, d->name);
            func_sig_t *sig = sig_of_decl(fsym, d);
            if (!sig) continue;
            const char *nm = inst_fn_name(cg, NULL, d, sig);
            sb_puts(h, cg_fmt(cg, "#ifndef SALAM_IP_%s\n#define SALAM_IP_%s\n%s;\n#endif\n",
                              nm, nm, func_signature(cg, d, NULL, sig, false)));
        } else if (d->kind == AST_STRUCT_DEF) {
            symbol_t *ssym = scope_lookup_local(cg->sem->global, d->name);
            if (!ssym) continue;
            { size_t j = 0; for (; j < d->list.len; j++) {
                ast_node_t *m = (ast_node_t *)d->list.data[j];
                if (m->kind != AST_FUNC_DEF) continue;
                symbol_t *msym = scope_lookup_local(ssym->members, m->name);
                func_sig_t *sig = sig_of_decl(msym, m);
                if (!sig) continue;
                const char *nm = inst_fn_name(cg, ssym, m, sig);
                sb_puts(h, cg_fmt(cg, "#ifndef SALAM_IP_%s\n#define SALAM_IP_%s\n%s;\n#endif\n",
                                  nm, nm, func_signature(cg, m, ssym, sig, false)));
            } }
        }
    } }
    
    sb_t *savec = cg->c; cg->c = h;
    { size_t i = 0; for (; i < program->list.len; i++) {
        ast_node_t *d = (ast_node_t *)program->list.data[i];
        if (!d->synthetic) continue;
        if (d->kind == AST_FUNC_DEF && !d->is_extern && d->a) {
            symbol_t *fsym = scope_lookup_local(cg->sem->global, d->name);
            func_sig_t *sig = sig_of_decl(fsym, d);
            if (!sig) continue;
            const char *nm = inst_fn_name(cg, NULL, d, sig);
            sb_puts(h, cg_fmt(cg, "#ifndef SALAM_IB_%s\n#define SALAM_IB_%s\n", nm, nm));
            cg_function(cg, d, NULL);
            sb_puts(h, "#endif\n");
        } else if (d->kind == AST_STRUCT_DEF) {
            symbol_t *ssym = scope_lookup_local(cg->sem->global, d->name);
            if (!ssym) continue;
            { size_t j = 0; for (; j < d->list.len; j++) {
                ast_node_t *m = (ast_node_t *)d->list.data[j];
                if (m->kind != AST_FUNC_DEF) continue;
                symbol_t *msym = scope_lookup_local(ssym->members, m->name);
                func_sig_t *sig = sig_of_decl(msym, m);
                if (!sig) continue;
                const char *nm = inst_fn_name(cg, ssym, m, sig);
                sb_puts(h, cg_fmt(cg, "#ifndef SALAM_IB_%s\n#define SALAM_IB_%s\n", nm, nm));
                cg_function(cg, m, ssym);
                sb_puts(h, "#endif\n");
            } }
        }
    } }
    cg->c = savec;
}

static const char *cg_tidy(arena_t *a, const char *src)
{
    sb_t out; sb_init(&out);
    bool emitted = false;       
    bool pending_blank = false; 
    { const char *p = src; for (; *p; ) {
        const char *nl = strchr(p, '\n');
        size_t len = nl ? (size_t)(nl - p) : strlen(p);
        size_t end = len;
        while (end > 0 && (p[end-1] == '\r' || p[end-1] == ' ' || p[end-1] == '\t')) end--;
        if (end == 0) {                         
            if (emitted) pending_blank = true;  
        } else {
            if (pending_blank) { sb_putc(&out, '\n'); pending_blank = false; }
            { size_t i = 0; for (; i < end; i++) sb_putc(&out, p[i]); }
            sb_putc(&out, '\n');
            emitted = true;
        }
        if (!nl) break;
        p = nl + 1;
    } }
    const char *r = arena_strdup(a, sb_cstr(&out)); sb_free(&out); return r;
}

codegen_output_t *codegen_run(arena_t *a, logger_t *log, ast_node_t *program,
                              sema_result_t *sem, const char *module, bool safe,
                              bool debug_info, const char *src_path,
                              const char *entry)
{
    cg_t cg; memset(&cg, 0, sizeof(cg));
    cg.a = a; cg.log = log; cg.sem = sem; cg.module = module; cg.safe = safe;
    cg.debug_info = debug_info;
    cg.src_path = src_path ? src_path : "";
    cg.pkg = program->name ? program->name : "main";
    cg.entry = (entry && entry[0]) ? entry : "main";
    vec_init(&cg.locals);
    vec_init(&cg.vec_types);
    vec_init(&cg.dyn_ifaces);
    vec_init(&cg.dyn_impls);
    vec_init(&cg.deferred);
    vec_init(&cg.fn_defers);
    sb_t h, c, lamd, lamf; sb_init(&h); sb_init(&c); sb_init(&lamd); sb_init(&lamf);
    cg.h = &h; cg.c = &c;
    cg.lam_decls = &lamd; cg.lam_defs = &lamf; cg.lam_n = 0;
    LOG_I(log, PH_CODEGEN, "generating C for module '%s'", module);
    cg_header(&cg, program);
    emit_instances_header(&cg, program);   
    sb_puts(&h, "\n#endif\n");
    sb_puts(&c, cg_fmt(&cg, "#include \"%s%s.h\"\n\n", SALAM_MOD_PREFIX, module));
    
    cg_emit_dyn_vtables(&cg, &c);
    emit_private_protos(&cg, program);
    emit_impl_protos(&cg);

    cg_lift_walk(&cg, program);
    if (lamd.len) { sb_puts(&c, sb_cstr(&lamd)); sb_putc(&c, '\n'); }
    emit_globals(&cg, program);
    
    if (lamf.len) { sb_puts(&c, sb_cstr(&lamf)); sb_putc(&c, '\n'); }
    emit_function_bodies(&cg, program);
    emit_impl_bodies(&cg);
    codegen_output_t *out = (codegen_output_t *)arena_alloc(a, sizeof(*out));
    out->module = module;
    out->h_src = cg_tidy(a, sb_cstr(&h));
    out->c_src = cg_tidy(a, sb_cstr(&c));
    sb_free(&h); sb_free(&c);
    LOG_I(log, PH_CODEGEN, "generated %zu bytes of C", strlen(out->c_src));
    return out;
}
