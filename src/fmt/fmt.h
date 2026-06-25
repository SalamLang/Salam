#ifndef SALAM_FMT_FMT_H
#define SALAM_FMT_FMT_H

#include "core/prelude.h"
#include "core/arena.h"
#include "core/sb.h"
#include "logger/logger.h"
#include "langpack/langpack.h"
#include "source/source.h"
#include "token/token.h"

void fmt_tokens(const token_stream_t *toks, sb_t *out);

bool fmt_source(arena_t *a, logger_t *log, const langpack_t *pack,
                const source_file_t *src, sb_t *out);

#endif /* SALAM_FMT_FMT_H */
