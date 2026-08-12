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

#ifndef SALAM_CORE_SAL_PATH_H
#define SALAM_CORE_SAL_PATH_H

#include "core/prelude.h"
#include "core/arena.h"

/*
 * One place that knows what a path separator is.
 *
 * Both '/' and '\\' are separators, on every platform. Windows accepts
 * either in every path it is handed (CreateProcess, the CRT, cmd.exe), and
 * a user on Windows types, pastes and gets back backslashes: %PATH% entries,
 * drag-and-dropped filenames and `cd`-completed directories all arrive that
 * way. Meanwhile everything the compiler builds itself joins with '/', so a
 * path that mixes the two ("C:\gcc\bin/gcc.exe") is the normal case, not an
 * odd one, and anything that scans for only one of the two silently gets the
 * wrong answer on half of them.
 *
 * Treating '\\' as a separator on POSIX too - where it is a legal filename
 * character - is deliberate: the compiler's own path handling then behaves
 * identically on every host, which is what keeps the C and self-hosted
 * backends emitting byte-identical output for the same sources. The cost is
 * that a POSIX file whose name literally contains a backslash cannot be
 * imported; that was already true of every basename/dirname helper this
 * module replaces.
 *
 * '/' is the canonical form. Everything produced here is slash-only, so
 * paths used as dedup keys ("did I already load this module?") compare equal
 * no matter which spelling they came in as.
 */

bool sal_path_is_sep(char c);

/* True for "/x", "\\x", "c:/x", "c:\\x". */
bool sal_path_is_absolute(const char *p);

/* In place: '\\' -> '/'. Returns p. */
char *sal_path_to_slash(char *p);

/*
 * In place: to_slash, then strip leading "./", collapse "/./", "//" and
 * "seg/../". Lexical only - the filesystem is never consulted, so a
 * symlinked "seg" is not resolved. Returns p.
 *
 * The ".." collapse keeps import dedup keys canonical: the same file
 * imported as "token.salam" from the root and as "../token.salam" from a
 * sibling directory must resolve to ONE module, or its globals get linked
 * twice ('defined twice' at link time).
 */
char *sal_path_normalize(char *p);

/* In place: drop trailing separators, keeping a root ("/", "c:/") intact. */
char *sal_path_trim_sep(char *p);

/* The component after the last separator; points into p. */
const char *sal_path_base(const char *p);

/* Basename with its last extension removed. */
size_t sal_path_stem_buf(const char *p, char *out, size_t n);
const char *sal_path_stem(arena_t *a, const char *p);

/* Everything before the last separator; "" when there is none. */
size_t sal_path_dir_buf(const char *p, char *out, size_t n);
const char *sal_path_dir(arena_t *a, const char *p);

/*
 * dir + "/" + name, with exactly one separator between them and no trailing
 * one, slash-canonical throughout. An empty dir yields name alone. Returns
 * the length that was needed, so a caller can detect truncation.
 */
size_t sal_path_join(char *out, size_t n, const char *dir, const char *name);
const char *sal_path_joina(arena_t *a, const char *dir, const char *name);

#endif /* SALAM_CORE_SAL_PATH_H */
