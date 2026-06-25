#ifndef SALAM_FMT_INTERNAL_H
#define SALAM_FMT_INTERNAL_H
#include "core/prelude.h"
#include "token/token.h"

#define FMT_INDENT_WIDTH 4

bool fmt_is_value_end(token_kind_t k);
bool fmt_is_open(token_kind_t k);
bool fmt_is_close(token_kind_t k);
bool fmt_gap_in_source(const token_t *a, const token_t *b);
bool fmt_need_space(const token_t *pt, const token_t *ct, bool prev_unary);
bool fmt_is_prefix(token_kind_t k, const token_t *prev);
#endif /* SALAM_FMT_INTERNAL_H */
