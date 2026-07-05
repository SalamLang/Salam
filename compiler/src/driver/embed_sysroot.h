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
 * Materialize a cross-compilation sysroot that is embedded in the salam binary
 * (a ustar archive) into a per-user cache directory, so a single self-contained
 * salam needs no sysroot files on disk beside it.
 */

#ifndef SALAM_DRIVER_EMBED_SYSROOT_H
#define SALAM_DRIVER_EMBED_SYSROOT_H

#include "core/prelude.h"

/*
 * Extract the embedded ustar archive [tar, tar+tar_len) into
 *   <cache>/salam/sysroots/<name>_<tar_len>/
 * (cache = $SALAM_CACHE, else $XDG_CACHE_HOME, else $HOME/.cache, else the
 * system temp dir), unless already extracted. On success writes the directory
 * path to `out` and returns 1; returns 0 on failure.
 */
int salam_materialize_sysroot(const char *name, const unsigned char *tar, size_t tar_len,
                              char *out, size_t out_n);

#endif /* SALAM_DRIVER_EMBED_SYSROOT_H */
