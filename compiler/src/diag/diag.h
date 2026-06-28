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

#ifndef SALAM_DIAG_DIAG_H
#define SALAM_DIAG_DIAG_H

#include "core/prelude.h"
#include "core/arena.h"
#include "core/vec.h"
#include "core/span.h"
#include "logger/logger.h"

typedef enum { SEV_ERROR = 0, SEV_WARNING } severity_t;

typedef struct {
    severity_t  sev;
    int         code;
    src_span_t  span;
    const char *file;
    const char *message;
} diag_t;

typedef struct {
    vec_t      items;
    size_t     errors;
    size_t     warnings;
    arena_t   *a;
    logger_t  *log;
    phase_t    phase;
} diag_engine_t;

diag_engine_t *diag_new(arena_t *a, logger_t *log, phase_t phase);

void diag_report(diag_engine_t *e, severity_t sev, int code,
                 const char *file, const src_span_t *span, const char *fmt, ...);

bool diag_has_errors(const diag_engine_t *e);

#endif /* SALAM_DIAG_DIAG_H */
