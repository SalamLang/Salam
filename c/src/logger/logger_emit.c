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
#include "diag/diag_render.h"
#include "i18n/i18n.h"
#include "core/sal_format.h"
#include <time.h>

static const char *const k_level_colors[] = {"\033[90m", "\033[36m", "\033[32m",
                                             "\033[33m", "\033[31m", "\033[0m"};
#define RESET_COLOR "\033[0m"
static void logger_write_timestamp(FILE *sink)
{
    time_t now = time(NULL);

    struct tm *tmv = localtime(&now);
    char buf[24];
    if (tmv && strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", tmv) > 0) {
        fprintf(sink, "[%s]", buf);
    }
}

static void logger_emit_rich(logger_t *lg, log_level_t level, const char *file,
                             const src_span_t *span, const char *fmt, va_list ap)
{
    char msg_buf[640];
    sal_vsnprintf(msg_buf, sizeof(msg_buf), fmt, ap);

    int code = 0;
    bool is_error = (level == LOG_ERROR);
    const char *body = msg_buf;
    if ((msg_buf[0] == 'E' || msg_buf[0] == 'W') && (unsigned char)msg_buf[1] >= '0' &&
        (unsigned char)msg_buf[1] <= '9' && (unsigned char)msg_buf[2] >= '0' &&
        (unsigned char)msg_buf[2] <= '9' && (unsigned char)msg_buf[3] >= '0' &&
        (unsigned char)msg_buf[3] <= '9' && msg_buf[4] == ':' && msg_buf[5] == ' ') {
        is_error = (msg_buf[0] == 'E');
        code = (msg_buf[1] - '0') * 100 + (msg_buf[2] - '0') * 10 + (msg_buf[3] - '0');
        body = msg_buf + 6;
    }
    const char *diag_level = is_error ? "error" : "warning";
    diag_render(lg->sink, diag_level, code, is_error, file, span, lg->src_text,
                lg->src_len, body, NULL, lg->diag_style, lg->diag_format, lg->color);
}

static void logger_emit_plain(logger_t *lg, phase_t phase, log_level_t level,
                              const char *file, const src_span_t *span, const char *fmt,
                              va_list ap)
{
    if (lg->timestamps) logger_write_timestamp(lg->sink);
    if (lg->color) {
        fprintf(lg->sink, "[%s][%s%s%s] ", i18n_tr(log_phase_name(phase)),
                k_level_colors[level], i18n_tr(log_level_name(level)), RESET_COLOR);
    } else {
        fprintf(lg->sink, "[%s][%s] ", i18n_tr(log_phase_name(phase)),
                i18n_tr(log_level_name(level)));
    }

    sal_vfprintf(lg->sink, i18n_tr(fmt), ap);
    if (file && span) {
        fprintf(lg->sink, " (%s:%u:%u)", file, span->begin.line, span->begin.col);
    }
    fputc('\n', lg->sink);
}

void logger_log(logger_t *lg, phase_t phase, log_level_t level, const char *file,
                const src_span_t *span, const char *fmt, ...)
{
    if (level < lg->min_level || lg->min_level == LOG_OFF) return;
    va_list ap;
    va_start(ap, fmt);
    if (lg->diag_rich && file && span && (level == LOG_ERROR || level == LOG_WARN)) {
        logger_emit_rich(lg, level, file, span, fmt, ap);
    } else {
        logger_emit_plain(lg, phase, level, file, span, fmt, ap);
    }
    va_end(ap);
}
