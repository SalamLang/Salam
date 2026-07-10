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
#include "condcomp/condcomp.h"
#include "core/vec.h"

struct cc_table_t {
    vec_t entries;
};

static void cc_push_bool(arena_t *a, cc_table_t *t, const char *name, bool val)
{
    cc_const_t *c = (cc_const_t *)arena_alloc(a, sizeof(*c));
    c->name = name;
    c->kind = CC_BOOL;
    c->bval = val;
    c->sval = NULL;
    vec_push(a, &t->entries, c);
}

static void cc_push_str(arena_t *a, cc_table_t *t, const char *name, const char *val)
{
    cc_const_t *c = (cc_const_t *)arena_alloc(a, sizeof(*c));
    c->name = name;
    c->kind = CC_STR;
    c->bval = false;
    c->sval = val;
    vec_push(a, &t->entries, c);
}

static void cc_push_host_defs(arena_t *a, cc_table_t *t)
{
#if defined(_WIN32)
    cc_push_bool(a, t, "SALAM_OS_WINDOWS", true);
    cc_push_str(a, t, "SALAM_OS", "windows");
#  if defined(_WIN64)
    cc_push_bool(a, t, "SALAM_OS_WIN64", true);
#  else
    cc_push_bool(a, t, "SALAM_OS_WIN32", true);
#  endif
#elif defined(__EMSCRIPTEN__)
    cc_push_bool(a, t, "SALAM_OS_WASM", true);
    cc_push_bool(a, t, "SALAM_OS_UNIX", true);
    cc_push_str(a, t, "SALAM_OS", "wasm");
#elif defined(__ANDROID__)
    cc_push_bool(a, t, "SALAM_OS_ANDROID", true);
    cc_push_bool(a, t, "SALAM_OS_LINUX", true);
    cc_push_bool(a, t, "SALAM_OS_UNIX", true);
    cc_push_str(a, t, "SALAM_OS", "android");
#elif defined(__linux__)
    cc_push_bool(a, t, "SALAM_OS_LINUX", true);
    cc_push_bool(a, t, "SALAM_OS_UNIX", true);
    cc_push_str(a, t, "SALAM_OS", "linux");
#elif defined(__APPLE__)
    cc_push_bool(a, t, "SALAM_OS_MAC", true);
    cc_push_bool(a, t, "SALAM_OS_BSD", true);
    cc_push_bool(a, t, "SALAM_OS_UNIX", true);
    cc_push_str(a, t, "SALAM_OS", "mac");
#elif defined(__FreeBSD__)
    cc_push_bool(a, t, "SALAM_OS_FREEBSD", true);
    cc_push_bool(a, t, "SALAM_OS_BSD", true);
    cc_push_bool(a, t, "SALAM_OS_UNIX", true);
    cc_push_str(a, t, "SALAM_OS", "freebsd");
#elif defined(__OpenBSD__)
    cc_push_bool(a, t, "SALAM_OS_OPENBSD", true);
    cc_push_bool(a, t, "SALAM_OS_BSD", true);
    cc_push_bool(a, t, "SALAM_OS_UNIX", true);
    cc_push_str(a, t, "SALAM_OS", "openbsd");
#elif defined(__NetBSD__)
    cc_push_bool(a, t, "SALAM_OS_NETBSD", true);
    cc_push_bool(a, t, "SALAM_OS_BSD", true);
    cc_push_bool(a, t, "SALAM_OS_UNIX", true);
    cc_push_str(a, t, "SALAM_OS", "netbsd");
#else
    cc_push_str(a, t, "SALAM_OS", "unknown");
#endif

#if defined(__x86_64__) || defined(_M_X64)
    cc_push_bool(a, t, "SALAM_ARCH_X64", true);
    cc_push_str(a, t, "SALAM_ARCH", "x64");
#elif defined(__i386__) || defined(_M_IX86)
    cc_push_bool(a, t, "SALAM_ARCH_X86", true);
    cc_push_str(a, t, "SALAM_ARCH", "x86");
#elif defined(__aarch64__) || defined(_M_ARM64)
    cc_push_bool(a, t, "SALAM_ARCH_ARM64", true);
    cc_push_str(a, t, "SALAM_ARCH", "arm64");
#elif defined(__arm__) || defined(_M_ARM)
    cc_push_bool(a, t, "SALAM_ARCH_ARM", true);
    cc_push_str(a, t, "SALAM_ARCH", "arm");
#elif defined(__wasm__) || defined(__EMSCRIPTEN__)
    cc_push_bool(a, t, "SALAM_ARCH_WASM", true);
    cc_push_str(a, t, "SALAM_ARCH", "wasm");
#endif
}

static bool trip_has(const char *hay, const char *needle)
{
    return strstr(hay, needle) != NULL;
}

static void cc_push_target_defs(arena_t *a, cc_table_t *t, const char *trip)
{
    bool bits64 = false;
    if (trip_has(trip, "x86_64") || trip_has(trip, "amd64")) {
        bits64 = true;
        cc_push_bool(a, t, "SALAM_ARCH_X64", true);
        cc_push_str(a, t, "SALAM_ARCH", "x64");
    } else if (trip_has(trip, "aarch64") || trip_has(trip, "arm64")) {
        bits64 = true;
        cc_push_bool(a, t, "SALAM_ARCH_ARM64", true);
        cc_push_str(a, t, "SALAM_ARCH", "arm64");
    } else if (trip_has(trip, "i386") || trip_has(trip, "i486") ||
               trip_has(trip, "i586") || trip_has(trip, "i686") ||
               trip_has(trip, "x86")) {
        cc_push_bool(a, t, "SALAM_ARCH_X86", true);
        cc_push_str(a, t, "SALAM_ARCH", "x86");
    } else if (trip_has(trip, "wasm64")) {
        bits64 = true;
        cc_push_bool(a, t, "SALAM_ARCH_WASM", true);
        cc_push_str(a, t, "SALAM_ARCH", "wasm");
    } else if (trip_has(trip, "wasm") || trip_has(trip, "wasm32")) {
        cc_push_bool(a, t, "SALAM_ARCH_WASM", true);
        cc_push_str(a, t, "SALAM_ARCH", "wasm");
    } else if (trip_has(trip, "arm")) {
        cc_push_bool(a, t, "SALAM_ARCH_ARM", true);
        cc_push_str(a, t, "SALAM_ARCH", "arm");
    }

    if (trip_has(trip, "windows") || trip_has(trip, "mingw") || trip_has(trip, "win32")) {
        cc_push_bool(a, t, "SALAM_OS_WINDOWS", true);
        cc_push_str(a, t, "SALAM_OS", "windows");
        cc_push_bool(a, t, bits64 ? "SALAM_OS_WIN64" : "SALAM_OS_WIN32", true);
    } else if (trip_has(trip, "darwin") || trip_has(trip, "macos") ||
               trip_has(trip, "macosx") || trip_has(trip, "apple") ||
               trip_has(trip, "ios")) {
        cc_push_bool(a, t, "SALAM_OS_MAC", true);
        cc_push_bool(a, t, "SALAM_OS_BSD", true);
        cc_push_bool(a, t, "SALAM_OS_UNIX", true);
        cc_push_str(a, t, "SALAM_OS", "mac");
    } else if (trip_has(trip, "android")) {
        cc_push_bool(a, t, "SALAM_OS_ANDROID", true);
        cc_push_bool(a, t, "SALAM_OS_LINUX", true);
        cc_push_bool(a, t, "SALAM_OS_UNIX", true);
        cc_push_str(a, t, "SALAM_OS", "android");
    } else if (trip_has(trip, "linux")) {
        cc_push_bool(a, t, "SALAM_OS_LINUX", true);
        cc_push_bool(a, t, "SALAM_OS_UNIX", true);
        cc_push_str(a, t, "SALAM_OS", "linux");
    } else if (trip_has(trip, "freebsd")) {
        cc_push_bool(a, t, "SALAM_OS_FREEBSD", true);
        cc_push_bool(a, t, "SALAM_OS_BSD", true);
        cc_push_bool(a, t, "SALAM_OS_UNIX", true);
        cc_push_str(a, t, "SALAM_OS", "freebsd");
    } else if (trip_has(trip, "openbsd")) {
        cc_push_bool(a, t, "SALAM_OS_OPENBSD", true);
        cc_push_bool(a, t, "SALAM_OS_BSD", true);
        cc_push_bool(a, t, "SALAM_OS_UNIX", true);
        cc_push_str(a, t, "SALAM_OS", "openbsd");
    } else if (trip_has(trip, "netbsd")) {
        cc_push_bool(a, t, "SALAM_OS_NETBSD", true);
        cc_push_bool(a, t, "SALAM_OS_BSD", true);
        cc_push_bool(a, t, "SALAM_OS_UNIX", true);
        cc_push_str(a, t, "SALAM_OS", "netbsd");
    } else if (trip_has(trip, "wasi") || trip_has(trip, "emscripten") ||
               trip_has(trip, "wasm")) {
        cc_push_bool(a, t, "SALAM_OS_WASM", true);
        cc_push_bool(a, t, "SALAM_OS_UNIX", true);
        cc_push_str(a, t, "SALAM_OS", "wasm");
    } else {
        cc_push_str(a, t, "SALAM_OS", "unknown");
    }
}

static const char *const s_all_bool_flags[] = {
    "SALAM_OS_WINDOWS", "SALAM_OS_WIN64",   "SALAM_OS_WIN32",   "SALAM_OS_WASM",
    "SALAM_OS_ANDROID", "SALAM_OS_LINUX",   "SALAM_OS_MAC",     "SALAM_OS_BSD",
    "SALAM_OS_FREEBSD", "SALAM_OS_OPENBSD", "SALAM_OS_NETBSD",  "SALAM_OS_UNIX",
    "SALAM_ARCH_X64",   "SALAM_ARCH_X86",   "SALAM_ARCH_ARM64", "SALAM_ARCH_ARM",
    "SALAM_ARCH_WASM",  "SALAM_MEM_DEBUG",  "SALAM_ASAN",       NULL};

static void cc_push_missing_bool_defaults(arena_t *a, cc_table_t *t)
{
    int i = 0;
    for (; s_all_bool_flags[i]; i++)
        if (!cc_table_lookup(t, s_all_bool_flags[i]))
            cc_push_bool(a, t, s_all_bool_flags[i], false);
}

static void cc_push_define(arena_t *a, cc_table_t *t, const char *def)
{
    const char *eq = strchr(def, '=');
    if (!eq) {
        cc_push_bool(a, t, def, true);
        return;
    }
    cc_push_str(a, t, arena_strndup(a, def, (size_t)(eq - def)), eq + 1);
}

cc_table_t *cc_table_build(arena_t *a, const char *target_triple,
                           const char *const *defines, int ndefines)
{
    cc_table_t *t = (cc_table_t *)arena_alloc(a, sizeof(*t));
    vec_init(&t->entries);
    if (target_triple && target_triple[0])
        cc_push_target_defs(a, t, target_triple);
    else
        cc_push_host_defs(a, t);
    {
        int i = 0;
        for (; i < ndefines; i++)
            cc_push_define(a, t, defines[i]);
    }
    cc_push_missing_bool_defaults(a, t);
    return t;
}

const cc_const_t *cc_table_lookup(const cc_table_t *tbl, const char *name)
{
    if (!tbl || !name) return NULL;
    {
        size_t i = 0;
        for (; i < tbl->entries.len; i++) {
            const cc_const_t *c = (const cc_const_t *)tbl->entries.data[i];
            if (strcmp(c->name, name) == 0) return c;
        }
    }
    return NULL;
}
