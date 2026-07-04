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
#include "preproc/preproc.h"
#include "preproc/preproc_internal.h"
#include "core/sb.h"

#define PP_MAX_DEPTH 64
typedef struct {
    bool active;
    bool taken;
    bool parent;
} pp_frame_t;
static const char *s_os_defs[] = {
#if defined(_WIN32)
    "SALAM_OS_WINDOWS",
    "SALAM_OS=windows",
#  if defined(_WIN64)
    "SALAM_OS_WIN64",
#  else
    "SALAM_OS_WIN32",
#  endif
#elif defined(__EMSCRIPTEN__)
    "SALAM_OS_WASM",
    "SALAM_OS_UNIX",
    "SALAM_OS=wasm",
#elif defined(__ANDROID__)
    "SALAM_OS_ANDROID",
    "SALAM_OS_LINUX",
    "SALAM_OS_UNIX",
    "SALAM_OS=android",
#elif defined(__linux__)
    "SALAM_OS_LINUX",
    "SALAM_OS_UNIX",
    "SALAM_OS=linux",
#elif defined(__APPLE__)
    "SALAM_OS_MAC",
    "SALAM_OS_BSD",
    "SALAM_OS_UNIX",
    "SALAM_OS=mac",
#elif defined(__FreeBSD__)
    "SALAM_OS_FREEBSD", "SALAM_OS_BSD", "SALAM_OS_UNIX", "SALAM_OS=freebsd",
#elif defined(__OpenBSD__)
    "SALAM_OS_OPENBSD", "SALAM_OS_BSD", "SALAM_OS_UNIX", "SALAM_OS=openbsd",
#elif defined(__NetBSD__)
    "SALAM_OS_NETBSD", "SALAM_OS_BSD", "SALAM_OS_UNIX", "SALAM_OS=netbsd",
#else
    "SALAM_OS=unknown",
#endif

#if defined(__x86_64__) || defined(_M_X64)
    "SALAM_ARCH_X64",
    "SALAM_ARCH=x64",
#elif defined(__i386__) || defined(_M_IX86)
    "SALAM_ARCH_X86",
    "SALAM_ARCH=x86",
#elif defined(__aarch64__) || defined(_M_ARM64)
    "SALAM_ARCH_ARM64",
    "SALAM_ARCH=arm64",
#elif defined(__arm__) || defined(_M_ARM)
    "SALAM_ARCH_ARM",
    "SALAM_ARCH=arm",
#elif defined(__wasm__) || defined(__EMSCRIPTEN__)
    "SALAM_ARCH_WASM",
    "SALAM_ARCH=wasm",
#endif
    NULL};
static int s_nos = -1;
static int count_os_defs(void)
{
    if (s_nos < 0) {
        s_nos = 0;
        while (s_os_defs[s_nos])
            s_nos++;
    }
    return s_nos;
}

char *preproc_run(arena_t *a, logger_t *log, const char *text, const char *const *defines,
                  int ndefines)
{
    int nos = count_os_defs();
    int total = nos + ndefines;
    const char **all =
        (const char **)arena_alloc(a, (size_t)(total + 1) * sizeof(char *));
    {
        int i = 0;
        for (; i < nos; i++)
            all[i] = s_os_defs[i];
    }
    {
        int i = 0;
        for (; i < ndefines; i++)
            all[nos + i] = defines[i];
    }
    all[total] = NULL;
    defines = all;
    ndefines = total;
    sb_t out;
    sb_init(&out);
    pp_frame_t stack[PP_MAX_DEPTH];
    int depth = 0;
    int over = 0;
    const char *p = text;
    while (*p) {
        const char *line = p;
        while (*p && *p != '\n')
            p++;
        size_t len = (size_t)(p - line);
        bool has_nl = (*p == '\n');
        if (has_nl) p++;
        bool emitting = (depth == 0) || stack[depth - 1].active;
        const char *t = pp_skip_ws(line);

        if (strncmp(t, "@ifdef", 6) == 0 && (t[6] == ' ' || t[6] == '\t')) {
            bool parent = emitting;
            bool c = parent && pp_eval(t + 6, defines, ndefines);
            if (depth < PP_MAX_DEPTH) {
                stack[depth].active = c;
                stack[depth].taken = c;
                stack[depth].parent = parent;
                depth++;
            } else
                over++;
            LOG_T(log, PH_DRIVER, "preproc: @ifdef -> %s", c ? "active" : "skipped");
        } else if (strncmp(t, "@ifndef", 7) == 0 && (t[7] == ' ' || t[7] == '\t')) {
            bool parent = emitting;
            bool c = parent && !pp_eval(t + 7, defines, ndefines);
            if (depth < PP_MAX_DEPTH) {
                stack[depth].active = c;
                stack[depth].taken = c;
                stack[depth].parent = parent;
                depth++;
            } else
                over++;
            LOG_T(log, PH_DRIVER, "preproc: @ifndef -> %s", c ? "active" : "skipped");
        } else if (strncmp(t, "@if", 3) == 0 && (t[3] == ' ' || t[3] == '\t')) {
            bool parent = emitting;
            bool c = parent && pp_eval(t + 3, defines, ndefines);
            if (depth < PP_MAX_DEPTH) {
                stack[depth].active = c;
                stack[depth].taken = c;
                stack[depth].parent = parent;
                depth++;
            } else
                over++;
            LOG_T(log, PH_DRIVER, "preproc: @if -> %s", c ? "active" : "skipped");
        } else if (strncmp(t, "@else", 5) == 0) {
            const char *cond_start = pp_skip_ws(t + 5);
            bool has_cond =
                (*cond_start != '\0' && *cond_start != '\n' && *cond_start != '\r');
            if (depth > 0) {
                pp_frame_t *f = &stack[depth - 1];
                if (has_cond) {
                    if (f->parent && !f->taken) {
                        bool c = pp_eval(cond_start, defines, ndefines);
                        f->active = c;
                        if (c) f->taken = true;
                    } else {
                        f->active = false;
                    }
                    LOG_T(log, PH_DRIVER, "preproc: @else <cond> -> %s",
                          f->active ? "active" : "skipped");
                } else {
                    f->active = f->parent && !f->taken;
                    if (f->active) f->taken = true;
                    LOG_T(log, PH_DRIVER, "preproc: @else -> %s",
                          f->active ? "active" : "skipped");
                }
            }
        } else if (strncmp(t, "@end", 4) == 0) {
            if (over > 0)
                over--;
            else if (depth > 0)
                depth--;
        } else {
            if (emitting) {
                {
                    size_t i = 0;
                    for (; i < len; i++)
                        sb_putc(&out, line[i]);
                }
            }
        }
        if (has_nl) sb_putc(&out, '\n');
    }
    char *r = arena_strdup(a, sb_cstr(&out));
    sb_free(&out);
    return r;
}

source_file_t *preproc_source(arena_t *a, logger_t *log, const source_file_t *src,
                              const char *const *defines, int ndefines)
{
    if (!strstr(src->text, "@if")) return CONST_CAST(src);
    char *text = preproc_run(a, log, src->text, defines, ndefines);
    source_file_t *out = (source_file_t *)arena_alloc(a, sizeof(*out));
    out->path = src->path;
    out->text = text;
    out->len = strlen(text);
    return out;
}
