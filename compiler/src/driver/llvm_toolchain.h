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

#ifndef SALAM_DRIVER_LLVM_TOOLCHAIN_H
#define SALAM_DRIVER_LLVM_TOOLCHAIN_H

#include "core/prelude.h"
#include "logger/logger.h"
#include "llvm/codegen_llvm.h"

int salam_llvm_toolchain(logger_t *log, const char *ll_path,
                         const codegen_llvm_options_t *opts);

#endif /* SALAM_DRIVER_LLVM_TOOLCHAIN_H */
