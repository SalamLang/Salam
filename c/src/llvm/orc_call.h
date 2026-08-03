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

#ifndef SALAM_LLVM_ORC_CALL_H
#define SALAM_LLVM_ORC_CALL_H

#include <stdint.h>

/*
 * Trampoline for std/llvm/orc.salam's RunJIT: LLVMOrcLLJITLookup only
 * hands back a runtime-resolved address (LLVMOrcExecutorAddress, a
 * uint64_t) - there's no LLVM-C call that invokes it for you, and Salam's
 * `as` cast has no i64 -> callable-function-type conversion (a JIT'd
 * address is only known at runtime, so it could never bind to an `extern:`
 * declaration, which is a link-time construct). This is the minimal C-ABI
 * bridge that closes the gap, in the same spirit as lld_link.cc's shim
 * over LLD: one call, no LLVM/LLD headers involved.
 */
int salam_orc_call_main(int64_t addr);

#endif /* SALAM_LLVM_ORC_CALL_H */
