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

#ifndef SALAM_DRIVER_EMBED_SYSROOT_H
#define SALAM_DRIVER_EMBED_SYSROOT_H

#include "core/prelude.h"

int salam_materialize_sysroot(const char *name, const unsigned char *tar, size_t tar_len,
                              char *out, size_t out_n);

#endif /* SALAM_DRIVER_EMBED_SYSROOT_H */
