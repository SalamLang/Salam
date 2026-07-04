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

#include "codegen/codegen_internal.h"
#include "core/sal_format.h"

const char *cg_fmt(cg_t *cg, const char *fmt, ...)
{
    va_list ap, ap2;
    va_start(ap, fmt);
    SAL_VA_COPY(ap2, ap);
    int n = sal_vsnprintf(NULL, 0, fmt, ap);
    va_end(ap);
    char *buf = (char *)arena_alloc(cg->a, (size_t)n + 1);
    sal_vsnprintf(buf, (size_t)n + 1, fmt, ap2);
    va_end(ap2);
    return buf;
}

void cg_indent(cg_t *cg)
{
    {
        int i = 0;
        for (; i < cg->indent; i++)
            sb_puts(cg->c, "    ");
    }
}

void cg_source_line(cg_t *cg, const src_span_t *span)
{
    if (!cg->debug_info || !span || span->begin.line == 0) return;
    if (!cg->src_path || !cg->src_path[0]) return;
    char escaped[1024];
    size_t ei = 0;
    {
        const char *p = cg->src_path;
        for (; *p && ei < sizeof(escaped) - 2; p++) {
            if (*p == '\\') escaped[ei++] = '\\';
            escaped[ei++] = *p;
        }
    }
    escaped[ei] = '\0';
    sb_puts(cg->c, cg_fmt(cg, "#line %u \"%s\"\n", span->begin.line, escaped));
}

void cg_line(cg_t *cg, const char *fmt, ...)
{
    cg_indent(cg);
    va_list ap, ap2;
    va_start(ap, fmt);
    SAL_VA_COPY(ap2, ap);
    int n = sal_vsnprintf(NULL, 0, fmt, ap);
    va_end(ap);
    char *buf = (char *)arena_alloc(cg->a, (size_t)n + 1);
    sal_vsnprintf(buf, (size_t)n + 1, fmt, ap2);
    va_end(ap2);
    sb_puts(cg->c, buf);
    sb_putc(cg->c, '\n');
}

bool local_known(cg_t *cg, const char *name)
{
    {
        size_t i = 0;
        for (; i < cg->locals.len; i++)
            if (strcmp((const char *)cg->locals.data[i], name) == 0) return true;
    }
    return false;
}

void local_add(cg_t *cg, const char *name)
{
    vec_push(cg->a, &cg->locals, CONST_CAST(name));
}
