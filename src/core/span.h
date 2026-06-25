#ifndef SALAM_CORE_SPAN_H
#define SALAM_CORE_SPAN_H
#include "core/prelude.h"
typedef struct {
    uint32_t line;
    uint32_t col;
    uint32_t index;
} src_pos_t;
typedef struct {
    src_pos_t begin;
    src_pos_t end;
    uint32_t  length;
} src_span_t;
#endif /* SALAM_CORE_SPAN_H */
