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

#ifndef SALAM_PARALLEL_OPT_H
#define SALAM_PARALLEL_OPT_H

#include "llvm-c/TargetMachine.h"
#include "llvm-c/Types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* How many partitions salam_parallel_opt_emit should use for this module
 * with at most `jobs` threads; 1 means "optimize it as one module". A
 * partition gets at least 20000 instructions, or SALAM_LLVM_PARTITION_MIN
 * when that is set (tests use a small value to exercise this path). */
int salam_parallel_partitions(LLVMModuleRef mod, int jobs);

/* Optimizes `mod` with `pipeline` (a new-PM pipeline string, may be empty)
 * and writes `parts` object files named "<out_prefix>.<i>.o", one thread per
 * partition. `mod` is changed (local symbols become hidden externals) and
 * must not be optimized or emitted again afterwards. Returns 0 on success;
 * on failure *err gets a malloc'd message the caller frees. */
int salam_parallel_opt_emit(LLVMModuleRef mod, LLVMTargetMachineRef tm,
                            const char *pipeline, int parts, const char *out_prefix,
                            char **err);

#ifdef __cplusplus
}
#endif

#endif
