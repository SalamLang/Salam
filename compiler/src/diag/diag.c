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
#include "diag/diag.h"
#include "i18n/i18n.h"
#include "core/sal_format.h"

diag_engine_t *diag_new(arena_t *a, logger_t *log, phase_t phase)
{
    diag_engine_t *e = (diag_engine_t *)arena_alloc(a, sizeof(*e));
    vec_init(&e->items);
    e->errors = 0;
    e->warnings = 0;
    e->a = a;
    e->log = log;
    e->phase = phase;
    return e;
}

void diag_report(diag_engine_t *e, severity_t sev, int code, const char *file,
                 const src_span_t *span, const char *fmt, ...)
{
    char body[512];
    va_list ap;
    va_start(ap, fmt);
    sal_vsnprintf(body, sizeof(body), i18n_tr(fmt), ap);
    va_end(ap);
    char full[640];
    sal_snprintf(full, sizeof(full), "%c%03d: %s", sev == SEV_ERROR ? 'E' : 'W', code,
                 body);
    diag_t *d = (diag_t *)arena_alloc(e->a, sizeof(*d));
    d->sev = sev;
    d->code = code;
    d->file = file;
    if (span)
        d->span = *span;
    else
        memset(&d->span, 0, sizeof(d->span));
    d->message = arena_strdup(e->a, full);
    vec_push(e->a, &e->items, d);
    if (sev == SEV_ERROR)
        e->errors++;
    else
        e->warnings++;
    logger_log(e->log, e->phase, sev == SEV_ERROR ? LOG_ERROR : LOG_WARN,
               span ? file : NULL, span, "%s", full);
}

bool diag_has_errors(const diag_engine_t *e)
{
    return e->errors > 0;
}
