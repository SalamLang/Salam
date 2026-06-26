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

static const char *const k_level_names[] = {
    "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "OFF"
};
static const char *const k_phase_names[] = {
    "DRIVER", "CLI", "LEXER", "PARSER", "SEMANTIC", "IR", "CODEGEN"
};
const char *log_level_name(log_level_t level)
{
    if (level < LOG_TRACE || level > LOG_OFF) return "?";
    return k_level_names[level];
}

const char *log_phase_name(phase_t phase)
{
    size_t n = sizeof(k_phase_names) / sizeof(k_phase_names[0]);
    if ((size_t)phase >= n) return "?";
    return k_phase_names[phase];
}

bool log_level_from_string(const char *s, log_level_t *out)
{
    if (!s) return false;
    struct { const char *name; log_level_t level; } table[] = {
        { "trace", LOG_TRACE }, { "debug", LOG_DEBUG }, { "info", LOG_INFO },
        { "warn", LOG_WARN }, { "warning", LOG_WARN }, { "error", LOG_ERROR },
        { "off", LOG_OFF },
    };
    { size_t i = 0; for (; i < sizeof(table) / sizeof(table[0]); i++) {
#if defined(_WIN32)
        if (_stricmp(s, table[i].name) == 0) { *out = table[i].level; return true; }
#else
        if (strcasecmp(s, table[i].name) == 0) { *out = table[i].level; return true; }
#endif
    } }
    return false;
}
