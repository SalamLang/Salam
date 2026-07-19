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

#ifndef SALAM_SEMANTIC_DCE_H
#define SALAM_SEMANTIC_DCE_H

#include "core/arena.h"
#include <stdbool.h>

void dce_reset(arena_t *a);
void dce_enable(void);
void dce_disable(void);
bool dce_enabled(void);

void dce_note_call(const char *caller_pkg, const char *caller_fn, const char *callee_pkg,
                   const char *callee_fn);

void dce_mark_root(const char *pkg, const char *fn);

void dce_finish(void);

bool dce_reachable(const char *pkg, const char *fn);

#endif /* SALAM_SEMANTIC_DCE_H */
