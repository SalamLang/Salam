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

#ifndef SALAM_LEXER_LEXER_H
#define SALAM_LEXER_LEXER_H

#include "core/prelude.h"
#include "core/arena.h"
#include "logger/logger.h"
#include "langpack/langpack.h"
#include "source/source.h"
#include "token/token.h"

bool lexer_run(arena_t *a, logger_t *log, const langpack_t *pack,
               const source_file_t *src, token_stream_t **out);

bool lexer_run_ex(arena_t *a, logger_t *log, const langpack_t *pack,
                  const source_file_t *src, bool keep_comments,
                  token_stream_t **out);

/*
 * Per-file language detection. Returns the langpack matching a source file's
 * own language (explicit marker comment, else keyword-count inference), falling
 * back to `fallback` when nothing is recognised. See lexer_detect.c.
 */
const char *langpack_marker_code(const char *text, size_t len);
const langpack_t *langpack_detect(arena_t *a, const source_file_t *src,
                                  const langpack_t *fallback);

#endif /* SALAM_LEXER_LEXER_H */
