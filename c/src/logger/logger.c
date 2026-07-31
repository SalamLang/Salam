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

#include "core/prelude.h"
#include "logger/logger.h"
#include "logger/logger_internal.h"

logger_t *logger_new(FILE *sink, log_level_t min_level, bool color)
{
    logger_t *lg = (logger_t *)malloc(sizeof(*lg));
    if (!lg) abort();
    lg->sink = sink ? sink : stderr;
    lg->min_level = min_level;
    lg->color = color;
    lg->timestamps = true;
    lg->diag_rich = false;
    lg->diag_style = DIAG_STYLE_RUST;
    lg->diag_format = DIAG_FORMAT_HUMAN;
    lg->src_text = NULL;
    lg->src_len = 0;
    lg->src_table_len = 0;
    return lg;
}

void logger_free(logger_t *lg)
{
    free(lg);
}

void logger_set_diag_source(logger_t *lg, const char *src_text, size_t src_len,
                            diag_style_t style, diag_format_t format)
{
    lg->diag_rich = (src_text != NULL);
    lg->src_text = src_text;
    lg->src_len = src_len;
    lg->diag_style = style;
    lg->diag_format = format;
}

void logger_add_diag_source(logger_t *lg, const char *file, const char *src_text,
                            size_t src_len)
{
    if (!lg || !file) return;
    {
        int i = 0;
        for (; i < lg->src_table_len; i++) {
            if (strcmp(lg->src_table[i].file, file) == 0) {
                lg->src_table[i].text = src_text;
                lg->src_table[i].len = src_len;
                return;
            }
        }
    }
    if (lg->src_table_len < LOGGER_MAX_DIAG_SOURCES) {
        lg->src_table[lg->src_table_len].file = file;
        lg->src_table[lg->src_table_len].text = src_text;
        lg->src_table[lg->src_table_len].len = src_len;
        lg->src_table_len++;
    }
}

void logger_set_timestamps(logger_t *lg, bool enabled)
{
    lg->timestamps = enabled;
}

void logger_set_level(logger_t *lg, log_level_t min_level)
{
    lg->min_level = min_level;
}

log_level_t logger_get_level(const logger_t *lg)
{
    return lg->min_level;
}
