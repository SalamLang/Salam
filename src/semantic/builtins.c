#include "core/prelude.h"
#include "semantic/builtins.h"

static const salam_builtin_t k_builtins[] = {
    { "char_from_code", "salam_char_from_code", 1, "str", "i32" },
    { "strcmp", "strcmp", 2, "i32", "str" },
    { "join",         "salam_thread_join",  1, "void", "i64" },
};
const salam_builtin_t *salam_builtin_lookup(const char *name)
{
    { size_t i = 0; for (; i < sizeof(k_builtins)/sizeof(k_builtins[0]); i++)
        if (strcmp(k_builtins[i].name, name) == 0) return &k_builtins[i]; }
    return NULL;
}
