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
    va_list ap;
    va_start(ap, fmt);
    sal_vsnprintf(I->errmsg, sizeof I->errmsg, i18n_tr(fmt), ap);
    va_end(ap);
    if (n) {
        I->errspan = n->span;
        I->have_errspan = true;
    }
    longjmp(I->on_error, 1);
}

void tick(interp_t *I)
{
    if ((++I->steps & 0xFFFFF) == 0 && I->deadline && clock() > I->deadline)
        rt_error(I, NULL, "execution timed out (possible infinite loop)");
}

static size_t itab_hash(const char *s)
{
    uint64_t h = 1469598103934665603ULL;
    const unsigned char *p = (const unsigned char *)s;
    for (; *p; p++) {
        h ^= *p;
        h *= 1099511628211ULL;
    }
    return (size_t)h;
}

static size_t ptab_hash(const void *p)
{
    uint64_t x = (uint64_t)(uintptr_t)p;
    x ^= x >> 33;
    x *= 0xff51afd7ed558ccdULL;
    x ^= x >> 33;
    return (size_t)x;
}

static void itab_grow(interp_t *I, itab_t *t, size_t want)
{
    const char **ok = t->keys;
    void **ov = t->vals;
    size_t oc = t->cap;
    size_t nc = 16;
    while (nc < want * 2)
        nc *= 2;
    t->keys = (const char **)arena_alloc(I->a, salam_size_mul(nc, sizeof(char *)));
    t->vals = (void **)arena_alloc(I->a, salam_size_mul(nc, sizeof(void *)));
    memset(t->keys, 0, salam_size_mul(nc, sizeof(char *)));
    t->cap = nc;
    {
        size_t i = 0;
        for (; i < oc; i++) {
            if (!ok[i]) continue;
            {
                size_t mask = nc - 1;
                size_t j = itab_hash(ok[i]) & mask;
                while (t->keys[j])
                    j = (j + 1) & mask;
                t->keys[j] = ok[i];
                t->vals[j] = ov[i];
            }
        }
    }
}

static void *itab_get(itab_t *t, const char *key)
{
    size_t mask, i;
    if (!t->cap) return NULL;
    mask = t->cap - 1;
    i = itab_hash(key) & mask;
    for (; t->keys[i]; i = (i + 1) & mask)
        if (strcmp(t->keys[i], key) == 0) return t->vals[i];
    return NULL;
}

static void itab_put(interp_t *I, itab_t *t, const char *key, void *val)
{
    size_t mask, i;
    if ((t->count + 1) * 4 >= t->cap * 3) itab_grow(I, t, t->count + 1);
    mask = t->cap - 1;
    i = itab_hash(key) & mask;
    for (; t->keys[i]; i = (i + 1) & mask)
        if (strcmp(t->keys[i], key) == 0) return;
    t->keys[i] = key;
    t->vals[i] = val;
    t->count++;
}

static void *ptab_get(ptab_t *t, const void *key)
{
    size_t mask, i;
    if (!t->cap) return NULL;
    mask = t->cap - 1;
    i = ptab_hash(key) & mask;
    for (; t->keys[i]; i = (i + 1) & mask)
        if (t->keys[i] == key) return t->vals[i];
    return NULL;
}

static void ptab_put(interp_t *I, ptab_t *t, const void *key, void *val)
{
    size_t mask, i;
    if ((t->count + 1) * 4 >= t->cap * 3) {
        const void **ok = t->keys;
        void **ov = t->vals;
        size_t oc = t->cap;
        size_t nc = 16;
        while (nc < (t->count + 1) * 2)
            nc *= 2;
        t->keys = (const void **)arena_alloc(I->a, salam_size_mul(nc, sizeof(void *)));
        t->vals = (void **)arena_alloc(I->a, salam_size_mul(nc, sizeof(void *)));
        memset(t->keys, 0, salam_size_mul(nc, sizeof(void *)));
        t->cap = nc;
        {
            size_t k = 0;
            for (; k < oc; k++) {
                if (!ok[k]) continue;
                {
                    size_t m2 = nc - 1;
                    size_t j = ptab_hash(ok[k]) & m2;
                    while (t->keys[j])
                        j = (j + 1) & m2;
                    t->keys[j] = ok[k];
                    t->vals[j] = ov[k];
                }
            }
        }
    }
    mask = t->cap - 1;
    i = ptab_hash(key) & mask;
    for (; t->keys[i]; i = (i + 1) & mask)
        if (t->keys[i] == key) return;
    t->keys[i] = key;
    t->vals[i] = val;
    t->count++;
}

#define ENV_INDEX_THRESHOLD 12

env_t *env_new(interp_t *I, env_t *parent)
{
    env_t *e = (env_t *)arena_alloc(I->a, sizeof *e);
    e->parent = parent;
    vec_init(&e->bindings);
    e->index = NULL;
    return e;
}

binding_t *env_find_local(env_t *e, const char *name)
{
    if (e->index) return (binding_t *)itab_get(e->index, name);
    {
        size_t i = 0;
        for (; i < e->bindings.len; i++) {
            binding_t *b = (binding_t *)e->bindings.data[i];
            if (strcmp(b->name, name) == 0) return b;
        }
    }
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
    if (b) {
        b->val = v;
        return;
    }
    b = (binding_t *)arena_alloc(I->a, sizeof *b);
    b->name = name;
    b->val = v;
    vec_push(I->a, &e->bindings, b);
    if (e->index) {
        itab_put(I, e->index, name, b);
    } else if (e->bindings.len >= ENV_INDEX_THRESHOLD) {
        e->index = (itab_t *)arena_alloc(I->a, sizeof *e->index);
        memset(e->index, 0, sizeof *e->index);
        {
            size_t i = 0;
            for (; i < e->bindings.len; i++) {
                binding_t *bi = (binding_t *)e->bindings.data[i];
                itab_put(I, e->index, bi->name, bi);
            }
        }
    }
}

ast_node_t *find_struct(interp_t *I, const char *name)
{
    return (ast_node_t *)itab_get(&I->tab_structs, name);
}

ast_node_t *find_enum(interp_t *I, const char *name)
{
    return (ast_node_t *)itab_get(&I->tab_enums, name);
}

static void register_func(interp_t *I, ast_node_t *f)
{
    vec_t *l;
    vec_push(I->a, &I->funcs, f);
    if (!f->name) return;
    l = (vec_t *)itab_get(&I->tab_funcs, f->name);
    if (!l) {
        l = (vec_t *)arena_alloc(I->a, sizeof *l);
        vec_init(l);
        itab_put(I, &I->tab_funcs, f->name, l);
    }
    vec_push(I->a, l, f);
}

ast_node_t *find_func(interp_t *I, const char *name, size_t nargs)
{
    vec_t *l = (vec_t *)itab_get(&I->tab_funcs, name);
    if (!l || !l->len) return NULL;
    {
        size_t i = 0;
        for (; i < l->len; i++) {
            ast_node_t *f = (ast_node_t *)l->data[i];
            if (f->list.len == nargs) return f;
        }
    }
    return (ast_node_t *)l->data[0];
}

module_t *find_module(interp_t *I, const char *name)
{
    return (module_t *)itab_get(&I->tab_modules, name);
}

value_t *find_extern_fn(interp_t *I, const char *name)
{
    return (value_t *)itab_get(&I->tab_extern_fns, name);
}

ast_node_t *find_extern_decl(interp_t *I, const char *name)
{
    return (ast_node_t *)itab_get(&I->tab_extern_decls, name);
}

ast_node_t *find_impl_method(interp_t *I, const char *typestr, const char *method,
                             size_t nargs)
{
    if (!typestr) return NULL;
    {
        size_t i = 0;
        for (; i < I->impls.len; i++) {
            ast_node_t *d = (ast_node_t *)I->impls.data[i];
            const char *tt = d->type ? d->type->type_str : NULL;
            if (!tt || strcmp(tt, typestr) != 0) continue;
            ast_node_t *m = struct_method_arity(d, method, nargs);
            if (!m) m = struct_method(d, method);
            if (m) return m;
        }
    }
    return NULL;
}

ast_node_t *struct_method(ast_node_t *sdef, const char *name)
{
    {
        size_t i = 0;
        for (; i < sdef->list.len; i++) {
            ast_node_t *m = (ast_node_t *)sdef->list.data[i];
            if (m->kind == AST_FUNC_DEF && m->name && strcmp(m->name, name) == 0)
                return m;
        }
    }
    return NULL;
}

ast_node_t *struct_method_arity(ast_node_t *sdef, const char *name, size_t nparams)
{
    {
        size_t i = 0;
        for (; i < sdef->list.len; i++) {
            ast_node_t *m = (ast_node_t *)sdef->list.data[i];
            if (m->kind == AST_FUNC_DEF && m->name && strcmp(m->name, name) == 0 &&
                m->list.len == nparams)
                return m;
        }
    }
    return NULL;
}

typedef struct {
    ast_node_t *def;
    env_t *env;
} def_env_entry_t;

static void register_def_env_one(interp_t *I, ast_node_t *def, env_t *env)
{
    def_env_entry_t *e = (def_env_entry_t *)arena_alloc(I->a, sizeof *e);
    e->def = def;
    e->env = env;
    vec_push(I->a, &I->def_envs, e);
    ptab_put(I, &I->tab_def_envs, def, env);
}

void register_method_envs(interp_t *I, ast_node_t *def, env_t *env)
{
    register_def_env_one(I, def, env);
    size_t i = 0;
    for (; i < def->list.len; i++) {
        ast_node_t *m = (ast_node_t *)def->list.data[i];
        if (m->kind != AST_FUNC_DEF) continue;
        register_def_env_one(I, m, env);
    }
}

env_t *find_def_env(interp_t *I, ast_node_t *def)
{
    return (env_t *)ptab_get(&I->tab_def_envs, def);
}

static void register_extern_decl(interp_t *I, env_t *env, ast_node_t *d)
{
    if (d->a) {
        value_t cl = mk_closure(I, d, env);
        value_t *box = (value_t *)arena_alloc(I->a, sizeof *box);
        *box = cl;
        vec_push(I->a, &I->extern_fns, box);
        if (d->name) itab_put(I, &I->tab_extern_fns, d->name, box);
        if (d->name && !env_find_local(env, d->name)) env_define(I, env, d->name, cl);
    } else {
        vec_push(I->a, &I->extern_decls, d);
        if (d->name) itab_put(I, &I->tab_extern_decls, d->name, d);
    }
}

static void collect_decls(interp_t *I, ast_node_t *program)
{
    {
        size_t i = 0;
        for (; i < program->list.len; i++) {
            ast_node_t *d = (ast_node_t *)program->list.data[i];
            switch (d->kind) {
            case AST_FUNC_DEF:
                if (d->is_extern) {
                    register_extern_decl(I, I->globals, d);
                    break;
                }
                register_func(I, d);
                break;
            case AST_STRUCT_DEF:
                vec_push(I->a, &I->structs, d);
                if (d->name) itab_put(I, &I->tab_structs, d->name, d);
                if (!d->synthetic) register_method_envs(I, d, I->globals);
                break;
            case AST_ENUM_DEF:
                vec_push(I->a, &I->enums, d);
                if (d->name) itab_put(I, &I->tab_enums, d->name, d);
                break;
            case AST_IMPL_DEF:
                vec_push(I->a, &I->impls, d);
                if (!d->synthetic) register_method_envs(I, d, I->globals);
                break;

            case AST_CONST_DECL:
            case AST_VAR_DECL: {
                value_t v = d->a ? eval(I, I->globals, d->a) : val_null();
                env_define(I, I->globals, d->name, v);
                break;
            }
            default:
                break;
            }
        }
    }

    {
        size_t i = 0;
        for (; i < I->funcs.len; i++) {
            ast_node_t *f = (ast_node_t *)I->funcs.data[i];
            if (f->name && !env_find_local(I->globals, f->name))
                env_define(I, I->globals, f->name, mk_closure(I, f, I->globals));
        }
    }
}

static void collect_module_funcs(interp_t *I, module_t *mod, ast_node_t *prog)
{
    {
        size_t i = 0;
        for (; i < prog->list.len; i++) {
            ast_node_t *d = (ast_node_t *)prog->list.data[i];
            switch (d->kind) {
            case AST_FUNC_DEF:
                if (d->is_extern) {
                    register_extern_decl(I, mod->env, d);
                    break;
                }
                if (d->name && !env_find_local(mod->env, d->name))
                    env_define(I, mod->env, d->name, mk_closure(I, d, mod->env));
                break;

            case AST_STRUCT_DEF:
                if (!find_struct(I, d->name)) {
                    vec_push(I->a, &I->structs, d);
                    if (d->name) itab_put(I, &I->tab_structs, d->name, d);
                    if (!d->synthetic) register_method_envs(I, d, mod->env);
                }
                break;
            case AST_ENUM_DEF:
                if (!find_enum(I, d->name)) {
                    vec_push(I->a, &I->enums, d);
                    if (d->name) itab_put(I, &I->tab_enums, d->name, d);
                }
                break;
            case AST_IMPL_DEF:
                vec_push(I->a, &I->impls, d);
                if (!d->synthetic) register_method_envs(I, d, mod->env);
                break;
            default:
                break;
            }
        }
    }
}

static void wire_imports(interp_t *I, env_t *env, scope_t *scope)
{
    if (!scope) return;
    {
        size_t i = 0;
        for (; i < scope->symbols.len; i++) {
            symbol_t *sym = (symbol_t *)scope->symbols.data[i];
            if (sym->kind != SYM_PACKAGE || !sym->pkgname) continue;
            module_t *target = find_module(I, sym->pkgname);
            if (target) env_define(I, env, sym->name, val_module(target));
        }
    }
}

static void eval_module_consts(interp_t *I, module_t *mod, ast_node_t *prog)
{
    {
        size_t i = 0;
        for (; i < prog->list.len; i++) {
            ast_node_t *d = (ast_node_t *)prog->list.data[i];
            if (d->kind == AST_CONST_DECL || d->kind == AST_VAR_DECL)
                env_define(I, mod->env, d->name,
                           d->a ? eval(I, mod->env, d->a) : val_null());
        }
    }
}

void build_modules(interp_t *I, ast_node_t *program)
{
    (void)program;
    sema_result_t *sem = I->sem;
    if (!sem) return;
    vec_t *pkgs = &sem->packages;

    {
        size_t i = 0;
        for (; i < pkgs->len; i++) {
            symbol_t *pk = (symbol_t *)pkgs->data[i];
            const char *name =
                pk->pkgname ? pk->pkgname : (pk->decl ? pk->decl->name : NULL);
            if (!name || !pk->decl || find_module(I, name)) continue;
            module_t *mod = (module_t *)arena_alloc(I->a, sizeof *mod);
            mod->name = name;
            mod->env = env_new(I, NULL);
            vec_push(I->a, &I->modules, mod);
            itab_put(I, &I->tab_modules, name, mod);
            collect_module_funcs(I, mod, pk->decl);
        }
    }

    {
        size_t i = 0;
        for (; i < pkgs->len; i++) {
            symbol_t *pk = (symbol_t *)pkgs->data[i];
            const char *name =
                pk->pkgname ? pk->pkgname : (pk->decl ? pk->decl->name : NULL);
            module_t *mod = name ? find_module(I, name) : NULL;
            if (mod) wire_imports(I, mod->env, pk->members);
        }
    }
    wire_imports(I, I->globals, sem->global);

    {
        size_t i = 0;
        for (; i < pkgs->len; i++) {
            symbol_t *pk = (symbol_t *)pkgs->data[i];
            const char *name =
                pk->pkgname ? pk->pkgname : (pk->decl ? pk->decl->name : NULL);
            module_t *mod = name ? find_module(I, name) : NULL;
            if (mod && pk->decl) eval_module_consts(I, mod, pk->decl);
        }
    }
}

static void fixup_generic_envs_in(interp_t *I, vec_t *defs)
{
    size_t i = 0;
    for (; i < defs->len; i++) {
        ast_node_t *d = (ast_node_t *)defs->data[i];
        if (!d->synthetic || !d->name || find_def_env(I, d)) continue;
        symbol_t *sym = I->sem ? scope_lookup(I->sem->global, d->name) : NULL;
        if (!sym || !sym->generic_base) continue;
        size_t j = 0;
        for (; j < defs->len; j++) {
            ast_node_t *bd = (ast_node_t *)defs->data[j];
            if (bd == d || bd->synthetic || !bd->name) continue;
            if (strcmp(bd->name, sym->generic_base) != 0) continue;
            env_t *base_env = find_def_env(I, bd);
            if (base_env) register_method_envs(I, d, base_env);
            break;
        }
    }
}

static void fixup_generic_envs(interp_t *I)
{
    fixup_generic_envs_in(I, &I->structs);
    fixup_generic_envs_in(I, &I->impls);
}

int interp_run(arena_t *a, logger_t *log, ast_node_t *program, sema_result_t *sem,
               const char *entry, const interp_options_t *opts)
{
    interp_t I;
    memset(&I, 0, sizeof I);
    I.a = a;
    I.log = log;
    I.sem = sem;
    I.entry = entry;
    I.out = opts && opts->out ? opts->out : stdout;
    I.err = opts && opts->err ? opts->err : stderr;
    I.lang = opts && opts->lang ? opts->lang : "en";
    I.in_data = opts ? opts->input_data : NULL;

    {
        int ms = opts && opts->timeout_ms > 0 ? opts->timeout_ms : 5000;
        I.deadline = clock() + (clock_t)((double)ms / 1000.0 * CLOCKS_PER_SEC);
    }
    I.globals = env_new(&I, NULL);
    vec_init(&I.funcs);
    vec_init(&I.structs);
    vec_init(&I.enums);
    vec_init(&I.modules);
    vec_init(&I.extern_fns);
    vec_init(&I.extern_decls);
    vec_init(&I.def_envs);
    if (setjmp(I.on_error)) {
        if (I.have_errspan)
            LOG_E(log, PH_DRIVER, i18n_tr("runtime error at line %u: %s"),
                  I.errspan.begin.line, I.errmsg);
        else
            LOG_E(log, PH_DRIVER, i18n_tr("runtime error: %s"), I.errmsg);
        fflush(I.out);
        return 1;
    }
    collect_decls(&I, program);
    build_modules(&I, program);
    fixup_generic_envs(&I);
    ast_node_t *main_fn = find_func(&I, entry, (size_t)-1);
    if (!main_fn) {
        LOG_E(log, PH_DRIVER, i18n_tr("no entry point: define a '%s' function"), entry);
        return 1;
    }
    value_t rc = call_func(&I, main_fn, I.globals, NULL, NULL, 0);
    fflush(I.out);
    return rc.kind == VAL_INT ? (int)rc.as.i : 0;
}
