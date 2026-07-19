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

#ifdef _WIN32
#  include <direct.h>
#  include <io.h>
#  include <process.h>
#  include <windows.h>
#else
#  include <dirent.h>
#  include <sys/stat.h>
#  include <time.h>
#  include <unistd.h>
#endif

typedef int64_t (*native_fn8_t)(int64_t, int64_t, int64_t, int64_t, int64_t, int64_t,
                                int64_t, int64_t);

typedef struct {
    const char *name;
    void *fnptr;
} native_sym_t;

static const native_sym_t NATIVE_SYMS[] = {
    {"getenv", (void *)&getenv},
    {"malloc", (void *)&malloc},
    {"realloc", (void *)&realloc},
    {"free", (void *)&free},
    {"memset", (void *)&memset},
    {"memcpy", (void *)&memcpy},
    {"memmove", (void *)&memmove},
    {"remove", (void *)&remove},
    {"rename", (void *)&rename},
    {"fopen", (void *)&fopen},
    {"fclose", (void *)&fclose},
    {"fseek", (void *)&fseek},
    {"ftell", (void *)&ftell},
    {"fread", (void *)&fread},
    {"fwrite", (void *)&fwrite},
    {"fgetc", (void *)&fgetc},
    {"fgets", (void *)&fgets},
    {"exit", (void *)&exit},
    {"abort", (void *)&abort},
#ifdef _WIN32
    {"_mkdir", (void *)&_mkdir},
    {"_rmdir", (void *)&_rmdir},
    {"_chdir", (void *)&_chdir},
    {"GetCurrentDirectoryA", (void *)&GetCurrentDirectoryA},
    {"GetFileAttributesA", (void *)&GetFileAttributesA},
    {"_fseeki64", (void *)&_fseeki64},
    {"FindFirstFileA", (void *)&FindFirstFileA},
    {"FindNextFileA", (void *)&FindNextFileA},
    {"FindClose", (void *)&FindClose},
    {"GetSystemTimeAsFileTime", (void *)&GetSystemTimeAsFileTime},
    {"_getpid", (void *)&_getpid},
    {"_popen", (void *)&_popen},
    {"_pclose", (void *)&_pclose},
#else
    {"mkdir", (void *)&mkdir},     {"rmdir", (void *)&rmdir},
    {"chdir", (void *)&chdir},     {"access", (void *)&access},
    {"getcwd", (void *)&getcwd},   {"opendir", (void *)&opendir},
    {"readdir", (void *)&readdir}, {"closedir", (void *)&closedir},
    {"fseeko", (void *)&fseeko},   {"clock_gettime", (void *)&clock_gettime},
    {"popen", (void *)&popen},     {"pclose", (void *)&pclose},
    {"getpid", (void *)&getpid},   {"system", (void *)&system},
#endif
};

static void *find_native_sym(const char *name)
{
    size_t i = 0;
    for (; i < sizeof NATIVE_SYMS / sizeof NATIVE_SYMS[0]; i++)
        if (!strcmp(NATIVE_SYMS[i].name, name)) return NATIVE_SYMS[i].fnptr;
    return NULL;
}

static bool typestr_is_float(const char *ts)
{
    char base[96];
    base_typename(ts, base, sizeof base);
    return is_float_typename(base);
}

static int64_t marshal_arg(value_t v)
{
    switch (v.kind) {
    case VAL_STR:
        return (int64_t)(intptr_t)(v.as.s ? v.as.s : "");
    case VAL_PTR:
        return (int64_t)(intptr_t)v.as.ptr.addr;
    case VAL_BOOL:
        return v.as.b ? 1 : 0;
    case VAL_CHAR:
    case VAL_INT:
        return v.as.i;
    default:
        return 0;
    }
}

value_t call_native_extern(interp_t *I, ast_node_t *call, ast_node_t *decl, value_t *args,
                           size_t nargs)
{
    if (decl->is_variadic)
        rt_error(I, call,
                 "extern function '%s' is variadic, which the interpreter does not "
                 "support (use 'salam run' instead)",
                 decl->name);
    if (decl->list.len > 8)
        rt_error(I, call,
                 "extern function '%s' takes too many parameters for the "
                 "interpreter's native call support",
                 decl->name);
    {
        size_t i = 0;
        for (; i < decl->list.len; i++) {
            ast_node_t *p = (ast_node_t *)decl->list.data[i];
            if (typestr_is_float(p->type ? p->type->type_str : NULL))
                rt_error(I, call,
                         "extern function '%s' takes a floating-point argument, which "
                         "the interpreter does not support (use 'salam run' instead)",
                         decl->name);
        }
    }
    const char *rts = decl->type ? decl->type->type_str : NULL;
    if (rts && typestr_is_float(rts))
        rt_error(I, call,
                 "extern function '%s' returns a floating-point value, which the "
                 "interpreter does not support (use 'salam run' instead)",
                 decl->name);

    void *sym = find_native_sym(decl->name);
    if (!sym)
        rt_error(I, call,
                 "extern function '%s' is not supported by the interpreter (use "
                 "'salam run' instead)",
                 decl->name);

    int64_t a[8] = {0};
    {
        size_t i = 0;
        for (; i < decl->list.len && i < nargs; i++)
            a[i] = marshal_arg(args[i]);
    }
    native_fn8_t f = (native_fn8_t)sym;
    int64_t r = f(a[0], a[1], a[2], a[3], a[4], a[5], a[6], a[7]);

    if (!rts) return val_null();
    if (rts[strlen(rts) - 1] == '*')
        return val_ptr((void *)(intptr_t)r, ptr_elem_from_typestr(rts));
    char rbase[96];
    base_typename(rts, rbase, sizeof rbase);
    if (!strcmp(rbase, "bool")) return val_bool(r != 0);
    return val_int(r);
}
