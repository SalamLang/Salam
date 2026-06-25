#include "codegen/codegen_internal.h"

void cg_emit_defers(cg_t *cg)
{
    { size_t i = cg->fn_defers.len; for (; i > 0; i--)
        cg_stmt(cg, (ast_node_t *)cg->fn_defers.data[i - 1]); }
}

static const char *cg_vardecl_inline(cg_t *cg, ast_node_t *n)  
{
    const char *ts = n->type_str ? n->type_str : "int32_t";
    const char *decl = cg_decl(cg, ts, n->name);
    local_add(cg, n->name);
    
    bool is_ref = ts && (strchr(ts, '[') ||
                         ts[strlen(ts) - 1] == '*' ||
                         struct_by_name(cg, ts) != NULL);
    
    bool already_const = (strncmp(cg_ctype(cg, ts), "const ", 6) == 0);
    const char *prefix = (n->is_mut || is_ref || already_const) ? "" : "const ";
    if (n->a) return cg_fmt(cg, "%s%s = %s", prefix, decl, cg_expr(cg, n->a));
    
    if (ts && strchr(ts, '[')) return cg_fmt(cg, "%s%s = {0}", prefix, decl);
    return cg_fmt(cg, "%s%s", prefix, decl);
}

static const char *cg_simple_inline(cg_t *cg, ast_node_t *n)
{
    if (n->kind == AST_VAR_DECL)   return cg_vardecl_inline(cg, n);
    if (n->kind == AST_ASSIGN)     return cg_fmt(cg, "%s %s %s", cg_expr(cg, n->a), cg_op(n->op), cg_expr(cg, n->b));
    if (n->kind == AST_EXPR_STMT)  return cg_expr(cg, n->a);
    return cg_expr(cg, n);
}

void cg_stmt(cg_t *cg, ast_node_t *n)
{
    if (!n) return;
    cg_source_line(cg, &n->span);
    switch (n->kind) {
        case AST_BLOCK: cg_block(cg, n); break;
        case AST_VAR_DECL:
            
            if (n->type_str && !strncmp(n->type_str, "dyn ", 4) && strchr(n->type_str, '[') &&
                n->a && n->a->kind == AST_ARRAY_LIT) {
                local_add(cg, n->name);
                cg_line(cg, "%s;", cg_decl(cg, n->type_str, n->name));
                { size_t i = 0; for (; i < n->a->list.len; i++)
                    cg_line(cg, "%s[%zu] = %s;", cg_cident(cg, n->name), i,
                            cg_expr(cg, (ast_node_t *)n->a->list.data[i])); }
                break;
            }
            cg_line(cg, "%s;", cg_vardecl_inline(cg, n)); break;
        case AST_CONST_DECL: {
            const char *cts = n->type_str ? n->type_str : "int32_t";
            const char *decl = cg_decl(cg, cts, n->name);
            local_add(cg, n->name);
            const char *cpfx = (strncmp(cg_ctype(cg, cts), "const ", 6) == 0) ? "" : "const ";
            cg_line(cg, "%s%s = %s;", cpfx, decl, cg_expr(cg, n->a));
            break;
        }
        case AST_ASSIGN: {
            
            if (n->op == TK_ASSIGN && n->a && n->a->kind == AST_INDEX &&
                n->a->a && n->a->a->type_str) {
                char sname[96];
                symbol_t *ssym = cg_struct_of(cg, n->a->a->type_str, sname, sizeof sname);
                if (ssym) {
                    symbol_t *m = scope_lookup_local(ssym->members, "operator_index_set");
                    if (m && m->kind == SYM_METHOD) {
                        func_sig_t *sig = pick_op_overload(cg, m, 2); 
                        if (sig) {
                            const char *mangled = cg_mangle_method(cg, sname, ssym, "operator_index_set", &sig->params);
                            ast_node_t *container = n->a->a;
                            const char *idx_c  = cg_expr(cg, n->a->b);
                            const char *val_c  = cg_expr(cg, n->b);
                            bool addressable   = container->kind == AST_IDENTIFIER ||
                                                 container->kind == AST_MEMBER    ||
                                                 container->kind == AST_INDEX     ||
                                                 container->kind == AST_THIS;
                            if (addressable) {
                                const char *recv = cg_fmt(cg, "&(%s)", cg_expr(cg, container));
                                cg_line(cg, "%s(%s, %s, %s);", mangled, recv, idx_c, val_c);
                            } else {
                                int t = ++cg->tmpn;
                                const char *ct = cg_ctype(cg, container->type_str ? container->type_str : sname);
                                const char *lc = cg_expr(cg, container);
                                cg_line(cg, "{ %s __t%d = (%s); %s(&__t%d, %s, %s); }",
                                        ct, t, lc, mangled, t, idx_c, val_c);
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
                            const char *call = cg_emit_op_call(cg, n->a, ssym, sname, mname, n->b);
                            if (call) {
                                cg_line(cg, "%s = %s;", cg_expr(cg, n->a), call);
                                break;
                            }
                        }
                    }
                }
            }
            
            if (n->op == TK_PLUS_EQ && n->a->type_str && !strcmp(n->a->type_str, "str")) {
                cg_line(cg, "%s = salam_strcat(%s, %s);",
                        cg_expr(cg, n->a), cg_expr(cg, n->a), cg_str_operand(cg, n->b));
                break;
            }
            cg_line(cg, "%s %s %s;", cg_expr(cg, n->a), cg_op(n->op), cg_expr(cg, n->b));
            break;
        }
        case AST_EXPR_STMT:
            cg_line(cg, "%s;", cg_expr(cg, n->a));
            break;
        case AST_DEFER:
            vec_push(cg->a, &cg->fn_defers, n->a);
            break;
        case AST_RETURN:
            cg_emit_defers(cg);
            if (n->a && cg->cur_sret) cg_line(cg, "*__ret = (%s); return;", cg_expr(cg, n->a));
            else if (n->a)            cg_line(cg, "return %s;", cg_expr(cg, n->a));
            else                      cg_line(cg, "return;");
            break;
        case AST_BREAK:    cg_line(cg, "break;"); break;
        case AST_CONTINUE: cg_line(cg, "continue;"); break;
        case AST_IF:
            cg_line(cg, "if (%s) {", cg_expr(cg, n->a));
            cg->indent++; { size_t m = cg->locals.len;
                { size_t i = 0; for (; i < n->b->list.len; i++) cg_stmt(cg, (ast_node_t *)n->b->list.data[i]); }
                cg->locals.len = m; } cg->indent--;
            if (n->c && n->c->kind == AST_IF) { cg_indent(cg); sb_puts(cg->c, "} else ");
                
                sb_puts(cg->c, cg_fmt(cg, "if (%s) {\n", cg_expr(cg, n->c->a)));
                cg->indent++; { size_t m = cg->locals.len;
                    { size_t i = 0; for (; i < n->c->b->list.len; i++) cg_stmt(cg, (ast_node_t *)n->c->b->list.data[i]); }
                    cg->locals.len = m; } cg->indent--;
                if (n->c->c) { cg_line(cg, "} else {");
                    cg->indent++; cg_stmt(cg, n->c->c); cg->indent--; }
                cg_line(cg, "}");
            } else if (n->c) {
                cg_line(cg, "} else {");
                cg->indent++; { size_t m = cg->locals.len;
                    { size_t i = 0; for (; i < n->c->list.len; i++) cg_stmt(cg, (ast_node_t *)n->c->list.data[i]); }
                    cg->locals.len = m; } cg->indent--;
                cg_line(cg, "}");
            } else {
                cg_line(cg, "}");
            }
            break;
        case AST_WHILE:
            cg_line(cg, "while (%s) {", cg_expr(cg, n->a));
            cg->indent++; { size_t m = cg->locals.len;
                { size_t i = 0; for (; i < n->b->list.len; i++) cg_stmt(cg, (ast_node_t *)n->b->list.data[i]); }
                cg->locals.len = m; } cg->indent--;
            cg_line(cg, "}");
            break;
        case AST_REPEAT: {
            int t = ++cg->tmpn;
            if (n->c) {
                
                const char *from = cg_expr(cg, n->a);
                const char *to   = cg_expr(cg, n->c);
                const char *step = n->d ? cg_expr(cg, n->d) : "1";
                cg_line(cg, "for (int64_t __rep%d = (int64_t)(%s), __repe%d = (int64_t)(%s), __reps%d = (int64_t)(%s);"
                            " __rep%d <= __repe%d; __rep%d += __reps%d) {",
                        t, from, t, to, t, step, t, t, t, t);
            } else {
                
                cg_line(cg, "for (int64_t __rep%d = 0, __repn%d = (int64_t)(%s); __rep%d < __repn%d; __rep%d++) {",
                        t, t, cg_expr(cg, n->a), t, t, t);
            }
            cg->indent++; { size_t m = cg->locals.len;
                { size_t i = 0; for (; i < n->b->list.len; i++) cg_stmt(cg, (ast_node_t *)n->b->list.data[i]); }
                cg->locals.len = m; } cg->indent--;
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
            { size_t i = 0; for (; i < n->d->list.len; i++) cg_stmt(cg, (ast_node_t *)n->d->list.data[i]); }
            cg->indent--;
            cg_line(cg, "}");
            cg->locals.len = mark;
            break;
        }
        default: break;
    }
}

void cg_block(cg_t *cg, ast_node_t *block)
{
    size_t mark = cg->locals.len;
    { size_t i = 0; for (; i < block->list.len; i++)
        cg_stmt(cg, (ast_node_t *)block->list.data[i]); }
    cg->locals.len = mark;
}
