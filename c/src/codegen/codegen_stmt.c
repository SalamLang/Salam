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

/*
 * `defer` is scoped to the block it is written in, not to the whole function.
 * The registration list is still one flat per-function stack, but every nested
 * block remembers its depth on entry and flushes back down to it on the way
 * out, so a defer that names a block-local sees that name in scope:
 *
 *     if cond:
 *         mut v := ...
 *         defer v.free()     // emitted here, at the closing brace
 *     end                    // - not at function exit, where v is gone
 *
 * That also makes the cleanup conditional (it only runs when the branch runs)
 * and makes a defer in a loop body run once per iteration, which is what the
 * interpreter does by registering defers as it reaches them.
 *
 * `ret` still flushes the whole stack, since it leaves every open block.
 */
void cg_emit_defers_from(cg_t *cg, size_t mark)
{
    {
        size_t i = cg->fn_defers.len;
        for (; i > mark; i--)
            cg_stmt(cg, (ast_node_t *)cg->fn_defers.data[i - 1]);
    }
}

void cg_emit_defers(cg_t *cg)
{
    cg_emit_defers_from(cg, 0);
}

/*
 * A block whose last statement jumps has already emitted its defers on that
 * path (ret flushes everything, break/continue flush back to the loop mark),
 * so a second copy at the closing brace would be dead code.
 */
static bool cg_list_terminates(const vec_t *list)
{
    ast_node_t *last;
    if (!list->len) return false;
    last = (ast_node_t *)list->data[list->len - 1];
    return last && (last->kind == AST_RETURN || last->kind == AST_BREAK ||
                    last->kind == AST_CONTINUE);
}

void cg_scoped_stmts(cg_t *cg, vec_t *list)
{
    size_t lmark = cg->locals.len;
    size_t dmark = cg->fn_defers.len;
    {
        size_t i = 0;
        for (; i < list->len; i++)
            cg_stmt(cg, (ast_node_t *)list->data[i]);
    }
    if (!cg_list_terminates(list)) cg_emit_defers_from(cg, dmark);
    cg->fn_defers.len = dmark;
    cg->locals.len = lmark;
}

/* Loop bodies additionally record their defer depth so that break/continue,
 * which skip the closing brace, still run what the body registered. */
static void cg_loop_push(cg_t *cg)
{
    if (cg->nloop < CG_MAX_LOOP_DEPTH) cg->loop_dmark[cg->nloop] = cg->fn_defers.len;
    cg->nloop++;
}

static void cg_loop_pop(cg_t *cg)
{
    if (cg->nloop > 0) cg->nloop--;
}

static void cg_emit_loop_exit_defers(cg_t *cg)
{
    if (cg->nloop > 0 && cg->nloop <= CG_MAX_LOOP_DEPTH)
        cg_emit_defers_from(cg, cg->loop_dmark[cg->nloop - 1]);
}

/*
 * cg_expr parenthesizes every binary expression it builds, so `if (%s)` came
 * out as `if ((f == NULL))`. clang reads doubled parens around an equality
 * test as the "did you mean = ?" idiom and warns on every one of them
 * (-Wparentheses-equality; 22 in a benchmark-runner build alone), and gcc does
 * the same under -Wall. Dropping the redundant layer at the condition sites
 * fixes it without changing how expressions are built anywhere else.
 *
 * The outer parens only come off when they wrap the whole expression: `(a) &&
 * (b)` opens and closes before the end and is left alone. String and char
 * literals are skipped so a parenthesis inside one is not counted.
 */
const char *cg_unparen(cg_t *cg, const char *s)
{
    size_t len = s ? strlen(s) : 0;
    int depth = 0;
    size_t i = 0;
    /* `({ ... })` is a GNU statement expression: those parens are syntax, not
     * grouping, and taking them off leaves a block where an expression goes. */
    if (len < 2 || s[0] != '(' || s[len - 1] != ')' || s[1] == '{') return s;
    for (; i < len; i++) {
        if (s[i] == '"' || s[i] == '\'') {
            char q = s[i];
            for (i++; i < len && s[i] != q; i++)
                if (s[i] == '\\') i++;
            continue;
        }
        if (s[i] == '(') {
            depth++;
        } else if (s[i] == ')') {
            depth--;
            if (depth == 0 && i + 1 != len) return s;
        }
    }
    if (depth != 0) return s;
    return cg_fmt(cg, "%.*s", (int)(len - 2), s + 1);
}

static const char *cg_cond(cg_t *cg, ast_node_t *n)
{
    return cg_unparen(cg, cg_expr(cg, n));
}

static const char *cg_vardecl_inline(cg_t *cg, ast_node_t *n)
{
    const char *ts = n->type_str ? n->type_str : "int32_t";
    const char *decl = cg_decl(cg, ts, n->name);
    local_add(cg, n->name);

    bool is_ref = ts && (strchr(ts, '[') || ts[strlen(ts) - 1] == '*' ||
                         struct_by_name(cg, ts) != NULL);

    bool already_const = (strncmp(cg_ctype(cg, ts), "const ", 6) == 0);
    const char *prefix = (n->is_mut || is_ref || already_const) ? "" : "const ";
    if (n->a) return cg_fmt(cg, "%s%s = %s", prefix, decl, cg_expr(cg, n->a));

    /* No initializer. Sema rejects every read that no assignment precedes, but
     * a `&:` parameter the callee only writes on some paths would still leave
     * this slot indeterminate, so zero it: a defined value beats UB. `= {0}`
     * initializes scalars, pointers, arrays and structs alike (C11 6.7.9p11). */
    return cg_fmt(cg, "%s%s = {0}", prefix, decl);
}

static const char *cg_simple_inline(cg_t *cg, ast_node_t *n)
{
    if (n->kind == AST_VAR_DECL) return cg_vardecl_inline(cg, n);
    if (n->kind == AST_ASSIGN) {
        if (n->op == TK_POWER_EQ) {
            const char *dst = cg_expr(cg, n->a);
            const char *base = cg_expr(cg, n->a);
            const char *exp = cg_expr(cg, n->b);
            return cg_fmt(cg, "%s = pow(SALAM_FPARG_D((double)(%s)), (double)(%s))", dst,
                          base, exp);
        }
        {
            const char *lhs = cg_expr(cg, n->a);
            const char *rhs = cg_expr(cg, n->b);
            return cg_fmt(cg, "%s %s %s", lhs, cg_op(n->op), rhs);
        }
    }
    if (n->kind == AST_EXPR_STMT) return cg_expr(cg, n->a);
    return cg_expr(cg, n);
}

void cg_stmt(cg_t *cg, ast_node_t *n)
{
    if (!n) return;
    cg_source_line(cg, &n->span);
    switch (n->kind) {
    case AST_BLOCK:
        cg_block(cg, n);
        break;
    case AST_VAR_DECL:

        if (n->type_str && !strncmp(n->type_str, "dyn ", 4) && strchr(n->type_str, '[') &&
            n->a && n->a->kind == AST_ARRAY_LIT) {
            local_add(cg, n->name);
            cg_line(cg, "%s;", cg_decl(cg, n->type_str, n->name));
            {
                size_t i = 0;
                for (; i < n->a->list.len; i++)
                    cg_line(cg, "%s[%zu] = %s;", cg_cident(cg, n->name), i,
                            cg_expr(cg, (ast_node_t *)n->a->list.data[i]));
            }
            break;
        }
        cg_line(cg, "%s;", cg_vardecl_inline(cg, n));
        /*
         * A leading underscore is how Salam spells "declared on purpose,
         * never read" - it is what stops sema's unused-name rule firing. The
         * C compiler has no way to know that, so gcc and clang report
         * -Wunused-variable for exactly the bindings the author already
         * marked as deliberate (std/compress reads DEFLATE's NLEN field into
         * one just to advance the bit stream). Carry the intent across.
         *
         * The initializer still runs: it is the call's side effect that was
         * wanted, so this suppresses the warning without dropping the work.
         */
        if (n->name && n->name[0] == '_')
            cg_line(cg, "(void)%s;", cg_cident(cg, n->name));
        break;
    case AST_CONST_DECL: {
        const char *cts = n->type_str ? n->type_str : "int32_t";
        const char *decl = cg_decl(cg, cts, n->name);
        local_add(cg, n->name);
        const char *cpfx = (strncmp(cg_ctype(cg, cts), "const ", 6) == 0) ? "" : "const ";
        cg_line(cg, "%s%s = %s;", cpfx, decl, cg_expr(cg, n->a));
        break;
    }
    case AST_ASSIGN: {
        if (n->op == TK_ASSIGN && n->a && n->a->kind == AST_INDEX && n->a->a &&
            n->a->a->type_str) {
            char sname[96];
            symbol_t *ssym = cg_struct_of(cg, n->a->a->type_str, sname, sizeof sname);
            if (ssym) {
                symbol_t *m = scope_lookup_local(ssym->members, "operator_index_set");
                if (m && m->kind == SYM_METHOD) {
                    func_sig_t *sig = pick_op_overload(cg, m, 2);
                    if (sig) {
                        const char *mangled = cg_mangle_method(
                            cg, sname, ssym, "operator_index_set", &sig->params);
                        ast_node_t *container = n->a->a;
                        const char *idx_c = cg_expr(cg, n->a->b);
                        const char *val_c = cg_expr(cg, n->b);
                        bool addressable = container->kind == AST_IDENTIFIER ||
                                           container->kind == AST_MEMBER ||
                                           container->kind == AST_INDEX ||
                                           container->kind == AST_THIS;
                        if (addressable) {
                            const char *recv =
                                cg_fmt(cg, "&(%s)", cg_expr(cg, container));
                            cg_line(cg, "%s(%s, %s, %s);", mangled, recv, idx_c, val_c);
                        } else {
                            int t = ++cg->tmpn;
                            const char *ct = cg_ctype(
                                cg, container->type_str ? container->type_str : sname);
                            const char *lc = cg_expr(cg, container);
                            cg_line(cg, "{ %s __t%d = (%s); %s(&__t%d, %s, %s); }", ct, t,
                                    lc, mangled, t, idx_c, val_c);
                        }
                        break;
                    }
                }
            }
        }

        if (n->op != TK_ASSIGN && n->a && n->a->type_str) {
            token_kind_t base_op = cg_compound_base(n->op);
            if (base_op != TK_EOF) {
                char sname[96];
                symbol_t *ssym = cg_struct_of(cg, n->a->type_str, sname, sizeof sname);
                if (ssym) {
                    const char *mname = cg_op_method(base_op);
                    if (mname) {
                        const char *call =
                            cg_emit_op_call(cg, n->a, ssym, sname, mname, n->b);
                        if (call) {
                            cg_line(cg, "%s = %s;", cg_expr(cg, n->a), call);
                            break;
                        }
                    }
                }
            }
        }

        if (n->op == TK_PLUS_EQ && n->a->type_str && !strcmp(n->a->type_str, "str")) {
            const char *dst = cg_expr(cg, n->a);
            const char *cur = cg_expr(cg, n->a);
            const char *add = cg_str_operand(cg, n->b);
            cg_line(cg, "%s = salam_strcat(%s, %s);", dst, cur, add);
            break;
        }
        if (n->op == TK_POWER_EQ) {
            const char *ct = cg_ctype(cg, n->a->type_str ? n->a->type_str : "double");
            int t = ++cg->tmpn;
            const char *lhs = cg_expr(cg, n->a);
            const char *rhs = cg_expr(cg, n->b);
            cg_line(cg,
                    "{ %s *__pw%d = &(%s); *__pw%d = (%s)pow("
                    "SALAM_FPARG_D((double)(*__pw%d)), (double)(%s)); }",
                    ct, t, lhs, t, ct, t, rhs);
            break;
        }
        {
            const char *lhs = cg_expr(cg, n->a);
            const char *rhs = cg_expr(cg, n->b);
            cg_line(cg, "%s %s %s;", lhs, cg_op(n->op), rhs);
        }
        break;
    }
    case AST_EXPR_STMT: {
        /*
         * A call returning a struct is built as a GNU statement expression
         * whose value is the temporary it filled in. Called for its effect
         * only, that trailing value is dropped, and clang says so
         * (-Wunused-value). Casting to void is how C spells "yes, on
         * purpose".
         */
        const char *e = cg_expr(cg, n->a);
        cg_line(cg, (e[0] == '(' && e[1] == '{') ? "(void)%s;" : "%s;", e);
        break;
    }
    case AST_DEFER:
        vec_push(cg->a, &cg->fn_defers, n->a);
        break;
    case AST_RETURN:
        if (cg->match_result_tmp) {
            if (n->a)
                cg_line(cg, "%s = (%s)(%s); goto %s;", cg->match_result_tmp,
                        cg->match_result_ctype, cg_expr(cg, n->a), cg->match_end_label);
            else
                cg_line(cg, "goto %s;", cg->match_end_label);
            break;
        }
        if (n->a && cg->fn_defers.len > 0) {
            const char *expr = cg_expr(cg, n->a);
            const char *ts = n->a->type_str;
            int t = ++cg->tmpn;
            const char *tmp = cg_fmt(cg, "__retv%d", t);
            cg_line(cg, "%s %s = (%s);", cg_ctype(cg, ts ? ts : "int32_t"), tmp, expr);
            cg_emit_defers(cg);
            if (cg->cur_sret)
                cg_line(cg, "*__ret = %s; return;", tmp);
            else
                cg_line(cg, "return %s;", tmp);
        } else if (n->a && cg->cur_sret) {
            cg_emit_defers(cg);
            cg_line(cg, "*__ret = (%s); return;", cg_expr(cg, n->a));
        } else if (n->a) {
            cg_emit_defers(cg);
            cg_line(cg, "return %s;", cg_expr(cg, n->a));
        } else {
            cg_emit_defers(cg);
            /*
             * `func main:` has no declared return type but is emitted as C's
             * `int main`, so a bare `ret` in it is a bare `return;` from a
             * non-void function. tcc accepts that silently; gcc 16 makes
             * -Wreturn-mismatch an error, which took out every telegram/
             * websocket example that ends a branch with a bare `ret`. The
             * language already says main implicitly returns i32, so the
             * value that `ret` means here is 0.
             */
            cg_line(cg, cg->cur_is_main ? "return 0;" : "return;");
        }
        break;
    case AST_MATCH: {
        const char *subj_ts = n->a->type_str;
        int t = ++cg->tmpn;
        const char *subj_var = cg_fmt(cg, "__msubj%d", t);
        cg_line(cg, "{");
        cg->indent++;
        cg_line(cg, "%s %s = (%s);", cg_ctype(cg, subj_ts ? subj_ts : "int32_t"),
                subj_var, cg_expr(cg, n->a));
        {
            size_t i = 0;
            for (; i < n->list.len; i++) {
                ast_node_t *arm = (ast_node_t *)n->list.data[i];
                const char *cond =
                    cg_unparen(cg, cg_match_arm_cond(cg, arm, subj_var, subj_ts));
                cg_line(cg, "%sif (%s) {", i ? "} else " : "", cond);
                cg->indent++;
                cg_scoped_stmts(cg, &arm->b->list);
                cg->indent--;
            }
        }
        if (n->list.len) cg_line(cg, "}");
        cg->indent--;
        cg_line(cg, "}");
        break;
    }
    case AST_BREAK:
        cg_emit_loop_exit_defers(cg);
        cg_line(cg, "break;");
        break;
    case AST_CONTINUE:
        cg_emit_loop_exit_defers(cg);
        cg_line(cg, "continue;");
        break;
    case AST_IF:
        cg_line(cg, "if (%s) {", cg_cond(cg, n->a));
        cg->indent++;
        cg_scoped_stmts(cg, &n->b->list);
        cg->indent--;
        if (n->c && n->c->kind == AST_IF) {
            cg_indent(cg);
            sb_puts(cg->c, "} else ");

            sb_puts(cg->c, cg_fmt(cg, "if (%s) {\n", cg_cond(cg, n->c->a)));
            cg->indent++;
            cg_scoped_stmts(cg, &n->c->b->list);
            cg->indent--;
            if (n->c->c) {
                cg_line(cg, "} else {");
                cg->indent++;
                cg_stmt(cg, n->c->c);
                cg->indent--;
            }
            cg_line(cg, "}");
        } else if (n->c) {
            cg_line(cg, "} else {");
            cg->indent++;
            cg_scoped_stmts(cg, &n->c->list);
            cg->indent--;
            cg_line(cg, "}");
        } else {
            cg_line(cg, "}");
        }
        break;
    case AST_UNTIL:
        cg_line(cg, "while (%s) {", cg_cond(cg, n->a));
        cg->indent++;
        cg_loop_push(cg);
        cg_scoped_stmts(cg, &n->b->list);
        cg_loop_pop(cg);
        cg->indent--;
        cg_line(cg, "}");
        break;
    case AST_REPEAT: {
        int t = ++cg->tmpn;
        if (n->c) {
            const char *from = cg_expr(cg, n->a);
            const char *to = cg_expr(cg, n->c);
            const char *step = n->d ? cg_expr(cg, n->d) : "1";
            cg_line(cg,
                    "for (int64_t __rep%d = (int64_t)(%s), __repe%d = (int64_t)(%s), "
                    "__repa%d = (int64_t)(%s), "
                    "__reps%d = (__rep%d <= __repe%d) ? __repa%d : -__repa%d;"
                    " (__reps%d > 0) ? (__rep%d <= __repe%d) : (__rep%d >= __repe%d); "
                    "__rep%d += __reps%d) {",
                    t, from, t, to, t, step, t, t, t, t, t, t, t, t, t, t, t, t);
        } else {
            cg_line(cg,
                    "for (int64_t __rep%d = 0, __repn%d = (int64_t)(%s); __rep%d < "
                    "__repn%d; __rep%d++) {",
                    t, t, cg_expr(cg, n->a), t, t, t);
        }
        cg->indent++;
        cg_loop_push(cg);
        {
            size_t m = cg->locals.len;
            if (n->name) {
                const char *ivct = cg_ctype(cg, n->type_str ? n->type_str : "i32");
                cg_line(cg, "const %s %s = (%s)__rep%d;", ivct, cg_cident(cg, n->name),
                        ivct, t);
                /*
                 * `repeat n with i:` may legitimately ignore i - the count is
                 * the point and the binding is there for the cases that do
                 * use it - so the declaration alone draws -Wunused-variable
                 * from gcc and clang. Salam's own unused-name rule does not
                 * apply to a loop binding, so this cannot be pushed back to
                 * the source. (void) is the portable way to say "declared on
                 * purpose"; __attribute__((unused)) is not available on tcc.
                 */
                cg_line(cg, "(void)%s;", cg_cident(cg, n->name));
                local_add(cg, n->name);
            }
            cg_scoped_stmts(cg, &n->b->list);
            cg->locals.len = m;
        }
        cg_loop_pop(cg);
        cg->indent--;
        cg_line(cg, "}");
        break;
    }
    case AST_FOR: {
        size_t mark = cg->locals.len;
        const char *init = n->a ? cg_simple_inline(cg, n->a) : "";
        const char *cond = n->b ? cg_expr(cg, n->b) : "";
        const char *post = n->c ? cg_simple_inline(cg, n->c) : "";
        cg_line(cg, "for (%s; %s; %s) {", init, cond, post);
        cg->indent++;
        cg_loop_push(cg);
        cg_scoped_stmts(cg, &n->d->list);
        cg_loop_pop(cg);
        cg->indent--;
        cg_line(cg, "}");
        cg->locals.len = mark;
        break;
    }
    default:
        break;
    }
}

void cg_block(cg_t *cg, ast_node_t *block)
{
    cg_scoped_stmts(cg, &block->list);
}
