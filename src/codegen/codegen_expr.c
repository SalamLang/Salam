#include "codegen/codegen_internal.h"

const char *cg_op(token_kind_t k)
{
    switch (k) {
        case TK_PLUS:return "+"; case TK_MINUS:return "-"; case TK_STAR:return "*";
        case TK_SLASH:return "/"; case TK_PERCENT:return "%";
        case TK_EQ:return "=="; case TK_NE:return "!="; case TK_LT:return "<";
        case TK_GT:return ">"; case TK_LE:return "<="; case TK_GE:return ">=";
        case TK_AND:return "&&"; case TK_OR:return "||"; case TK_NOT:return "!";
        case TK_ASSIGN:return "="; case TK_PLUS_EQ:return "+="; case TK_MINUS_EQ:return "-=";
        case TK_STAR_EQ:return "*="; case TK_SLASH_EQ:return "/="; case TK_PERCENT_EQ:return "%=";
        default: return "?";
    }
}

const char *cg_str_operand(cg_t *cg, ast_node_t *n)
{
    if (n->type_str && !strcmp(n->type_str, "str"))
        return cg_expr(cg, n);
    return cg_fmt(cg, "salam_tostr_%s(%s)", prim_suffix(print_tag(n->type_str)), cg_expr(cg, n));
}

const char *cg_cescape(cg_t *cg, const char *s)
{
    sb_t b; sb_init(&b); sb_putc(&b, '"');
    { const char *p = s; for (; *p; p++) {
        unsigned char ch = (unsigned char)*p;
        switch (ch) {
            case '\n': sb_puts(&b, "\\n");  break;
            case '\t': sb_puts(&b, "\\t");  break;
            case '\r': sb_puts(&b, "\\r");  break;
            case '\a': sb_puts(&b, "\\a");  break;
            case '\b': sb_puts(&b, "\\b");  break;
            case '\f': sb_puts(&b, "\\f");  break;
            case '\v': sb_puts(&b, "\\v");  break;
            case '"':  sb_puts(&b, "\\\""); break;
            case '\\': sb_puts(&b, "\\\\"); break;
            default:
                if (ch < 0x20 || ch >= 0x7f) {

                    char oct[5]; snprintf(oct, sizeof(oct), "\\%03o", ch);
                    sb_puts(&b, oct);
                } else {
                    sb_putc(&b, (char)ch);
                }
                break;
        }
    } }
    sb_putc(&b, '"');
    const char *r = arena_strdup(cg->a, sb_cstr(&b)); sb_free(&b); return r;
}

static const char *cg_dyn_box(cg_t *cg, ast_node_t *n)
{
    const char *iface    = n->type_str + 4;                 
    const char *concrete = n->a->type_str ? n->a->type_str : "";
    const char *cI = cg_cident(cg, iface);
    const char *cC = cg_cident(cg, concrete);
    int t = ++cg->tmpn;
    return cg_fmt(cg,
        "({ %s* __dp%d = (%s*)salam_alloc(sizeof(%s)); *__dp%d = (%s); "
        "(_Salam_dyn_%s){ (void*)__dp%d, &_Salam_vtbl_%s_%s }; })",
        cC, t, cC, cC, t, cg_expr(cg, n->a), cI, t, cI, cC);
}

const char *cg_emit_op_call(cg_t *cg, ast_node_t *lhs_node, symbol_t *ssym,
                            const char *sname, const char *mname,
                            ast_node_t *rhs_node)
{
    symbol_t *m = scope_lookup_local(ssym->members, mname);
    if (!m || m->kind != SYM_METHOD) return NULL;
    size_t nparams = rhs_node ? 1 : 0;
    func_sig_t *sig = pick_op_overload(cg, m, nparams);
    if (!sig) return NULL;
    const char *mangled = cg_mangle_method(cg, sname, ssym, mname, &sig->params);
    bool ret_struct = type_is_byval_agg(sig->ret);
    const char *Rc = ret_struct ? cg_ctype(cg, type_to_string(cg->sem->tc, sig->ret)) : NULL;
    const char *lhs_ts = lhs_node->type_str ? lhs_node->type_str : "";
    bool lhs_is_ptr = (strlen(lhs_ts) > 0 && lhs_ts[strlen(lhs_ts) - 1] == '*');
    bool addressable = cg_addressable(lhs_node);
    const char *rhs_c = rhs_node ? cg_expr(cg, rhs_node) : NULL;
    const char *sep   = rhs_c ? ", " : "";
    if (lhs_is_ptr || addressable) {
        const char *recv = lhs_is_ptr ? cg_expr(cg, lhs_node)
                                      : cg_fmt(cg, "&(%s)", cg_expr(cg, lhs_node));
        if (ret_struct) {
            int s = ++cg->tmpn;
            return cg_fmt(cg, "({ %s __s%d; %s(%s%s%s, &__s%d); __s%d; })",
                          Rc, s, mangled, recv, sep, rhs_c ? rhs_c : "", s, s);
        }
        return cg_fmt(cg, "%s(%s%s%s)", mangled, recv, sep, rhs_c ? rhs_c : "");
    }
    
    int t = ++cg->tmpn;
    const char *lhs_c = cg_expr(cg, lhs_node);
    if (ret_struct) {
        int sv = ++cg->tmpn;
        return cg_fmt(cg, "({ %s __t%d = (%s); %s __s%d; %s(&__t%d%s%s, &__s%d); __s%d; })",
                      cg_ctype(cg, lhs_ts), t, lhs_c, Rc, sv, mangled, t, sep,
                      rhs_c ? rhs_c : "", sv, sv);
    }
    return cg_fmt(cg, "({ %s __t%d = (%s); %s(&__t%d%s%s); })",
                  cg_ctype(cg, lhs_ts), t, lhs_c, mangled, t, sep, rhs_c ? rhs_c : "");
}

static const char *cg_struct_lit(cg_t *cg, ast_node_t *n)
{
    symbol_t *ssym = struct_by_name(cg, n->name);
    sb_t b; sb_init(&b);
    const char *sl_cn = (ssym && ssym->type && ssym->type->name) ? ssym->type->name : n->name;
    sb_puts(&b, cg_fmt(cg, "(%s){ ", cg_cident(cg, sl_cn)));
    bool first = true;
    if (ssym) {
        { size_t i = 0; for (; i < ssym->members->symbols.len; i++) {
            symbol_t *f = (symbol_t *)ssym->members->symbols.data[i];
            if (f->kind != SYM_FIELD) continue;
            ast_node_t *provided = NULL;
            { size_t j = 0; for (; j < n->list.len; j++) {
                ast_node_t *fi = (ast_node_t *)n->list.data[j];
                if (fi->name && !strcmp(fi->name, f->name)) { provided = fi; break; }
            } }
            const char *val = provided        ? cg_expr(cg, provided->a)
                            : (f->decl && f->decl->a) ? cg_expr(cg, f->decl->a)
                            : NULL;
            if (!val) continue;
            if (!first) sb_puts(&b, ", ");
            sb_puts(&b, cg_fmt(cg, ".%s = %s", cg_cident(cg, f->name), val));
            first = false;
        } }
    }
    if (first) sb_puts(&b, "0");
    sb_puts(&b, " }");
    const char *r = arena_strdup(cg->a, sb_cstr(&b)); sb_free(&b); return r;
}

const char *cg_expr(cg_t *cg, ast_node_t *n)
{
    if (!n) return "0";
    switch (n->kind) {
        case AST_LITERAL:
            switch (n->op) {
                case TK_INT: {
                    
                    unsigned long long u = (unsigned long long)n->value.as.i;
                    const char *suf = u > 9223372036854775807ULL ? "ULL"
                                    : u > 2147483647ULL          ? "LL" : "";
                    return cg_fmt(cg, "%llu%s", u, suf);
                }
                case TK_FLOAT: {
                    
                    char buf[64];
                    snprintf(buf, sizeof buf, "%.17g", n->value.as.f);
                    if (!strpbrk(buf, ".eEnN"))   
                        return cg_fmt(cg, "%s.0", buf);
                    return cg_fmt(cg, "%s", buf);
                }
                
                case TK_STRING:
                case TK_TRIPLE_STRING:
                case TK_RAW_STRING:
                case TK_UTF8_CHAR:     return cg_cescape(cg, n->value.as.s ? n->value.as.s : "");
                
                case TK_CHAR:          return cg_fmt(cg, "%llu", (unsigned long long)n->value.as.i);
                case TK_KW_TRUE:       return "true";
                case TK_KW_FALSE:      return "false";
                case TK_KW_NULL:       return "NULL";
                default:               return "0";
            }
        case AST_IDENTIFIER:
            
            if (cg->cur_lambda) {
                vec_t *caps = &cg->cur_lambda->captures;
                { size_t i = 0; for (; i < caps->len; i++)
                    if (!strcmp(((ast_node_t *)caps->data[i])->name, n->name))
                        return cg_fmt(cg, "__env->%s", cg_cident(cg, n->name)); }
            }
            if (local_known(cg, n->name)) {
                if (n->is_ref) return cg_fmt(cg, "(*%s)", cg_cident(cg, n->name));
                return cg_cident(cg, n->name);
            }
            if (cg->cur_struct) {
                symbol_t *f = scope_lookup_local(cg->cur_struct->members, n->name);
                if (f && f->kind == SYM_FIELD) return cg_fmt(cg, "this->%s", cg_cident(cg, n->name));
            }
            return cg_cident(cg, n->name);
        case AST_THIS: return "this";
        case AST_BINARY: {
            
            if (n->a && n->a->type_str) {
                char sname[96];
                symbol_t *ssym = cg_struct_of(cg, n->a->type_str, sname, sizeof sname);
                if (ssym) {
                    
                    bool neg_eq = (n->op == TK_NE && n->synthetic);
                    const char *mname = neg_eq ? "operator_eq" : cg_op_method(n->op);
                    if (mname) {
                        const char *call = cg_emit_op_call(cg, n->a, ssym, sname, mname, n->b);
                        if (call) return neg_eq ? cg_fmt(cg, "(!(%s))", call) : call;
                    }
                }
            }
            if (n->op == TK_POWER)
                return cg_fmt(cg, "pow((double)(%s), (double)(%s))",
                              cg_expr(cg, n->a), cg_expr(cg, n->b));
            
            if (n->op == TK_PLUS &&
                ((n->a->type_str && !strcmp(n->a->type_str,"str")) ||
                 (n->b->type_str && !strcmp(n->b->type_str,"str"))))
                return cg_fmt(cg, "salam_strcat(%s, %s)",
                              cg_str_operand(cg, n->a), cg_str_operand(cg, n->b));
            
            if (n->a->type_str && !strcmp(n->a->type_str, "str") &&
                n->b->type_str && !strcmp(n->b->type_str, "str")) {
                const char *op = NULL;
                switch (n->op) {
                    case TK_EQ: op = "=="; break;
                    case TK_NE: op = "!="; break;
                    case TK_LT: op = "<";  break;
                    case TK_GT: op = ">";  break;
                    case TK_LE: op = "<="; break;
                    case TK_GE: op = ">="; break;
                    default: break;
                }
                if (op)
                    return cg_fmt(cg, "(strcmp(%s, %s) %s 0)",
                                  cg_expr(cg, n->a), cg_expr(cg, n->b), op);
            }
            return cg_fmt(cg, "(%s %s %s)", cg_expr(cg, n->a), cg_op(n->op), cg_expr(cg, n->b));
        }
        case AST_UNARY: {
            
            if (n->a && n->a->type_str) {
                char sname[96];
                symbol_t *ssym = cg_struct_of(cg, n->a->type_str, sname, sizeof sname);
                if (ssym) {
                    const char *mname = (n->op == TK_MINUS) ? "operator_sub"
                                      : (n->op == TK_NOT)   ? "operator_not" : NULL;
                    if (mname) {
                        symbol_t *m = scope_lookup_local(ssym->members, mname);
                        if (m && m->kind == SYM_METHOD && pick_op_overload(cg, m, 0)) {
                            const char *call = cg_emit_op_call(cg, n->a, ssym, sname, mname, NULL);
                            if (call) return call;
                        }
                    }
                }
            }
            return cg_fmt(cg, "(%s%s)", cg_op(n->op), cg_expr(cg, n->a));
        }
        case AST_CAST: {
            
            if (n->type_str && !strncmp(n->type_str, "dyn ", 4) &&
                n->type_str[strlen(n->type_str) - 1] != '*')
                return cg_dyn_box(cg, n);
            
            {
                const char *dts = n->type && n->type->type_str ? n->type->type_str : "";
                const char *sts = n->a && n->a->type_str ? n->a->type_str : "";
                bool dptr = *dts && dts[strlen(dts)-1] == '*';
                bool sptr = *sts && sts[strlen(sts)-1] == '*';
                
                
                const char *tgt = (n->type && n->type->type_str && n->type->type_str[0])
                                  ? n->type->type_str
                                  : (n->type_str ? n->type_str : "");
                if ((dptr != sptr && (cg_is_int_typestr(dts) || cg_is_int_typestr(sts))) ||
                    (dptr && !strcmp(sts, "str") && strcmp(dts, "str") != 0))
                    return cg_fmt(cg, "((%s)(intptr_t)(%s))", cg_ctype(cg, tgt), cg_expr(cg, n->a));
                return cg_fmt(cg, "((%s)(%s))", cg_ctype(cg, tgt), cg_expr(cg, n->a));
            }
        }
        case AST_CALL:   return cg_call(cg, n);
        case AST_MEMBER: {
            if (n->a && n->a->kind == AST_IDENTIFIER) {
                symbol_t *e = scope_lookup(cg->sem->global, n->a->name);
                if (e && e->kind == SYM_ENUM)
                    return cg_fmt(cg, "%s_%s", cg_cident(cg, e->name), cg_cident(cg, n->name));
                
                if (e && e->kind == SYM_PACKAGE) {
                    symbol_t *m = scope_lookup_local(e->members, n->name);
                    if (m && m->kind == SYM_CONST && m->decl && m->decl->a)
                        return cg_expr(cg, m->decl->a);
                    if (m && m->decl && m->decl->a) return cg_expr(cg, m->decl->a);
                }
            }
            const char *objts = n->a->type_str ? n->a->type_str : "";
            bool ptr = (strlen(objts) && objts[strlen(objts)-1] == '*');
            return cg_fmt(cg, "%s%s%s", cg_expr(cg, n->a), ptr ? "->" : ".", cg_cident(cg, n->name));
        }
        case AST_INDEX: {
            
            if (n->a && n->a->type_str) {
                char sname[96];
                symbol_t *ssym = cg_struct_of(cg, n->a->type_str, sname, sizeof sname);
                if (ssym) {
                    symbol_t *m = scope_lookup_local(ssym->members, "operator_index");
                    if (m && m->kind == SYM_METHOD) {
                        const char *call = cg_emit_op_call(cg, n->a, ssym, sname, "operator_index", n->b);
                        if (call) return call;
                    }
                }
            }
            const char *arr = cg_expr(cg, n->a);
            const char *idx = cg_expr(cg, n->b);
            if (cg->safe) {
                long sz = array_size_of(n->a->type_str);
                if (sz > 0) return cg_fmt(cg, "%s[salam_idx(%s, %ld)]", arr, idx, sz);
            }
            return cg_fmt(cg, "%s[%s]", arr, idx);
        }
        case AST_LAMBDA:
            return cg_lambda_value(cg, n);
        case AST_STRUCT_LIT: return cg_struct_lit(cg, n);
        case AST_ARRAY_LIT: {
            sb_t b; sb_init(&b); sb_putc(&b, '{');
            { size_t i = 0; for (; i < n->list.len; i++)
                { if (i) sb_puts(&b, ", "); sb_puts(&b, cg_expr(cg, (ast_node_t *)n->list.data[i])); } }
            sb_putc(&b, '}');
            const char *r = arena_strdup(cg->a, sb_cstr(&b)); sb_free(&b); return r;
        }
        default: return "0";
    }
}
