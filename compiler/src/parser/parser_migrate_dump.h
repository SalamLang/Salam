/*
 * Salam Programming Language (2024-2026)
 *
 * Temporary migration instrumentation: dumps source offsets where a mandatory
 * ':' needs inserting, and where a for-loop ';' separator needs replacing
 * with ','. Removed once the one-time corpus migration is complete.
 */

#ifndef SALAM_PARSER_MIGRATE_DUMP_H
#define SALAM_PARSER_MIGRATE_DUMP_H

#include "parser/parser_internal.h"

void migrate_dump_colon(const parser_t *p);
void migrate_dump_comma(const parser_t *p);

#endif /* SALAM_PARSER_MIGRATE_DUMP_H */
