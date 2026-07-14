/*
 * Salam Programming Language (2024-2026)
 *
 * Temporary migration instrumentation: dumps source offsets where a mandatory
 * ':' needs inserting, and where a for-loop ';' separator needs replacing
 * with ','. Removed once the one-time corpus migration is complete.
 */

#include "core/prelude.h"
#include "parser/parser_migrate_dump.h"
#include <stdio.h>
#include <stdlib.h>

static int migrate_enabled(void)
{
    static int cached = -1;
    if (cached == -1) cached = getenv("SALAM_MIGRATE_DUMP") ? 1 : 0;
    return cached;
}

void migrate_dump_colon(const parser_t *p)
{
    if (!migrate_enabled()) return;
    const token_t *t = p_peek(p);
    printf("COLON\t%s\t%u\n", p->file, (unsigned)t->span.begin.index);
}

void migrate_dump_comma(const parser_t *p)
{
    if (!migrate_enabled()) return;
    const token_t *t = p_peek(p);
    if (t->kind == TK_STMT_END && t->span.length == 1)
        printf("COMMA\t%s\t%u\n", p->file, (unsigned)t->span.begin.index);
}
