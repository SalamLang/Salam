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
