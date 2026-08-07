/*
 * Salam Programming Language (2024-2026)
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

#ifndef SALAM_DRIVER_BUILD_H
#define SALAM_DRIVER_BUILD_H

#include "cli/options.h"
#include "core/arena.h"
#include "logger/logger.h"
#include "langpack/langpack.h"

/* The fixed project entry filename. When `salam build`/`run`/`exec`/
 * `web`/`layout build` are given a directory (or no input at all),
 * <dir>/salam.salam - if present - IS the project's entry file; no
 * scanning or guessing happens. Single-file invocations
 * (`salam build test.salam`) are unaffected. */
#define SALAM_PROJECT_FILE "salam.salam"

int driver_build(options_t *opt);

bool driver_path_is_dir(const char *p);

/* <dir>/salam.salam when it exists (arena-owned path, bare filename for
 * dir="."), else NULL. */
const char *driver_project_entry_file(arena_t *a, const char *dir);

/* Resolve the program entry file of `dir`: salam.salam when present,
 * else the single top-level .salam file defining the langpack's entry
 * function. Logs its own error and returns NULL on failure. */
const char *driver_resolve_dir_entry(arena_t *arena, logger_t *log,
                                     langpack_t *pack, const char *dir);

/* Front-end (layout DSL) counterpart, for projects that have no entry
 * function: salam.salam when present, else the .salam files containing
 * a `layout` block. `single` demands exactly one page (salam web);
 * otherwise every page found is returned (salam layout build). Fills
 * out[] (up to max_out) and returns the count; 0 = error already
 * logged. */
int driver_resolve_dir_layout(arena_t *arena, logger_t *log, langpack_t *pack,
                              const char *dir, const char **out, int max_out,
                              bool single);

/* Stem for a default output name: normally the entry file's module
 * name, but the fixed project entry file salam.salam would make every
 * project's default executable "salam(.exe)" - which cmd.exe would then
 * run from that directory instead of the real compiler - so a project
 * executable is named after its directory instead. */
const char *driver_output_stem(arena_t *a, const char *path);

/* Stem for a default web/layout output name: the page file's module
 * name, except the fixed project entry file salam.salam, whose page is
 * the site's landing page - index.html, the name every web server
 * (including `salam serve`) treats as the directory's entry. */
const char *driver_page_stem(arena_t *a, const char *path);

#endif /* SALAM_DRIVER_BUILD_H */
