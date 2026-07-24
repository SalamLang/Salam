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

#ifndef SALAM_FMT_FMT_ORDER_H
#define SALAM_FMT_FMT_ORDER_H

#include "core/prelude.h"
#include "core/arena.h"
#include "core/sb.h"
#include "core/vec.h"
#include "logger/logger.h"
#include "langpack/langpack.h"
#include "source/source.h"

bool fmt_reorder_toplevel(arena_t *a, logger_t *log, const langpack_t *pack,
                          const source_file_t *src, const char *file, sb_t *out,
                          vec_t *notes);

#endif /* SALAM_FMT_FMT_ORDER_H */
