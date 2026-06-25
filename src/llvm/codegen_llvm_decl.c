
#include "llvm/codegen_llvm_internal.h"

void ll_emit_struct_types(ll_t *ll, ast_node_t *program)
{
    { size_t i = 0; for (; i < program->list.len; i++) {
        ast_node_t *d = (ast_node_t *)program->list.data[i];
        if (d->kind != AST_STRUCT_DEF || d->typarams.len > 0) continue;
        symbol_t *ssym = ll_struct_sym(ll, d->name);
        if (!ssym || !ssym->members) continue;
        const char *sname = ssym->type ? type_to_string(ll->sem->tc, ssym->type) : d->name;
        sb_t b; sb_init(&b);
        sb_puts(&b, ll_fmt(ll, "%s = type { ", ll_struct_ltype(ll, sname)));
        int nf = 0;
        { size_t j = 0; for (; j < ssym->members->symbols.len; j++) {
            symbol_t *f = (symbol_t *)ssym->members->symbols.data[j];
            if (f->kind != SYM_FIELD) continue;
            if (nf) sb_puts(&b, ", ");
            sb_puts(&b, ll_ty(ll, type_to_string(ll->sem->tc, f->type)));
            nf++;
        } }
        sb_puts(&b, " }\n");
        sb_puts(ll->g, sb_cstr(&b)); sb_free(&b);
    } }
    sb_puts(ll->g, "\n");
}

static void ll_put_ident(sb_t *b, const char *name)
{
    { const unsigned char *p = (const unsigned char *)name; for (; p && *p; p++) {
        if (isalnum(*p) || *p == '_') sb_putc(b, (char)*p);
        else { char h[5]; snprintf(h, sizeof h, "_%02x", *p); sb_puts(b, h); }
    } }
}


static void ll_put_type_code(sb_t *b, const char *ts)
{
    { const unsigned char *p = (const unsigned char *)ts; for (; p && *p; p++) {
        if (*p == '*') sb_puts(b, "_ptr");
        else if (*p == '[') sb_puts(b, "_arr");
        else if (*p == ']' || *p == ' ') {}
        else if (isalnum(*p) || *p == '_') sb_putc(b, (char)*p);
        else { char h[5]; snprintf(h, sizeof h, "_%02x", *p); sb_puts(b, h); }
    } }
}

const char *ll_mangle(ll_t *ll, const char *owner, const char *fn, func_sig_t *sig)
{
    if (!owner && !strcmp(fn, ll->entry)) return "main";
    sb_t b; sb_init(&b); sb_puts(&b, "salam_");
    if (owner) { ll_put_ident(&b, owner); sb_putc(&b, '_'); }
    ll_put_ident(&b, fn);
    if (sig)
        { size_t i = 0; for (; i < sig->params.len; i++) {
            sb_putc(&b, '_');
            ll_put_type_code(&b, type_to_string(ll->sem->tc, (type_t *)sig->params.data[i]));
        } }
    const char *r = arena_strdup(ll->a, sb_cstr(&b)); sb_free(&b); return r;
}


func_sig_t *ll_pick_overload(ll_t *ll, symbol_t *sym, ast_node_t *call)
{
    func_sig_t *arity = NULL;
    { size_t i = 0; for (; i < sym->overloads.len; i++) {
        func_sig_t *sig = (func_sig_t *)sym->overloads.data[i];
        if (sig->params.len != call->list.len) continue;
        if (!arity) arity = sig;
        bool ok = true;
        { size_t j = 0; for (; j < sig->params.len && ok; j++) {
            ast_node_t *arg = (ast_node_t *)call->list.data[j];
            const char *pt = type_to_string(ll->sem->tc, (type_t *)sig->params.data[j]);
            if (!arg->type_str || strcmp(arg->type_str, pt) != 0) ok = false;
        } }
        if (ok) return sig;
    } }
    return arity ? arity : (sym->overloads.len ? (func_sig_t *)sym->overloads.data[0] : NULL);
}

static func_sig_t *ll_sig_of(ll_t *ll, ast_node_t *fn, symbol_t *owner)
{
    symbol_t *s = owner ? scope_lookup_local(owner->members, fn->name)
                        : ll_sym(ll, fn->name);
    if (!s) return NULL;
    { size_t i = 0; for (; i < s->overloads.len; i++) {
        func_sig_t *sig = (func_sig_t *)s->overloads.data[i];
        if (sig->decl == fn) return sig;
    } }
    return s->overloads.len ? (func_sig_t *)s->overloads.data[0] : NULL;
}

static const char *ll_fn_header(ll_t *ll, ast_node_t *fn, func_sig_t *sig,
                                const char *ret_lty, const char *fname, const char *recv_param)
{
    sb_t hdr; sb_init(&hdr);
    sb_puts(&hdr, ll_fmt(ll, "define %s @%s(", ret_lty, fname));
    bool first = true;
    if (recv_param) { sb_puts(&hdr, recv_param); first = false; }
    { size_t i = 0; for (; i < fn->list.len; i++) {
        ast_node_t *p = (ast_node_t *)fn->list.data[i];
        const char *pts = type_to_string(ll->sem->tc, (type_t *)sig->params.data[i]);
        if (!first) sb_puts(&hdr, ", ");
        sb_puts(&hdr, p->is_ref ? ll_fmt(ll, "ptr %%arg%zu", i)
                                : ll_fmt(ll, "%s %%arg%zu", ll_ty(ll, pts), i));
        first = false;
    } }
    sb_puts(&hdr, ")");
    if (ll->debug && ll->cur_sp) sb_puts(&hdr, ll_fmt(ll, " !dbg %s", ll->cur_sp));
    sb_puts(&hdr, " {\n");
    const char *r = arena_strdup(ll->a, sb_cstr(&hdr)); sb_free(&hdr); return r;
}

static void ll_spill_params(ll_t *ll, ast_node_t *fn, func_sig_t *sig)
{
    { size_t i = 0; for (; i < fn->list.len; i++) {
        ast_node_t *p = (ast_node_t *)fn->list.data[i];
        const char *pts = type_to_string(ll->sem->tc, (type_t *)sig->params.data[i]);
        if (p->is_ref) {
            ll_local_add(ll, p->name, ll_fmt(ll, "%%arg%zu", i), pts);
            continue;
        }
        const char *ptr = ll_fmt(ll, "%%p.%s", p->name);
        ll_emit(ll, "%s = alloca %s", ptr, ll_ty(ll, pts));
        ll_emit(ll, "store %s %%arg%zu, ptr %s", ll_ty(ll, pts), i, ptr);
        ll_local_add(ll, p->name, ptr, pts);
    } }
}

void ll_function(ll_t *ll, ast_node_t *fn, symbol_t *owner)
{
    if (!owner && fn->is_extern && !fn->a) return;   
    func_sig_t *sig = ll_sig_of(ll, fn, owner);
    if (!sig) return;
    bool is_main = !owner && !strcmp(fn->name, ll->entry);
    bool is_impl = owner && owner->kind == SYM_TYPEIMPL;  
    const char *rts     = type_to_string(ll->sem->tc, sig->ret);
    const char *ret_lty = is_main ? "i32" : ll_ty(ll, rts);
    
    const char *recv_ts = is_impl ? type_to_string(ll->sem->tc, owner->type)
                        : owner ? owner->name : NULL;
    
    const char *fname   = is_impl ? ll_mangle_ti(ll, recv_ts, fn->name, sig)
                        : (!owner && fn->is_extern) ? fn->name
                        : ll_mangle(ll, owner ? owner->name : NULL, fn->name, sig);
    const char *recv_param = !owner ? NULL
                           : is_impl ? ll_fmt(ll, "%s %%this", ll_ty(ll, recv_ts)) : "ptr %this";

    
    { size_t i = 0; for (; i < ll->emitted.len; i++)
        if (!strcmp(fname, (const char *)ll->emitted.data[i])) return; }
    vec_push(ll->a, &ll->emitted, CONST_CAST(fname));

    sb_t body; sb_init(&body);
    sb_t  *saved_b = ll->b;
    int    saved_tmp = ll->tmp, saved_lbl = ll->lbl, saved_nloop = ll->nloop;
    bool   saved_main = ll->is_main, saved_byval = ll->self_byval, saved_term = ll->term;
    const char *saved_ret = ll->ret_ts, *saved_sp = ll->cur_sp, *saved_dbg = ll->cur_dbg;
    const char *saved_self = ll->self_ts, *saved_this = ll->this_ref;
    vec_t saved_locals = ll->locals, saved_defers = ll->defers;
    ll->b = &body; ll->tmp = 0; ll->lbl = 0; ll->nloop = 0; ll->term = false;
    ll->ret_ts = rts; ll->is_main = is_main;
    ll->self_ts = recv_ts; ll->self_byval = is_impl;
    ll->this_ref = owner ? "%this" : NULL;
    vec_init(&ll->locals); vec_init(&ll->defers);
    if (ll->debug) {
        ll_debug_subprogram(ll, fn->name, fn->span.begin.line);
        ll->cur_dbg = ll_debug_location(ll, fn->span.begin.line, fn->span.begin.col);
    }
    const char *header = ll_fn_header(ll, fn, sig, ret_lty, fname, recv_param);
    ll_emit_label(ll, "entry");
    
    if (is_impl) {
        ll_emit(ll, "%%p.this = alloca %s", ll_ty(ll, recv_ts));
        ll_emit(ll, "store %s %%this, ptr %%p.this", ll_ty(ll, recv_ts));
        ll->this_ref = "%p.this";
    }
    ll_spill_params(ll, fn, sig);
    if (is_main) ll_emit_global_inits(ll);
    if (fn->a) ll_block(ll, fn->a);
    if (!ll->term) ll_emit_return(ll, NULL);
    sb_puts(ll->g, header);
    sb_puts(ll->g, sb_cstr(&body));
    sb_puts(ll->g, "}\n\n");
    sb_free(&body);
    ll->b = saved_b; ll->tmp = saved_tmp; ll->lbl = saved_lbl; ll->nloop = saved_nloop;
    ll->is_main = saved_main; ll->ret_ts = saved_ret; ll->self_byval = saved_byval;
    ll->term = saved_term; ll->cur_sp = saved_sp; ll->cur_dbg = saved_dbg;
    ll->self_ts = saved_self; ll->this_ref = saved_this;
    ll->locals = saved_locals; ll->defers = saved_defers;
}


void ll_ensure_fn(ll_t *ll, ast_node_t *fn, symbol_t *owner, scope_t *pscope)
{
    if (!fn) return;
    scope_t *saved = ll->pkg_scope;
    ll->pkg_scope = pscope;
    ll_function(ll, fn, owner);
    ll->pkg_scope = saved;
}


static const char *ll_vtbl_name(ll_t *ll, const char *iface, const char *concrete)
{
    sb_t b; sb_init(&b); sb_puts(&b, "@vtbl.");
    ll_put_ident(&b, iface); sb_putc(&b, '.'); ll_put_ident(&b, concrete);
    const char *r = arena_strdup(ll->a, sb_cstr(&b)); sb_free(&b); return r;
}


static func_sig_t *ll_match_sig(symbol_t *m, size_t want)
{
    func_sig_t *first = m->overloads.len ? (func_sig_t *)m->overloads.data[0] : NULL;
    { size_t i = 0; for (; i < m->overloads.len; i++) {
        func_sig_t *sig = (func_sig_t *)m->overloads.data[i];
        if (sig->params.len == want) return sig;
    } }
    return first;
}


static void ll_ensure_vtbl(ll_t *ll, const char *iface, const char *concrete)
{
    const char *name = ll_vtbl_name(ll, iface, concrete);
    { size_t i = 0; for (; i < ll->emitted.len; i++)
        if (!strcmp(name, (const char *)ll->emitted.data[i])) return; }
    vec_push(ll->a, &ll->emitted, CONST_CAST(name));
    symbol_t *isym = ll_sym(ll, iface), *csym = ll_sym(ll, concrete);
    if (!isym || isym->kind != SYM_INTERFACE || !csym || !csym->members) return;
    sb_t slots; sb_init(&slots); int n = 0;
    { size_t m = 0; for (; m < isym->members->symbols.len; m++) {
        symbol_t *im = (symbol_t *)isym->members->symbols.data[m];
        if (im->kind != SYM_METHOD || !im->overloads.len) continue;
        func_sig_t *isig = (func_sig_t *)im->overloads.data[0];
        symbol_t *cm = scope_lookup_local(csym->members, im->name);
        func_sig_t *csig = cm && cm->kind == SYM_METHOD ? ll_match_sig(cm, isig->params.len) : NULL;
        if (n) sb_puts(&slots, ", ");
        if (csig) {
            ll_ensure_fn(ll, csig->decl, csym, ll->pkg_scope);
            sb_puts(&slots, ll_fmt(ll, "ptr @%s", ll_mangle(ll, concrete, im->name, csig)));
        } else {
            sb_puts(&slots, "ptr null");   
        }
        n++;
    } }
    sb_puts(ll->g, ll_fmt(ll, "%s = constant [%d x ptr] [%s]\n", name, n, sb_cstr(&slots)));
    sb_free(&slots);
}


const char *ll_box_dyn(ll_t *ll, llv_t v, const char *iface)
{
    char ib[160]; size_t k = 0;                 
    { const char *p = iface; for (; *p && *p != '*' && *p != '[' && k < sizeof ib - 1; p++) ib[k++] = *p; }
    ib[k] = 0;
    ll_ensure_vtbl(ll, ib, v.ts);
    const char *cty = ll_struct_ltype(ll, v.ts);
    const char *szp = ll_new_tmp(ll), *sz = ll_new_tmp(ll), *data = ll_new_tmp(ll);
    ll_emit(ll, "%s = getelementptr %s, ptr null, i32 1", szp, cty);
    ll_emit(ll, "%s = ptrtoint ptr %s to i64", sz, szp);
    ll_emit(ll, "%s = call ptr @malloc(i64 %s)", data, sz);
    ll_emit(ll, "store %s %s, ptr %s", cty, v.ref, data);
    const char *t0 = ll_new_tmp(ll), *t1 = ll_new_tmp(ll);
    ll_emit(ll, "%s = insertvalue %%dyn undef, ptr %s, 0", t0, data);
    ll_emit(ll, "%s = insertvalue %%dyn %s, ptr %s, 1", t1, t0, ll_vtbl_name(ll, ib, v.ts));
    return t1;
}


const char *ll_mangle_ti(ll_t *ll, const char *typestr, const char *fn, func_sig_t *sig)
{
    sb_t b; sb_init(&b); sb_puts(&b, "salam_ti_");
    ll_put_type_code(&b, typestr);
    sb_putc(&b, '_'); ll_put_ident(&b, fn);
    if (sig)
        { size_t i = 0; for (; i < sig->params.len; i++) {
            sb_putc(&b, '_');
            ll_put_type_code(&b, type_to_string(ll->sem->tc, (type_t *)sig->params.data[i]));
        } }
    const char *r = arena_strdup(ll->a, sb_cstr(&b)); sb_free(&b); return r;
}


static void ll_emit_impls_in(ll_t *ll, scope_t *g)
{
    { size_t i = 0; for (; i < g->symbols.len; i++) {
        symbol_t *owner = (symbol_t *)g->symbols.data[i];
        if (owner->kind != SYM_TYPEIMPL || !owner->members) continue;
        { size_t j = 0; for (; j < owner->members->symbols.len; j++) {
            symbol_t *m = (symbol_t *)owner->members->symbols.data[j];
            if (m->kind != SYM_METHOD) continue;
            { size_t k = 0; for (; k < m->overloads.len; k++) {
                func_sig_t *sig = (func_sig_t *)m->overloads.data[k];
                if (sig && sig->decl) ll_function(ll, sig->decl, owner);
            } }
        } }
    } }
}

void ll_emit_impls(ll_t *ll) { ll_emit_impls_in(ll, ll->sem->global); }


void ll_emit_lambda(ll_t *ll, ast_node_t *n)
{
    if (n->name) return;
    int idx = ll->lam_n++;
    const char *name  = ll_fmt(ll, "salam_lambda_%d", idx);
    const char *envty = ll_fmt(ll, "%%lam.%d.env", idx);
    n->name = name;
    const char *rts = ll_func_ret(ll, n->type_str);
    size_t ncap = n->captures.len;

    sb_t et; sb_init(&et); sb_puts(&et, ll_fmt(ll, "%s = type { ptr", envty));
    { size_t i = 0; for (; i < ncap; i++)
        sb_puts(&et, ll_fmt(ll, ", %s", ll_ty(ll, ((ast_node_t *)n->captures.data[i])->type_str))); }
    sb_puts(&et, " }\n");
    sb_puts(ll->g, sb_cstr(&et)); sb_free(&et);
    if (ncap == 0)
        sb_puts(ll->g, ll_fmt(ll, "@%s.env = global %s { ptr @%s }\n", name, envty, name));

    
    sb_t body; sb_init(&body);
    sb_t *saved_b = ll->b;
    int saved_tmp = ll->tmp, saved_lbl = ll->lbl, saved_nloop = ll->nloop;
    bool saved_main = ll->is_main, saved_term = ll->term;
    const char *saved_ret = ll->ret_ts, *saved_self = ll->self_ts, *saved_this = ll->this_ref;
    const char *saved_sp = ll->cur_sp, *saved_dbg = ll->cur_dbg;
    ast_node_t *saved_lam = ll->cur_lambda;
    const char *saved_env = ll->env_ref, *saved_envty = ll->env_ty;
    vec_t saved_locals = ll->locals, saved_defers = ll->defers;

    ll->b = &body; ll->tmp = 0; ll->lbl = 0; ll->nloop = 0; ll->term = false;
    ll->is_main = false; ll->ret_ts = rts; ll->self_ts = NULL; ll->this_ref = NULL;
    ll->cur_lambda = n; ll->env_ref = "%env"; ll->env_ty = envty;
    vec_init(&ll->locals); vec_init(&ll->defers);

    sb_t hdr; sb_init(&hdr);
    sb_puts(&hdr, ll_fmt(ll, "define %s @%s(ptr %%env", ll_ty(ll, rts), name));
    { size_t i = 0; for (; i < n->list.len; i++) {
        ast_node_t *p = (ast_node_t *)n->list.data[i];
        sb_puts(&hdr, p->is_ref ? ll_fmt(ll, ", ptr %%arg%zu", i)
                                : ll_fmt(ll, ", %s %%arg%zu", ll_ty(ll, p->type_str), i));
    } }
    sb_puts(&hdr, ") {\n");
    const char *header = arena_strdup(ll->a, sb_cstr(&hdr)); sb_free(&hdr);

    ll_emit_label(ll, "entry");
    { size_t i = 0; for (; i < n->list.len; i++) {
        ast_node_t *p = (ast_node_t *)n->list.data[i];
        if (p->is_ref) { ll_local_add(ll, p->name, ll_fmt(ll, "%%arg%zu", i), p->type_str); continue; }
        const char *ptr = ll_fmt(ll, "%%p.%s", p->name);
        ll_emit(ll, "%s = alloca %s", ptr, ll_ty(ll, p->type_str));
        ll_emit(ll, "store %s %%arg%zu, ptr %s", ll_ty(ll, p->type_str), i, ptr);
        ll_local_add(ll, p->name, ptr, p->type_str);
    } }
    if (n->a) ll_block(ll, n->a);
    if (!ll->term) ll_emit_return(ll, NULL);

    sb_puts(ll->g, header);
    sb_puts(ll->g, sb_cstr(&body));
    sb_puts(ll->g, "}\n\n");
    sb_free(&body);

    ll->b = saved_b; ll->tmp = saved_tmp; ll->lbl = saved_lbl; ll->nloop = saved_nloop;
    ll->is_main = saved_main; ll->term = saved_term; ll->ret_ts = saved_ret;
    ll->self_ts = saved_self; ll->this_ref = saved_this;
    ll->cur_sp = saved_sp; ll->cur_dbg = saved_dbg;
    ll->cur_lambda = saved_lam; ll->env_ref = saved_env; ll->env_ty = saved_envty;
    ll->locals = saved_locals; ll->defers = saved_defers;
}

static bool ll_const_init(const ast_node_t *init)
{
    return init && init->kind == AST_LITERAL;
}

void ll_emit_globals(ll_t *ll, ast_node_t *program)
{
    int any = 0;
    { size_t i = 0; for (; i < program->list.len; i++) {
        ast_node_t *d = (ast_node_t *)program->list.data[i];
        if (d->kind != AST_CONST_DECL && d->kind != AST_VAR_DECL) continue;
        if (d->is_extern) continue;
        const char *ts = d->type_str ? d->type_str : "i32";
        const char *gref = ll_fmt(ll, "@g.%s", ll_struct_ltype(ll, d->name) + strlen("%struct."));
        const char *lty = ll_ty(ll, ts);
        const char *init;
        if (ll_const_init(d->a)) {
            init = ll_expr(ll, d->a).ref;
        } else {
            init = ll_zero(ts);
            if (d->a) vec_push(ll->a, &ll->gdefer, d);   
        }
        sb_puts(ll->g, ll_fmt(ll, "%s = global %s %s\n", gref, lty, init));
        lvar_t *gv = (lvar_t *)arena_alloc(ll->a, sizeof *gv);
        gv->name = d->name; gv->ptr = gref; gv->ts = ts;
        vec_push(ll->a, &ll->globals, gv);
        any = 1;
    } }
    if (any) sb_puts(ll->g, "\n");
}



static bool ll_extern_seen(ll_t *ll, const char *name)
{
    static const char *prologue[] = { "printf","dprintf","strlen","strcmp",
                                      "malloc","memcpy","realloc","free","memmove",
                                      "abort","exit","snprintf","strtol","strtod",
                                      "strstr", NULL };
    { int p = 0; for (; prologue[p]; p++) if (!strcmp(name, prologue[p])) return true; }
    { size_t i = 0; for (; i < ll->extern_names.len; i++)
        if (!strcmp(name, (const char *)ll->extern_names.data[i])) return true; }
    return false;
}

static void ll_emit_externs_in(ll_t *ll, scope_t *g)
{
    { size_t i = 0; for (; i < g->symbols.len; i++) {
        symbol_t *s = (symbol_t *)g->symbols.data[i];
        if (s->kind != SYM_FUNC || s->overloads.len == 0) continue;
        func_sig_t *sig = (func_sig_t *)s->overloads.data[0];
        if (!sig->decl || !sig->decl->is_extern || sig->decl->a) continue;   
        if (ll_extern_seen(ll, s->name)) continue;
        vec_push(ll->a, &ll->extern_names, CONST_CAST(s->name));
        sb_t b; sb_init(&b);
        sb_puts(&b, ll_fmt(ll, "declare %s @%s(", ll_ty(ll, type_to_string(ll->sem->tc, sig->ret)), s->name));
        { size_t j = 0; for (; j < sig->params.len; j++) {
            if (j) sb_puts(&b, ", ");
            sb_puts(&b, ll_ty(ll, type_to_string(ll->sem->tc, (type_t *)sig->params.data[j])));
        } }
        if (sig->variadic) sb_puts(&b, sig->params.len ? ", ..." : "...");
        sb_puts(&b, ")\n");
        sb_puts(ll->g, sb_cstr(&b)); sb_free(&b);
    } }
}

void ll_emit_externs(ll_t *ll) { ll_emit_externs_in(ll, ll->sem->global); }


void ll_emit_packages(ll_t *ll)
{
    vec_t *pkgs = &ll->sem->packages;
    { size_t p = 0; for (; p < pkgs->len; p++) {
        symbol_t *pk = (symbol_t *)pkgs->data[p];
        if (!pk || pk->kind != SYM_PACKAGE || !pk->decl) continue;
        ast_node_t *prog = pk->decl;
        
        scope_t *saved = ll->pkg_scope;
        ll->pkg_scope = pk->members;
        ll_emit_struct_types(ll, prog);
        if (pk->members) ll_emit_externs_in(ll, pk->members);
        ll_emit_globals(ll, prog);
        if (pk->members) ll_emit_impls_in(ll, pk->members);
        
        ll->pkg_scope = saved;
    } }
}

void ll_emit_global_inits(ll_t *ll)
{
    { size_t i = 0; for (; i < ll->gdefer.len; i++) {
        ast_node_t *d = (ast_node_t *)ll->gdefer.data[i];
        lvar_t *g = ll_global_find(ll, d->name);
        if (!g) continue;
        const char *v = ll_conv(ll, ll_expr(ll, d->a), g->ts);
        ll_emit(ll, "store %s %s, ptr %s", ll_ty(ll, g->ts), v, g->ptr);
    } }
}
