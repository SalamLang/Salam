#ifndef SALAM_LOGGER_LOGGER_H
#define SALAM_LOGGER_LOGGER_H

#include "core/prelude.h"
#include "core/span.h"
#include "diag/diag_render.h"

typedef enum {
    LOG_TRACE = 0,
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR,
    LOG_OFF        /* sentinel: silences everything */
} log_level_t;

typedef enum {
    PH_DRIVER = 0,
    PH_CLI,
    PH_LEXER,
    PH_PARSER,
    PH_SEMANTIC,
    PH_IR,
    PH_CODEGEN
} phase_t;

typedef struct logger logger_t;

logger_t *logger_new(FILE *sink, log_level_t min_level, bool color);

void      logger_free(logger_t *lg);

void logger_set_level(logger_t *lg, log_level_t min_level);

log_level_t logger_get_level(const logger_t *lg);

void logger_set_timestamps(logger_t *lg, bool enabled);

void logger_set_diag_source(logger_t *lg,
                             const char   *src_text,
                             size_t        src_len,
                             diag_style_t  style,
                             diag_format_t format);

void logger_log(logger_t *lg, phase_t phase, log_level_t level,
                const char *file, const src_span_t *span, const char *fmt, ...);

bool log_level_from_string(const char *s, log_level_t *out);

const char *log_level_name(log_level_t level);

const char *log_phase_name(phase_t phase);

#define LOG_T(lg, ph, ...) logger_log((lg), (ph), LOG_TRACE, NULL, NULL, __VA_ARGS__)

#define LOG_D(lg, ph, ...) logger_log((lg), (ph), LOG_DEBUG, NULL, NULL, __VA_ARGS__)

#define LOG_I(lg, ph, ...) logger_log((lg), (ph), LOG_INFO,  NULL, NULL, __VA_ARGS__)

#define LOG_W(lg, ph, ...) logger_log((lg), (ph), LOG_WARN,  NULL, NULL, __VA_ARGS__)

#define LOG_E(lg, ph, ...) logger_log((lg), (ph), LOG_ERROR, NULL, NULL, __VA_ARGS__)

#define LOG_T_AT(lg, ph, file, span, ...) logger_log((lg), (ph), LOG_TRACE, (file), &(span), __VA_ARGS__)

#define LOG_D_AT(lg, ph, file, span, ...) logger_log((lg), (ph), LOG_DEBUG, (file), &(span), __VA_ARGS__)

#define LOG_W_AT(lg, ph, file, span, ...) logger_log((lg), (ph), LOG_WARN,  (file), &(span), __VA_ARGS__)

#define LOG_E_AT(lg, ph, file, span, ...) logger_log((lg), (ph), LOG_ERROR, (file), &(span), __VA_ARGS__)

#endif /* SALAM_LOGGER_LOGGER_H */
