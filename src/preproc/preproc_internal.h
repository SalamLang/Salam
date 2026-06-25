#ifndef SALAM_PREPROC_INTERNAL_H
#define SALAM_PREPROC_INTERNAL_H
#include "core/prelude.h"

const char *pp_skip_ws(const char *p);
bool pp_eval(const char *cond, const char *const *defs, int n);
#endif /* SALAM_PREPROC_INTERNAL_H */
