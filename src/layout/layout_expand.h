#ifndef SALAM_LAYOUT_LAYOUT_EXPAND_H
#define SALAM_LAYOUT_LAYOUT_EXPAND_H

#include "core/prelude.h"
#include "core/arena.h"
#include "logger/logger.h"
#include "langpack/langpack.h"
#include "ast/ast.h"

size_t layout_expand(arena_t *a, logger_t *log, const langpack_t *pack,
                     ast_node_t *program, const char *base_dir);

#endif /* SALAM_DRIVER_LAYOUT_EXPAND_H */
