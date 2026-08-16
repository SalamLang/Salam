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

#include "codegen/codegen_internal.h"

static bool detect_gui_imports(ast_node_t *program)
{
    size_t i = 0;
    for (; i < program->list.len; i++) {
        ast_node_t *d = (ast_node_t *)program->list.data[i];
        if (d->kind != AST_IMPORT) continue;
        const char *import_name = d->name;
        if (!import_name) continue;
        if (strcmp(import_name, "webview") == 0 || strcmp(import_name, "layout") == 0) {
            return true;
        }
    }
    return false;
}

static void emit_private_protos(cg_t *cg, ast_node_t *program)
{
    {
        size_t i = 0;
        for (; i < program->list.len; i++) {
            ast_node_t *d = (ast_node_t *)program->list.data[i];
            if (d->typarams.len > 0) continue;
            if (d->synthetic) continue;
            if (d->kind == AST_FUNC_DEF && !d->is_extern &&
                strcmp(d->name, cg->entry) != 0 && !d->is_pub) {
                symbol_t *fsym = scope_lookup_local(cg->sem->global, d->name);
                func_sig_t *sig = sig_of_decl(fsym, d);
                if (sig)
                    sb_puts(cg->c,
                            cg_fmt(cg, "%s;\n", func_signature(cg, d, NULL, sig, false)));
            }
        }
    }
    sb_puts(cg->c, "\n");
}

static void emit_globals(cg_t *cg, ast_node_t *program)
{
    {
        size_t i = 0;
        for (; i < program->list.len; i++) {
            ast_node_t *d = (ast_node_t *)program->list.data[i];
            if (d->kind != AST_CONST_DECL && d->kind != AST_VAR_DECL) continue;
            if (d->is_extern) continue;
            const char *ts = d->type_str ? d->type_str : "int32_t";
            const char *decl = cg_decl(cg, ts, d->name);
            bool is_array = ts && strchr(ts, '[');
            bool can_defer =
                d->kind == AST_VAR_DECL && d->a && d->a->kind != AST_LITERAL && !is_array;
            if (can_defer) {
                sb_puts(cg->c, cg_fmt(cg, "%s;\n", decl));
                vec_push(cg->a, &cg->deferred,
                         CONST_CAST(cg_fmt(cg, "%s = %s;", cg_cident(cg, d->name),
                                           cg_expr(cg, d->a))));
            } else {
                /*
                 * Arrays are excluded on purpose. A Salam array parameter is
                 * a plain pointer the callee may write through - `func f(a:
                 * u64[8])` can do `a[0] = x` - so a const array global can
                 * reach a mutable T*, and C says writing through a pointer to
                 * a const object is undefined. std/tls' `const IV384` fed to
                 * _sha512_init_with(iv: u64[8]) is exactly that, and gcc's
                 * discarded-qualifiers diagnostic was reporting a real defect
                 * rather than being noisy. Scalars are passed by value and
                 * can never alias, so they keep the qualifier (and .rodata).
                 * codegen_header.c's extern declaration must agree, or the
                 * definition and the declaration conflict.
                 */
                bool want_const = (d->kind == AST_CONST_DECL) && !is_array;
                bool gct_const =
                    want_const && (strncmp(cg_ctype(cg, ts), "const ", 6) == 0);
                const char *pfx = (want_const && !gct_const) ? "const " : "";
                if (d->a)
                    sb_puts(cg->c,
                            cg_fmt(cg, "%s%s = %s;\n", pfx, decl, cg_expr(cg, d->a)));
                else
                    sb_puts(cg->c, cg_fmt(cg, "%s%s;\n", pfx, decl));
            }
        }
    }
    sb_puts(cg->c, "\n");
}

static void emit_impl_protos(cg_t *cg)
{
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
                            if (sig && sig->decl)
                                sb_puts(cg->c, cg_fmt(cg, "%s;\n",
                                                      func_signature(cg, sig->decl, owner,
                                                                     sig, false)));
                        }
                    }
                }
            }
        }
    }
}

static void emit_impl_bodies(cg_t *cg)
{
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
                            if (sig && sig->decl) cg_function(cg, sig->decl, owner);
                        }
                    }
                }
            }
        }
    }
}

static bool inl_name_in(vec_t *names, const char *name)
{
    size_t i = 0;
    for (; i < names->len; i++)
        if (strcmp((const char *)names->data[i], name) == 0) return true;
    return false;
}

static void inl_collect_locals(cg_t *cg, vec_t *names, ast_node_t *n)
{
    if (!n) return;
    if (n->name && (n->kind == AST_PARAM || n->kind == AST_VAR_DECL ||
                    n->kind == AST_FOR || n->kind == AST_REPEAT))
        vec_push(cg->a, names, CONST_CAST(n->name));
    inl_collect_locals(cg, names, n->a);
    inl_collect_locals(cg, names, n->b);
    inl_collect_locals(cg, names, n->c);
    inl_collect_locals(cg, names, n->d);
    {
        size_t i = 0;
        for (; i < n->list.len; i++)
            inl_collect_locals(cg, names, (ast_node_t *)n->list.data[i]);
    }
}

static ast_node_t *inl_sym_decl(symbol_t *s)
{
    if (s->decl) return s->decl;
    if (s->overloads.len) {
        func_sig_t *sig = (func_sig_t *)s->overloads.data[0];
        if (sig) return sig->decl;
    }
    return NULL;
}

static bool inl_body_exportable(cg_t *cg, vec_t *names, ast_node_t *n)
{
    if (!n) return true;
    if (n->kind == AST_LAMBDA) return false;
    if (n->kind == AST_IDENTIFIER && n->name && !inl_name_in(names, n->name)) {
        symbol_t *s = scope_lookup_local(cg->sem->global, n->name);
        if (s && (s->kind == SYM_FUNC || s->kind == SYM_VAR || s->kind == SYM_CONST)) {
            ast_node_t *decl = inl_sym_decl(s);
            if (decl && !decl->is_pub && !decl->is_extern) return false;
        }
    }
    if (!inl_body_exportable(cg, names, n->a)) return false;
    if (!inl_body_exportable(cg, names, n->b)) return false;
    if (!inl_body_exportable(cg, names, n->c)) return false;
    if (!inl_body_exportable(cg, names, n->d)) return false;
    {
        size_t i = 0;
        for (; i < n->list.len; i++)
            if (!inl_body_exportable(cg, names, (ast_node_t *)n->list.data[i]))
                return false;
    }
    return true;
}

/*
 * A `pub inline` body is copied into the header, so it may only name things
 * every includer can also see. One that reaches for a module-private symbol
 * loses the inline rather than the compile.
 */
static void demote_if_nonexportable(cg_t *cg, ast_node_t *fn)
{
    vec_t names;
    vec_init(&names);
    inl_collect_locals(cg, &names, fn);
    if (inl_body_exportable(cg, &names, fn->a)) return;
    fn->is_inline = false;
    LOG_I(cg->log, PH_CODEGEN,
          "inline '%s' uses module-private symbols; emitting it as a regular function",
          fn->name);
}

/* Methods of a generic struct are already emitted `static inline` once per
 * instantiation, so `inline` on them is a no-op rather than a header export. */
static void demote_struct_inlines(cg_t *cg, ast_node_t *d)
{
    size_t j = 0;
    if (d->typarams.len > 0 || d->synthetic) return;
    for (; j < d->list.len; j++) {
        ast_node_t *m = (ast_node_t *)d->list.data[j];
        if (m->kind != AST_FUNC_DEF || !m->is_inline || !m->is_pub) continue;
        if (m->is_extern) continue;
        demote_if_nonexportable(cg, m);
    }
}

static void demote_nonexportable_inlines(cg_t *cg, ast_node_t *program)
{
    size_t i = 0;
    for (; i < program->list.len; i++) {
        ast_node_t *d = (ast_node_t *)program->list.data[i];
        if (d->kind == AST_STRUCT_DEF) {
            demote_struct_inlines(cg, d);
            continue;
        }
        if (d->kind != AST_FUNC_DEF || !d->is_inline) continue;
        if (d->typarams.len > 0 || d->synthetic || d->is_extern) continue;
        if (strcmp(d->name, cg->entry) == 0) {
            d->is_inline = false;
            continue;
        }
        if (!d->is_pub) continue;
        demote_if_nonexportable(cg, d);
    }
}

static bool is_header_inline(ast_node_t *fn)
{
    return fn->kind == AST_FUNC_DEF && fn->is_inline && fn->is_pub && !fn->is_extern &&
           fn->typarams.len == 0 && !fn->synthetic && fn->a != NULL;
}

static void emit_struct_inline_bodies(cg_t *cg, ast_node_t *d)
{
    symbol_t *ssym;
    size_t j = 0;
    if (d->typarams.len > 0 || d->synthetic) return;
    ssym = scope_lookup_local(cg->sem->global, d->name);
    if (!ssym) return;
    for (; j < d->list.len; j++) {
        ast_node_t *m = (ast_node_t *)d->list.data[j];
        if (is_header_inline(m)) cg_function(cg, m, ssym);
    }
}

static void emit_inline_header(cg_t *cg, ast_node_t *program)
{
    sb_t *savec = cg->c;
    cg->c = cg->h;
    {
        size_t i = 0;
        for (; i < program->list.len; i++) {
            ast_node_t *d = (ast_node_t *)program->list.data[i];
            if (d->kind == AST_STRUCT_DEF) {
                emit_struct_inline_bodies(cg, d);
                continue;
            }
            if (is_header_inline(d)) cg_function(cg, d, NULL);
        }
    }
    cg->c = savec;
}

static void emit_function_bodies(cg_t *cg, ast_node_t *program)
{
    {
        size_t i = 0;
        for (; i < program->list.len; i++) {
            ast_node_t *d = (ast_node_t *)program->list.data[i];
            if (d->typarams.len > 0) continue;
            if (d->synthetic) continue;
            if (d->kind == AST_FUNC_DEF) {
                if (d->is_extern && !d->a) continue;
                if (d->is_inline && d->is_pub) continue;
                cg_function(cg, d, NULL);
            } else if (d->kind == AST_STRUCT_DEF) {
                symbol_t *ssym = scope_lookup_local(cg->sem->global, d->name);
                {
                    size_t j = 0;
                    for (; j < d->list.len; j++) {
                        ast_node_t *m = (ast_node_t *)d->list.data[j];
                        if (m->kind != AST_FUNC_DEF) continue;
                        /* pub inline methods already have their body in the
                           header, same as pub inline free functions above. */
                        if (m->is_inline && m->is_pub) continue;
                        cg_function(cg, m, ssym);
                    }
                }
            } else if (d->kind == AST_LAYOUT_BLOCK) {
                sb_puts(cg->c, "/* layout block omitted in general (C) mode */\n\n");
            }
        }
    }
}

static const char *inst_fn_name(cg_t *cg, symbol_t *owner, ast_node_t *fn,
                                func_sig_t *sig)
{
    return cg_mangle_in(cg, "g", owner ? owner->name : NULL, fn->name, &sig->params);
}

/*
 * Prototypes for the other-package functions a generic instance body calls.
 *
 * An instance is emitted as a `static inline` body appended to *this* module's
 * header, but the module whose header that is need not be the one declaring
 * the callee: `Vector<str>.zeros` lands in str's header and calls mem's
 * allocator. str.h is included well before mem.h declares it, so the call
 * compiled only by C89 implicit declaration - which gcc 16 rejects outright,
 * and which every other compiler resolved to a wrong `int` return. Same
 * reasoning as codegen_header.c's hdr_impl_prototypes, one scope out.
 *
 * Only callees actually reached from an instance body are declared, so a
 * package nothing instantiates against costs nothing. Each is guarded, since
 * the owning module emits the same prototype itself.
 */
static void inst_pkg_protos(cg_t *cg, ast_node_t *n, sb_t *h)
{
    if (!n) return;
    if (n->kind == AST_CALL && n->a && n->a->kind == AST_MEMBER && n->a->a &&
        n->a->a->kind == AST_IDENTIFIER && n->a->name) {
        symbol_t *pk = scope_lookup(cg->sem->global, n->a->a->name);
        if (pk && pk->kind == SYM_PACKAGE && pk->members && pk->pkgname &&
            (!cg->pkg || strcmp(pk->pkgname, cg->pkg) != 0)) {
            symbol_t *fn = scope_lookup_local(pk->members, n->a->name);
            if (fn && fn->kind == SYM_FUNC) {
                const char *save = cg->pkg;
                size_t k = 0;
                /* func_signature mangles with cg->pkg; borrow the callee's. */
                cg->pkg = pk->pkgname;
                for (; k < fn->overloads.len; k++) {
                    func_sig_t *sig = (func_sig_t *)fn->overloads.data[k];
                    if (!sig || !sig->decl) continue;
                    if (sig->decl->is_extern || sig->decl->synthetic) continue;
                    if (sig->decl->typarams.len > 0) continue;
                    {
                        const char *nm = cg_mangle_in(cg, pk->pkgname, NULL,
                                                      sig->decl->name, &sig->params);
                        sb_puts(h,
                                cg_fmt(cg,
                                       "#ifndef SALAM_PP_%s\n#define SALAM_PP_%s\n"
                                       "%s;\n#endif\n",
                                       nm, nm,
                                       func_signature(cg, sig->decl, NULL, sig, false)));
                    }
                }
                cg->pkg = save;
            }
        }
    }
    inst_pkg_protos(cg, n->type, h);
    inst_pkg_protos(cg, n->a, h);
    inst_pkg_protos(cg, n->b, h);
    inst_pkg_protos(cg, n->c, h);
    inst_pkg_protos(cg, n->d, h);
    {
        size_t i = 0;
        for (; i < n->list.len; i++)
            inst_pkg_protos(cg, (ast_node_t *)n->list.data[i], h);
    }
    {
        size_t i = 0;
        for (; i < n->dims.len; i++)
            inst_pkg_protos(cg, (ast_node_t *)n->dims.data[i], h);
    }
}

static void emit_instances_header(cg_t *cg, ast_node_t *program)
{
    sb_t *h = cg->h;

    {
        size_t i = 0;
        for (; i < program->list.len; i++) {
            ast_node_t *d = (ast_node_t *)program->list.data[i];
            if (d->synthetic) inst_pkg_protos(cg, d, h);
        }
    }

    {
        size_t i = 0;
        for (; i < program->list.len; i++) {
            ast_node_t *d = (ast_node_t *)program->list.data[i];
            if (!d->synthetic) continue;
            if (d->kind == AST_FUNC_DEF && !d->is_extern) {
                symbol_t *fsym = scope_lookup_local(cg->sem->global, d->name);
                func_sig_t *sig = sig_of_decl(fsym, d);
                if (!sig) continue;
                const char *nm = inst_fn_name(cg, NULL, d, sig);
                sb_puts(h,
                        cg_fmt(cg,
                               "#ifndef SALAM_IP_%s\n#define SALAM_IP_%s\n%s;\n#endif\n",
                               nm, nm, func_signature(cg, d, NULL, sig, false)));
            } else if (d->kind == AST_STRUCT_DEF) {
                symbol_t *ssym = scope_lookup_local(cg->sem->global, d->name);
                if (!ssym) continue;
                {
                    size_t j = 0;
                    for (; j < d->list.len; j++) {
                        ast_node_t *m = (ast_node_t *)d->list.data[j];
                        if (m->kind != AST_FUNC_DEF) continue;
                        symbol_t *msym = scope_lookup_local(ssym->members, m->name);
                        func_sig_t *sig = sig_of_decl(msym, m);
                        if (!sig) continue;
                        const char *nm = inst_fn_name(cg, ssym, m, sig);
                        sb_puts(
                            h,
                            cg_fmt(
                                cg,
                                "#ifndef SALAM_IP_%s\n#define SALAM_IP_%s\n%s;\n#endif\n",
                                nm, nm, func_signature(cg, m, ssym, sig, false)));
                    }
                }
            }
        }
    }

    sb_t *savec = cg->c;
    cg->c = h;
    {
        size_t i = 0;
        for (; i < program->list.len; i++) {
            ast_node_t *d = (ast_node_t *)program->list.data[i];
            if (!d->synthetic) continue;
            if (d->kind == AST_FUNC_DEF && !d->is_extern && d->a) {
                symbol_t *fsym = scope_lookup_local(cg->sem->global, d->name);
                func_sig_t *sig = sig_of_decl(fsym, d);
                if (!sig) continue;
                const char *nm = inst_fn_name(cg, NULL, d, sig);
                sb_puts(h,
                        cg_fmt(cg, "#ifndef SALAM_IB_%s\n#define SALAM_IB_%s\n", nm, nm));
                cg_function(cg, d, NULL);
                sb_puts(h, "#endif\n");
            } else if (d->kind == AST_STRUCT_DEF) {
                symbol_t *ssym = scope_lookup_local(cg->sem->global, d->name);
                if (!ssym) continue;
                {
                    size_t j = 0;
                    for (; j < d->list.len; j++) {
                        ast_node_t *m = (ast_node_t *)d->list.data[j];
                        if (m->kind != AST_FUNC_DEF) continue;
                        symbol_t *msym = scope_lookup_local(ssym->members, m->name);
                        func_sig_t *sig = sig_of_decl(msym, m);
                        if (!sig) continue;
                        const char *nm = inst_fn_name(cg, ssym, m, sig);
                        sb_puts(h,
                                cg_fmt(cg, "#ifndef SALAM_IB_%s\n#define SALAM_IB_%s\n",
                                       nm, nm));
                        cg_function(cg, m, ssym);
                        sb_puts(h, "#endif\n");
                    }
                }
            }
        }
    }
    cg->c = savec;
}

static const char *cg_tidy(arena_t *a, const char *src)
{
    sb_t out;
    sb_init(&out);
    bool emitted = false;
    bool pending_blank = false;
    {
        const char *p = src;
        for (; *p;) {
            const char *nl = strchr(p, '\n');
            size_t len = nl ? (size_t)(nl - p) : strlen(p);
            size_t end = len;
            while (end > 0 &&
                   (p[end - 1] == '\r' || p[end - 1] == ' ' || p[end - 1] == '\t'))
                end--;
            if (end == 0) {
                if (emitted) pending_blank = true;
            } else {
                if (pending_blank) {
                    sb_putc(&out, '\n');
                    pending_blank = false;
                }
                {
                    size_t i = 0;
                    for (; i < end; i++)
                        sb_putc(&out, p[i]);
                }
                sb_putc(&out, '\n');
                emitted = true;
            }
            if (!nl) break;
            p = nl + 1;
        }
    }
    const char *r = arena_strdup(a, sb_cstr(&out));
    sb_free(&out);
    return r;
}

static bool node_uses_spawn(ast_node_t *n)
{
    if (!n) return false;
    if (n->kind == AST_CALL && n->a && n->a->kind == AST_IDENTIFIER && n->a->name &&
        strcmp(n->a->name, "spawn") == 0)
        return true;
    if (node_uses_spawn(n->a) || node_uses_spawn(n->b) || node_uses_spawn(n->c) ||
        node_uses_spawn(n->d))
        return true;
    {
        size_t i = 0;
        for (; i < n->list.len; i++)
            if (node_uses_spawn((ast_node_t *)n->list.data[i])) return true;
    }
    return false;
}

bool salam_module_single_threaded(ast_node_t *program)
{
    size_t i = 0;
    for (; i < program->list.len; i++) {
        ast_node_t *d = (ast_node_t *)program->list.data[i];
        if (d->kind == AST_IMPORT) {
            const char *p =
                (d->value.kind == TV_STRING && d->value.as.s) ? d->value.as.s : d->name;
            if (p && strstr(p, "thread")) return false;
        }
        if (node_uses_spawn(d)) return false;
    }
    return true;
}

codegen_output_t *codegen_run(arena_t *a, logger_t *log, ast_node_t *program,
                              sema_result_t *sem, const char *module, bool safe,
                              bool debug_info, const char *src_path, const char *entry,
                              const char *target_triple)
{
    cg_t cg;
    memset(&cg, 0, sizeof(cg));
    cg.a = a;
    cg.log = log;
    cg.sem = sem;
    cg.module = module;
    cg.safe = safe;
    cg.debug_info = debug_info;
    cg.single_threaded = salam_module_single_threaded(program);
    cg.src_path = src_path ? src_path : "";
    cg.pkg = program->name ? program->name : "main";
    cg.entry = (entry && entry[0]) ? entry : "main";
    cg.target_triple = target_triple;
    cg.is_gui_mode = detect_gui_imports(program);
    vec_init(&cg.locals);
    vec_init(&cg.vec_types);
    vec_init(&cg.dyn_ifaces);
    vec_init(&cg.dyn_impls);
    vec_init(&cg.deferred);
    vec_init(&cg.fn_defers);
    cg.nloop = 0;
    sb_t h, c, lamd, lamf;
    sb_init(&h);
    sb_init(&c);
    sb_init(&lamd);
    sb_init(&lamf);
    cg.h = &h;
    cg.c = &c;
    cg.lam_decls = &lamd;
    cg.lam_defs = &lamf;
    cg.lam_n = 0;
    LOG_I(log, PH_CODEGEN, "generating C for module '%s'", module);
    demote_nonexportable_inlines(&cg, program);
    cg_header(&cg, program);
    emit_instances_header(&cg, program);
    emit_inline_header(&cg, program);
    sb_puts(&h, "\n#endif\n");
    sb_puts(&c, cg_fmt(&cg, "#include \"%s%s.h\"\n\n", SALAM_MOD_PREFIX, module));

    cg_emit_dyn_vtables(&cg, &c);
    emit_private_protos(&cg, program);
    emit_impl_protos(&cg);

    cg_lift_walk(&cg, program);
    if (lamd.len) {
        sb_puts(&c, sb_cstr(&lamd));
        sb_putc(&c, '\n');
    }
    emit_globals(&cg, program);

    if (lamf.len) {
        sb_puts(&c, sb_cstr(&lamf));
        sb_putc(&c, '\n');
    }
    emit_function_bodies(&cg, program);
    emit_impl_bodies(&cg);
    codegen_output_t *out = (codegen_output_t *)arena_alloc(a, sizeof(*out));
    out->module = module;
    out->h_src = cg_tidy(a, sb_cstr(&h));
    out->c_src = cg_tidy(a, sb_cstr(&c));
    sb_free(&h);
    sb_free(&c);
    sb_free(&lamd);
    sb_free(&lamf);
    LOG_I(log, PH_CODEGEN, "generated %zu bytes of C", strlen(out->c_src));
    return out;
}
