#ifndef SALAM_SOURCE_SOURCE_H
#define SALAM_SOURCE_SOURCE_H
#include "core/prelude.h"
#include "core/arena.h"

typedef struct {
    const char *path;
    char       *text;
    size_t      len;
} source_file_t;
source_file_t *source_load(arena_t *a, const char *path);
#endif /* SALAM_SOURCE_SOURCE_H */
