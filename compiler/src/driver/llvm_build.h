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

#ifndef SALAM_DRIVER_LLVM_BUILD_H
#define SALAM_DRIVER_LLVM_BUILD_H

#include "cli/options.h"

int driver_llvm(options_t *opt);

int driver_llvm_build(options_t *opt);

#endif /* SALAM_DRIVER_LLVM_BUILD_H */
