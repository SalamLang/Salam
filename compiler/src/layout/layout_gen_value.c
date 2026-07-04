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

#include "core/prelude.h"
#include "core/sal_format.h"
#include "layout/layout_internal.h"
#include "i18n/i18n.h"

const char *lfmt(layout_ctx_t *cx, const char *fmt, ...)
{
    va_list ap, ap2;
    va_start(ap, fmt);
    SAL_VA_COPY(ap2, ap);
    int n = sal_vsnprintf(NULL, 0, fmt, ap);
    va_end(ap);
    char *b = (char *)arena_alloc(cx->a, (size_t)n + 1);
    sal_vsnprintf(b, (size_t)n + 1, fmt, ap2);
    va_end(ap2);
    return b;
}

void html_line(layout_ctx_t *cx, const char *fmt, ...)
{
    {
        int i = 0;
        for (; i < cx->indent; i++)
            sb_puts(cx->html, "  ");
    }
    va_list ap, ap2;
    va_start(ap, fmt);
    SAL_VA_COPY(ap2, ap);
    int n = sal_vsnprintf(NULL, 0, fmt, ap);
    va_end(ap);
    char *b = (char *)arena_alloc(cx->a, (size_t)n + 1);
    sal_vsnprintf(b, (size_t)n + 1, fmt, ap2);
    va_end(ap2);
    sb_puts(cx->html, b);
    sb_putc(cx->html, '\n');
}

unsigned djb2(const char *s)
{
    unsigned h = 5381;
    for (; *s; s++)
        h = ((h << 5) + h) + (unsigned char)*s;
    return h;
}

bool starts_with(const char *s, const char *p)
{
    size_t n = strlen(p);
    return strncmp(s, p, n) == 0;
}

const char *hyphenate(layout_ctx_t *cx, const char *s)
{
    char *r = (char *)arena_strdup(cx->a, s);
    {
        char *p = r;
        for (; *p; p++)
            if (*p == '_' || *p == ' ') *p = '-';
    }
    return r;
}

void sb_put_html_escaped(sb_t *b, const char *s)
{
    const char *p = s;
    for (; *p; p++) {
        switch (*p) {
        case '&':
            sb_puts(b, "&amp;");
            break;
        case '<':
            sb_puts(b, "&lt;");
            break;
        case '>':
            sb_puts(b, "&gt;");
            break;
        case '"':
            sb_puts(b, "&quot;");
            break;
        case '\'':
            sb_puts(b, "&#39;");
            break;
        default:
            sb_putc(b, *p);
            break;
        }
    }
}

const char *html_escape(layout_ctx_t *cx, const char *s)
{
    sb_t b;
    sb_init(&b);
    sb_put_html_escaped(&b, s);
    {
        const char *r = arena_strdup(cx->a, sb_cstr(&b));
        sb_free(&b);
        return r;
    }
}

const char *val_str(layout_ctx_t *cx, ast_node_t *v)
{
    if (!v) return "";
    switch (v->kind) {
    case AST_LITERAL:
        switch (v->op) {
        case TK_STRING:
        case TK_TRIPLE_STRING:
            return v->value.as.s ? v->value.as.s : "";
        case TK_INT:
            return lfmt(cx, "%llu", (unsigned long long)v->value.as.i);
        case TK_FLOAT:
            return lfmt(cx, "%g", v->value.as.f);
        case TK_KW_TRUE:
            return "true";
        case TK_KW_FALSE:
            return "false";
        default:
            return v->name ? v->name : "";
        }
    case AST_IDENTIFIER:
        return v->name ? v->name : "";
    case AST_CALL: {
        const char *fn = (v->a && v->a->kind == AST_IDENTIFIER) ? v->a->name : "fn";
        sb_t b;
        sb_init(&b);
        sb_puts(&b, fn);
        sb_putc(&b, '(');
        {
            size_t i = 0;
            for (; i < v->list.len; i++) {
                ast_node_t *arg = (ast_node_t *)v->list.data[i];
                if (i) sb_puts(&b, ", ");
                if (arg->kind == AST_LITERAL &&
                    (arg->op == TK_STRING || arg->op == TK_TRIPLE_STRING)) {
                    sb_putc(&b, '"');
                    sb_puts(&b, val_str(cx, arg));
                    sb_putc(&b, '"');
                } else {
                    sb_puts(&b, val_str(cx, arg));
                }
            }
        }
        sb_putc(&b, ')');
        const char *r = arena_strdup(cx->a, sb_cstr(&b));
        sb_free(&b);
        return r;
    }
    default:
        return "";
    }
}

static const char *class_seq_name(arena_t *a, unsigned n)
{
    char tmp[16];
    int i = 0;
    unsigned x = n + 1;
    while (x > 0) {
        x--;
        tmp[i++] = (char)('a' + (x % 26));
        x /= 26;
    }
    char buf[16];
    int j = 0;
    while (i > 0)
        buf[j++] = tmp[--i];
    buf[j] = '\0';
    return arena_strdup(a, buf);
}

const char *class_for_key(layout_ctx_t *cx, const char *key)
{
    {
        size_t i = 0;
        for (; i < cx->cls_keys.len; i++)
            if (strcmp((const char *)cx->cls_keys.data[i], key) == 0)
                return (const char *)cx->cls_names.data[i];
    }
    const char *name = class_seq_name(cx->a, cx->cls_next++);
    vec_push(cx->a, &cx->cls_keys, CONST_CAST(arena_strdup(cx->a, key)));
    vec_push(cx->a, &cx->cls_names, CONST_CAST(name));
    return name;
}

void emit_rule(layout_ctx_t *cx, const char *rule)
{
    {
        size_t i = 0;
        for (; i < cx->css_seen.len; i++)
            if (strcmp((const char *)cx->css_seen.data[i], rule) == 0) return;
    }
    vec_push(cx->a, &cx->css_seen, CONST_CAST(rule));
    sb_puts(cx->css, rule);
    sb_putc(cx->css, '\n');
}

bool is_bool_attr(const char *n)
{
    return !strcmp(n, "required") || !strcmp(n, "checked") || !strcmp(n, "disabled") ||
           !strcmp(n, "hidden") || !strcmp(n, "selected") || !strcmp(n, "multiple");
}
