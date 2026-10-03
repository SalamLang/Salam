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

#include "orc_call.h"

#include <stdlib.h>
#include <string.h>

static char **g_orc_argv = NULL;
static int g_orc_argc = 0;
static int g_orc_cap = 0;

void salam_orc_args_clear(void)
{
    for (int i = 0; i < g_orc_argc; i++) {
        free(g_orc_argv[i]);
    }
    free(g_orc_argv);
    g_orc_argv = NULL;
    g_orc_argc = 0;
    g_orc_cap = 0;
}

void salam_orc_args_push(const char *arg)
{
    if (g_orc_argc + 2 > g_orc_cap) {
        int cap = g_orc_cap ? g_orc_cap * 2 : 8;
        char **grown = realloc(g_orc_argv, (size_t)cap * sizeof(char *));
        if (!grown) {
            return;
        }
        g_orc_argv = grown;
        g_orc_cap = cap;
    }
    g_orc_argv[g_orc_argc++] = strdup(arg ? arg : "");
    g_orc_argv[g_orc_argc] = NULL;
}

int salam_orc_call_main(int64_t addr)
{
    static char *empty_argv[] = {"salam-jit", NULL};
    int (*fn)(int, char **) = (int (*)(int, char **))(intptr_t)addr;
    if (g_orc_argc == 0) {
        return fn(1, empty_argv);
    }
    return fn(g_orc_argc, g_orc_argv);
}

/*
 * See the header: LLVM-C's LLVMInitializeAll* are header-inline and so
 * unlinkable from Salam. Compiled only where the LLVM headers are
 * available; without them this file still provides the trampoline above,
 * which needs no LLVM at all.
 */
#ifdef SALAM_HAVE_LLVM

#  include <llvm-c/Target.h>

void salam_llvm_init_all_target_infos(void)
{
    LLVMInitializeAllTargetInfos();
}

void salam_llvm_init_all_targets(void)
{
    LLVMInitializeAllTargets();
}

void salam_llvm_init_all_target_mcs(void)
{
    LLVMInitializeAllTargetMCs();
}

void salam_llvm_init_all_asm_printers(void)
{
    LLVMInitializeAllAsmPrinters();
}

void salam_llvm_init_all_asm_parsers(void)
{
    LLVMInitializeAllAsmParsers();
}

#endif /* SALAM_HAVE_LLVM */
