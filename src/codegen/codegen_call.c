#include "codegen/codegen_internal.h"
#include "codegen/print_fmt.h"
#include "i18n/i18n.h"
#include "semantic/builtins.h"

static const char *arg_at(cg_t *cg, ast_node_t *n, size_t i)
{
    return n->list.len > i ? cg_expr(cg, (ast_node_t *)n->list.data[i]) : "0";
}

static const char *call_args(cg_t *cg, ast_node_t *call, func_sig_t *sig)
{
    sb_t b; sb_init(&b);
    { size_t i = 0; for (; i < call->list.len; i++) {
        ast_node_t *arg = (ast_node_t *)call->list.data[i];
        if (i) sb_puts(&b, ", ");
        bool arg_is_ref = sig && sig->decl && i < sig->decl->list.len &&
                          ((ast_node_t *)sig->decl->list.data[i])->is_ref;
        if (arg_is_ref) {
            
            type_t *pt = (i < sig->params.len) ? (type_t *)sig->params.data[i] : NULL;
            if (pt)
                sb_puts(&b, cg_fmt(cg, "(%s*)&(%s)",
                        cg_ctype(cg, type_to_string(cg->sem->tc, pt)), cg_expr(cg, arg)));
            else
                sb_puts(&b, cg_fmt(cg, "&(%s)", cg_expr(cg, arg)));
        }
        else            sb_puts(&b, cg_expr(cg, arg));
    } }
    const char *r = arena_strdup(cg->a, sb_cstr(&b)); sb_free(&b); return r;
}

static const char *call_args_lead(cg_t *cg, ast_node_t *call)
{
    sb_t b; sb_init(&b);
    { size_t i = 0; for (; i < call->list.len; i++)
        { sb_puts(&b, ", "); sb_puts(&b, cg_expr(cg, (ast_node_t *)call->list.data[i])); } }
    const char *r = arena_strdup(cg->a, sb_cstr(&b)); sb_free(&b); return r;
}

static const char *call_sret(cg_t *cg, type_t *ret, const char *mangled,
                             const char *args, bool has_args)
{
    const char *Rc = cg_ctype(cg, type_to_string(cg->sem->tc, ret));
    int t = ++cg->tmpn; const char *sep = has_args ? ", " : "";
    return cg_fmt(cg, "({ %s __s%d; %s(%s%s&__s%d); __s%d; })", Rc, t, mangled, args, sep, t, t);
}

static const char *call_value(cg_t *cg, ast_node_t *n, ast_node_t *callee)
{
    const char *cret = cg_ctype(cg, func_ret_of(callee->type_str));
    const char *cps  = func_cast_params_env(cg, callee->type_str);
    const char *args = call_args_lead(cg, n);
    const char *tmp = cg_fmt(cg, "_clz%d", cg->clos_n++);
    return cg_fmt(cg, "({ void *%s = (void*)(%s); ((%s(*)%s)(*(void**)%s))(%s%s); })",
                  tmp, cg_expr(cg, callee), cret, cps, tmp, tmp, args);
}

static const char *cg_print_legacy(cg_t *cg, ast_node_t *n, bool nl, int err)
{
    sb_t b; sb_init(&b); sb_puts(&b, "({ ");
    { size_t i = 0; for (; i < n->list.len; i++) {
        ast_node_t *arg = (ast_node_t *)n->list.data[i];
        char piece[40];
        if (i) { snprintf(piece, sizeof piece, "salam_emit(\" \", %d); ", err); sb_puts(&b, piece); }
        if (arg->type_str && !strcmp(arg->type_str, "str")) {
            sb_puts(&b, "salam_emit("); sb_puts(&b, cg_expr(cg, arg));
            snprintf(piece, sizeof piece, ", %d); ", err); sb_puts(&b, piece);
        } else {
            snprintf(piece, sizeof piece, "salam_emit_owned(salam_tostr_%s(", prim_suffix(print_tag(arg->type_str)));
            sb_puts(&b, piece); sb_puts(&b, cg_expr(cg, arg));
            snprintf(piece, sizeof piece, "), %d); ", err); sb_puts(&b, piece);
        }
    } }
    if (nl) { char z[40]; snprintf(z, sizeof z, "salam_emit(\"\\n\", %d); ", err); sb_puts(&b, z); }
    sb_puts(&b, "})");
    const char *r = arena_strdup(cg->a, sb_cstr(&b)); sb_free(&b); return r;
}

static const char *cg_lower_print(cg_t *cg, ast_node_t *n, bool nl, int err)
{
    if (err) return cg_print_legacy(cg, n, nl, err);
    vec_t segs; vec_init(&segs);
    pf_build(cg->a, n, nl, &segs);
    if (segs.len == 0) return "(void)0";
    { size_t i = 0; for (; i < segs.len; i++)
        if (((pf_seg_t *)segs.data[i])->kind == PF_CHAR)
            return cg_print_legacy(cg, n, nl, err); }
    sb_t fmt; sb_init(&fmt);
    sb_t args; sb_init(&args);
    { size_t i = 0; for (; i < segs.len; i++) {
        pf_seg_t *s = (pf_seg_t *)segs.data[i];
        if (s->kind == PF_LIT) {
            { const char *p = s->text; for (; *p; p++) { if (*p == '%') sb_putc(&fmt, '%'); sb_putc(&fmt, *p); } }
            continue;
        }
        sb_puts(&fmt, pf_spec(s->kind));
        const char *e = cg_expr(cg, s->expr);
        const char *a;
        switch (s->kind) {
            case PF_BOOL: a = cg_fmt(cg, "((%s) ? \"true\" : \"false\")", e); break;
            case PF_F64:  a = cg_fmt(cg, "(double)(%s)", e); break;
            case PF_U32:  a = cg_fmt(cg, "(unsigned)(%s)", e); break;
            case PF_I64:  a = cg_fmt(cg, "(long long)(%s)", e); break;
            case PF_U64:  a = cg_fmt(cg, "(unsigned long long)(%s)", e); break;
            case PF_I32:  a = cg_fmt(cg, "(int)(%s)", e); break;
            default:      a = e; break;
        }
        sb_puts(&args, ", "); sb_puts(&args, a);
    } }
    const char *cfmt = cg_cescape(cg, sb_cstr(&fmt));
    const char *al = arena_strdup(cg->a, sb_cstr(&args));
    sb_free(&fmt); sb_free(&args);
    return cg_fmt(cg, "printf(%s%s)", cfmt, al);
}

static const char *call_ident(cg_t *cg, ast_node_t *n, ast_node_t *callee)
{
    const char *nm = callee->name;
    if (!strcmp(nm, "spawn") && n->list.len == 1) {
        ast_node_t *fn = (ast_node_t *)n->list.data[0];
        symbol_t *fsym = scope_lookup(cg->sem->global, fn->name);
        vec_t empty; vec_init(&empty);
        const char *mangled = fsym ? cg_mangle(cg, NULL, fn->name, &empty) : fn->name;
        return cg_fmt(cg, "salam_thread_spawn((salam_thread_fn)%s)", mangled);
    }
    if (!strcmp(nm, "funcptr") && n->list.len == 1) {
        ast_node_t *fn = (ast_node_t *)n->list.data[0];
        symbol_t *fsym = scope_lookup(cg->sem->global, fn->name);
        const char *mangled;
        if (fsym && fsym->overloads.len > 0) {
            func_sig_t *sig = (func_sig_t *)fsym->overloads.data[0];
            mangled = cg_mangle(cg, NULL, fn->name, &sig->params);
        } else {
            mangled = fn->name;
        }
        return cg_fmt(cg, "(int64_t)(void*)%s", mangled);
    }
    if (!strcmp(nm, "callhandler") && n->list.len == 2) {
        const char *fnp = cg_expr(cg, (ast_node_t *)n->list.data[0]);
        const char *arg = cg_expr(cg, (ast_node_t *)n->list.data[1]);
        return cg_fmt(cg, "((void(*)(int64_t))(intptr_t)(%s))((int64_t)(%s))", fnp, arg);
    }
    bool is_print      = (!strcmp(nm,"print")  || !strcmp(nm,"_"));
    bool is_println    = (!strcmp(nm,"println")|| !strcmp(nm,"__"));
    bool is_printerr   = !strcmp(nm,"printerr");
    bool is_printerrln = !strcmp(nm,"printerrln");
    if (is_print || is_println || is_printerr || is_printerrln) {
        int err = (is_printerr || is_printerrln) ? 1 : 0;
        bool nl  = is_println || is_printerrln;
        return cg_lower_print(cg, n, nl, err);
    }
    if (!strcmp(nm,"input")) return "salam_input()";
    if (!strcmp(nm,"lang")) return cg_fmt(cg, "\"%s\"", i18n_lang());
    if (!strcmp(nm,"args")) {
        const char *cn = cg_ctype(cg, n->type_str ? n->type_str : "Vector_str");
        int t = ++cg->tmpn;
        return cg_fmt(cg, "({ int32_t __an%d; char** __ad%d = salam_args(&__an%d); (%s){ __ad%d, __an%d, __an%d }; })",
                      t, t, t, cn, t, t, t);
    }
    if (!strcmp(nm,"listdir") && n->list.len == 1) {
        const char *cn = cg_ctype(cg, n->type_str ? n->type_str : "Vector_str");
        const char *arg = cg_expr(cg, (ast_node_t *)n->list.data[0]);
        int t = ++cg->tmpn;
        return cg_fmt(cg, "({ int32_t __vn%d; char** __vd%d = salam_os_listdir(%s, &__vn%d); (%s){ __vd%d, __vn%d, __vn%d }; })",
                      t, t, arg, t, cn, t, t, t);
    }
    if (!strcmp(nm,"sizeof") && n->list.len == 1) {
        ast_node_t *op = (ast_node_t *)n->list.data[0];
        return cg_fmt(cg, "(uint64_t)sizeof(%s)", cg_ctype(cg, op->type_str));
    }
    
    if (!scope_lookup(cg->sem->global, nm)) {
        if (!strcmp(nm,"hash") && n->list.len == 1) {
            ast_node_t *a = (ast_node_t *)n->list.data[0];
            const char *ats = a->type_str ? a->type_str : "";
            if (!strcmp(ats, "str"))
                return cg_fmt(cg, "salam_str_hash(%s)", cg_expr(cg, a));
            
            return cg_fmt(cg, "salam_hash_int((uint64_t)(intptr_t)(%s))", cg_expr(cg, a));
        }
    }
    if (!strcmp(nm,"len") && n->list.len == 1) {
        ast_node_t *arg = (ast_node_t *)n->list.data[0];
        if (arg->type_str && strchr(arg->type_str, '[')) {
            long sz = array_size_of(arg->type_str);
            return cg_fmt(cg, "%ld", sz);
        }
        return cg_fmt(cg, "(int32_t)strlen(%s)", cg_expr(cg, arg));
    }
    if (!strcmp(nm,"char_code") && n->list.len == 1) {
        const char *arg = cg_expr(cg, (ast_node_t *)n->list.data[0]);
        return cg_fmt(cg, "(int32_t)(unsigned char)(%s)[0]", arg);
    }
    const salam_builtin_t *bi = salam_builtin_lookup(nm);
    symbol_t *fsym0 = scope_lookup(cg->sem->global, nm);
    if (bi && !fsym0) {
        sb_t b; sb_init(&b); sb_puts(&b, bi->runtime); sb_putc(&b, '(');
        { size_t i = 0; for (; i < n->list.len; i++) {
            if (i) sb_puts(&b, ", ");
            sb_puts(&b, cg_expr(cg, (ast_node_t *)n->list.data[i]));
        } }
        sb_putc(&b, ')');
        const char *r = arena_strdup(cg->a, sb_cstr(&b)); sb_free(&b); return r;
    }
    symbol_t *fsym = scope_lookup(cg->sem->global, nm);
    func_sig_t *sig = fsym ? pick_overload(cg, fsym, n) : NULL;
    bool is_extern_call = sig && sig->decl && sig->decl->is_extern;
    bool inst_fn = sig && sig->decl && sig->decl->synthetic;   
    const char *mangled = is_extern_call ? nm
                        : inst_fn ? cg_mangle_in(cg, "g", NULL, nm, &sig->params)
                        : sig ? cg_mangle(cg, NULL, nm, &sig->params) : nm;
    const char *args = call_args(cg, n, sig);
    if (sig && type_is_byval_agg(sig->ret))
        return call_sret(cg, sig->ret, mangled, args, n->list.len > 0);
    return cg_fmt(cg, "%s(%s)", mangled, args);
}

static const char *call_pkg(cg_t *cg, ast_node_t *n, symbol_t *pk, ast_node_t *callee)
{
    symbol_t *fn = scope_lookup_local(pk->members, callee->name);
    func_sig_t *sig = fn ? pick_overload(cg, fn, n) : NULL;
    vec_t empty; vec_init(&empty);
    
    const char *mangled = (sig && sig->decl && sig->decl->synthetic)
                        ? cg_mangle_in(cg, "g", NULL, callee->name, &sig->params)
                        : cg_mangle_in(cg, pk->pkgname, NULL, callee->name,
                                       sig ? &sig->params : &empty);
    const char *args = call_args(cg, n, sig);
    if (sig && type_is_byval_agg(sig->ret))
        return call_sret(cg, sig->ret, mangled, args, n->list.len > 0);
    return cg_fmt(cg, "%s(%s)", mangled, args);
}

static const char *call_dyn(cg_t *cg, ast_node_t *n, ast_node_t *obj,
                            ast_node_t *callee, const char *objts)
{
    bool is_ptr = objts[strlen(objts) - 1] == '*';
    const char *recv = cg_expr(cg, obj);
    const char *acc = is_ptr ? "->" : ".";
    int t = ++cg->tmpn;
    const char *dynct = cg_ctype(cg, objts);
    if (!strcmp(callee->name, "free"))
        return cg_fmt(cg, "({ %s __dv%d = (%s); salam_free(__dv%d%sdata); })",
                      dynct, t, recv, t, acc);
    const char *as = call_args_lead(cg, n);
    return cg_fmt(cg, "({ %s __dv%d = (%s); __dv%d%svtable->%s(__dv%d%sdata%s); })",
                  dynct, t, recv, t, acc, cg_cident(cg, callee->name), t, acc, as);
}

static const char *call_file(cg_t *cg, ast_node_t *n, ast_node_t *obj, ast_node_t *callee)
{
    const char *recv = cg_expr(cg, obj);
    const char *m = callee->name;
    const char *a0 = arg_at(cg, n, 0);
    const char *a1 = arg_at(cg, n, 1);
    if (!strcmp(m,"read"))     return cg_fmt(cg, "salam_file_read(%s, (unsigned long long)(%s))", recv, a0);
    if (!strcmp(m,"readline")) return cg_fmt(cg, "salam_file_readline(%s)", recv);
    if (!strcmp(m,"write"))    return cg_fmt(cg, "salam_file_write(%s, %s)", recv, a0);
    if (!strcmp(m,"seek"))     return cg_fmt(cg, "salam_file_seek(%s, (long long)(%s), %s)", recv, a0, a1);
    if (!strcmp(m,"close"))    return cg_fmt(cg, "salam_file_close(%s)", recv);
    return "0";
}

static const char *call_str(cg_t *cg, ast_node_t *n, ast_node_t *obj, ast_node_t *callee)
{
    const char *recv = cg_expr(cg, obj);
    const char *m = callee->name;
    const char *a0 = arg_at(cg, n, 0);
    const char *a1 = arg_at(cg, n, 1);
    if (!strcmp(m,"len"))      return cg_fmt(cg, "(int32_t)strlen(%s)", recv);
    if (!strcmp(m,"concat"))   return cg_fmt(cg, "salam_strcat(%s, %s)", recv, a0);
    if (!strcmp(m,"substr"))   return cg_fmt(cg, "salam_str_substr(%s, %s, %s)", recv, a0, a1);
    if (!strcmp(m,"find")) {
        int tf = ++cg->tmpn;
        return cg_fmt(cg, "({ const char *__fh%d=(%s); const char *__fn%d=(%s); const char *__fp%d=(__fh%d&&__fn%d)?strstr(__fh%d,__fn%d):0; __fp%d?(int32_t)(__fp%d-__fh%d):-1; })",
                      tf, recv, tf, a0, tf, tf, tf, tf, tf, tf, tf, tf);
    }
    if (!strcmp(m,"trim"))     return cg_fmt(cg, "salam_str_trim(%s)", recv);
    if (!strcmp(m,"to_int"))   return cg_fmt(cg, "(int32_t)strtol(%s, ((void*)0), 10)", recv);
    if (!strcmp(m,"to_float")) return cg_fmt(cg, "strtod(%s, ((void*)0))", recv);
    if (!strcmp(m,"split")) {
        const char *cn = cg_ctype(cg, n->type_str ? n->type_str : "Vector_str");
        int t = ++cg->tmpn;
        return cg_fmt(cg, "({ int32_t __sn%d; char** __sd%d = salam_str_split(%s, %s, &__sn%d); (%s){ __sd%d, __sn%d, __sn%d }; })",
                      t, t, recv, a0, t, cn, t, t, t);
    }
    return "0";
}

static const char *call_method(cg_t *cg, ast_node_t *n, ast_node_t *obj,
                               ast_node_t *callee, const char *objts)
{
    bool obj_is_ptr = (strlen(objts) && objts[strlen(objts)-1] == '*');
    char sname[96];
    symbol_t *ssym = cg_struct_of(cg, objts, sname, sizeof sname);
    symbol_t *msym = ssym ? scope_lookup_local(ssym->members, callee->name) : NULL;
    func_sig_t *sig = msym ? pick_overload(cg, msym, n) : NULL;
    vec_t empty; vec_init(&empty);
    const char *mangled = cg_mangle_method(cg, sname, ssym, callee->name, sig ? &sig->params : &empty);
    const char *as = call_args_lead(cg, n);
    bool addressable = cg_addressable(obj);
    bool ret_struct = sig && type_is_byval_agg(sig->ret);
    const char *Rc = ret_struct ? cg_ctype(cg, type_to_string(cg->sem->tc, sig->ret)) : NULL;
    if (obj_is_ptr || addressable) {
        const char *recv = obj_is_ptr ? cg_expr(cg, obj) : cg_fmt(cg, "&(%s)", cg_expr(cg, obj));
        if (ret_struct) {
            int s = ++cg->tmpn;
            return cg_fmt(cg, "({ %s __s%d; %s(%s%s, &__s%d); __s%d; })", Rc, s, mangled, recv, as, s, s);
        }
        return cg_fmt(cg, "%s(%s%s)", mangled, recv, as);
    }
    int t = ++cg->tmpn;
    const char *oexpr = cg_expr(cg, obj);
    if (ret_struct) {
        int s = ++cg->tmpn;
        return cg_fmt(cg, "({ %s __t%d = (%s); %s __s%d; %s(&__t%d%s, &__s%d); __s%d; })",
                      cg_ctype(cg, objts), t, oexpr, Rc, s, mangled, t, as, s, s);
    }
    return cg_fmt(cg, "({ %s __t%d = (%s); %s(&__t%d%s); })",
                  cg_ctype(cg, objts), t, oexpr, mangled, t, as);
}


static const char *call_impl(cg_t *cg, ast_node_t *n, ast_node_t *obj,
                             ast_node_t *callee, const char *objts, symbol_t *owner)
{
    symbol_t *msym = scope_lookup_local(owner->members, callee->name);
    func_sig_t *sig = msym ? pick_overload(cg, msym, n) : NULL;
    vec_t empty; vec_init(&empty);
    const char *mangled = cg_mangle_ti(cg, objts, callee->name, sig ? &sig->params : &empty);
    const char *as = call_args_lead(cg, n);
    const char *recv = cg_expr(cg, obj);
    if (sig && type_is_byval_agg(sig->ret)) {
        const char *Rc = cg_ctype(cg, type_to_string(cg->sem->tc, sig->ret));
        int s = ++cg->tmpn;
        return cg_fmt(cg, "({ %s __s%d; %s((%s)%s, &__s%d); __s%d; })", Rc, s, mangled, recv, as, s, s);
    }
    return cg_fmt(cg, "%s((%s)%s)", mangled, recv, as);
}

static const char *call_member(cg_t *cg, ast_node_t *n, ast_node_t *callee)
{
    ast_node_t *obj = callee->a;
    if (obj && obj->kind == AST_IDENTIFIER) {
        symbol_t *pk = scope_lookup(cg->sem->global, obj->name);
        
        if ((!pk || pk->kind != SYM_PACKAGE) && cg->cur_struct && cg->cur_struct->home)
            pk = scope_lookup(cg->cur_struct->home, obj->name);
        if (pk && pk->kind == SYM_PACKAGE) return call_pkg(cg, n, pk, callee);
    }
    const char *objts = obj->type_str ? obj->type_str : "";
    
    {
        symbol_t *impl = scope_lookup(cg->sem->global, impl_owner_key(cg->a, objts));
        if (impl && impl->kind == SYM_TYPEIMPL && scope_lookup_local(impl->members, callee->name))
            return call_impl(cg, n, obj, callee, objts, impl);
    }
    if (!strncmp(objts, "dyn ", 4))                                       return call_dyn(cg, n, obj, callee, objts);
    if (!strcmp(objts, "File*"))                                          return call_file(cg, n, obj, callee);
    if (!strcmp(objts, "str"))                                            return call_str(cg, n, obj, callee);
    return call_method(cg, n, obj, callee, objts);
}

const char *cg_call(cg_t *cg, ast_node_t *n)
{
    ast_node_t *callee = n->a;
    if (callee && callee->type_str && !strncmp(callee->type_str, "func(", 5))
        return call_value(cg, n, callee);
    if (callee && callee->kind == AST_IDENTIFIER) return call_ident(cg, n, callee);
    if (callee && callee->kind == AST_MEMBER)     return call_member(cg, n, callee);
    return "/*uncallable*/0";
}
