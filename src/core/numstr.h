#ifndef SALAM_CORE_NUMSTR_H
#define SALAM_CORE_NUMSTR_H

#include "core/prelude.h"

#define SAL_NUMSTR_MAX 21

char *sal_u64toa(uint64_t v, char *buf);

char *sal_i64toa(int64_t  v, char *buf);

#endif /* SALAM_CORE_NUMSTR_H */
