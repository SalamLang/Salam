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

#ifndef SALAM_PREPROC_PREPROC_H
#define SALAM_PREPROC_PREPROC_H

#include "core/prelude.h"
#include "core/arena.h"
#include "logger/logger.h"
#include "source/source.h"

char *preproc_run(arena_t *a, logger_t *log, const char *text, const char *const *defines,
                  int ndefines);

source_file_t *preproc_source(arena_t *a, logger_t *log, const source_file_t *src,
                              const char *const *defines, int ndefines);

void preproc_set_target(const char *triple);

#endif /* SALAM_PREPROC_PREPROC_H */
