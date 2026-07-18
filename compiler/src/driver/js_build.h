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

#ifndef SALAM_DRIVER_JS_BUILD_H
#define SALAM_DRIVER_JS_BUILD_H

#include "core/arena.h"
#include "core/vec.h"
#include "cli/options.h"
#include "logger/logger.h"

int driver_js(options_t *opt);

/* pkg_cache is an optional caller-owned, vec_init()'d cache (path ->
 * symbol_t* pairs) that lets this bundle's per-module sema passes reuse
 * already-loaded stdlib packages instead of re-parsing them; pass NULL to
 * always load fresh. If non-NULL, it must live in `arena` alongside any
 * other sema_run_cached() calls sharing it (e.g. a prior page-level pass). */
const char *js_build_bundle(arena_t *arena, logger_t *log, options_t *opt,
                            const char **entries, int nentries, const char **module_out,
                            int *rc_out, vec_t *pkg_cache);

#endif /* SALAM_DRIVER_JS_BUILD_H */
