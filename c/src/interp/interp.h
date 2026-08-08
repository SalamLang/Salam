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

#ifndef SALAM_INTERP_INTERP_H
#define SALAM_INTERP_INTERP_H

#include "core/prelude.h"
#include "core/arena.h"
#include "logger/logger.h"
#include "ast/ast.h"
#include "semantic/sema.h"

/* Where a program's stdout/stderr goes while it is still running.
 *
 * `out`/`err` are the ordinary destination and are all a native run needs: the
 * driver hands over real stdio and the program's writes land there directly.
 * The playground cannot use that. It runs the interpreter inside a Web Worker
 * and has to show each frame of an animated program as it is produced, so it
 * installs a `sink` instead: every write is handed to the callback the moment
 * it happens rather than being accumulated and read back after the run ends.
 * When `sink` is non-NULL it replaces `out`/`err` entirely; `to_err` tells the
 * two streams apart so a caller can still route diagnostics separately. */
typedef void (*interp_sink_fn)(void *ctx, bool to_err, const char *buf, size_t len);

typedef struct {
    FILE *out;
    FILE *err;
    const char *input_data;
    const char *lang;
    int timeout_ms; /* >0 = that many ms, 0 = built-in default, <0 = no deadline */
    interp_sink_fn sink;
    void *sink_ctx;
} interp_options_t;

int interp_run(arena_t *a, logger_t *log, ast_node_t *program, sema_result_t *sem,
               const char *entry, const interp_options_t *opts);

#endif /* SALAM_INTERP_INTERP_H */
