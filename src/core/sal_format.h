#ifndef SALAM_CORE_SAL_FORMAT_H
#define SALAM_CORE_SAL_FORMAT_H

#include "core/prelude.h"
#include <stdarg.h>

int  sal_vsnprintf(char *buf, size_t cap, const char *fmt, va_list ap);

int  sal_snprintf (char *buf, size_t cap, const char *fmt, ...);

void sal_vfprintf (FILE *f, const char *fmt, va_list ap);

#endif /* SALAM_CORE_SAL_FORMAT_H */
