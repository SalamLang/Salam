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

#include "interp/interp_internal.h"
#include <math.h>
#include <time.h>
#ifdef _WIN32
#  include <windows.h>
#else
#  include <unistd.h>
#endif

static const char *str_trim(interp_t *I, const char *s)
{
    while (*s == ' ' || *s == '\t' || *s == '\n' || *s == '\r')
        s++;
    size_t n = strlen(s);
    while (n > 0 &&
           (s[n - 1] == ' ' || s[n - 1] == '\t' || s[n - 1] == '\n' || s[n - 1] == '\r'))
        n--;
    return arena_strndup(I->a, s, n);
}

static const char *str_substr(interp_t *I, const char *s, int64_t start, int64_t len)
{
    int64_t n = (int64_t)strlen(s);
    if (start < 0) start = 0;
    if (start > n) start = n;
    if (len < 0) len = 0;

    if (len > n - start) len = n - start;
    return arena_strndup(I->a, s + start, (size_t)len);
}

static const char *str_upper(interp_t *I, const char *s)
{
    size_t n = strlen(s);
    char *buf = (char *)arena_alloc(I->a, n + 1);
    size_t i = 0;
    for (; i < n; i++)
        buf[i] = (s[i] >= 'a' && s[i] <= 'z') ? (char)(s[i] - 32) : s[i];
    buf[n] = '\0';
    return buf;
}

static const char *str_lower(interp_t *I, const char *s)
{
    size_t n = strlen(s);
    char *buf = (char *)arena_alloc(I->a, n + 1);
    size_t i = 0;
    for (; i < n; i++)
        buf[i] = (s[i] >= 'A' && s[i] <= 'Z') ? (char)(s[i] + 32) : s[i];
    buf[n] = '\0';
    return buf;
}

static value_t str_split(interp_t *I, const char *s, const char *sep)
{
    sarray_t *out = array_new(I, 8);
    if (!sep || !*sep) {
        array_push(I, out, val_str(arena_strdup(I->a, s)));
        return mk_array(I, out);
    }
    size_t seplen = strlen(sep);
    const char *p = s, *hit;
    while ((hit = strstr(p, sep)) != NULL) {
        array_push(I, out, val_str(arena_strndup(I->a, p, (size_t)(hit - p))));
        p = hit + seplen;
    }
    array_push(I, out, val_str(arena_strdup(I->a, p)));
    return mk_array(I, out);
}

void do_print(interp_t *I, env_t *env, ast_node_t *call, bool newline, bool to_err)
{
    FILE *f = to_err ? I->err : I->out;
    {
        size_t i = 0;
        for (; i < call->list.len; i++) {
            if (i) fputc(' ', f);
            value_t v = eval(I, env, (ast_node_t *)call->list.data[i]);
            fputs(to_str(I, v), f);
        }
    }
    if (newline) fputc('\n', f);
}

value_t do_input(interp_t *I)
{
    if (I->in_data) {
        const char *p = I->in_data + I->in_pos;
        if (!*p) return val_str("");
        const char *nl = strchr(p, '\n');
        size_t len = nl ? (size_t)(nl - p) : strlen(p);
        I->in_pos += len + (nl ? 1 : 0);
        return val_str(arena_strndup(I->a, p, len));
    }
    char buf[4096] = "";
    if (!fgets(buf, sizeof buf, stdin)) return val_str("");
    size_t n = strlen(buf);
    while (n > 0 && (buf[n - 1] == '\n' || buf[n - 1] == '\r'))
        n--;
    return val_str(arena_strndup(I->a, buf, n));
}

value_t call_module_func(interp_t *I, ast_node_t *call, module_t *mod, const char *fn,
                         value_t *args, size_t nargs)
{
    binding_t *b = env_find_local(mod->env, fn);
    if (!b || b->val.kind != VAL_FUNC)
        rt_error(I, call, "package '%s' has no exported function '%s'", mod->name, fn);
    return call_func(I, b->val.as.fn->fn, b->val.as.fn->env, NULL, args, nargs);
}

typedef value_t (*intrinsic_fn)(interp_t *I, ast_node_t *call, value_t *a, size_t n);

#define IN1F(name, expr)                                                                 \
    static value_t name(interp_t *I, ast_node_t *call, value_t *a, size_t n)             \
    {                                                                                    \
        (void)I;                                                                         \
        (void)call;                                                                      \
        (void)n;                                                                         \
        double x = to_float(a[0]);                                                       \
        return val_float(expr);                                                          \
    }

#define IN2F(name, expr)                                                                 \
    static value_t name(interp_t *I, ast_node_t *call, value_t *a, size_t n)             \
    {                                                                                    \
        (void)I;                                                                         \
        (void)call;                                                                      \
        (void)n;                                                                         \
        double x = to_float(a[0]), y = to_float(a[1]);                                   \
        return val_float(expr);                                                          \
    }

IN1F(in_sqrt, sqrt(x))
IN1F(in_sin, sin(x))
IN1F(in_cos, cos(x))

IN1F(in_tan, tan(x))
IN1F(in_asin, asin(x))
IN1F(in_acos, acos(x))

IN1F(in_atan, atan(x))
IN1F(in_log, log(x))
IN1F(in_log10, log10(x))

IN1F(in_exp, exp(x))
IN1F(in_floor, floor(x))
IN1F(in_ceil, ceil(x))

IN1F(in_fabs, fabs(x))

IN2F(in_atan2, atan2(x, y))
IN2F(in_fmod, fmod(x, y))
IN2F(in_pow, pow(x, y))

static value_t in_strlen(interp_t *I, ast_node_t *call, value_t *a, size_t n)
{
    (void)call;
    (void)n;
    return val_int((int64_t)strlen(to_str(I, a[0])));
}

static value_t in_strcmp(interp_t *I, ast_node_t *call, value_t *a, size_t n)
{
    (void)call;
    (void)n;
    return val_int(strcmp(to_str(I, a[0]), to_str(I, a[1])));
}

static value_t in_strtol(interp_t *I, ast_node_t *call, value_t *a, size_t n)
{
    (void)call;
    (void)n;
    return val_int((int64_t)strtol(to_str(I, a[0]), NULL, 10));
}

static value_t in_strtod(interp_t *I, ast_node_t *call, value_t *a, size_t n)
{
    (void)call;
    (void)n;
    return val_float(strtod(to_str(I, a[0]), NULL));
}

static value_t in_input(interp_t *I, ast_node_t *call, value_t *a, size_t n)
{
    (void)call;
    (void)a;
    (void)n;
    return do_input(I);
}

static value_t in_time(interp_t *I, ast_node_t *call, value_t *a, size_t n)
{
    (void)I;
    (void)call;
    (void)a;
    (void)n;
    return val_int((int64_t)time(NULL));
}
#ifdef _WIN32
static value_t in_sleepex(interp_t *I, ast_node_t *call, value_t *a, size_t n)
{
    (void)I;
    (void)call;
    (void)n;
    Sleep((DWORD)to_int(a[0]));
    return val_int(0);
}
#else
static value_t in_usleep(interp_t *I, ast_node_t *call, value_t *a, size_t n)
{
    (void)I;
    (void)call;
    (void)n;
    usleep((useconds_t)to_int(a[0]));
    return val_int(0);
}
#endif
static const struct {
    const char *name;
    intrinsic_fn fn;
} INTRINSICS[] = {
    {"sqrt", in_sqrt},       {"sin", in_sin},       {"cos", in_cos},
    {"tan", in_tan},         {"asin", in_asin},     {"acos", in_acos},
    {"atan", in_atan},       {"atan2", in_atan2},   {"log", in_log},
    {"log10", in_log10},     {"exp", in_exp},       {"floor", in_floor},
    {"ceil", in_ceil},       {"fabs", in_fabs},     {"fmod", in_fmod},
    {"pow", in_pow},         {"strlen", in_strlen}, {"strcmp", in_strcmp},
    {"strtol", in_strtol},   {"strtod", in_strtod}, {"salam_input", in_input},
    {"time", in_time},
#ifdef _WIN32
    {"SleepEx", in_sleepex},
#else
    {"usleep", in_usleep},
#endif
};
value_t call_intrinsic(interp_t *I, ast_node_t *call, const char *name, value_t *args,
                       size_t nargs, bool *handled)
{
    {
        size_t i = 0;
        for (; i < sizeof INTRINSICS / sizeof INTRINSICS[0]; i++)
            if (strcmp(INTRINSICS[i].name, name) == 0) {
                *handled = true;
                return INTRINSICS[i].fn(I, call, args, nargs);
            }
    }
    *handled = false;
    return val_null();
}

value_t call_builtin_method(interp_t *I, ast_node_t *call, value_t recv,
                            const char *method, value_t *args, size_t nargs)
{
    value_t a0 = nargs > 0 ? args[0] : val_null();
    value_t a1 = nargs > 1 ? args[1] : val_null();
    switch (recv.kind) {
    case VAL_STR: {
        const char *s = recv.as.s;
        if (!strcmp(method, "len")) return val_int((int64_t)strlen(s));
        if (!strcmp(method, "concat")) return val_str(afmt(I, "%s%s", s, to_str(I, a0)));
        if (!strcmp(method, "substr"))
            return val_str(str_substr(I, s, to_int(a0), to_int(a1)));
        if (!strcmp(method, "find") || !strcmp(method, "search") ||
            !strcmp(method, "indexOf")) {
            const char *h = strstr(s, to_str(I, a0));
            return val_int(h ? (int64_t)(h - s) : -1);
        }
        if (!strcmp(method, "trim")) return val_str(str_trim(I, s));
        if (!strcmp(method, "to_int")) return val_int((int64_t)strtol(s, NULL, 10));
        if (!strcmp(method, "to_float")) return val_float(strtod(s, NULL));
        if (!strcmp(method, "split")) return str_split(I, s, to_str(I, a0));
        if (!strcmp(method, "lower")) return val_str(str_lower(I, s));
        if (!strcmp(method, "upper")) return val_str(str_upper(I, s));
        if (!strcmp(method, "repeat")) return val_str(str_repeat(I, s, to_int(a0)));
        break;
    }
    case VAL_ARRAY: {
        sarray_t *a = recv.as.arr;
        if (!strcmp(method, "push")) {
            array_push(I, a, a0);
            return val_null();
        }
        if (!strcmp(method, "pop")) {
            if (a->len == 0) rt_error(I, call, "pop from empty vector");
            return a->data[--a->len];
        }
        if (!strcmp(method, "get")) {
            int64_t i = to_int(a0);
            if (i < 0 || (size_t)i >= a->len)
                rt_error(I, call, "vector index %lld out of range (len %zu)",
                         (long long)i, a->len);

            sarray_t *box = array_new(I, 1);
            array_push(I, box, a->data[i]);
            return mk_array(I, box);
        }
        if (!strcmp(method, "set")) {
            int64_t i = to_int(a0);
            if (i < 0 || (size_t)i >= a->len)
                rt_error(I, call, "vector index %lld out of range (len %zu)",
                         (long long)i, a->len);
            a->data[i] = a1;
            return val_null();
        }
        if (!strcmp(method, "len")) return val_int((int64_t)a->len);
        if (!strcmp(method, "cap")) return val_int((int64_t)a->cap);
        if (!strcmp(method, "free")) return val_null();
        break;
    }
    case VAL_MAP: {
        smap_t *m = recv.as.map;
        if (!strcmp(method, "put")) {
            map_put(I, m, a0, a1);
            return val_null();
        }
        if (!strcmp(method, "get")) {
            smap_entry_t *e = map_find(m, a0);
            return e ? e->val : val_null();
        }
        if (!strcmp(method, "has")) return val_bool(map_find(m, a0) != NULL);
        if (!strcmp(method, "remove")) return val_bool(map_remove(m, a0));
        if (!strcmp(method, "size")) return val_int((int64_t)m->count);
        if (!strcmp(method, "iter")) {
            smapiter_t *it = (smapiter_t *)arena_alloc(I->a, sizeof *it);
            it->map = m;
            it->idx = 0;
            value_t v;
            v.kind = VAL_MAPITER;
            v.as.iter = it;
            return v;
        }
        break;
    }
    case VAL_PTR: {
        static const struct {
            const char *method;
            const char *fn;
        } FILE_METHODS[] = {
            {"read", "salam_file_read"},   {"readline", "salam_file_readline"},
            {"write", "salam_file_write"}, {"seek", "salam_file_seek"},
            {"close", "salam_file_close"},
        };
        size_t i = 0;
        for (; i < sizeof FILE_METHODS / sizeof FILE_METHODS[0]; i++) {
            if (strcmp(method, FILE_METHODS[i].method) != 0) continue;
            value_t *ef = find_extern_fn(I, FILE_METHODS[i].fn);
            if (!ef) break;
            value_t *allargs =
                (value_t *)arena_alloc(I->a, salam_size_mul(nargs + 1, sizeof(value_t)));
            allargs[0] = recv;
            {
                size_t j = 0;
                for (; j < nargs; j++)
                    allargs[j + 1] = args[j];
            }
            return call_func(I, ef->as.fn->fn, ef->as.fn->env, NULL, allargs, nargs + 1);
        }
        break;
    }
    case VAL_MAPITER: {
        smapiter_t *it = recv.as.iter;
        if (!strcmp(method, "has_next")) {
            {
                size_t i = it->idx;
                for (; i < it->map->cap; i++)
                    if (it->map->entries[i].used) return val_bool(true);
            }
            return val_bool(false);
        }
        if (!strcmp(method, "next")) {
            while (it->idx < it->map->cap && !it->map->entries[it->idx].used)
                it->idx++;
            if (it->idx < it->map->cap) it->idx++;
            return val_null();
        }
        if (!strcmp(method, "key")) {
            size_t i = it->idx;
            while (i < it->map->cap && !it->map->entries[i].used)
                i++;
            return i < it->map->cap ? it->map->entries[i].key : val_null();
        }
        if (!strcmp(method, "value")) {
            size_t i = it->idx;
            while (i < it->map->cap && !it->map->entries[i].used)
                i++;
            return i < it->map->cap ? it->map->entries[i].val : val_null();
        }
        break;
    }
    default:
        break;
    }
    rt_error(I, call, "method '%s' is not available on this value", method);
    return val_null();
}
