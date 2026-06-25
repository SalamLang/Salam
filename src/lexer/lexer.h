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

#endif /* SALAM_LEXER_LEXER_H */
