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

#include "llvm/codegen_llvm_internal.h"
#include "core/sal_format.h"

const char *ll_fmt(ll_t *ll, const char *fmt, ...)
{
    va_list ap, ap2;
    va_start(ap, fmt); SAL_VA_COPY(ap2, ap);
    int n = sal_vsnprintf(NULL, 0, fmt, ap); va_end(ap);
    char *buf = (char *)arena_alloc(ll->a, (size_t)n + 1);
    sal_vsnprintf(buf, (size_t)n + 1, fmt, ap2); va_end(ap2);
    return buf;
}

static void ll_attach_dbg(ll_t *ll)
{
    if (ll->debug && ll->cur_dbg) { sb_puts(ll->b, ", !dbg "); sb_puts(ll->b, ll->cur_dbg); }
}

void ll_emit(ll_t *ll, const char *fmt, ...)
{
    if (ll->term) return;
    sb_puts(ll->b, "  ");
    va_list ap, ap2;
    va_start(ap, fmt); SAL_VA_COPY(ap2, ap);
    int n = sal_vsnprintf(NULL, 0, fmt, ap); va_end(ap);
    char *buf = (char *)arena_alloc(ll->a, (size_t)n + 1);
    sal_vsnprintf(buf, (size_t)n + 1, fmt, ap2); va_end(ap2);
    sb_puts(ll->b, buf); ll_attach_dbg(ll); sb_putc(ll->b, '\n');
}

void ll_emit_label(ll_t *ll, const char *label)
{
    sb_puts(ll->b, label); sb_puts(ll->b, ":\n");
    ll->term = false;
}

void ll_emit_term(ll_t *ll, const char *fmt, ...)
{
    if (ll->term) return;
    sb_puts(ll->b, "  ");
    va_list ap, ap2;
    va_start(ap, fmt); SAL_VA_COPY(ap2, ap);
    int n = sal_vsnprintf(NULL, 0, fmt, ap); va_end(ap);
    char *buf = (char *)arena_alloc(ll->a, (size_t)n + 1);
    sal_vsnprintf(buf, (size_t)n + 1, fmt, ap2); va_end(ap2);
    sb_puts(ll->b, buf); ll_attach_dbg(ll); sb_putc(ll->b, '\n');
    ll->term = true;
}

const char *ll_new_tmp(ll_t *ll)  { return ll_fmt(ll, "%%t%d", ll->tmp++); }

const char *ll_new_lbl(ll_t *ll, const char *tag) { return ll_fmt(ll, "L%d_%s", ll->lbl++, tag); }

void ll_error(ll_t *ll, const ast_node_t *n, const char *fmt, ...)
{
    ll->ok = false;
    char buf[256];
    va_list ap; va_start(ap, fmt);
    sal_vsnprintf(buf, sizeof buf, fmt, ap); va_end(ap);
    const char *msg = (n && n->span.begin.line)
        ? ll_fmt(ll, "LLVM backend (line %u): %s", n->span.begin.line, buf)
        : ll_fmt(ll, "LLVM backend: %s", buf);
    if (!ll->first_error) ll->first_error = msg;   
    LOG_E(ll->log, PH_CODEGEN, "%s", msg);
}

static const char *ll_escape(ll_t *ll, const char *s, size_t len, size_t *arr_len)
{
    sb_t b; sb_init(&b);
    { size_t i = 0; for (; i < len; i++) {
        unsigned char c = (unsigned char)s[i];
        if (c == '"' || c == '\\' || c < 0x20 || c > 0x7e) {
            char hx[8]; snprintf(hx, sizeof hx, "\\%02X", c); sb_puts(&b, hx);
        } else {
            sb_putc(&b, (char)c);
        }
    } }
    sb_puts(&b, "\\00");
    *arr_len = len + 1;
    const char *r = arena_strdup(ll->a, sb_cstr(&b)); sb_free(&b); return r;
}

const char *ll_strconst(ll_t *ll, const char *s)
{
    size_t len = strlen(s);
    { size_t i = 0; for (; i < ll->strings.len; i++) {
        lstr_t *e = (lstr_t *)ll->strings.data[i];
        if (e->len == len && memcmp(e->bytes, s, len) == 0) return e->gref;
    } }
    size_t arr;
    const char *esc = ll_escape(ll, s, len, &arr);
    const char *gref = ll_fmt(ll, "@.str.%zu", ll->strings.len);
    sb_puts(ll->g, ll_fmt(ll, "%s = private unnamed_addr constant [%zu x i8] c\"%s\"\n",
                          gref, arr, esc));
    lstr_t *e = (lstr_t *)arena_alloc(ll->a, sizeof *e);
    e->bytes = arena_strndup(ll->a, s, len); e->len = len; e->gref = gref;
    vec_push(ll->a, &ll->strings, e);
    return gref;
}

void ll_local_add(ll_t *ll, const char *name, const char *ptr, const char *ts)
{
    lvar_t *v = (lvar_t *)arena_alloc(ll->a, sizeof *v);
    v->name = name; v->ptr = ptr; v->ts = ts;
    vec_push(ll->a, &ll->locals, v);
}

lvar_t *ll_local_find(ll_t *ll, const char *name)
{
    { size_t i = ll->locals.len; for (; i > 0; i--) {
        lvar_t *v = (lvar_t *)ll->locals.data[i - 1];
        if (!strcmp(v->name, name)) return v;
    } }
    return NULL;
}

lvar_t *ll_global_find(ll_t *ll, const char *name)
{
    { size_t i = 0; for (; i < ll->globals.len; i++) {
        lvar_t *v = (lvar_t *)ll->globals.data[i];
        if (!strcmp(v->name, name)) return v;
    } }
    return NULL;
}
