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

#include "llvm/orc_call.h"

int salam_orc_call_main(int64_t addr)
{
    int (*fn)(void) = (int (*)(void))(intptr_t)addr;
    return fn();
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
