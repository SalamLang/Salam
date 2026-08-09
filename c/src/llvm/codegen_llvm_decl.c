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

#include "llvm/codegen_llvm_internal.h"
#include "core/sal_format.h"

void ll_emit_struct_types(ll_t *ll, ast_node_t *program)
{
    {
        size_t i = 0;
        for (; i < program->list.len; i++) {
            ast_node_t *d = (ast_node_t *)program->list.data[i];
            if (d->kind != AST_STRUCT_DEF || d->typarams.len > 0) continue;
            symbol_t *ssym = ll_struct_sym(ll, d->name);
            if (!ssym || !ssym->members) continue;
            const char *sname =
                ssym->type ? type_to_string(ll->sem->tc, ssym->type) : d->name;
            const char *ltname = ll_struct_ltype(ll, sname);
            {
                bool seen = false;
                size_t k = 0;
                for (; k < ll->emitted.len; k++)
                    if (!strcmp(ltname, (const char *)ll->emitted.data[k])) {
                        seen = true;
                        break;
                    }
                if (seen) continue;
                vec_push(ll->a, &ll->emitted, CONST_CAST(ltname));
            }
            sb_t b;
            sb_init(&b);
            sb_puts(&b, ll_fmt(ll, "%s = type { ", ltname));
            int nf = 0;
            {
                size_t j = 0;
                for (; j < ssym->members->symbols.len; j++) {
                    symbol_t *f = (symbol_t *)ssym->members->symbols.data[j];
                    if (f->kind != SYM_FIELD) continue;
                    if (nf) sb_puts(&b, ", ");
                    sb_puts(&b, ll_ty(ll, type_to_string(ll->sem->tc, f->type)));
                    nf++;
                }
            }
            sb_puts(&b, " }\n");
            sb_puts(ll->g, sb_cstr(&b));
            sb_free(&b);
        }
    }
    sb_puts(ll->g, "\n");
}

static void ll_put_ident_byte(sb_t *b, unsigned char c)
{
    if (isalnum(c))
        sb_putc(b, (char)c);
    else if (c == '_')
        sb_puts(b, "__");
    else {
        char h[5];
        sal_snprintf(h, sizeof h, "_%02x", c);
        sb_puts(b, h);
    }
}

static void ll_put_ident(sb_t *b, const char *name)
{
    {
        const unsigned char *p = (const unsigned char *)name;
        for (; p && *p; p++)
            ll_put_ident_byte(b, *p);
    }
}

static void ll_put_type_code(sb_t *b, const char *ts)
{
    {
        const unsigned char *p = (const unsigned char *)ts;
        for (; p && *p; p++) {
            if (*p == '*')
                sb_puts(b, "_ptr");
            else if (*p == '[')
                sb_puts(b, "_arr");
            else if (*p == ']' || *p == ' ') {
            } else
                ll_put_ident_byte(b, *p);
        }
    }
}

const char *ll_mangle(ll_t *ll, const char *owner, const char *fn, func_sig_t *sig)
{
    if (!owner && !strcmp(fn, ll->entry)) return "main";
    sb_t b;
    sb_init(&b);
    sb_puts(&b, "salam_");
    if (owner) {
        ll_put_ident(&b, owner);
        sb_putc(&b, '_');
    }
    ll_put_ident(&b, fn);
    if (sig) {
        size_t i = 0;
        for (; i < sig->params.len; i++) {
            sb_putc(&b, '_');
            ll_put_type_code(&b,
                             type_to_string(ll->sem->tc, (type_t *)sig->params.data[i]));
        }
    }
    const char *r = arena_strdup(ll->a, sb_cstr(&b));
    sb_free(&b);
    return r;
}

func_sig_t *ll_pick_overload(ll_t *ll, symbol_t *sym, ast_node_t *call)
{
    func_sig_t *arity = NULL;
    {
        size_t i = 0;
        for (; i < sym->overloads.len; i++) {
            func_sig_t *sig = (func_sig_t *)sym->overloads.data[i];
            if (call->list.len < sig->required) continue;
            if (!sig->variadic && call->list.len > sig->params.len) continue;
            if (!arity) arity = sig;
            size_t nfix =
                call->list.len < sig->params.len ? call->list.len : sig->params.len;
            bool ok = true;
            {
                size_t j = 0;
                for (; j < nfix && ok; j++) {
                    ast_node_t *arg = (ast_node_t *)call->list.data[j];
                    const char *pt =
                        type_to_string(ll->sem->tc, (type_t *)sig->params.data[j]);
                    if (!arg->type_str || strcmp(arg->type_str, pt) != 0) ok = false;
                }
            }
            if (ok) return sig;
        }
    }
    return arity ? arity
                 : (sym->overloads.len ? (func_sig_t *)sym->overloads.data[0] : NULL);
}

static func_sig_t *ll_sig_of(ll_t *ll, ast_node_t *fn, symbol_t *owner)
{
    symbol_t *s =
        owner ? scope_lookup_local(owner->members, fn->name) : ll_sym(ll, fn->name);
    if (!s) return NULL;
    {
        size_t i = 0;
        for (; i < s->overloads.len; i++) {
            func_sig_t *sig = (func_sig_t *)s->overloads.data[i];
            if (sig->decl == fn) return sig;
        }
    }
    return s->overloads.len ? (func_sig_t *)s->overloads.data[0] : NULL;
}

/* Defined below; needed by main's argc/argv wiring. */
static func_sig_t *ll_body_sig_for(ll_t *ll, const char *name, symbol_t **owner);
static void ll_declare_extern(ll_t *ll, const char *name, func_sig_t *sig);

static const char *ll_fn_header(ll_t *ll, ast_node_t *fn, func_sig_t *sig,
                                const char *ret_lty, const char *fname,
                                const char *recv_param, bool exported, bool is_main)
{
    sb_t hdr;
    sb_init(&hdr);
    sb_puts(&hdr,
            ll_fmt(ll, "define %s%s @%s(", exported ? "" : "internal ", ret_lty, fname));
    bool first = true;
    /*
     * main takes the real C argc/argv. Salam's own `func main:` declares no
     * parameters, so this emitted `define i32 @main()` and the process
     * arguments never reached the runtime - args() returned an empty vector
     * in every LLVM-built program (the C backend has always emitted
     * `main(int argc, char **argv)` and called salam_set_args, see
     * codegen_decl.c).
     */
    if (is_main) {
        sb_puts(&hdr, "i32 %argc, ptr %argv");
        first = false;
    }
    if (recv_param) {
        sb_puts(&hdr, recv_param);
        first = false;
    }
    {
        size_t i = 0;
        for (; i < fn->list.len; i++) {
            ast_node_t *p = (ast_node_t *)fn->list.data[i];
            const char *pts = type_to_string(ll->sem->tc, (type_t *)sig->params.data[i]);
            if (!first) sb_puts(&hdr, ", ");
            sb_puts(&hdr, p->is_ref ? ll_fmt(ll, "ptr noundef nonnull %%arg%zu", i)
                                    : ll_fmt(ll, "%s %%arg%zu", ll_ty(ll, pts), i));
            first = false;
        }
    }
    sb_puts(&hdr, ")");
    if (!ll->is_main) {
        if (fn->is_inline) sb_puts(&hdr, " alwaysinline");
        if (fn->is_noinline) sb_puts(&hdr, " noinline");
        if (fn->is_pure) sb_puts(&hdr, " memory(read) willreturn nofree nosync");
        if (fn->is_noret) sb_puts(&hdr, " noreturn");
    }
    if (ll->optsize) sb_puts(&hdr, " optsize");
    if (ll->minsize) sb_puts(&hdr, " minsize");
    sb_puts(&hdr, " nounwind");
    if (ll->debug && ll->cur_sp) sb_puts(&hdr, ll_fmt(ll, " !dbg %s", ll->cur_sp));
    sb_puts(&hdr, " {\n");
    const char *r = arena_strdup(ll->a, sb_cstr(&hdr));
    sb_free(&hdr);
    return r;
}

static void ll_spill_params(ll_t *ll, ast_node_t *fn, func_sig_t *sig)
{
    {
        size_t i = 0;
        for (; i < fn->list.len; i++) {
            ast_node_t *p = (ast_node_t *)fn->list.data[i];
            const char *pts = type_to_string(ll->sem->tc, (type_t *)sig->params.data[i]);
            if (p->is_ref) {
                ll_local_add(ll, p->name, ll_fmt(ll, "%%arg%zu", i), pts);
                continue;
            }
            const char *ptr = ll_fmt(ll, "%%p.%s", ll_safe_name(ll, p->name));
            ll_emit_alloca(ll, "%s = alloca %s", ptr, ll_ty(ll, pts));
            ll_emit(ll, "store %s %%arg%zu, ptr %s", ll_ty(ll, pts), i, ptr);
            ll_local_add(ll, p->name, ptr, pts);
        }
    }
}

void ll_function(ll_t *ll, ast_node_t *fn, symbol_t *owner)
{
    /*
     * A bodyless `extern func` has no definition to emit - but it does need a
     * `declare`, and this is the one place every call site funnels through
     * (ll_call_user -> ll_ensure_fn). Returning silently left the call
     * referencing an undeclared @name whenever the declaring package was
     * never touched by the scope walk in ll_emit_externs_in.
     */
    if (!owner && fn->is_extern && !fn->a) {
        func_sig_t *xsig = ll_sig_of(ll, fn, owner);
        if (xsig) ll_declare_extern(ll, fn->name, xsig);
        return;
    }
    func_sig_t *sig = ll_sig_of(ll, fn, owner);
    if (!sig) return;
    bool is_main = !owner && !strcmp(fn->name, ll->entry);
    bool is_impl = owner && owner->kind == SYM_TYPEIMPL;
    const char *rts = type_to_string(ll->sem->tc, sig->ret);
    const char *ret_lty = is_main ? "i32" : ll_ty(ll, rts);

    const char *recv_ts = is_impl ? type_to_string(ll->sem->tc, owner->type)
                          : owner ? owner->name
                                  : NULL;

    const char *fname = is_impl ? ll_mangle_ti(ll, recv_ts, fn->name, sig)
                        : (!owner && fn->is_extern)
                            ? fn->name
                            : ll_mangle(ll, owner ? owner->name : NULL, fn->name, sig);
    const char *recv_param = !owner    ? NULL
                             : is_impl ? ll_fmt(ll, "%s %%this", ll_ty(ll, recv_ts))
                                       : "ptr noundef %this";
    bool exported = is_main || (!owner && fn->is_extern);

    {
        size_t i = 0;
        for (; i < ll->emitted.len; i++)
            if (!strcmp(fname, (const char *)ll->emitted.data[i])) return;
    }
    vec_push(ll->a, &ll->emitted, CONST_CAST(fname));

    sb_t body;
    sb_init(&body);
    sb_t allocs;
    sb_init(&allocs);
    sb_t *saved_b = ll->b, *saved_allocas = ll->allocas;
    int saved_tmp = ll->tmp, saved_lbl = ll->lbl, saved_nloop = ll->nloop;
    /*
     * The break/continue target stacks, not just their depth. A function
     * can be emitted *nested* inside another's body (ll_ensure_fn fires
     * while lowering a call expression), and the nested emission resets
     * nloop to 0 and then writes brk[0]/cont[0] for its own loops - which
     * are the outer function's live entries. The outer `continue` then
     * branched to a label belonging to the inner function and never
     * defined in its own, producing IR that fails to parse ("use of
     * undefined value '%L4_wcond'"). Only the entries actually live for
     * the outer function need preserving.
     */
    const char *saved_brk[64], *saved_cont[64];
    {
        int bi = 0;
        for (; bi < saved_nloop && bi < 64; bi++) {
            saved_brk[bi] = ll->brk[bi];
            saved_cont[bi] = ll->cont[bi];
        }
    }
    bool saved_main = ll->is_main, saved_byval = ll->self_byval, saved_term = ll->term;
    const char *saved_ret = ll->ret_ts, *saved_sp = ll->cur_sp, *saved_dbg = ll->cur_dbg;
    const char *saved_self = ll->self_ts, *saved_this = ll->this_ref;
    vec_t saved_locals = ll->locals, saved_defers = ll->defers;
    ll->b = &body;
    ll->allocas = &allocs;
    ll->tmp = 0;
    ll->lbl = 0;
    ll->nloop = 0;
    ll->term = false;
    ll->ret_ts = rts;
    ll->is_main = is_main;
    ll->self_ts = recv_ts;
    ll->self_byval = is_impl;
    ll->this_ref = owner ? "%this" : NULL;
    vec_init(&ll->locals);
    vec_init(&ll->defers);
    if (ll->debug) {
        ll_debug_subprogram(ll, fn->name, fn->span.begin.line);
        ll->cur_dbg = ll_debug_location(ll, fn->span.begin.line, fn->span.begin.col);
    }
    const char *header =
        ll_fn_header(ll, fn, sig, ret_lty, fname, recv_param, exported, is_main);
    if (is_impl) {
        ll_emit_alloca(ll, "%%p.this = alloca %s", ll_ty(ll, recv_ts));
        ll_emit(ll, "store %s %%this, ptr %%p.this", ll_ty(ll, recv_ts));
        ll->this_ref = "%p.this";
    }
    ll_spill_params(ll, fn, sig);
    if (is_main) {
        /* Every return from main flushes the print buffer (see
         * ll_emit_return), so the helper has to exist even in a program
         * whose own prints all take the printf path. Only where the buffer
         * is reachable at all: ll->single_threaded is already false for
         * Windows targets, whose CRT has no `write` for the helper to call. */
        if (ll->single_threaded) ll_need(ll, LL_H_OUTBUF);
        /* Hand argc/argv to the runtime before anything can call args(). */
        symbol_t *sa_owner = NULL;
        func_sig_t *sa = ll_body_sig_for(ll, "salam_set_args", &sa_owner);
        if (sa && sa->decl) {
            ll_ensure_fn(ll, sa->decl, NULL, sa_owner->members);
            ll_emit(ll, "call void @salam_set_args(i32 %%argc, ptr %%argv)");
        }
        ll_emit_global_inits(ll);
    }
    if (fn->a) ll_block_top(ll, fn->a);
    if (!ll->term) ll_emit_return(ll, NULL);
    sb_puts(ll->g, header);
    sb_puts(ll->g, "entry:\n");
    sb_puts(ll->g, sb_cstr(&allocs));
    sb_puts(ll->g, sb_cstr(&body));
    sb_puts(ll->g, "}\n\n");
    sb_free(&body);
    sb_free(&allocs);
    ll->allocas = saved_allocas;
    ll->b = saved_b;
    ll->tmp = saved_tmp;
    ll->lbl = saved_lbl;
    ll->nloop = saved_nloop;
    {
        int bi = 0;
        for (; bi < saved_nloop && bi < 64; bi++) {
            ll->brk[bi] = saved_brk[bi];
            ll->cont[bi] = saved_cont[bi];
        }
    }
    ll->is_main = saved_main;
    ll->ret_ts = saved_ret;
    ll->self_byval = saved_byval;
    ll->term = saved_term;
    ll->cur_sp = saved_sp;
    ll->cur_dbg = saved_dbg;
    ll->self_ts = saved_self;
    ll->this_ref = saved_this;
    ll->locals = saved_locals;
    ll->defers = saved_defers;
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
    sb_t b;
    sb_init(&b);
    sb_puts(&b, "@vtbl.");
    ll_put_ident(&b, iface);
    sb_putc(&b, '.');
    ll_put_ident(&b, concrete);
    const char *r = arena_strdup(ll->a, sb_cstr(&b));
    sb_free(&b);
    return r;
}

static func_sig_t *ll_match_sig(symbol_t *m, size_t want)
{
    func_sig_t *first = m->overloads.len ? (func_sig_t *)m->overloads.data[0] : NULL;
    {
        size_t i = 0;
        for (; i < m->overloads.len; i++) {
            func_sig_t *sig = (func_sig_t *)m->overloads.data[i];
            if (sig->params.len == want) return sig;
        }
    }
    return first;
}

static void ll_ensure_vtbl(ll_t *ll, const char *iface, const char *concrete)
{
    const char *name = ll_vtbl_name(ll, iface, concrete);
    {
        size_t i = 0;
        for (; i < ll->emitted.len; i++)
            if (!strcmp(name, (const char *)ll->emitted.data[i])) return;
    }
    vec_push(ll->a, &ll->emitted, CONST_CAST(name));
    symbol_t *isym = ll_sym(ll, iface), *csym = ll_sym(ll, concrete);
    if (!isym || isym->kind != SYM_INTERFACE || !csym || !csym->members) return;
    sb_t slots;
    sb_init(&slots);
    int n = 0;
    {
        size_t m = 0;
        for (; m < isym->members->symbols.len; m++) {
            symbol_t *im = (symbol_t *)isym->members->symbols.data[m];
            if (im->kind != SYM_METHOD || !im->overloads.len) continue;
            func_sig_t *isig = (func_sig_t *)im->overloads.data[0];
            symbol_t *cm = scope_lookup_local(csym->members, im->name);
            func_sig_t *csig =
                cm && cm->kind == SYM_METHOD ? ll_match_sig(cm, isig->params.len) : NULL;
            if (n) sb_puts(&slots, ", ");
            if (csig) {
                ll_ensure_fn(ll, csig->decl, csym, ll->pkg_scope);
                sb_puts(&slots,
                        ll_fmt(ll, "ptr @%s", ll_mangle(ll, concrete, im->name, csig)));
            } else {
                sb_puts(&slots, "ptr null");
            }
            n++;
        }
    }
    sb_puts(ll->g, ll_fmt(ll, "%s = private unnamed_addr constant [%d x ptr] [%s]\n",
                          name, n, sb_cstr(&slots)));
    sb_free(&slots);
}

const char *ll_box_dyn(ll_t *ll, llv_t v, const char *iface)
{
    char ib[160];
    size_t k = 0;
    {
        const char *p = iface;
        for (; *p && *p != '*' && *p != '[' && k < sizeof ib - 1; p++)
            ib[k++] = *p;
    }
    ib[k] = 0;
    ll_ensure_vtbl(ll, ib, v.ts);
    const char *cty = ll_struct_ltype(ll, v.ts);
    const char *szp = ll_new_tmp(ll), *sz = ll_new_tmp(ll), *data = ll_new_tmp(ll);
    ll_emit(ll, "%s = getelementptr %s, ptr null, i32 1", szp, cty);
    ll_emit(ll, "%s = ptrtoint ptr %s to %s", sz, szp, ll->usize);
    ll_emit(ll, "%s = call ptr @malloc(%s %s)", data, ll->usize, sz);
    ll_emit(ll, "store %s %s, ptr %s", cty, v.ref, data);
    const char *t0 = ll_new_tmp(ll), *t1 = ll_new_tmp(ll);
    ll_emit(ll, "%s = insertvalue %%dyn undef, ptr %s, 0", t0, data);
    ll_emit(ll, "%s = insertvalue %%dyn %s, ptr %s, 1", t1, t0,
            ll_vtbl_name(ll, ib, v.ts));
    return t1;
}

const char *ll_box_variant(ll_t *ll, llv_t v, int tag, const char *variant_ts)
{
    const char *VT = ll_ty(ll, variant_ts);
    const char *slot = ll_new_tmp(ll);
    const char *tagp = ll_new_tmp(ll);
    const char *payp = ll_new_tmp(ll);
    const char *result = ll_new_tmp(ll);
    ll_emit_alloca(ll, "%s = alloca %s", slot, VT);
    ll_emit(ll, "%s = getelementptr %s, ptr %s, i32 0, i32 0", tagp, VT, slot);
    ll_emit(ll, "store i32 %d, ptr %s", tag, tagp);
    ll_emit(ll, "%s = getelementptr %s, ptr %s, i32 0, i32 1", payp, VT, slot);
    ll_emit(ll, "store %s %s, ptr %s", ll_ty(ll, v.ts), v.ref, payp);
    ll_emit(ll, "%s = load %s, ptr %s", result, VT, slot);
    return result;
}

llv_t ll_unwrap_variant(ll_t *ll, llv_t v, const char *member_ts)
{
    const char *VT = ll_ty(ll, v.ts);
    const char *slot = ll_new_tmp(ll);
    const char *payp = ll_new_tmp(ll);
    const char *r = ll_new_tmp(ll);
    ll_emit_alloca(ll, "%s = alloca %s", slot, VT);
    ll_emit(ll, "store %s %s, ptr %s", VT, v.ref, slot);
    ll_emit(ll, "%s = getelementptr %s, ptr %s, i32 0, i32 1", payp, VT, slot);
    ll_emit(ll, "%s = load %s, ptr %s", r, ll_ty(ll, member_ts), payp);
    return (llv_t){r, member_ts};
}

const char *ll_mangle_ti(ll_t *ll, const char *typestr, const char *fn, func_sig_t *sig)
{
    sb_t b;
    sb_init(&b);
    sb_puts(&b, "salam_ti_");
    ll_put_type_code(&b, typestr);
    sb_putc(&b, '_');
    ll_put_ident(&b, fn);
    if (sig) {
        size_t i = 0;
        for (; i < sig->params.len; i++) {
            sb_putc(&b, '_');
            ll_put_type_code(&b,
                             type_to_string(ll->sem->tc, (type_t *)sig->params.data[i]));
        }
    }
    const char *r = arena_strdup(ll->a, sb_cstr(&b));
    sb_free(&b);
    return r;
}

static void ll_emit_impls_in(ll_t *ll, scope_t *g)
{
    {
        size_t i = 0;
        for (; i < g->symbols.len; i++) {
            symbol_t *owner = (symbol_t *)g->symbols.data[i];
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
                            if (sig && sig->decl) ll_function(ll, sig->decl, owner);
                        }
                    }
                }
            }
        }
    }
}

void ll_emit_impls(ll_t *ll)
{
    ll_emit_impls_in(ll, ll->sem->global);
}

void ll_emit_lambda(ll_t *ll, ast_node_t *n)
{
    if (n->name) return;
    int idx = ll->lam_n++;
    const char *name = ll_fmt(ll, "salam_lambda_%d", idx);
    const char *envty = ll_fmt(ll, "%%lam.%d.env", idx);
    n->name = name;
    const char *rts = ll_func_ret(ll, n->type_str);
    size_t ncap = n->captures.len;

    sb_t et;
    sb_init(&et);
    sb_puts(&et, ll_fmt(ll, "%s = type { ptr", envty));
    {
        size_t i = 0;
        for (; i < ncap; i++)
            sb_puts(&et,
                    ll_fmt(ll, ", %s",
                           ll_ty(ll, ((ast_node_t *)n->captures.data[i])->type_str)));
    }
    sb_puts(&et, " }\n");
    sb_puts(ll->g, sb_cstr(&et));
    sb_free(&et);
    if (ncap == 0)
        sb_puts(ll->g, ll_fmt(ll, "@%s.env = internal global %s { ptr @%s }\n", name,
                              envty, name));

    sb_t body;
    sb_init(&body);
    sb_t allocs;
    sb_init(&allocs);
    sb_t *saved_b = ll->b, *saved_allocas = ll->allocas;
    int saved_tmp = ll->tmp, saved_lbl = ll->lbl, saved_nloop = ll->nloop;
    /*
     * The break/continue target stacks, not just their depth. A function
     * can be emitted *nested* inside another's body (ll_ensure_fn fires
     * while lowering a call expression), and the nested emission resets
     * nloop to 0 and then writes brk[0]/cont[0] for its own loops - which
     * are the outer function's live entries. The outer `continue` then
     * branched to a label belonging to the inner function and never
     * defined in its own, producing IR that fails to parse ("use of
     * undefined value '%L4_wcond'"). Only the entries actually live for
     * the outer function need preserving.
     */
    const char *saved_brk[64], *saved_cont[64];
    {
        int bi = 0;
        for (; bi < saved_nloop && bi < 64; bi++) {
            saved_brk[bi] = ll->brk[bi];
            saved_cont[bi] = ll->cont[bi];
        }
    }
    bool saved_main = ll->is_main, saved_term = ll->term;
    const char *saved_ret = ll->ret_ts, *saved_self = ll->self_ts,
               *saved_this = ll->this_ref;
    const char *saved_sp = ll->cur_sp, *saved_dbg = ll->cur_dbg;
    ast_node_t *saved_lam = ll->cur_lambda;
    const char *saved_env = ll->env_ref, *saved_envty = ll->env_ty;
    vec_t saved_locals = ll->locals, saved_defers = ll->defers;
    const char *saved_mres = ll->match_result_ptr, *saved_mts = ll->match_result_ts,
               *saved_mmerge = ll->match_merge_block;

    ll->b = &body;
    ll->allocas = &allocs;
    ll->tmp = 0;
    ll->lbl = 0;
    ll->nloop = 0;
    ll->term = false;
    ll->is_main = false;
    ll->ret_ts = rts;
    ll->self_ts = NULL;
    ll->this_ref = NULL;
    ll->cur_lambda = n;
    ll->env_ref = "%env";
    ll->env_ty = envty;
    ll->match_result_ptr = NULL;
    ll->match_result_ts = NULL;
    ll->match_merge_block = NULL;
    vec_init(&ll->locals);
    vec_init(&ll->defers);

    sb_t hdr;
    sb_init(&hdr);
    sb_puts(&hdr,
            ll_fmt(ll, "define internal %s @%s(ptr noundef %%env", ll_ty(ll, rts), name));
    {
        size_t i = 0;
        for (; i < n->list.len; i++) {
            ast_node_t *p = (ast_node_t *)n->list.data[i];
            sb_puts(&hdr, p->is_ref
                              ? ll_fmt(ll, ", ptr %%arg%zu", i)
                              : ll_fmt(ll, ", %s %%arg%zu", ll_ty(ll, p->type_str), i));
        }
    }
    sb_puts(&hdr, ") nounwind {\n");
    const char *header = arena_strdup(ll->a, sb_cstr(&hdr));
    sb_free(&hdr);

    {
        size_t i = 0;
        for (; i < n->list.len; i++) {
            ast_node_t *p = (ast_node_t *)n->list.data[i];
            if (p->is_ref) {
                ll_local_add(ll, p->name, ll_fmt(ll, "%%arg%zu", i), p->type_str);
                continue;
            }
            const char *ptr = ll_fmt(ll, "%%p.%s", ll_safe_name(ll, p->name));
            ll_emit_alloca(ll, "%s = alloca %s", ptr, ll_ty(ll, p->type_str));
            ll_emit(ll, "store %s %%arg%zu, ptr %s", ll_ty(ll, p->type_str), i, ptr);
            ll_local_add(ll, p->name, ptr, p->type_str);
        }
    }
    if (n->a) ll_block_top(ll, n->a);
    if (!ll->term) ll_emit_return(ll, NULL);

    sb_puts(ll->g, header);
    sb_puts(ll->g, "entry:\n");
    sb_puts(ll->g, sb_cstr(&allocs));
    sb_puts(ll->g, sb_cstr(&body));
    sb_puts(ll->g, "}\n\n");
    sb_free(&body);
    sb_free(&allocs);
    ll->allocas = saved_allocas;

    ll->b = saved_b;
    ll->tmp = saved_tmp;
    ll->lbl = saved_lbl;
    ll->nloop = saved_nloop;
    {
        int bi = 0;
        for (; bi < saved_nloop && bi < 64; bi++) {
            ll->brk[bi] = saved_brk[bi];
            ll->cont[bi] = saved_cont[bi];
        }
    }
    ll->is_main = saved_main;
    ll->term = saved_term;
    ll->ret_ts = saved_ret;
    ll->self_ts = saved_self;
    ll->this_ref = saved_this;
    ll->cur_sp = saved_sp;
    ll->cur_dbg = saved_dbg;
    ll->cur_lambda = saved_lam;
    ll->env_ref = saved_env;
    ll->env_ty = saved_envty;
    ll->locals = saved_locals;
    ll->defers = saved_defers;
    ll->match_result_ptr = saved_mres;
    ll->match_result_ts = saved_mts;
    ll->match_merge_block = saved_mmerge;
}

static const char *ll_const_fpfix(ll_t *ll, const char *lety, const char *v)
{
    if (!strcmp(lety, "double") || !strcmp(lety, "float")) return ll_fp_text(ll, v);
    return v;
}

static bool ll_const_agg(ll_t *ll, ast_node_t *n, const char **out);

/* True (with *out set to the LLVM constant-expr text) when `n` folds to a
 * value the LLVM verifier accepts directly as a global initializer, so
 * codegen can skip the zero-init + runtime-store fallback in main(). */
static bool ll_const_value(ll_t *ll, ast_node_t *n, const char **out)
{
    if (!n) return false;
    if (n->kind == AST_CAST) return ll_const_value(ll, n->a, out);
    if (n->kind == AST_LITERAL) {
        switch (n->op) {
        case TK_INT:
        case TK_FLOAT:
        case TK_STRING:
        case TK_TRIPLE_STRING:
        case TK_RAW_STRING:
        case TK_UTF8_CHAR:
        case TK_CHAR:
        case TK_KW_TRUE:
        case TK_KW_FALSE:
        case TK_KW_NULL:
            *out = ll_expr(ll, n).ref;
            return true;
        default:
            return false;
        }
    }
    if (n->kind == AST_ARRAY_LIT || n->kind == AST_STRUCT_LIT)
        return ll_const_agg(ll, n, out);
    return false;
}

static bool ll_const_agg(ll_t *ll, ast_node_t *n, const char **out)
{
    if (n->kind == AST_ARRAY_LIT) {
        const char *ats = n->type_str;
        if (!ats || !strchr(ats, '[')) return false;
        const char *ety = ll_array_elem(ll, ats);
        const char *lety = ll_ty(ll, ety);
        if (n->list.len == 0) {
            *out = "zeroinitializer";
            return true;
        }
        sb_t b;
        sb_init(&b);
        sb_puts(&b, "[");
        {
            size_t i = 0;
            for (; i < n->list.len; i++) {
                ast_node_t *el = (ast_node_t *)n->list.data[i];
                const char *v;
                if (!ll_const_value(ll, el, &v)) {
                    sb_free(&b);
                    return false;
                }
                if (i) sb_puts(&b, ", ");
                sb_puts(&b, ll_fmt(ll, "%s %s", lety, ll_const_fpfix(ll, lety, v)));
            }
        }
        sb_puts(&b, "]");
        *out = arena_strdup(ll->a, sb_cstr(&b));
        sb_free(&b);
        return true;
    }
    if (n->kind == AST_STRUCT_LIT) {
        symbol_t *ss = ll_struct_sym(ll, n->type_str ? n->type_str : n->name);
        if (!ss) return false;
        sb_t b;
        sb_init(&b);
        sb_puts(&b, "{ ");
        int idx = 0;
        {
            size_t i = 0;
            for (; i < ss->members->symbols.len; i++) {
                symbol_t *f = (symbol_t *)ss->members->symbols.data[i];
                if (f->kind != SYM_FIELD) continue;
                const char *fts = type_to_string(ll->sem->tc, f->type);
                ast_node_t *prov = NULL;
                {
                    size_t j = 0;
                    for (; j < n->list.len; j++) {
                        ast_node_t *fi = (ast_node_t *)n->list.data[j];
                        if (fi->name && !strcmp(fi->name, f->name)) {
                            prov = fi;
                            break;
                        }
                    }
                }
                const char *v;
                if (prov) {
                    if (!ll_const_value(ll, prov->a, &v)) {
                        sb_free(&b);
                        return false;
                    }
                } else if (f->decl && f->decl->a) {
                    if (!ll_const_value(ll, f->decl->a, &v)) {
                        sb_free(&b);
                        return false;
                    }
                } else {
                    v = ll_zero(fts);
                }
                const char *flety = ll_ty(ll, fts);
                if (idx) sb_puts(&b, ", ");
                sb_puts(&b, ll_fmt(ll, "%s %s", flety, ll_const_fpfix(ll, flety, v)));
                idx++;
            }
        }
        if (!idx) {
            sb_free(&b);
            *out = "zeroinitializer";
            return true;
        }
        sb_puts(&b, " }");
        *out = arena_strdup(ll->a, sb_cstr(&b));
        sb_free(&b);
        return true;
    }
    return false;
}

void ll_emit_globals(ll_t *ll, ast_node_t *program)
{
    int any = 0;
    {
        size_t i = 0;
        for (; i < program->list.len; i++) {
            ast_node_t *d = (ast_node_t *)program->list.data[i];
            if (d->kind != AST_CONST_DECL && d->kind != AST_VAR_DECL) continue;
            /* Idempotent: emitting the same global twice would produce a
             * duplicate definition in the module. This lets the function be
             * called again for a package whose globals may or may not have
             * been emitted already, which ll_addr_of relies on. */
            if (ll_global_find(ll, d->name)) continue;
            /*
             * An extern *variable* - POSIX `environ`, which std/os/process
             * declares in its non-Windows branch. ll_emit_externs_in only
             * handles SYM_FUNC, and this loop used to skip is_extern
             * outright, so nothing emitted these at all and every reference
             * became "address of an unknown identifier 'environ'". Declared
             * under its real C name, never the @g. prefix: the system linker
             * is what resolves it.
             */
            if (d->is_extern) {
                const char *ets = d->type_str ? d->type_str : "i32";
                const char *eref = ll_fmt(ll, "@%s", d->name);
                lvar_t *ev = (lvar_t *)arena_alloc(ll->a, sizeof *ev);
                sb_puts(ll->g,
                        ll_fmt(ll, "%s = external global %s\n", eref, ll_ty(ll, ets)));
                ev->name = d->name;
                ev->ptr = eref;
                ev->ts = ets;
                vec_push(ll->a, &ll->globals, ev);
                any = 1;
                continue;
            }
            const char *ts = d->type_str ? d->type_str : "i32";
            const char *gref =
                ll_fmt(ll, "@g.%s", ll_struct_ltype(ll, d->name) + strlen("%struct."));
            const char *lty = ll_ty(ll, ts);
            const char *init;
            const char *cv;
            if (d->a && ll_const_value(ll, d->a, &cv)) {
                init = cv;
            } else {
                init = ll_zero(ts);
                if (d->a) vec_push(ll->a, &ll->gdefer, d);
            }
            sb_puts(ll->g, ll_fmt(ll, "%s = internal global %s %s\n", gref, lty, init));
            lvar_t *gv = (lvar_t *)arena_alloc(ll->a, sizeof *gv);
            gv->name = d->name;
            gv->ptr = gref;
            gv->ts = ts;
            vec_push(ll->a, &ll->globals, gv);
            any = 1;
        }
    }
    if (any) sb_puts(ll->g, "\n");
}

static bool ll_extern_seen(ll_t *ll, const char *name)
{
    static const char *prologue[] = {"printf", "dprintf",  "strlen", "strcmp",  "malloc",
                                     "memcpy", "realloc",  "free",   "memmove", "abort",
                                     "exit",   "snprintf", "strtol", "strtod",  "strstr",
                                     "write",  NULL};
    {
        int p = 0;
        for (; prologue[p]; p++)
            if (!strcmp(name, prologue[p])) return true;
    }
    {
        size_t i = 0;
        for (; i < ll->extern_names.len; i++)
            if (!strcmp(name, (const char *)ll->extern_names.data[i])) return true;
    }
    return false;
}

/*
 * A function of this name that some package declares *with* a body, if any.
 * A bodyless `extern:` re-declaration is how one package calls another's
 * runtime entry point - std/collections redeclares `noret func
 * salam_panic(msg: str)` so its bounds checks can call it, while the real
 * body lives in std/core. Both are SYM_FUNCs of the same name, and which
 * one a given emission order happens to reach first decided whether the
 * module got a definition or only a declaration.
 */
static func_sig_t *ll_body_sig_for(ll_t *ll, const char *name, symbol_t **owner)
{
    size_t p = 0;
    for (; p < ll->sem->packages.len; p++) {
        symbol_t *pk = (symbol_t *)ll->sem->packages.data[p];
        symbol_t *fs;
        size_t o = 0;
        if (!pk || pk->kind != SYM_PACKAGE || !pk->members) continue;
        fs = scope_lookup_local(pk->members, name);
        if (!fs || fs->kind != SYM_FUNC) continue;
        for (; o < fs->overloads.len; o++) {
            func_sig_t *sg = (func_sig_t *)fs->overloads.data[o];
            /*
             * `is_extern` too, not just "has a body": the point of this
             * lookup is to find the definition that emits the *unmangled*
             * @name the bodyless declaration would otherwise stand in for,
             * and only `extern:`-block functions get that name. An ordinary
             * same-named function is a different symbol entirely - std/net's
             * `send(method, url, headers, body)` shadowed the libc `send`
             * every socket write calls, so nothing declared @send and no
             * server program could be built.
             */
            if (sg->decl && sg->decl->a && sg->decl->is_extern) {
                *owner = pk;
                return sg;
            }
        }
    }
    return NULL;
}

/*
 * One bodyless `extern func` -> one `declare` line, de-duplicated against
 * everything already declared or defined.
 *
 * Split out of ll_emit_externs_in so ll_function can reach it too: that scope
 * walk only ever runs over the global scope and over packages ll_touch_pkg
 * has visited, but a call can reach an extern in a package nothing touched -
 * std/fs' `fopen`, std/net's `send` - and then the module referenced @fopen
 * with no declaration anywhere ("use of undefined value '@fopen'", which
 * every program doing file or socket I/O hit).
 */
static void ll_declare_extern(ll_t *ll, const char *name, func_sig_t *sig)
{
    if (!sig || !sig->decl) return;
    if (ll_extern_seen(ll, name)) return;
    {
        size_t k = 0;
        for (; k < ll->emitted.len; k++)
            if (!strcmp(name, (const char *)ll->emitted.data[k])) return;
    }
    /*
     * Emit the body instead of a declaration whenever one exists, so the
     * outcome no longer depends on which package is walked first. Without
     * this, `salam_panic` was declared (from collections) and never defined
     * (from core) in any program where collections was reached first - every
     * Vector bounds check then failed to link.
     */
    {
        symbol_t *bowner = NULL;
        func_sig_t *bsig = ll_body_sig_for(ll, name, &bowner);
        if (bsig) {
            ll_ensure_fn(ll, bsig->decl, NULL, bowner->members);
            return;
        }
    }
    vec_push(ll->a, &ll->extern_names, CONST_CAST(name));
    sb_t b;
    sb_init(&b);
    sb_puts(&b, ll_fmt(ll, "declare %s @%s(",
                       ll_ty(ll, type_to_string(ll->sem->tc, sig->ret)), name));
    {
        size_t j = 0;
        for (; j < sig->params.len; j++) {
            if (j) sb_puts(&b, ", ");
            sb_puts(&b, ll_ty(ll, type_to_string(ll->sem->tc,
                                                 (type_t *)sig->params.data[j])));
        }
    }
    if (sig->variadic) sb_puts(&b, sig->params.len ? ", ..." : "...");
    sb_puts(&b, ")");
    if (sig->decl->is_pure)
        sb_puts(&b, " nounwind willreturn nofree nosync memory(read)");
    if (sig->decl->is_noret) sb_puts(&b, " noreturn");
    sb_puts(&b, "\n");
    sb_puts(ll->g, sb_cstr(&b));
    sb_free(&b);
}

static void ll_emit_externs_in(ll_t *ll, scope_t *g)
{
    {
        size_t i = 0;
        for (; i < g->symbols.len; i++) {
            symbol_t *s = (symbol_t *)g->symbols.data[i];
            if (s->kind != SYM_FUNC || s->overloads.len == 0) continue;
            func_sig_t *sig = (func_sig_t *)s->overloads.data[0];
            if (!sig->decl || !sig->decl->is_extern || sig->decl->a) continue;
            ll_declare_extern(ll, s->name, sig);
        }
    }
}

void ll_emit_externs(ll_t *ll)
{
    ll_emit_externs_in(ll, ll->sem->global);
}

void ll_touch_pkg(ll_t *ll, symbol_t *pk)
{
    if (!pk || pk->kind != SYM_PACKAGE || !pk->decl) return;
    {
        size_t i = 0;
        for (; i < ll->pkg_touched.len; i++)
            if (ll->pkg_touched.data[i] == pk) return;
    }
    vec_push(ll->a, &ll->pkg_touched, pk);

    ast_node_t *prog = pk->decl;
    scope_t *saved = ll->pkg_scope;
    ll->pkg_scope = pk->members;
    ll_emit_struct_types(ll, prog);
    if (pk->members) ll_emit_externs_in(ll, pk->members);
    ll_emit_globals(ll, prog);
    if (pk->members) ll_emit_impls_in(ll, pk->members);
    ll->pkg_scope = saved;
}

void ll_touch_pkg_named(ll_t *ll, const char *pkgname)
{
    if (!pkgname) return;
    vec_t *pkgs = &ll->sem->packages;
    size_t i = 0;
    for (; i < pkgs->len; i++) {
        symbol_t *pk = (symbol_t *)pkgs->data[i];
        if (pk && pk->pkgname && !strcmp(pk->pkgname, pkgname)) {
            ll_touch_pkg(ll, pk);
            return;
        }
    }
}

void ll_emit_global_inits(ll_t *ll)
{
    {
        size_t i = 0;
        for (; i < ll->gdefer.len; i++) {
            ast_node_t *d = (ast_node_t *)ll->gdefer.data[i];
            lvar_t *g = ll_global_find(ll, d->name);
            if (!g) continue;
            const char *v = ll_conv(ll, ll_expr(ll, d->a), g->ts);
            ll_emit(ll, "store %s %s, ptr %s", ll_ty(ll, g->ts), v, g->ptr);
        }
    }
}
