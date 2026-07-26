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

#include "core/prelude.h"
#include "core/vec.h"

void vec_init(vec_t *v)
{
    v->data = NULL;
    v->len = 0;
    v->cap = 0;
}

void vec_push(arena_t *a, vec_t *v, void *item)
{
    if (v->len == v->cap) {
        size_t ncap = salam_grow_cap(v->cap, v->len + 1, 8);
        void **ndata = (void **)arena_alloc(a, salam_size_mul(ncap, sizeof(void *)));
        if (v->data && v->len) {
            memcpy(ndata, v->data, salam_size_mul(v->len, sizeof(void *)));
        }
        v->data = ndata;
        v->cap = ncap;
    }
    v->data[v->len++] = item;
}
