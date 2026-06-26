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
#include "core/arena.h"

#define ARENA_ALIGN 16u

typedef struct arena_block {
    struct arena_block *next;
    size_t used;
    size_t cap;
    char  *data;
} arena_block;

struct arena {
    arena_block *head;     
    size_t       block_size;
};

static size_t align_up(size_t n)
{
    
    return salam_size_add(n, ARENA_ALIGN - 1) & ~(size_t)(ARENA_ALIGN - 1);
}

arena_t *arena_new(size_t block_size)
{
    arena_t *a = (arena_t *)malloc(sizeof(*a));
    if (!a) abort();
    a->head = NULL;
    a->block_size = block_size ? block_size : (size_t)(64 * 1024);
    return a;
}

static arena_block *arena_add_block(arena_t *a, size_t need)
{
    size_t cap = need > a->block_size ? need : a->block_size;
    size_t total_size = salam_size_add(sizeof(arena_block), cap);
    arena_block *b = (arena_block *)malloc(total_size);
    if (!b) abort();
    b->data = (char *)(b + 1);
    b->cap = cap;
    b->used = 0;
    b->next = a->head;
    a->head = b;
    return b;
}

void *arena_alloc(arena_t *a, size_t n)
{
    n = align_up(n ? n : 1);
    if (!a->head || salam_size_add(a->head->used, n) > a->head->cap) {
        arena_add_block(a, n);
    }
    void *p = a->head->data + a->head->used;
    a->head->used += n;
    return p;
}

char *arena_strndup(arena_t *a, const char *s, size_t n)
{
    char *p = (char *)arena_alloc(a, salam_size_add(n, 1));
    if (n) memcpy(p, s, n);
    p[n] = '\0';
    return p;
}

char *arena_strdup(arena_t *a, const char *s)
{
    return arena_strndup(a, s, strlen(s));
}

void arena_free(arena_t *a)
{
    if (!a) return;
    arena_block *b = a->head;
    while (b) {
        arena_block *next = b->next;
        free(b);
        b = next;
    }
    free(a);
}
