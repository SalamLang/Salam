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

static value_t *eval_args(interp_t *I, env_t *env, ast_node_t *call, size_t *outn)
{
    size_t n = call->list.len;
    *outn = n;
    if (n == 0) return NULL;
    value_t *args = (value_t *)arena_alloc(I->a, salam_size_mul(n, sizeof(value_t)));
    {
        size_t i = 0;
        for (; i < n; i++)
            args[i] = eval(I, env, (ast_node_t *)call->list.data[i]);
    }
    return args;
}

static int64_t interp_sizeof_typename(interp_t *I, const char *ts);

static int64_t interp_alignof_typename(interp_t *I, const char *ts)
{
    if (!ts || !*ts) return 8;
    size_t len = strlen(ts);
    if (ts[len - 1] == '*') return 8;
    if (!strcmp(ts, "i8") || !strcmp(ts, "u8") || !strcmp(ts, "bool") ||
        !strcmp(ts, "char"))
        return 1;
    if (!strcmp(ts, "i16") || !strcmp(ts, "u16")) return 2;
    if (!strcmp(ts, "i32") || !strcmp(ts, "u32") || !strcmp(ts, "f32")) return 4;
    if (!strcmp(ts, "i64") || !strcmp(ts, "u64") || !strcmp(ts, "f64") ||
        !strcmp(ts, "str"))
        return 8;
    ast_node_t *sd = find_struct(I, ts);
    if (sd) {
        int64_t maxalign = 1;
        {
            size_t i = 0;
            for (; i < sd->list.len; i++) {
                ast_node_t *f = (ast_node_t *)sd->list.data[i];
                if (f->kind != AST_FIELD) continue;
                int64_t al =
                    interp_alignof_typename(I, f->type ? f->type->type_str : NULL);
                if (al > maxalign) maxalign = al;
            }
        }
        return maxalign;
    }
    return 8;
}

static int64_t interp_sizeof_typename(interp_t *I, const char *ts)
{
    if (!ts || !*ts) return 8;
    size_t len = strlen(ts);
    if (ts[len - 1] == '*') return 8;
    if (!strcmp(ts, "i8") || !strcmp(ts, "u8") || !strcmp(ts, "bool") ||
        !strcmp(ts, "char"))
        return 1;
    if (!strcmp(ts, "i16") || !strcmp(ts, "u16")) return 2;
    if (!strcmp(ts, "i32") || !strcmp(ts, "u32") || !strcmp(ts, "f32")) return 4;
    if (!strcmp(ts, "i64") || !strcmp(ts, "u64") || !strcmp(ts, "f64") ||
        !strcmp(ts, "str"))
        return 8;
    ast_node_t *sd = find_struct(I, ts);
    if (sd) {
        int64_t off = 0, maxalign = 1;
        {
            size_t i = 0;
            for (; i < sd->list.len; i++) {
                ast_node_t *f = (ast_node_t *)sd->list.data[i];
                if (f->kind != AST_FIELD) continue;
                const char *fts = f->type ? f->type->type_str : NULL;
                int64_t sz = interp_sizeof_typename(I, fts);
                int64_t al = interp_alignof_typename(I, fts);
                off = (off + al - 1) & ~(al - 1);
                off += sz;
                if (al > maxalign) maxalign = al;
            }
        }
        off = (off + maxalign - 1) & ~(maxalign - 1);
        return off ? off : 8;
    }
    return 8;
}

static value_t eval_call(interp_t *I, env_t *env, ast_node_t *n)
{
    ast_node_t *callee = n->a;

    if (callee->kind == AST_IDENTIFIER) {
        const char *nm = callee->name;
        if (!strcmp(nm, "print")) {
            do_print(I, env, n, false, false);
            return val_null();
        }
        if (!strcmp(nm, "println")) {
            do_print(I, env, n, true, false);
            return val_null();
        }
        if (!strcmp(nm, "printerr")) {
            do_print(I, env, n, false, true);
            return val_null();
        }
        if (!strcmp(nm, "printerrln")) {
            do_print(I, env, n, true, true);
            return val_null();
        }
        if (!strcmp(nm, "input")) return do_input(I);
        if (!strcmp(nm, "lang")) return val_str(I->lang);

        if (!strcmp(nm, "sizeof") && n->list.len == 1) {
            ast_node_t *op = (ast_node_t *)n->list.data[0];
            return val_int(interp_sizeof_typename(I, op->type_str));
        }
        size_t na;
        value_t *args = eval_args(I, env, n, &na);
        if (!strcmp(nm, "len") && na == 1) {
            value_t a = args[0];
            if (a.kind == VAL_ARRAY) return val_int((int64_t)a.as.arr->len);
            if (a.kind == VAL_STR) return val_int((int64_t)strlen(a.as.s));
            if (a.kind == VAL_MAP) return val_int((int64_t)a.as.map->count);
            rt_error(I, n, "len() expects an array, string, or map");
        }
        if (!strcmp(nm, "char_code") && na == 1)
            return val_int((int64_t)(unsigned char)to_str(I, args[0])[0]);
        if (!strcmp(nm, "char_from_code") && na == 1) {
            char b[2] = {(char)to_int(args[0]), 0};
            return val_str(arena_strdup(I->a, b));
        }
        if (!strcmp(nm, "strcmp") && na == 2)
            return val_int(strcmp(to_str(I, args[0]), to_str(I, args[1])));
        if (!strcmp(nm, "funcptr") && na == 1) return args[0];

        binding_t *b = env_find(env, nm);
        if (b && b->val.kind == VAL_FUNC)
            return call_func(I, b->val.as.fn->fn, b->val.as.fn->env, NULL, args, na);
        ast_node_t *fn = find_func(I, nm, na);
        if (fn) return call_func(I, fn, I->globals, NULL, args, na);

        bool handled;
        value_t ir = call_intrinsic(I, n, nm, args, na, &handled);
        if (handled) return ir;
        rt_error(I, n, "call to undefined function '%s'", nm);
    }

    if (callee->kind == AST_MEMBER) {
        ast_node_t *obj = callee->a;
        const char *method = callee->name;

        if (obj->kind == AST_IDENTIFIER) {
            binding_t *mb = env_find(env, obj->name);
            if (mb && mb->val.kind == VAL_MODULE) {
                size_t na;
                value_t *args = eval_args(I, env, n, &na);
                return call_module_func(I, n, mb->val.as.mod, method, args, na);
            }
        }
        value_t recv = eval(I, env, obj);
        size_t na;
        value_t *args = eval_args(I, env, n, &na);

        if (!strcmp(method, "free") && na == 0) return val_null();
        if (recv.kind == VAL_STRUCT) {
            ast_node_t *m = struct_method_arity(recv.as.st->def, method, na);
            if (!m) m = struct_method(recv.as.st->def, method);
            if (m) return call_func(I, m, I->globals, &recv, args, na);

            {
                size_t i = 0;
                for (; i < recv.as.st->nfields; i++)
                    if (strcmp(recv.as.st->fields[i].name, method) == 0 &&
                        recv.as.st->fields[i].val.kind == VAL_FUNC) {
                        value_t fv = recv.as.st->fields[i].val;
                        return call_func(I, fv.as.fn->fn, fv.as.fn->env, NULL, args, na);
                    }
            }
            rt_error(I, n, "struct '%s' has no method '%s'", recv.as.st->type_name,
                     method);
        }

        {
            ast_node_t *im = find_impl_method(I, obj->type_str, method, na);
            if (im) return call_func(I, im, I->globals, &recv, args, na);
        }
        return call_builtin_method(I, n, recv, method, args, na);
    }

    value_t fv = eval(I, env, callee);
    if (fv.kind == VAL_FUNC) {
        size_t na;
        value_t *args = eval_args(I, env, n, &na);
        return call_func(I, fv.as.fn->fn, fv.as.fn->env, NULL, args, na);
    }
    rt_error(I, n, "value is not callable");
    return val_null();
}

value_t call_func(interp_t *I, ast_node_t *fn, env_t *defenv, value_t *thisv,
                  value_t *args, size_t nargs)
{
    if (++I->depth > INTERP_MAX_DEPTH)
        rt_error(I, fn, "call stack too deep (possible infinite recursion)");
    env_t *env = env_new(I, defenv);
    if (thisv) env_define(I, env, "this", *thisv);
    {
        size_t i = 0;
        for (; i < fn->list.len; i++) {
            ast_node_t *p = (ast_node_t *)fn->list.data[i];
            if (i < nargs)
                env_define(I, env, p->name, args[i]);
            else if (p->a)
                env_define(I, env, p->name, eval(I, env, p->a));
            else
                env_define(I, env, p->name, val_null());
        }
    }
    frame_t fr;
    vec_init(&fr.defers);
    value_t ret = val_null();
    if (fn->a) exec_list(I, env, &fr, &fn->a->list, &ret);
    {
        size_t i = fr.defers.len;
        for (; i > 0; i--) {
            defer_t *d = (defer_t *)fr.defers.data[i - 1];
            value_t dummy = val_null();
            frame_t inner;
            vec_init(&inner.defers);
            exec_stmt(I, d->env, &inner, d->stmt, &dummy);
        }
    }
    I->depth--;
    return ret;
}

value_t eval(interp_t *I, env_t *env, ast_node_t *n)
{
    if (!n) return val_null();
    switch (n->kind) {
    case AST_LITERAL:
        switch (n->op) {
        case TK_INT:
            return val_int((int64_t)n->value.as.i);
        case TK_FLOAT:
            return val_float(n->value.as.f);
        case TK_CHAR:
            return val_char((int64_t)n->value.as.i);
        case TK_STRING:
        case TK_TRIPLE_STRING:
        case TK_RAW_STRING:
        case TK_UTF8_CHAR:
            return val_str(n->value.as.s ? n->value.as.s : "");
        case TK_KW_TRUE:
            return val_bool(true);
        case TK_KW_FALSE:
            return val_bool(false);
        case TK_KW_NULL:
            return val_null();
        default:
            return val_null();
        }
    case AST_IDENTIFIER: {
        binding_t *b = env_find(env, n->name);
        if (b) return b->val;

        binding_t *self = env_find(env, "this");
        if (self && self->val.kind == VAL_STRUCT) {
            {
                size_t i = 0;
                for (; i < self->val.as.st->nfields; i++)
                    if (strcmp(self->val.as.st->fields[i].name, n->name) == 0)
                        return self->val.as.st->fields[i].val;
            }
        }
        ast_node_t *fn = find_func(I, n->name, (size_t)-1);
        if (fn) return mk_closure(I, fn, I->globals);
        rt_error(I, n, "undefined variable '%s'", n->name);
    }
    case AST_THIS: {
        binding_t *b = env_find(env, "this");
        if (!b) rt_error(I, n, "'this' used outside a method");
        return b->val;
    }
    case AST_BINARY: {
        token_kind_t op = n->op;

        if (op == TK_AND) {
            value_t a = eval(I, env, n->a);
            return to_bool(a) ? val_bool(to_bool(eval(I, env, n->b))) : val_bool(false);
        }
        if (op == TK_OR) {
            value_t a = eval(I, env, n->a);
            return to_bool(a) ? val_bool(true) : val_bool(to_bool(eval(I, env, n->b)));
        }
        value_t a = eval(I, env, n->a);
        value_t b = eval(I, env, n->b);
        bool found;
        value_t r = try_struct_op(I, op, a, b, true, &found);
        if (found) return r;
        switch (op) {
        case TK_EQ:
            return val_bool(value_eq(I, a, b));
        case TK_NE:
            return val_bool(!value_eq(I, a, b));
        case TK_LT:
            return val_bool(value_cmp(I, a, b) < 0);
        case TK_GT:
            return val_bool(value_cmp(I, a, b) > 0);
        case TK_LE:
            return val_bool(value_cmp(I, a, b) <= 0);
        case TK_GE:
            return val_bool(value_cmp(I, a, b) >= 0);
        default:
            return arith(I, n, op, a, b);
        }
    }
    case AST_UNARY: {
        value_t a = eval(I, env, n->a);
        if (a.kind == VAL_STRUCT) {
            bool found;
            value_t r = try_struct_op(I, n->op, a, val_null(), false, &found);
            if (found) return r;
        }

        if (n->op == TK_MINUS)
            return a.kind == VAL_FLOAT ? val_float(-a.as.f)
                                       : val_int((int64_t)(0 - (uint64_t)to_int(a)));
        if (n->op == TK_NOT) return val_bool(!to_bool(a));
        return a;
    }
    case AST_CAST: {
        value_t a = eval(I, env, n->a);
        char base[96];
        base_typename(n->type ? n->type->type_str : NULL, base, sizeof base);
        if (!strcmp(base, "char")) return val_char(to_int(a));
        if (is_int_typename(base)) return val_int(to_int(a));
        if (is_float_typename(base)) return val_float(to_float(a));
        if (!strcmp(base, "bool")) return val_bool(to_bool(a));
        if (!strcmp(base, "str")) return val_str(to_str(I, a));
        return a;
    }
    case AST_CALL:
        return eval_call(I, env, n);
    case AST_MEMBER: {
        ast_node_t *obj = n->a;
        if (obj->kind == AST_IDENTIFIER && !env_find(env, obj->name)) {
            ast_node_t *edef = find_enum(I, obj->name);
            if (edef) {
                {
                    size_t i = 0, val = 0;
                    for (; i < edef->list.len; i++, val++) {
                        ast_node_t *m = (ast_node_t *)edef->list.data[i];
                        if (m->a) val = (size_t)to_int(eval(I, I->globals, m->a));
                        if (m->name && strcmp(m->name, n->name) == 0)
                            return val_int((int64_t)val);
                    }
                }
                rt_error(I, n, "enum '%s' has no member '%s'", obj->name, n->name);
            }
        }
        value_t recv = eval(I, env, obj);
        if (recv.kind == VAL_MODULE) {
            binding_t *cb = env_find_local(recv.as.mod->env, n->name);
            if (cb) return cb->val;
            rt_error(I, n, "package '%s' has no member '%s'", recv.as.mod->name, n->name);
        }
        if (recv.kind == VAL_STRUCT) {
            {
                size_t i = 0;
                for (; i < recv.as.st->nfields; i++)
                    if (strcmp(recv.as.st->fields[i].name, n->name) == 0)
                        return recv.as.st->fields[i].val;
            }
            rt_error(I, n, "struct '%s' has no field '%s'", recv.as.st->type_name,
                     n->name);
        }
        rt_error(I, n, "cannot access member '%s'", n->name);
    }
    case AST_INDEX: {
        value_t a = eval(I, env, n->a);
        if (a.kind == VAL_STRUCT) {
            ast_node_t *m = struct_method(a.as.st->def, "operator_index");
            if (m) {
                value_t idx = eval(I, env, n->b);
                return call_func(I, m, I->globals, &a, &idx, 1);
            }
        }
        value_t idx = eval(I, env, n->b);
        if (a.kind == VAL_ARRAY) {
            int64_t i = to_int(idx);
            if (i < 0 || (size_t)i >= a.as.arr->len)
                rt_error(I, n, "index %lld out of range (len %zu)", (long long)i,
                         a.as.arr->len);
            return a.as.arr->data[i];
        }
        if (a.kind == VAL_STR) {
            int64_t i = to_int(idx);
            size_t len = strlen(a.as.s);
            if (i < 0 || (size_t)i >= len) rt_error(I, n, "string index out of range");
            return val_char((int64_t)(unsigned char)a.as.s[i]);
        }

        if (to_int(idx) == 0) return a;
        rt_error(I, n, "value is not indexable");
    }
    case AST_SLICE: {
        value_t base = eval(I, env, n->a);
        if (base.kind != VAL_ARRAY) rt_error(I, n, "cannot slice a non-array value");
        int64_t blen = (int64_t)base.as.arr->len;
        int64_t lo = n->b ? to_int(eval(I, env, n->b)) : 0;
        int64_t hi = n->c ? to_int(eval(I, env, n->c)) : blen;
        if (lo < 0) lo = 0;
        if (hi > blen) hi = blen;
        if (hi < lo) hi = lo;
        return mk_array(I, array_view(I, base.as.arr, (size_t)lo, (size_t)(hi - lo)));
    }
    case AST_LAMBDA:
        return mk_closure(I, n, env);
    case AST_STRUCT_LIT: {
        if (n->name && !strcmp(n->name, "HashMap")) return mk_map(map_new(I));
        if (n->name && !strcmp(n->name, "Vector")) return mk_array(I, array_new(I, 4));
        ast_node_t *sdef = find_struct(I, n->name);
        if (!sdef) rt_error(I, n, "unknown struct '%s'", n->name);

        size_t nf = 0;
        {
            size_t i = 0;
            for (; i < sdef->list.len; i++)
                if (((ast_node_t *)sdef->list.data[i])->kind == AST_FIELD) nf++;
        }
        value_t sv = mk_struct(I, sdef->name, sdef, nf);
        size_t fi = 0;
        {
            size_t i = 0;
            for (; i < sdef->list.len; i++) {
                ast_node_t *f = (ast_node_t *)sdef->list.data[i];
                if (f->kind != AST_FIELD) continue;
                ast_node_t *provided = NULL;
                {
                    size_t j = 0;
                    for (; j < n->list.len; j++) {
                        ast_node_t *p = (ast_node_t *)n->list.data[j];
                        if (p->name && strcmp(p->name, f->name) == 0) {
                            provided = p;
                            break;
                        }
                    }
                }
                value_t fv = provided ? eval(I, env, provided->a)
                                      : (f->a ? eval(I, I->globals, f->a) : val_null());
                sv.as.st->fields[fi].name = f->name;
                sv.as.st->fields[fi].val = fv;
                fi++;
            }
        }
        return sv;
    }
    case AST_ARRAY_LIT: {
        sarray_t *a = array_new(I, n->list.len ? n->list.len : 4);
        {
            size_t i = 0;
            for (; i < n->list.len; i++)
                array_push(I, a, eval(I, env, (ast_node_t *)n->list.data[i]));
        }
        return mk_array(I, a);
    }
    default:
        rt_error(I, n, "cannot evaluate %s", ast_kind_name(n->kind));
    }
    return val_null();
}
