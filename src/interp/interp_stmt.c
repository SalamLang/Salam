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

static void assign_to(interp_t *I, env_t *env, ast_node_t *target, value_t v)
{
    if (target->kind == AST_IDENTIFIER) {
        binding_t *b = env_find(env, target->name);
        if (b) { b->val = v; return; }
        binding_t *self = env_find(env, "this");
        if (self && self->val.kind == VAL_STRUCT) {
            { size_t i = 0; for (; i < self->val.as.st->nfields; i++)
                if (strcmp(self->val.as.st->fields[i].name, target->name) == 0)
                    { self->val.as.st->fields[i].val = v; return; } }
        }
        rt_error(I, target, "assignment to undeclared variable '%s'", target->name);
    }
    if (target->kind == AST_MEMBER) {
        value_t obj = eval(I, env, target->a);
        if (obj.kind != VAL_STRUCT) rt_error(I, target, "cannot assign to member of non-struct");
        { size_t i = 0; for (; i < obj.as.st->nfields; i++)
            if (strcmp(obj.as.st->fields[i].name, target->name) == 0)
                { obj.as.st->fields[i].val = v; return; } }
        rt_error(I, target, "struct '%s' has no field '%s'", obj.as.st->type_name, target->name);
    }
    if (target->kind == AST_INDEX) {
        value_t a = eval(I, env, target->a);
        if (a.kind == VAL_STRUCT) {
            ast_node_t *m = struct_method(a.as.st->def, "operator_index_set");
            if (m) { value_t idx = eval(I, env, target->b);
                     value_t args[2] = { idx, v };
                     call_func(I, m, I->globals, &a, args, 2); return; }
        }
        if (a.kind == VAL_ARRAY) {
            int64_t i = to_int(eval(I, env, target->b));
            if (i < 0 || (size_t)i >= a.as.arr->len) rt_error(I, target, "index out of range in assignment");
            a.as.arr->data[i] = v; return;
        }
        rt_error(I, target, "value is not index-assignable");
    }
    rt_error(I, target, "invalid assignment target");
}

flow_t exec_list(interp_t *I, env_t *env, frame_t *fr, vec_t *list, value_t *ret)
{
    { size_t i = 0; for (; i < list->len; i++) {
        flow_t f = exec_stmt(I, env, fr, (ast_node_t *)list->data[i], ret);
        if (f != FLOW_NORMAL) return f;
    } }
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
            if (n->a) v = eval(I, env, n->a);
            else      v = default_for_type(I, n->type_str);   
            env_define(I, env, n->name, v);
            return FLOW_NORMAL;
        }
        case AST_EXPR_STMT:
            eval(I, env, n->a);
            return FLOW_NORMAL;
        case AST_ASSIGN: {
            value_t rhs;
            if (n->op == TK_ASSIGN) {
                rhs = eval(I, env, n->b);
            } else {
                token_kind_t base = compound_base(n->op);
                value_t cur = eval(I, env, n->a);
                value_t r   = eval(I, env, n->b);
                bool found; value_t sr = try_struct_op(I, base, cur, r, true, &found);
                rhs = found ? sr : arith(I, n, base, cur, r);
            }
            assign_to(I, env, n->a, rhs);
            return FLOW_NORMAL;
        }
        case AST_IF: {
            if (to_bool(eval(I, env, n->a))) {
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
        case AST_WHILE:
            while (tick(I), to_bool(eval(I, env, n->a))) {
                env_t *c = env_new(I, env);
                flow_t f = exec_list(I, c, fr, &n->b->list, ret);
                if (f == FLOW_RETURN) return f;
                if (f == FLOW_BREAK)  break;
                
            }
            return FLOW_NORMAL;
        case AST_REPEAT: {
            #define REP_NUM(NODE, OUT, WHAT) do {                                  \
                value_t _v = eval(I, env, (NODE));                                 \
                if      (_v.kind == VAL_INT)   (OUT) = _v.as.i;                     \
                else if (_v.kind == VAL_FLOAT) (OUT) = (int64_t)_v.as.f;           \
                else { rt_error(I, (NODE), "repeat " WHAT " must be a number");     \
                       return FLOW_NORMAL; }                                     \
            } while (0)
            int64_t start, end, step;
            if (n->c) {                          
                REP_NUM(n->a, start, "range start");
                REP_NUM(n->c, end,   "range bound");
                if (n->d) REP_NUM(n->d, step, "step"); else step = 1;
                if (step <= 0) { rt_error(I, n->d ? n->d : n, "repeat step must be positive"); return FLOW_NORMAL; }
            } else {                             
                REP_NUM(n->a, end, "count");
                start = 0; end -= 1; step = 1;   
            }
            #undef REP_NUM
            { int64_t k = start; for (; k <= end; k += step) {
                tick(I);
                env_t *c = env_new(I, env);
                flow_t f = exec_list(I, c, fr, &n->b->list, ret);
                if (f == FLOW_RETURN) return f;
                if (f == FLOW_BREAK)  break;
            } }
            return FLOW_NORMAL;
        }
        case AST_FOR: {
            env_t *loop = env_new(I, env);
            if (n->a) exec_stmt(I, loop, fr, n->a, ret);
            while (tick(I), !n->b || to_bool(eval(I, loop, n->b))) {
                env_t *c = env_new(I, loop);
                flow_t f = exec_list(I, c, fr, &n->d->list, ret);
                if (f == FLOW_RETURN) return f;
                if (f == FLOW_BREAK)  break;
                if (n->c) exec_stmt(I, loop, fr, n->c, ret);
            }
            return FLOW_NORMAL;
        }
        case AST_RETURN:
            *ret = n->a ? eval(I, env, n->a) : val_null();
            return FLOW_RETURN;
        case AST_BREAK:    return FLOW_BREAK;
        case AST_CONTINUE: return FLOW_CONTINUE;
        case AST_DEFER: {
            defer_t *d = (defer_t *)arena_alloc(I->a, sizeof *d);
            d->stmt = n->a; d->env = env;
            vec_push(I->a, &fr->defers, d);
            return FLOW_NORMAL;
        }
        default:
            
            eval(I, env, n);
            return FLOW_NORMAL;
    }
}
