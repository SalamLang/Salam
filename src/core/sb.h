#ifndef SALAM_CORE_SB_H
#define SALAM_CORE_SB_H

#include "core/prelude.h"

typedef struct {
    char  *data;
    size_t len;
    size_t cap;
} sb_t;

void        sb_init(sb_t *s);

void        sb_free(sb_t *s);

void        sb_putc(sb_t *s, char c);

void        sb_puts(sb_t *s, const char *str);

void        sb_printf(sb_t *s, const char *fmt, ...);

const char *sb_cstr(const sb_t *s);   /* never NULL; "" when empty */

#endif /* SALAM_CORE_SB_H */
