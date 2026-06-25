#ifndef SALAM_CORE_ARENA_H
#define SALAM_CORE_ARENA_H

#include "core/prelude.h"

typedef struct arena arena_t;

arena_t *arena_new(size_t block_size);

void *arena_alloc(arena_t *a, size_t n);

char *arena_strndup(arena_t *a, const char *s, size_t n);

char *arena_strdup(arena_t *a, const char *s);

void arena_free(arena_t *a);

#endif /* SALAM_CORE_ARENA_H */
