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

#ifndef SALAM_CORE_STATUS_H
#define SALAM_CORE_STATUS_H

typedef enum {
    ST_OK = 0,   /* success */
    ST_ERROR,    /* recoverable failure; a diagnostic was/should be reported */
    ST_FATAL     /* unrecoverable (out of memory, missing file): abort phase */
} status_t;

#endif /* SALAM_CORE_STATUS_H */
