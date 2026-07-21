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

bool interp_match_arm(interp_t *I, env_t *env, ast_node_t *arm, value_t subj)
{
    size_t pi = 0;
    if (arm->op == TK_KW_ELSE) return true;
    for (; pi < arm->list.len; pi++) {
        ast_node_t *pat = (ast_node_t *)arm->list.data[pi];
        if (subj.kind == VAL_VARIANT) {
            if (subj.as.variant &&
                (int64_t)pat->value.as.i == (int64_t)subj.as.variant->tag)
                return true;
        } else {
            value_t pv = eval(I, env, pat->a);
            if (value_eq(I, pv, subj)) return true;
        }
    }
    return false;
}

ast_node_t *interp_find_match_arm(interp_t *I, env_t *env, ast_node_t *n, value_t subj)
{
    size_t i = 0;
    for (; i < n->list.len; i++) {
        ast_node_t *arm = (ast_node_t *)n->list.data[i];
        if (interp_match_arm(I, env, arm, subj)) return arm;
    }
    return NULL;
}

iloc_t interp_resolve_loc(interp_t *I, env_t *env, ast_node_t *target)
{
    iloc_t loc;
    memset(&loc, 0, sizeof(loc));
    loc.target = target;

    if (target->kind == AST_IDENTIFIER) {
        binding_t *b = env_find(env, target->name);
        if (b) {
            loc.kind = ILOC_VAR;
            loc.b = b;
            return loc;
        }
        binding_t *self = env_find(env, "this");
        if (self && self->val.kind == VAL_STRUCT) {
            size_t i = 0;
            for (; i < self->val.as.st->nfields; i++)
                if (strcmp(self->val.as.st->fields[i].name, target->name) == 0) {
                    loc.kind = ILOC_FIELD;
                    loc.obj = self->val;
                    loc.field_idx = i;
                    return loc;
                }
        }
        rt_error(I, target, "assignment to undeclared variable '%s'", target->name);
    }

    if (target->kind == AST_MEMBER) {
        value_t obj = eval(I, env, target->a);
        if (obj.kind != VAL_STRUCT)
            rt_error(I, target, "cannot assign to member of non-struct");
        {
            size_t i = 0;
            for (; i < obj.as.st->nfields; i++)
                if (strcmp(obj.as.st->fields[i].name, target->name) == 0) {
                    loc.kind = ILOC_FIELD;
                    loc.obj = obj;
                    loc.field_idx = i;
                    return loc;
                }
        }
        rt_error(I, target, "struct '%s' has no field '%s'", obj.as.st->type_name,
                 target->name);
    }

    if (target->kind == AST_INDEX) {
        value_t a = eval(I, env, target->a);
        if (a.kind == VAL_STRUCT) {
            ast_node_t *set_m = struct_method(a.as.st->def, "operator_index_set");
            if (set_m) {
                loc.kind = ILOC_OPIDX;
                loc.obj = a;
                loc.key = eval(I, env, target->b);
                loc.set_m = set_m;
                loc.get_m = struct_method(a.as.st->def, "operator_index");
                return loc;
            }
            rt_error(I, target, "value is not index-assignable");
        }
        if (a.kind == VAL_ARRAY) {
            int64_t i = to_int(eval(I, env, target->b));
            if (i < 0 || (size_t)i >= a.as.arr->len)
                rt_error(I, target, "index out of range in assignment");
            loc.kind = ILOC_ARR;
            loc.arr = a.as.arr;
            loc.idx = i;
            return loc;
        }
        if (a.kind == VAL_PTR) {
            if (!a.as.ptr.addr) rt_error(I, target, "write through a null pointer");
            loc.kind = ILOC_PTR;
            loc.ptr = a.as.ptr;
            loc.idx = to_int(eval(I, env, target->b));
            return loc;
        }
        rt_error(I, target, "value is not index-assignable");
    }

    rt_error(I, target, "invalid assignment target");
    return loc;
}

value_t interp_loc_get(interp_t *I, iloc_t *loc)
{
    switch (loc->kind) {
    case ILOC_VAR:
        return loc->b->val;
    case ILOC_FIELD:
        return loc->obj.as.st->fields[loc->field_idx].val;
    case ILOC_ARR:
        return loc->arr->data[loc->idx];
    case ILOC_PTR:
        return ptr_load(loc->ptr, loc->idx);
    case ILOC_OPIDX:
        if (loc->get_m) {
            env_t *denv = find_def_env(I, loc->get_m);
            return call_func(I, loc->get_m, denv ? denv : I->globals, &loc->obj,
                             &loc->key, 1);
        }
        rt_error(I, loc->target, "value is not index-readable");
    }
    return val_null();
}

void interp_loc_set(interp_t *I, iloc_t *loc, value_t v)
{
    switch (loc->kind) {
    case ILOC_VAR:
        loc->b->val = v;
        return;
    case ILOC_FIELD:
        loc->obj.as.st->fields[loc->field_idx].val = v;
        return;
    case ILOC_ARR:
        loc->arr->data[loc->idx] = v;
        return;
    case ILOC_PTR:
        ptr_store(loc->ptr, loc->idx, v);
        return;
    case ILOC_OPIDX: {
        value_t args[2];
        args[0] = loc->key;
        args[1] = v;
        env_t *denv = find_def_env(I, loc->set_m);
        call_func(I, loc->set_m, denv ? denv : I->globals, &loc->obj, args, 2);
        return;
    }
    }
}

void interp_assign_to(interp_t *I, env_t *env, ast_node_t *target, value_t v)
{
    iloc_t loc = interp_resolve_loc(I, env, target);
    interp_loc_set(I, &loc, v);
}

static bool take_pending_flow(interp_t *I, flow_t *out)
{
    if (I->pending_flow == FLOW_NORMAL) return false;
    *out = I->pending_flow;
    I->pending_flow = FLOW_NORMAL;
    return true;
}

flow_t exec_list(interp_t *I, env_t *env, frame_t *fr, vec_t *list, value_t *ret)
{
    {
        size_t i = 0;
        for (; i < list->len; i++) {
            flow_t f = exec_stmt(I, env, fr, (ast_node_t *)list->data[i], ret);
            if (f != FLOW_NORMAL) return f;
        }
    }
    return FLOW_NORMAL;
}

flow_t exec_stmt(interp_t *I, env_t *env, frame_t *fr, ast_node_t *n, value_t *ret)
{
    tick(I);
    switch (n->kind) {
    case AST_BLOCK: {
        env_t *child = env_new(I, env);
        return exec_list(I, child, fr, &n->list, ret);
    }
    case AST_VAR_DECL:
    case AST_CONST_DECL: {
        value_t v;
        if (n->a) {
            flow_t pf;
            v = eval(I, env, n->a);
            if (take_pending_flow(I, &pf)) return pf;
        } else
            v = default_for_type(I, n->type_str);
        env_define(I, env, n->name, v);
        return FLOW_NORMAL;
    }
    case AST_EXPR_STMT: {
        flow_t pf;
        eval(I, env, n->a);
        if (take_pending_flow(I, &pf)) return pf;
        return FLOW_NORMAL;
    }
    case AST_ASSIGN: {
        flow_t pf;
        if (n->op == TK_ASSIGN) {
            value_t rhs = eval(I, env, n->b);
            if (take_pending_flow(I, &pf)) return pf;
            interp_assign_to(I, env, n->a, rhs);
            return FLOW_NORMAL;
        }
        {
            token_kind_t base = compound_base(n->op);
            iloc_t loc = interp_resolve_loc(I, env, n->a);
            value_t cur = interp_loc_get(I, &loc);
            value_t r = eval(I, env, n->b);
            bool found;
            value_t sr, rhs;
            if (take_pending_flow(I, &pf)) return pf;
            sr = try_struct_op(I, base, cur, r, true, &found);
            rhs = found ? sr : arith(I, n, base, cur, r);
            interp_loc_set(I, &loc, rhs);
        }
        return FLOW_NORMAL;
    }
    case AST_IF: {
        flow_t pf;
        bool cond = to_bool(eval(I, env, n->a));
        if (take_pending_flow(I, &pf)) return pf;
        if (cond) {
            env_t *c = env_new(I, env);
            return exec_list(I, c, fr, &n->b->list, ret);
        }
        if (n->c) {
            if (n->c->kind == AST_IF) return exec_stmt(I, env, fr, n->c, ret);
            env_t *c = env_new(I, env);
            return exec_list(I, c, fr, &n->c->list, ret);
        }
        return FLOW_NORMAL;
    }
    case AST_UNTIL:
        for (;;) {
            flow_t pf;
            bool cond;
            tick(I);
            cond = to_bool(eval(I, env, n->a));
            if (take_pending_flow(I, &pf)) return pf;
            if (!cond) break;
            {
                env_t *c = env_new(I, env);
                flow_t f = exec_list(I, c, fr, &n->b->list, ret);
                if (f == FLOW_RETURN) return f;
                if (f == FLOW_BREAK) break;
            }
        }
        return FLOW_NORMAL;
    case AST_REPEAT: {
#define REP_NUM(NODE, OUT, WHAT)                                                         \
    do {                                                                                 \
        value_t _v = eval(I, env, (NODE));                                               \
        if (_v.kind == VAL_INT)                                                          \
            (OUT) = _v.as.i;                                                             \
        else if (_v.kind == VAL_FLOAT)                                                   \
            (OUT) = (int64_t)_v.as.f;                                                    \
        else {                                                                           \
            rt_error(I, (NODE), "repeat " WHAT " must be a number");                     \
            return FLOW_NORMAL;                                                          \
        }                                                                                \
    } while (0)
        int64_t start, end, step;
        if (n->c) {
            REP_NUM(n->a, start, "range start");
            REP_NUM(n->c, end, "range bound");
            if (n->d)
                REP_NUM(n->d, step, "step");
            else
                step = 1;
            if (step <= 0) {
                rt_error(I, n->d ? n->d : n, "repeat step must be positive");
                return FLOW_NORMAL;
            }
            if (start > end) step = -step;
        } else {
            REP_NUM(n->a, end, "count");
            start = 0;
            end -= 1;
            step = 1;
        }
#undef REP_NUM
        {
            int64_t k = start;
            for (; (step > 0) ? (k <= end) : (k >= end); k += step) {
                tick(I);
                env_t *c = env_new(I, env);
                if (n->name) env_define(I, c, n->name, val_int(k));
                flow_t f = exec_list(I, c, fr, &n->b->list, ret);
                if (f == FLOW_RETURN) return f;
                if (f == FLOW_BREAK) break;
            }
        }
        return FLOW_NORMAL;
    }
    case AST_FOR: {
        env_t *loop = env_new(I, env);
        if (n->a) exec_stmt(I, loop, fr, n->a, ret);
        for (;;) {
            flow_t pf;
            bool cond;
            tick(I);
            cond = !n->b || to_bool(eval(I, loop, n->b));
            if (take_pending_flow(I, &pf)) return pf;
            if (!cond) break;
            {
                env_t *c = env_new(I, loop);
                flow_t f = exec_list(I, c, fr, &n->d->list, ret);
                if (f == FLOW_RETURN) return f;
                if (f == FLOW_BREAK) break;
            }
            if (n->c) exec_stmt(I, loop, fr, n->c, ret);
        }
        return FLOW_NORMAL;
    }
    case AST_MATCH: {
        value_t subj = eval(I, env, n->a);
        ast_node_t *arm = interp_find_match_arm(I, env, n, subj);
        if (!arm) return FLOW_NORMAL;
        {
            env_t *c = env_new(I, env);
            return exec_list(I, c, fr, &arm->b->list, ret);
        }
    }
    case AST_RETURN:
        *ret = n->a ? eval(I, env, n->a) : val_null();
        return I->match_expr_depth > 0 ? FLOW_MATCH_RET : FLOW_RETURN;
    case AST_BREAK:
        return FLOW_BREAK;
    case AST_CONTINUE:
        return FLOW_CONTINUE;
    case AST_DEFER: {
        defer_t *d = (defer_t *)arena_alloc(I->a, sizeof *d);
        d->stmt = n->a;
        d->env = env;
        vec_push(I->a, &fr->defers, d);
        return FLOW_NORMAL;
    }
    default:

        eval(I, env, n);
        return FLOW_NORMAL;
    }
}
