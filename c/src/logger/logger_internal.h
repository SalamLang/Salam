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

#ifndef SALAM_LOGGER_INTERNAL_H
#define SALAM_LOGGER_INTERNAL_H

#include "core/prelude.h"
#include "logger/logger.h"
#include "diag/diag_render.h"

#define LOGGER_MAX_DIAG_SOURCES 512

typedef struct {
    const char *file;
    const char *text;
    size_t len;
} diag_source_entry_t;

struct logger {
    FILE *sink;
    log_level_t min_level;
    bool color;
    bool timestamps;
    bool diag_rich;
    diag_style_t diag_style;
    diag_format_t diag_format;
    const char *src_text;
    size_t src_len;
    diag_source_entry_t src_table[LOGGER_MAX_DIAG_SOURCES];
    int src_table_len;
};

#endif /* SALAM_LOGGER_INTERNAL_H */
