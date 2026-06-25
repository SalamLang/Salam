#ifndef SALAM_CORE_VEC_H

#define SALAM_CORE_VEC_H

#include "core/prelude.h"
#include "core/arena.h"

typedef struct {
    void **data;
    size_t len;
    size_t cap;
} vec_t;

void vec_init(vec_t *v);

void vec_push(arena_t *a, vec_t *v, void *item);

#define VEC_AT(v, T, i) ((T *)((v)->data[(i)]))

#endif /* SALAM_CORE_VEC_H */
