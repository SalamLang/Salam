#ifndef SALAM_INTERP_INTERP_H
#define SALAM_INTERP_INTERP_H

#include "core/prelude.h"
#include "core/arena.h"
#include "logger/logger.h"
#include "ast/ast.h"
#include "semantic/sema.h"

typedef struct {
    FILE       *out;          /* print / println sink (default stdout)   */
    FILE       *err;          /* printerr sink (default stderr)          */
    const char *input_data;   /* preloaded stdin (web); NULL => real stdin */
    const char *lang;         /* language code for lang() (default "en") */
    int         timeout_ms;   /* watchdog: max run time; 0 => default (5000)  */
} interp_options_t;

int interp_run(arena_t *a, logger_t *log, ast_node_t *program,
               sema_result_t *sem, const char *entry,
               const interp_options_t *opts);

#endif /* SALAM_INTERP_INTERP_H */
