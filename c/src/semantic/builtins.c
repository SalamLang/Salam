/*
 * Salam Programming Language (2024-2026)
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
#include "core/build_info.h"
#include "semantic/builtins.h"
#include <time.h>

static const salam_builtin_t k_builtins[] = {
    {"char_from_code", "salam_char_from_code", 1, "str", "i32"},
    {"strcmp", "strcmp", 2, "i32", "str"},
    {"join", "salam_thread_join", 1, "void", "i64"},
};
/*
 * Argument count for an atomic intrinsic, or 0 when the name is not one.
 * They are kept out of k_builtins because that table describes a call whose
 * arguments all share one type, and these mix a cell pointer with i64
 * operands. Every backend special-cases them anyway - there is no runtime
 * function to name.
 */
int salam_atomic_arity(const char *name)
{
    if (!name) return 0;
    if (!strcmp(name, "atomic_load")) return 1;
    if (!strcmp(name, "atomic_store")) return 2;
    if (!strcmp(name, "atomic_add")) return 2;
    if (!strcmp(name, "atomic_swap")) return 2;
    if (!strcmp(name, "atomic_cas")) return 3;
    return 0;
}

const salam_builtin_t *salam_builtin_lookup(const char *name)
{
    {
        size_t i = 0;
        for (; i < sizeof(k_builtins) / sizeof(k_builtins[0]); i++)
            if (strcmp(k_builtins[i].name, name) == 0) return &k_builtins[i];
    }
    return NULL;
}

/*
 * The -dNAME=VALUE table.
 *
 * Storage is a fixed pool rather than an arena because this is consulted
 * from name resolution, which has no allocator of its own to hand down, and
 * the values have to outlive every arena in the process anyway (the REPL
 * rebuilds its arena between lines but keeps the same command line). A
 * malloc'd copy would be a permanent allocation that the leak budget counts,
 * so a pool it is: bounded, freed with the process, and large enough that
 * overflowing it means something has gone wrong on the command line.
 */
#define CLI_CONST_MAX 256
#define CLI_CONST_POOL 16384

typedef struct {
    const char *name;
    token_kind_t op; /* TK_INT | TK_FLOAT | TK_STRING | TK_KW_TRUE | TK_KW_FALSE */
    uint64_t i;      /* magnitude; `neg` carries the sign */
    bool neg;
    double f;
    const char *s;
} cli_const_entry_t;

static cli_const_entry_t g_cli_consts[CLI_CONST_MAX];
static int g_cli_nconsts;
static char g_cli_pool[CLI_CONST_POOL];
static size_t g_cli_pool_used;

/* Copy `len` bytes into the pool and NUL-terminate; NULL when it is full. */
static const char *cli_pool_put(const char *src, size_t len)
{
    char *dst;
    if (g_cli_pool_used + len + 1 > sizeof g_cli_pool) return NULL;
    dst = g_cli_pool + g_cli_pool_used;
    memcpy(dst, src, len);
    dst[len] = '\0';
    g_cli_pool_used += len + 1;
    return dst;
}

static bool cli_all_digits(const char *v, size_t len)
{
    size_t i = 0;
    if (len == 0) return false;
    for (; i < len; i++)
        if (v[i] < '0' || v[i] > '9') return false;
    return true;
}

/*
 * Exactly the grammar a Salam float literal has: optional sign, digits with
 * at most one '.', and an optional decimal exponent - with at least one digit
 * and at least one of '.'/'e' present, so a plain integer is not claimed
 * here. Spelled out rather than deferred to strtod's endptr because the
 * selfhost compiler has to agree with this character for character, and
 * strtod additionally accepts "inf", "nan" and hex floats that the language
 * does not.
 */
static bool cli_is_float_lit(const char *v)
{
    size_t i = 0;
    int digits = 0;
    bool dot = false, exp = false;
    if (v[i] == '-' || v[i] == '+') i++;
    for (; v[i] && v[i] != 'e' && v[i] != 'E'; i++) {
        if (v[i] == '.') {
            if (dot) return false;
            dot = true;
        } else if (v[i] >= '0' && v[i] <= '9') {
            digits++;
        } else {
            return false;
        }
    }
    if (!digits) return false;
    if (v[i] == 'e' || v[i] == 'E') {
        int edigits = 0;
        exp = true;
        i++;
        if (v[i] == '-' || v[i] == '+') i++;
        for (; v[i]; i++) {
            if (v[i] < '0' || v[i] > '9') return false;
            edigits++;
        }
        if (!edigits) return false;
    }
    return dot || exp;
}

/*
 * Classify the text after '='. Numbers and booleans are recognised so
 * `-dAGE=22` is an integer the way it reads; everything else is a string,
 * which is why `-dVERSION=0.3.1` stays text (strtod stops at the second dot,
 * so it is not a float). Wrapping the value in double quotes forces the
 * string reading for the cases that would otherwise look numeric.
 */
static bool cli_const_value(const char *v, cli_const_entry_t *e)
{
    size_t len = strlen(v);
    const char *digits = v;
    bool neg = false;

    if (len >= 2 && v[0] == '"' && v[len - 1] == '"') {
        e->op = TK_STRING;
        e->s = cli_pool_put(v + 1, len - 2);
        return e->s != NULL;
    }
    if (strcmp(v, "true") == 0) {
        e->op = TK_KW_TRUE;
        return true;
    }
    if (strcmp(v, "false") == 0) {
        e->op = TK_KW_FALSE;
        return true;
    }
    if (*digits == '-' || *digits == '+') {
        neg = *digits == '-';
        digits++;
    }
    if (cli_all_digits(digits, strlen(digits))) {
        /* Accumulated by hand rather than via strtoull, which is not part of
         * the C89 this tree compiles as. */
        uint64_t u = 0;
        const char *d = digits;
        for (; *d; d++)
            u = u * 10u + (uint64_t)(*d - '0');
        e->op = TK_INT;
        e->i = u;
        e->neg = neg;
        return true;
    }
    if (cli_is_float_lit(v)) {
        e->op = TK_FLOAT;
        e->f = strtod(v, NULL);
        return true;
    }
    e->op = TK_STRING;
    e->s = cli_pool_put(v, len);
    return e->s != NULL;
}

void salam_builtin_set_consts(const char *const *defs, int ndefs)
{
    int i = 0;
    g_cli_nconsts = 0;
    g_cli_pool_used = 0;
    for (; i < ndefs; i++) {
        const char *def = defs[i];
        const char *eq = strchr(def, '=');
        cli_const_entry_t e;
        memset(&e, 0, sizeof e);
        if (g_cli_nconsts >= CLI_CONST_MAX) {
            fprintf(stderr, "salam: too many -d constants; '%s' and later ignored\n",
                    def);
            return;
        }
        e.name = cli_pool_put(def, eq ? (size_t)(eq - def) : strlen(def));
        if (!e.name) {
            fprintf(stderr, "salam: -d constant storage exhausted at '%s'\n", def);
            return;
        }
        if (!eq) {
            e.op = TK_KW_TRUE;
        } else if (!cli_const_value(eq + 1, &e)) {
            fprintf(stderr, "salam: -d constant storage exhausted at '%s'\n", def);
            return;
        }
        g_cli_consts[g_cli_nconsts++] = e;
    }
}

static const cli_const_entry_t *cli_const_lookup(const char *name)
{
    int i = 0;
    /* Last definition wins, so a repeated -d overrides an earlier one. */
    for (i = g_cli_nconsts - 1; i >= 0; i--)
        if (strcmp(g_cli_consts[i].name, name) == 0) return &g_cli_consts[i];
    return NULL;
}

static int salam_version_code(void)
{
    /* Follows a -dSALAM_VERSION override, so the code and the string a build
     * reports can never disagree. */
    const cli_const_entry_t *ov = cli_const_lookup("SALAM_VERSION");
    const char *p = (ov && ov->op == TK_STRING) ? ov->s : SALAM_VERSION;
    long maj = strtol(p, (char **)&p, 10);
    long min = (*p == '.') ? strtol(p + 1, (char **)&p, 10) : 0;
    long pat = (*p == '.') ? strtol(p + 1, (char **)&p, 10) : 0;
    return (int)(maj * 10000 + min * 100 + pat);
}

/*
 * SALAM_BUILD_DATE is when *this* program is being compiled, not when the
 * compiler reading it was built. That is the reading every use of it wants -
 * a program printing its own build date - and it means a binary carries a
 * true build stamp with nothing to pass on the command line. The compiler's
 * own build date is a separate thing and still comes from build_info.h's
 * __DATE__/__TIME__, which is what `salam version` reports. Same layout as
 * __DATE__ " " __TIME__ so the two are interchangeable in output.
 */
static const char *salam_compile_time(void)
{
    static char buf[32];
    if (!buf[0]) {
        time_t t = time(NULL);
        struct tm *lt = localtime(&t);
        if (lt)
            strftime(buf, sizeof buf, "%b %d %Y %H:%M:%S", lt);
        else
            strcpy(buf, "unknown");
    }
    return buf;
}

/* Reset the shared literal fields; the caller fills in op/value/type. */
static void bc_literal(ast_node_t *n)
{
    n->kind = AST_LITERAL;
    n->value.slen = 0;
    n->a = NULL;
    n->b = NULL;
    n->synthetic = false;
}

static void bc_str(ast_node_t *n, const char *s, bool *is_str)
{
    bc_literal(n);
    n->op = TK_STRING;
    n->value.kind = TV_STRING;
    n->value.as.s = s;
    n->value.slen = strlen(s);
    *is_str = true;
}

static bool bc_cli_const(arena_t *a, const cli_const_entry_t *e, ast_node_t *n,
                         bool *is_str)
{
    if (e->op == TK_STRING) {
        bc_str(n, e->s, is_str);
        return true;
    }
    /*
     * A negative integer becomes unary minus over its magnitude, exactly what
     * the parser produces for `-7` in source. Storing the two's complement in
     * a bare literal instead would type it u64 - the literal arm picks the
     * type from the magnitude, and 0xFFFF...F9 is not a small number.
     */
    if (e->op == TK_INT && e->neg) {
        ast_node_t *mag = ast_new(a, AST_LITERAL, &n->span);
        mag->op = TK_INT;
        mag->value.kind = TV_INT;
        mag->value.as.i = e->i;
        bc_literal(n);
        n->kind = AST_UNARY;
        n->op = TK_MINUS;
        n->a = mag;
        *is_str = false;
        return true;
    }
    bc_literal(n);
    n->op = e->op;
    *is_str = false;
    switch (e->op) {
    case TK_INT:
        n->value.kind = TV_INT;
        n->value.as.i = e->i;
        break;
    case TK_FLOAT:
        n->value.kind = TV_FLOAT;
        n->value.as.f = e->f;
        break;
    default: /* TK_KW_TRUE | TK_KW_FALSE */
        n->value.kind = TV_BOOL;
        n->value.as.i = e->op == TK_KW_TRUE;
        break;
    }
    return true;
}

bool salam_builtin_global_const(arena_t *a, const char *name, ast_node_t *n, bool *is_str)
{
    const cli_const_entry_t *e = cli_const_lookup(name);
    if (e) return bc_cli_const(a, e, n, is_str);
    if (strcmp(name, "SALAM_VERSION") == 0) {
        bc_str(n, SALAM_VERSION, is_str);
        return true;
    }
    if (strcmp(name, "SALAM_GIT_COMMIT") == 0) {
        bc_str(n, SALAM_GIT_COMMIT, is_str);
        return true;
    }
    if (strcmp(name, "SALAM_GIT_DATE") == 0) {
        bc_str(n, SALAM_GIT_DATE, is_str);
        return true;
    }
    if (strcmp(name, "SALAM_GIT_DIRTY") == 0) {
        bc_str(n, SALAM_GIT_DIRTY, is_str);
        return true;
    }
    if (strcmp(name, "SALAM_BUILD_DATE") == 0) {
        bc_str(n, salam_compile_time(), is_str);
        return true;
    }
    if (strcmp(name, "SALAM_VERSION_CODE") == 0) {
        bc_literal(n);
        n->op = TK_INT;
        n->value.kind = TV_INT;
        n->value.as.i = (uint64_t)salam_version_code();
        *is_str = false;
        return true;
    }
    return false;
}
