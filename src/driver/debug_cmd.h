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

#ifndef SALAM_DRIVER_DEBUG_CMD_H
#define SALAM_DRIVER_DEBUG_CMD_H

#include "cli/options.h"

int driver_debug(options_t *opt);

int driver_memcheck(options_t *opt);

#endif /* SALAM_DRIVER_DEBUG_CMD_H */
