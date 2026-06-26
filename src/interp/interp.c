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

#include "interp/interp_internal.h"
#include "i18n/i18n.h"
#include "core/sal_format.h"   

SALAM_NORETURN void rt_error(interp_t *I, ast_node_t *n, const char *fmt, ...)
{
    va_list ap; va_start(ap, fmt);
    sal_vsnprintf(I->errmsg, sizeof I->errmsg, i18n_tr(fmt), ap);   
    va_end(ap);
    if (n) { I->errspan = n->span; I->have_errspan = true; }
    longjmp(I->on_error, 1);
}

void tick(interp_t *I)
{
    if ((++I->steps & 0xFFFFF) == 0 && I->deadline && clock() > I->deadline)
        rt_error(I, NULL, "execution timed out (possible infinite loop)");
}
env_t *env_new(interp_t *I, env_t *parent)
{
    env_t *e = (env_t *)arena_alloc(I->a, sizeof *e);
    e->parent = parent; vec_init(&e->bindings);
    return e;
}
binding_t *env_find_local(env_t *e, const char *name)
{
    { size_t i = 0; for (; i < e->bindings.len; i++) {
        binding_t *b = (binding_t *)e->bindings.data[i];
        if (strcmp(b->name, name) == 0) return b;
    } }
    return NULL;
}
binding_t *env_find(env_t *e, const char *name)
{
    for (; e; e = e->parent) {
        binding_t *b = env_find_local(e, name);
        if (b) return b;
    }
    return NULL;
}

void env_define(interp_t *I, env_t *e, const char *name, value_t v)
{
    binding_t *b = env_find_local(e, name);     
    if (b) { b->val = v; return; }
    b = (binding_t *)arena_alloc(I->a, sizeof *b);
    b->name = name; b->val = v;
    vec_push(I->a, &e->bindings, b);
}

ast_node_t *find_struct(interp_t *I, const char *name)
{
    { size_t i = 0; for (; i < I->structs.len; i++) {
        ast_node_t *s = (ast_node_t *)I->structs.data[i];
        if (s->name && strcmp(s->name, name) == 0) return s;
    } }
    return NULL;
}

ast_node_t *find_enum(interp_t *I, const char *name)
{
    { size_t i = 0; for (; i < I->enums.len; i++) {
        ast_node_t *e = (ast_node_t *)I->enums.data[i];
        if (e->name && strcmp(e->name, name) == 0) return e;
    } }
    return NULL;
}

ast_node_t *find_func(interp_t *I, const char *name, size_t nargs)
{
    ast_node_t *by_name = NULL;
    { size_t i = 0; for (; i < I->funcs.len; i++) {
        ast_node_t *f = (ast_node_t *)I->funcs.data[i];
        if (!f->name || strcmp(f->name, name) != 0) continue;
        if (f->list.len == nargs) return f;
        if (!by_name) by_name = f;
    } }
    return by_name;
}
module_t *find_module(interp_t *I, const char *name)
{
    { size_t i = 0; for (; i < I->modules.len; i++) {
        module_t *m = (module_t *)I->modules.data[i];
        if (strcmp(m->name, name) == 0) return m;
    } }
    return NULL;
}

ast_node_t *find_impl_method(interp_t *I, const char *typestr, const char *method, size_t nargs)
{
    if (!typestr) return NULL;
    { size_t i = 0; for (; i < I->impls.len; i++) {
        ast_node_t *d = (ast_node_t *)I->impls.data[i];
        const char *tt = d->type ? d->type->type_str : NULL;
        if (!tt || strcmp(tt, typestr) != 0) continue;
        ast_node_t *m = struct_method_arity(d, method, nargs);
        if (!m) m = struct_method(d, method);
        if (m) return m;
    } }
    return NULL;
}

ast_node_t *struct_method(ast_node_t *sdef, const char *name)
{
    { size_t i = 0; for (; i < sdef->list.len; i++) {
        ast_node_t *m = (ast_node_t *)sdef->list.data[i];
        if (m->kind == AST_FUNC_DEF && m->name && strcmp(m->name, name) == 0) return m;
    } }
    return NULL;
}

ast_node_t *struct_method_arity(ast_node_t *sdef, const char *name, size_t nparams)
{
    { size_t i = 0; for (; i < sdef->list.len; i++) {
        ast_node_t *m = (ast_node_t *)sdef->list.data[i];
        if (m->kind == AST_FUNC_DEF && m->name && strcmp(m->name, name) == 0 &&
            m->list.len == nparams) return m;
    } }
    return NULL;
}

static void collect_decls(interp_t *I, ast_node_t *program)
{
    { size_t i = 0; for (; i < program->list.len; i++) {
        ast_node_t *d = (ast_node_t *)program->list.data[i];
        switch (d->kind) {
            case AST_FUNC_DEF:
                if (d->is_extern) break;   
                vec_push(I->a, &I->funcs, d);
                break;
            case AST_STRUCT_DEF: vec_push(I->a, &I->structs, d); break;
            case AST_ENUM_DEF:   vec_push(I->a, &I->enums, d);   break;
            case AST_IMPL_DEF:   vec_push(I->a, &I->impls, d);   break;

            case AST_CONST_DECL:
            case AST_VAR_DECL: {
                value_t v = d->a ? eval(I, I->globals, d->a) : val_null();
                env_define(I, I->globals, d->name, v);
                break;
            }
            default: break;
        }
    } }
    
    { size_t i = 0; for (; i < I->funcs.len; i++) {
        ast_node_t *f = (ast_node_t *)I->funcs.data[i];
        if (f->name && !env_find_local(I->globals, f->name))
            env_define(I, I->globals, f->name, mk_closure(I, f, I->globals));
    } }
}

static void collect_module_funcs(interp_t *I, module_t *mod, ast_node_t *prog)
{
    { size_t i = 0; for (; i < prog->list.len; i++) {
        ast_node_t *d = (ast_node_t *)prog->list.data[i];
        switch (d->kind) {
            case AST_FUNC_DEF:
                if (d->is_extern) break;   
                if (d->name && !env_find_local(mod->env, d->name))
                    env_define(I, mod->env, d->name, mk_closure(I, d, mod->env));
                break;
            
            case AST_STRUCT_DEF: if (!find_struct(I, d->name)) vec_push(I->a, &I->structs, d); break;
            case AST_ENUM_DEF:   if (!find_enum(I, d->name))   vec_push(I->a, &I->enums, d);   break;
            case AST_IMPL_DEF:   vec_push(I->a, &I->impls, d); break;
            default: break;
        }
    } }
}

static void wire_imports(interp_t *I, env_t *env, scope_t *scope)
{
    if (!scope) return;
    { size_t i = 0; for (; i < scope->symbols.len; i++) {
        symbol_t *sym = (symbol_t *)scope->symbols.data[i];
        if (sym->kind != SYM_PACKAGE || !sym->pkgname) continue;
        module_t *target = find_module(I, sym->pkgname);
        if (target) env_define(I, env, sym->name, val_module(target));
    } }
}

static void eval_module_consts(interp_t *I, module_t *mod, ast_node_t *prog)
{
    { size_t i = 0; for (; i < prog->list.len; i++) {
        ast_node_t *d = (ast_node_t *)prog->list.data[i];
        if (d->kind == AST_CONST_DECL || d->kind == AST_VAR_DECL)
            env_define(I, mod->env, d->name, d->a ? eval(I, mod->env, d->a) : val_null());
    } }
}

void build_modules(interp_t *I, ast_node_t *program)
{
    (void)program;
    sema_result_t *sem = I->sem;
    if (!sem) return;
    vec_t *pkgs = &sem->packages;
    
    { size_t i = 0; for (; i < pkgs->len; i++) {
        symbol_t *pk = (symbol_t *)pkgs->data[i];
        const char *name = pk->pkgname ? pk->pkgname : (pk->decl ? pk->decl->name : NULL);
        if (!name || !pk->decl || find_module(I, name)) continue;
        module_t *mod = (module_t *)arena_alloc(I->a, sizeof *mod);
        mod->name = name;
        mod->env  = env_new(I, NULL);   
        vec_push(I->a, &I->modules, mod);
        collect_module_funcs(I, mod, pk->decl);
    } }
    
    { size_t i = 0; for (; i < pkgs->len; i++) {
        symbol_t *pk = (symbol_t *)pkgs->data[i];
        const char *name = pk->pkgname ? pk->pkgname : (pk->decl ? pk->decl->name : NULL);
        module_t *mod = name ? find_module(I, name) : NULL;
        if (mod) wire_imports(I, mod->env, pk->members);
    } }
    wire_imports(I, I->globals, sem->global);
    
    { size_t i = 0; for (; i < pkgs->len; i++) {
        symbol_t *pk = (symbol_t *)pkgs->data[i];
        const char *name = pk->pkgname ? pk->pkgname : (pk->decl ? pk->decl->name : NULL);
        module_t *mod = name ? find_module(I, name) : NULL;
        if (mod && pk->decl) eval_module_consts(I, mod, pk->decl);
    } }
}

int interp_run(arena_t *a, logger_t *log, ast_node_t *program,
               sema_result_t *sem, const char *entry, const interp_options_t *opts)
{
    interp_t I;
    memset(&I, 0, sizeof I);
    I.a = a; I.log = log; I.sem = sem; I.entry = entry;
    I.out  = opts && opts->out ? opts->out : stdout;
    I.err  = opts && opts->err ? opts->err : stderr;
    I.lang = opts && opts->lang ? opts->lang : "en";
    I.in_data = opts ? opts->input_data : NULL;
    
    {
        int ms = opts && opts->timeout_ms > 0 ? opts->timeout_ms : 5000;
        I.deadline = clock() + (clock_t)((double)ms / 1000.0 * CLOCKS_PER_SEC);
    }
    I.globals = env_new(&I, NULL);
    vec_init(&I.funcs); vec_init(&I.structs); vec_init(&I.enums); vec_init(&I.modules);
    if (setjmp(I.on_error)) {
        if (I.have_errspan)
            LOG_E(log, PH_DRIVER, i18n_tr("runtime error at line %u: %s"), I.errspan.begin.line, I.errmsg);
        else
            LOG_E(log, PH_DRIVER, i18n_tr("runtime error: %s"), I.errmsg);
        fflush(I.out);
        return 1;
    }
    collect_decls(&I, program);
    build_modules(&I, program);   
    ast_node_t *main_fn = find_func(&I, entry, (size_t)-1);
    if (!main_fn) {
        LOG_E(log, PH_DRIVER, i18n_tr("no entry point: define a '%s' function"), entry);
        return 1;
    }
    value_t rc = call_func(&I, main_fn, I.globals, NULL, NULL, 0);
    fflush(I.out);
    return rc.kind == VAL_INT ? (int)rc.as.i : 0;
}
