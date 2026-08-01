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

#ifndef SALAM_DRIVER_DRIVER_H
#define SALAM_DRIVER_DRIVER_H

#include "core/sal_types.h"

int driver_main(int argc, char **argv);

bool resolve_color(int flag);

/* Directory where intermediate salam_mod_*.c/.h/.o codegen scratch files are
 * written instead of the current working directory. Created on first call
 * if it doesn't already exist; falls back to "." if it can't be created. */
const char *salam_scratch_dir(void);

#endif /* SALAM_DRIVER_DRIVER_H */
