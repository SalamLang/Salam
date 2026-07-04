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
#if defined(_WIN32) || defined(__CYGWIN__)
    /* Wrap in double quotes and escape per the CommandLineToArgvW / MSVCRT
       rules the invoked compilers (gcc/clang/tcc) use to split argv:
       backslashes are literal EXCEPT when they run up against a double quote
       (including the closing quote), where each backslash must be doubled and
       a quote must be preceded by a backslash. Without this a path ending in a
       backslash, e.g. C:\dir\, would turn its closing quote into a literal. */
    sb_putc(s, '"');
    { const char *p = arg;
      while (*p) {
        size_t nbs = 0;
        while (*p == '\\') { nbs++; p++; }
        if (*p == '\0') {
            /* trailing backslashes: double them so they don't escape the quote */
            size_t i; for (i = 0; i < nbs * 2; i++) sb_putc(s, '\\');
            break;
        } else if (*p == '"') {
            size_t i; for (i = 0; i < nbs * 2 + 1; i++) sb_putc(s, '\\');
            sb_putc(s, '"');
            p++;
        } else {
            size_t i; for (i = 0; i < nbs; i++) sb_putc(s, '\\');
            sb_putc(s, *p);
            p++;
        }
      }
    }
    sb_putc(s, '"');
#else
    /* POSIX sh: single quotes neutralize everything ($, `, \, ; etc.). A
       literal single quote is emitted as the sequence '\'' (close, escaped
       quote, reopen). This is the only fully injection-safe quoting. */
    sb_putc(s, '\'');
    { const char *p = arg; for (; *p; p++) {
        if (*p == '\'') sb_puts(s, "'\\''");
        else sb_putc(s, *p);
    } }
    sb_putc(s, '\'');
#endif
}

const char *sb_cstr(const sb_t *s)
{
    return s->data ? s->data : "";
}
