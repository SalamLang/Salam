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

static bool sig_is_sret(func_sig_t *sig, bool is_main)
{
    return !is_main && sig && type_is_byval_agg(sig->ret);
}

const char *func_signature(cg_t *cg, ast_node_t *fn, symbol_t *owner, func_sig_t *sig,
                           bool is_main)
{
    bool sret = sig_is_sret(sig, is_main);
    const char *ret = is_main ? "int"
                      : sret  ? "void"
                              : cg_ctype(cg, type_to_string(cg->sem->tc, sig->ret));

    bool ti = owner && owner->kind == SYM_TYPEIMPL;
    const char *ti_ts = ti ? type_to_string(cg->sem->tc, owner->type) : NULL;

    bool is_instance = (owner && owner->generic_base) || fn->synthetic;
    const char *name =
        is_main         ? "main"
        : fn->is_extern ? fn->name
        : ti            ? cg_mangle_ti(cg, ti_ts, fn->name, &sig->params)
        : is_instance
            ? cg_mangle_in(cg, "g", owner ? owner->name : NULL, fn->name, &sig->params)
            : cg_mangle(cg, owner ? owner->name : NULL, fn->name, &sig->params);
    if (is_main) return "int main(int argc, char** argv)";
    bool pkgmod = cg->pkg && strcmp(cg->pkg, "main") != 0;
    bool exported =
        (owner != NULL || fn->is_extern || fn->is_pub || pkgmod) && !is_instance;
    /* Struct methods take `inline` on the same terms as free functions: a pub
       one is exported as a body in the header, a private one only stays inline
       where nothing links against it. Generic instances are static inline
       already, so is_instance covers those. */
    bool inl = fn->is_inline && !fn->is_extern && (fn->is_pub || !pkgmod);
    sb_t b;
    sb_init(&b);
    if (is_instance || inl)
        sb_puts(&b, "static inline ");
    else if (!exported)
        sb_puts(&b, "static ");
    if (!fn->is_extern) {
        if (fn->is_noinline) sb_puts(&b, "SALAM_NOINLINE ");
        if (fn->is_pure) sb_puts(&b, "SALAM_PURE ");
        if (fn->is_deprecated) sb_puts(&b, "SALAM_DEPRECATED ");
    }
    if (fn->is_noret) sb_puts(&b, "SALAM_NORET ");
    sb_puts(&b, cg_fmt(cg, "%s %s(", ret, name));
    bool first = true;
    if (owner) {
        if (ti)
            sb_puts(&b, cg_fmt(cg, "%s this", cg_ctype(cg, ti_ts)));
        else
            sb_puts(&b, cg_fmt(cg, "%s* this",
                               cg_ctype(cg, type_to_string(cg->sem->tc, owner->type))));
        first = false;
    }
    {
        size_t i = 0;
        for (; i < fn->list.len; i++) {
            ast_node_t *p = (ast_node_t *)fn->list.data[i];
            const char *pts = type_to_string(cg->sem->tc, (type_t *)sig->params.data[i]);
            if (!first) sb_puts(&b, ", ");
            if (p->is_ref)
                sb_puts(&b,
                        cg_fmt(cg, "%s* %s", cg_ctype(cg, pts), cg_cident(cg, p->name)));
            else
                sb_puts(&b, cg_decl(cg, pts, p->name));
            first = false;
        }
    }
    if (sret) {
        if (!first) sb_puts(&b, ", ");
        sb_puts(&b, cg_fmt(cg, "%s* __ret",
                           cg_ctype(cg, type_to_string(cg->sem->tc, sig->ret))));
        first = false;
    }
    if (first && !owner) sb_puts(&b, "void");
    sb_putc(&b, ')');
    const char *r = arena_strdup(cg->a, sb_cstr(&b));
    sb_free(&b);
    return r;
}

const char *cg_extern_proto(cg_t *cg, ast_node_t *fn, func_sig_t *sig)
{
    const char *ret = cg_ctype(cg, type_to_string(cg->sem->tc, sig->ret));
    sb_t b;
    sb_init(&b);
    sb_puts(&b, "extern ");
    /* No SALAM_PURE here: many externs (libc math builtins like sin/cos/
     * floor) are already declared `__attribute__((const))` by the system
     * headers, and redeclaring them `__attribute__((pure))` makes GCC/Clang
     * emit an "ignoring attribute" warning at every include site. Purity is
     * still enforced at the Salam level (sema_call.c) via fn->is_pure; this
     * only skips the redundant/conflicting C attribute on the declaration. */
    if (fn->is_noret) sb_puts(&b, "SALAM_NORET ");
    sb_puts(&b, cg_fmt(cg, "%s %s(", ret, fn->name));
    bool first = true;
    {
        size_t i = 0;
        for (; i < fn->list.len; i++) {
            ast_node_t *p = (ast_node_t *)fn->list.data[i];
            const char *pts = type_to_string(cg->sem->tc, (type_t *)sig->params.data[i]);
            if (!first) sb_puts(&b, ", ");
            if (p->is_ref)
                sb_puts(&b,
                        cg_fmt(cg, "%s* %s", cg_ctype(cg, pts), cg_cident(cg, p->name)));
            else
                sb_puts(&b, cg_decl(cg, pts, p->name));
            first = false;
        }
    }
    if (fn->is_variadic) {
        if (!first) sb_puts(&b, ", ");
        sb_puts(&b, "...");
        first = false;
    }
    if (first) sb_puts(&b, "void");
    sb_putc(&b, ')');
    const char *r = arena_strdup(cg->a, sb_cstr(&b));
    sb_free(&b);
    return r;
}

/*
 * Closes a non-void function whose body does not end in `ret` - one ending in
 * an if/else chain, a match, or a loop that returns from inside it. C calls
 * falling off such a function undefined behaviour, and clang says so
 * (-Wreturn-type: "does not return a value in all control paths"), so a zero
 * of the return type is returned instead. `= {0}` covers scalars, pointers
 * and structs alike where `(T)0` would not compile for the last of those.
 *
 * sret functions return void in C - the value goes through the `__ret` out-
 * parameter - and need nothing here, and neither does a body already ending
 * in a return.
 */
static void cg_fallthrough_ret(cg_t *cg, ast_node_t *fn, func_sig_t *sig)
{
    if (cg->cur_sret || sig->ret->kind == TY_VOID) return;
    /* A `noret` function carries SALAM_NORET, and returning from one of those
     * is its own diagnostic on gcc and clang. Falling off its end is the
     * point. */
    if (fn->is_noret) return;
    if (fn->a && fn->a->list.len > 0) {
        ast_node_t *last = (ast_node_t *)fn->a->list.data[fn->a->list.len - 1];
        if (last->kind == AST_RETURN) return;
    }
    {
        int t = ++cg->tmpn;
        const char *ct = cg_ctype(cg, type_to_string(cg->sem->tc, sig->ret));
        cg_line(cg, "%s __fret%d = {0};", ct, t);
        cg_line(cg, "return __fret%d;", t);
    }
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
    cg->cur_fn_home = fsym ? fsym->home : NULL;
    cg->cur_sret = sig_is_sret(sig, is_main);
    cg->cur_is_main = is_main;
    cg->locals.len = 0;
    if (owner) local_add(cg, "this");
    {
        size_t i = 0;
        for (; i < fn->list.len; i++)
            local_add(cg, ((ast_node_t *)fn->list.data[i])->name);
    }
    vec_t saved_defers = cg->fn_defers;
    int saved_nloop = cg->nloop;
    vec_init(&cg->fn_defers);
    cg->nloop = 0;
    cg_source_line(cg, &fn->span);

    if (is_main && cg->is_gui_mode) {
        sb_puts(cg->c, "#ifdef _WIN32\n");
        cg_line(
            cg,
            "int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow) {");
        sb_puts(cg->c,
                "    (void)hInst; (void)hPrevInst; (void)lpCmdLine; (void)nCmdShow;\n");
        sb_puts(cg->c, "#else\n");
        cg_line(cg, "int main(int argc, char** argv) {");
        sb_puts(cg->c, "#endif\n");
    } else {
        cg_line(cg, "%s {", func_signature(cg, fn, owner, sig, is_main));
    }

    cg->indent++;
    if (is_main) {
        if (!cg->is_gui_mode) {
            cg_line(cg, "salam_set_args(argc, argv);");
        } else {
            cg_line(cg, "#ifdef _WIN32");
            cg_line(cg, "(void)0;");
            cg_line(cg, "#else");
            cg_line(cg, "salam_set_args(argc, argv);");
            cg_line(cg, "#endif");
        }
        /* Runs this module's own non-constant global initialisers and, ahead
         * of them, every imported module's. See emit_module_init. */
        cg_line(cg, "%s();", cg_module_init_name(cg, cg->module));
    }
    if (fn->a) cg_block(cg, fn->a);
    cg_emit_defers(cg);
    if (is_main && sig->ret->kind == TY_VOID)
        cg_line(cg, "return 0;");
    else if (!is_main)
        cg_fallthrough_ret(cg, fn, sig);
    cg->indent--;
    cg_line(cg, "}");
    sb_putc(cg->c, '\n');
    cg->cur_struct = NULL;
    cg->cur_fn_home = NULL;
    cg->cur_sret = false;
    cg->fn_defers = saved_defers;
    cg->nloop = saved_nloop;
}
