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
#include "core/sb.h"
#include "core/sal_format.h"

void sb_init(sb_t *s)
{
    s->data = NULL;
    s->len = 0;
    s->cap = 0;
}

void sb_free(sb_t *s)
{
    free(s->data);
    s->data = NULL;
    s->len = 0;
    s->cap = 0;
}

static void sb_ensure(sb_t *s, size_t add)
{
    size_t need = salam_size_add(salam_size_add(s->len, add), 1);
    if (need <= s->cap) return;
    size_t ncap = salam_grow_cap(s->cap, need, 64);
    char *nd = (char *)realloc(s->data, ncap);
    if (!nd) abort();
    s->data = nd;
    s->cap = ncap;
}

void sb_putc(sb_t *s, char c)
{
    sb_ensure(s, 1);
    s->data[s->len++] = c;
    s->data[s->len] = '\0';
}

void sb_puts(sb_t *s, const char *str)
{
    size_t n = strlen(str);
    sb_ensure(s, n);
    memcpy(s->data + s->len, str, n);
    s->len += n;
    s->data[s->len] = '\0';
}

void sb_printf(sb_t *s, const char *fmt, ...)
{
    va_list ap, ap2;
    va_start(ap, fmt);
    SAL_VA_COPY(ap2, ap);
    int n = sal_vsnprintf(NULL, 0, fmt, ap);
    va_end(ap);
    if (n > 0) {
        sb_ensure(s, (size_t)n);
        sal_vsnprintf(s->data + s->len, (size_t)n + 1, fmt, ap2);
        s->len += (size_t)n;
    }
    va_end(ap2);
}

void sb_put_shell_arg(sb_t *s, const char *arg)
{
    sb_putc(s, '"');
    { const char *p = arg; for (; *p; p++) {
        if (*p == '"') sb_putc(s, '\\');
        sb_putc(s, *p);
    } }
    sb_putc(s, '"');
}

const char *sb_cstr(const sb_t *s)
{
    return s->data ? s->data : "";
}
