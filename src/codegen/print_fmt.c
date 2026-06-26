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

#include "codegen/print_fmt.h"
#include "core/numstr.h"
#include "core/sb.h"
#include "token/token.h"

static bool ts_is_unsigned(const char *ts) { return ts && ts[0] == 'u'; }

static bool ts_is_int(const char *ts)
{
    if (!ts) return true;   
    static const char *k[] = { "i8","i16","i32","i64","int",
                               "u8","u16","u32","u64","uint", 0 };
    { int i = 0; for (; k[i]; i++) if (!strcmp(ts, k[i])) return true; }
    return false;
}

static bool ts_is_float(const char *ts) { return ts && (!strcmp(ts,"f32") || !strcmp(ts,"f64")); }

static pf_kind_t pf_kind_of(const char *ts)
{
    if (!ts || !strcmp(ts, "str")) return ts ? PF_STR : PF_I32;
    if (!strcmp(ts, "bool")) return PF_BOOL;
    if (!strcmp(ts, "char")) return PF_CHAR;
    if (ts_is_float(ts))     return PF_F64;
    if (!strcmp(ts, "i64"))  return PF_I64;
    if (!strcmp(ts, "u64"))  return PF_U64;
    if (ts_is_unsigned(ts))  return PF_U32;
    return PF_I32;
}

const char *pf_spec(pf_kind_t k)
{
    switch (k) {
        case PF_STR:  return "%s";
        case PF_I32:  return "%d";
        case PF_U32:  return "%u";
        case PF_I64:  return "%lld";
        case PF_U64:  return "%llu";
        case PF_F64:  return "%g";
        case PF_BOOL: return "%s";
        case PF_CHAR: return "%c";
        default:      return "";
    }
}

static bool fold_int(ast_node_t *n, long long *out)
{
    if (!n) return false;
    if (n->kind == AST_LITERAL && n->op == TK_INT) { *out = (long long)n->value.as.i; return true; }
    if (n->kind == AST_UNARY && n->op == TK_MINUS) {
        long long a; if (fold_int(n->a, &a)) { *out = -a; return true; }
        return false;
    }
    if (n->kind == AST_BINARY) {
        long long a, b;
        if (!fold_int(n->a, &a) || !fold_int(n->b, &b)) return false;
        switch (n->op) {
            case TK_PLUS:  *out = a + b; return true;
            case TK_MINUS: *out = a - b; return true;
            case TK_STAR:  *out = a * b; return true;
            default: return false;
        }
    }
    return false;
}

static bool fold_flt(ast_node_t *n, double *out)
{
    long long iv;
    if (!n) return false;
    if (n->kind == AST_LITERAL && n->op == TK_FLOAT) { *out = n->value.as.f; return true; }
    if (fold_int(n, &iv)) { *out = (double)iv; return true; }
    if (n->kind == AST_UNARY && n->op == TK_MINUS) {
        double a; if (fold_flt(n->a, &a)) { *out = -a; return true; }
        return false;
    }
    if (n->kind == AST_BINARY) {
        double a, b;
        if (!fold_flt(n->a, &a) || !fold_flt(n->b, &b)) return false;
        switch (n->op) {
            case TK_PLUS:  *out = a + b; return true;
            case TK_MINUS: *out = a - b; return true;
            case TK_STAR:  *out = a * b; return true;
            case TK_SLASH: if (b != 0) { *out = a / b; return true; } return false;
            default: return false;
        }
    }
    return false;
}

static void append_utf8(sb_t *sb, unsigned long cp)
{
    if (cp < 0x80) { sb_putc(sb, (char)cp); return; }
    if (cp < 0x800) {
        sb_putc(sb, (char)(0xC0 | (cp >> 6)));
        sb_putc(sb, (char)(0x80 | (cp & 0x3F)));
        return;
    }
    if (cp < 0x10000) {
        sb_putc(sb, (char)(0xE0 | (cp >> 12)));
        sb_putc(sb, (char)(0x80 | ((cp >> 6) & 0x3F)));
        sb_putc(sb, (char)(0x80 | (cp & 0x3F)));
        return;
    }
    sb_putc(sb, (char)(0xF0 | (cp >> 18)));
    sb_putc(sb, (char)(0x80 | ((cp >> 12) & 0x3F)));
    sb_putc(sb, (char)(0x80 | ((cp >> 6) & 0x3F)));
    sb_putc(sb, (char)(0x80 | (cp & 0x3F)));
}

static bool pf_try_fold(ast_node_t *arg, sb_t *sb)
{
    const char *ts = arg->type_str;
    if (arg->kind == AST_LITERAL) {
        switch (arg->op) {
            case TK_KW_TRUE:  sb_puts(sb, "true");  return true;
            case TK_KW_FALSE: sb_puts(sb, "false"); return true;
            case TK_STRING: case TK_TRIPLE_STRING: case TK_RAW_STRING:
                sb_puts(sb, arg->value.as.s ? arg->value.as.s : ""); return true;
            case TK_UTF8_CHAR:                       
                sb_puts(sb, arg->value.as.s ? arg->value.as.s : ""); return true;
            case TK_CHAR:                            
                append_utf8(sb, (unsigned long)arg->value.as.i); return true;
            default: break;
        }
    }
    if (ts_is_float(ts)) {
        double f; if (fold_flt(arg, &f)) { char b[40]; snprintf(b, sizeof b, "%g", f); sb_puts(sb, b); return true; }
        return false;
    }
    if (ts_is_int(ts)) {
        long long v;
        if (fold_int(arg, &v)) {
            char b[32];
            if (ts_is_unsigned(ts)) sal_u64toa((uint64_t)v, b);
            else                    sal_i64toa((int64_t)v, b);
            sb_puts(sb, b);
            return true;
        }
    }
    return false;
}

static void push_lit(arena_t *a, vec_t *out, sb_t *lit)
{
    if (lit->len == 0) return;
    pf_seg_t *s = (pf_seg_t *)arena_alloc(a, sizeof *s);
    s->kind = PF_LIT;
    s->text = arena_strdup(a, sb_cstr(lit));
    s->expr = NULL;
    vec_push(a, out, s);
    sb_free(lit); sb_init(lit);   
}

void pf_build(arena_t *a, ast_node_t *call, bool nl, vec_t *out)
{
    sb_t lit; sb_init(&lit);
    { size_t i = 0; for (; i < call->list.len; i++) {
        ast_node_t *arg = (ast_node_t *)call->list.data[i];
        if (i) sb_putc(&lit, ' ');                 
        if (pf_try_fold(arg, &lit)) continue;      
        push_lit(a, out, &lit);                    
        pf_seg_t *s = (pf_seg_t *)arena_alloc(a, sizeof *s);
        s->kind = pf_kind_of(arg->type_str);
        s->text = NULL;
        s->expr = arg;
        vec_push(a, out, s);
    } }
    if (nl) sb_putc(&lit, '\n');
    push_lit(a, out, &lit);
    sb_free(&lit);
}
