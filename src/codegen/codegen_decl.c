#include "codegen/codegen_internal.h"

static bool sig_is_sret(func_sig_t *sig, bool is_main)
{
    return !is_main && sig && type_is_byval_agg(sig->ret);
}

const char *func_signature(cg_t *cg, ast_node_t *fn, symbol_t *owner, func_sig_t *sig,
                           bool is_main)
{
    bool sret = sig_is_sret(sig, is_main);
    const char *ret = is_main ? "int" : sret ? "void"
                     : cg_ctype(cg, type_to_string(cg->sem->tc, sig->ret));
    
    bool ti = owner && owner->kind == SYM_TYPEIMPL;
    const char *ti_ts = ti ? type_to_string(cg->sem->tc, owner->type) : NULL;
    
    bool is_instance = (owner && owner->generic_base) || fn->synthetic;
    const char *name = is_main ? "main"
                     : fn->is_extern ? fn->name
                     : ti ? cg_mangle_ti(cg, ti_ts, fn->name, &sig->params)
                     : is_instance ? cg_mangle_in(cg, "g", owner ? owner->name : NULL, fn->name, &sig->params)
                     : cg_mangle(cg, owner ? owner->name : NULL, fn->name, &sig->params);
    if (is_main) return "int main(int argc, char** argv)";
    bool exported = (owner != NULL || fn->is_extern || fn->is_pub) && !is_instance;
    sb_t b; sb_init(&b);
    if (is_instance)    sb_puts(&b, "static inline ");
    else if (!exported) sb_puts(&b, "static ");
    sb_puts(&b, cg_fmt(cg, "%s %s(", ret, name));
    bool first = true;
    if (owner) {
        if (ti) sb_puts(&b, cg_fmt(cg, "%s this", cg_ctype(cg, ti_ts)));
        else    sb_puts(&b, cg_fmt(cg, "%s* this", cg_cident(cg, owner->name)));
        first = false;
    }
    { size_t i = 0; for (; i < fn->list.len; i++) {
        ast_node_t *p = (ast_node_t *)fn->list.data[i];
        const char *pts = type_to_string(cg->sem->tc, (type_t *)sig->params.data[i]);
        if (!first) sb_puts(&b, ", ");
        if (p->is_ref) sb_puts(&b, cg_fmt(cg, "%s* %s", cg_ctype(cg, pts), cg_cident(cg, p->name)));
        else           sb_puts(&b, cg_decl(cg, pts, p->name));
        first = false;
    } }
    if (sret) {
        if (!first) sb_puts(&b, ", ");
        sb_puts(&b, cg_fmt(cg, "%s* __ret", cg_ctype(cg, type_to_string(cg->sem->tc, sig->ret))));
        first = false;
    }
    if (first && !owner) sb_puts(&b, "void");
    sb_putc(&b, ')');
    const char *r = arena_strdup(cg->a, sb_cstr(&b)); sb_free(&b); return r;
}

const char *cg_extern_proto(cg_t *cg, ast_node_t *fn, func_sig_t *sig)
{
    const char *ret = cg_ctype(cg, type_to_string(cg->sem->tc, sig->ret));
    sb_t b; sb_init(&b);
    sb_puts(&b, cg_fmt(cg, "extern %s %s(", ret, fn->name));
    bool first = true;
    { size_t i = 0; for (; i < fn->list.len; i++) {
        ast_node_t *p = (ast_node_t *)fn->list.data[i];
        const char *pts = type_to_string(cg->sem->tc, (type_t *)sig->params.data[i]);
        if (!first) sb_puts(&b, ", ");
        if (p->is_ref) sb_puts(&b, cg_fmt(cg, "%s* %s", cg_ctype(cg, pts), cg_cident(cg, p->name)));
        else           sb_puts(&b, cg_decl(cg, pts, p->name));
        first = false;
    } }
    if (fn->is_variadic) { if (!first) sb_puts(&b, ", "); sb_puts(&b, "..."); first = false; }
    if (first) sb_puts(&b, "void");
    sb_putc(&b, ')');
    const char *r = arena_strdup(cg->a, sb_cstr(&b)); sb_free(&b); return r;
}

void cg_function(cg_t *cg, ast_node_t *fn, symbol_t *owner)
{
    symbol_t *fsym = owner ? scope_lookup_local(owner->members, fn->name)
                           : scope_lookup_local(cg->sem->global, fn->name);
    func_sig_t *sig = sig_of_decl(fsym, fn);
    if (!sig) return;
    bool is_main = (!owner && strcmp(fn->name, cg->entry) == 0);
    LOG_T(cg->log, PH_CODEGEN, "emit function %s%s", owner ? owner->name : "", fn->name);
    cg->cur_struct = owner;
    cg->cur_sret = sig_is_sret(sig, is_main);
    cg->locals.len = 0;
    if (owner) local_add(cg, "this");
    { size_t i = 0; for (; i < fn->list.len; i++)
        local_add(cg, ((ast_node_t *)fn->list.data[i])->name); }
    vec_t saved_defers = cg->fn_defers;
    vec_init(&cg->fn_defers);
    cg_source_line(cg, &fn->span);
    cg_line(cg, "%s {", func_signature(cg, fn, owner, sig, is_main));
    cg->indent++;
    if (is_main) {
        cg_line(cg, "salam_set_args(argc, argv);");
        { size_t i = 0; for (; i < cg->deferred.len; i++)
            cg_line(cg, "%s", (const char *)cg->deferred.data[i]); }
    }
    if (fn->a) cg_block(cg, fn->a);
    cg_emit_defers(cg);
    if (is_main && sig->ret->kind == TY_VOID) cg_line(cg, "return 0;");
    cg->indent--;
    cg_line(cg, "}");
    sb_putc(cg->c, '\n');
    cg->cur_struct = NULL;
    cg->cur_sret = false;
    cg->fn_defers = saved_defers;
}
