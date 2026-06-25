#ifndef SALAM_CORE_STATUS_H
#define SALAM_CORE_STATUS_H

typedef enum {
    ST_OK = 0,   /* success */
    ST_ERROR,    /* recoverable failure; a diagnostic was/should be reported */
    ST_FATAL     /* unrecoverable (out of memory, missing file): abort phase */
} status_t;

#endif /* SALAM_CORE_STATUS_H */
