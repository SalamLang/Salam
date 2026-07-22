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
#include "core/build_info.h"
#include "semantic/builtins.h"

static const salam_builtin_t k_builtins[] = {
    {"char_from_code", "salam_char_from_code", 1, "str", "i32"},
    {"strcmp", "strcmp", 2, "i32", "str"},
    {"join", "salam_thread_join", 1, "void", "i64"},
};
const salam_builtin_t *salam_builtin_lookup(const char *name)
{
    {
        size_t i = 0;
        for (; i < sizeof(k_builtins) / sizeof(k_builtins[0]); i++)
            if (strcmp(k_builtins[i].name, name) == 0) return &k_builtins[i];
    }
    return NULL;
}

static int salam_version_code(void)
{
    const char *p = SALAM_VERSION;
    long maj = strtol(p, (char **)&p, 10);
    long min = (*p == '.') ? strtol(p + 1, (char **)&p, 10) : 0;
    long pat = (*p == '.') ? strtol(p + 1, (char **)&p, 10) : 0;
    return (int)(maj * 10000 + min * 100 + pat);
}

bool salam_builtin_global_const(const char *name, ast_node_t *n, bool *is_str)
{
    if (strcmp(name, "SALAM_VERSION") == 0) {
        n->kind = AST_LITERAL;
        n->op = TK_STRING;
        n->value.kind = TV_STRING;
        n->value.as.s = SALAM_VERSION;
        n->value.slen = strlen(SALAM_VERSION);
        n->a = NULL;
        n->b = NULL;
        n->synthetic = false;
        *is_str = true;
        return true;
    }
    if (strcmp(name, "SALAM_VERSION_CODE") == 0) {
        n->kind = AST_LITERAL;
        n->op = TK_INT;
        n->value.kind = TV_INT;
        n->value.as.i = (uint64_t)salam_version_code();
        n->value.slen = 0;
        n->a = NULL;
        n->b = NULL;
        n->synthetic = false;
        *is_str = false;
        return true;
    }
    return false;
}
