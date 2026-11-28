/*
 * Salam Programming Language (2018-2026)
 *
 *   +-------------------+
 *   |     S A L A M     |
 *   +-------------------+
 *
 * Designed by Seyyed Ali Mohammadiyeh and the Salam Team
 *
 * Repository: https://github.com/SalamLang/Salam
 *
 */

/* LLJIT finds the C library through dlopen(NULL), which always fails in a
 * static musl binary ("Dynamic loading not supported"), and LLVM-C cannot
 * replace that step. This builder fills the process symbols library from a
 * table of addresses fixed at link time instead. */

#include "orc_static.h"

#include "llvm/ExecutionEngine/Orc/LLJIT.h"
#if __has_include("llvm/ExecutionEngine/Orc/AbsoluteSymbols.h")
#  include "llvm/ExecutionEngine/Orc/AbsoluteSymbols.h"
#endif

#include <utility>

using namespace llvm;
using namespace llvm::orc;

static Expected<JITDylibSP> salam_static_process_symbols(LLJIT &jit)
{
    JITDylib &jd = jit.getExecutionSession().createBareJITDylib("<Process Symbols>");
    SymbolMap symbols;
    for (const salam_jit_symbol *s = salam_jit_symbols; s->name; ++s) {
        symbols[jit.mangleAndIntern(s->name)] = ExecutorSymbolDef(
            ExecutorAddr::fromPtr(s->address), JITSymbolFlags::Exported);
    }
    if (Error err = jd.define(absoluteSymbols(std::move(symbols)))) {
        return std::move(err);
    }
    return &jd;
}

extern "C" LLVMOrcLLJITBuilderRef salam_orc_static_builder(void)
{
    if (!salam_jit_symbols[0].name) {
        return nullptr;
    }
    LLJITBuilder *builder = new LLJITBuilder();
    builder->setProcessSymbolsJITDylibSetup(salam_static_process_symbols);
    return reinterpret_cast<LLVMOrcLLJITBuilderRef>(builder);
}
