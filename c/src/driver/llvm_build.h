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

#ifndef SALAM_DRIVER_LLVM_BUILD_H
#define SALAM_DRIVER_LLVM_BUILD_H

#include "cli/options.h"

/*
 * Distinct exit code for "the LLVM backend does not lower some construct in
 * this program yet". Separated from the generic failure code so
 * driver_build() can fall back to the C backend on exactly this case and
 * still surface real errors (bad source, failed link) unchanged.
 */
#define SALAM_RC_LLVM_UNSUPPORTED 7

/*
 * Distinct exit code for "codegen was fine, producing the executable was
 * not". The in-process lld links against the bundled static musl sysroot,
 * which has none of the host's shared libraries - a program that links
 * -lsqlite3 or -lgtk-3 finds them through the C backend's host toolchain and
 * nowhere else. driver_build() retries those through the C backend so
 * defaulting to LLVM does not un-build programs that built before.
 */
#define SALAM_RC_LLVM_LINK_FAILED 8

int driver_llvm(options_t *opt);

int driver_llvm_build(options_t *opt);

#endif /* SALAM_DRIVER_LLVM_BUILD_H */
