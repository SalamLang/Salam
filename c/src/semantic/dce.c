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
#include "core/sal_format.h"
#include "core/vec.h"
#include "semantic/dce.h"

typedef struct {
    const char *from;
    const char *to;
} dce_edge_t;

static arena_t *g_arena = NULL;
static bool g_enabled = false;
static vec_t g_edges;     /* dce_edge_t* */
static vec_t g_roots;     /* const char* keys */
static vec_t g_reachable; /* const char* keys */

static const char *dce_key(const char *pkg, const char *fn)
{
    if (!pkg) pkg = "";
    if (!fn) fn = "";
    size_t n = strlen(pkg) + strlen(fn) + 3;
    char *buf = (char *)arena_alloc(g_arena, n);
    sal_snprintf(buf, n, "%s::%s", pkg, fn);
    return buf;
}

static bool key_in(const vec_t *v, const char *key)
{
    size_t i = 0;
    for (; i < v->len; i++)
        if (strcmp((const char *)v->data[i], key) == 0) return true;
    return false;
}

void dce_reset(arena_t *a)
{
    g_arena = a;
    vec_init(&g_edges);
    vec_init(&g_roots);
    vec_init(&g_reachable);
    g_enabled = false;
}

void dce_enable(void)
{
    g_enabled = true;
}

void dce_disable(void)
{
    g_enabled = false;
}

bool dce_enabled(void)
{
    return g_enabled;
}

void dce_note_call(const char *caller_pkg, const char *caller_fn, const char *callee_pkg,
                   const char *callee_fn)
{
    if (!g_enabled || !g_arena) return;
    if (!caller_fn) {
        dce_mark_root(callee_pkg, callee_fn);
        return;
    }
    const char *from = dce_key(caller_pkg, caller_fn);
    const char *to = dce_key(callee_pkg, callee_fn);
    dce_edge_t *e = (dce_edge_t *)arena_alloc(g_arena, sizeof(*e));
    e->from = from;
    e->to = to;
    vec_push(g_arena, &g_edges, e);
}

void dce_mark_root(const char *pkg, const char *fn)
{
    if (!g_enabled || !g_arena) return;
    const char *k = dce_key(pkg, fn);
    if (!key_in(&g_roots, k)) vec_push(g_arena, &g_roots, (void *)k);
}

void dce_finish(void)
{
    if (!g_arena) return;
    vec_init(&g_reachable);
    vec_t worklist;
    vec_init(&worklist);
    {
        size_t i = 0;
        for (; i < g_roots.len; i++) {
            const char *k = (const char *)g_roots.data[i];
            if (key_in(&g_reachable, k)) continue;
            vec_push(g_arena, &g_reachable, (void *)k);
            vec_push(g_arena, &worklist, (void *)k);
        }
    }
    while (worklist.len) {
        const char *cur = (const char *)worklist.data[worklist.len - 1];
        worklist.len--;
        size_t j = 0;
        for (; j < g_edges.len; j++) {
            dce_edge_t *e = (dce_edge_t *)g_edges.data[j];
            if (strcmp(e->from, cur) != 0) continue;
            if (key_in(&g_reachable, e->to)) continue;
            vec_push(g_arena, &g_reachable, (void *)e->to);
            vec_push(g_arena, &worklist, (void *)e->to);
        }
    }
}

bool dce_reachable(const char *pkg, const char *fn)
{
    if (!g_arena) return true;
    const char *k = dce_key(pkg, fn);
    return key_in(&g_reachable, k);
}
