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

#ifndef SALAM_LLVM_LLVM_NATIVE_H
#define SALAM_LLVM_LLVM_NATIVE_H

#include "core/prelude.h"
#include "logger/logger.h"
#include "llvm/codegen_llvm.h"

int salam_llvm_native_available(void);

int salam_llvm_native(logger_t *log, const char *ll_path,
                      const codegen_llvm_options_t *opts);

#endif /* SALAM_LLVM_LLVM_NATIVE_H */
