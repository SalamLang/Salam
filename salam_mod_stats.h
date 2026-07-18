#pragma once
#ifndef SALAM_MODULE_stats_H
#  define SALAM_MODULE_stats_H
#  include <stdint.h>
#  include <stdbool.h>
#  include <math.h>
#  include <stddef.h>
#  ifndef SALAM_RT_TYPES_DEFINED
#    define SALAM_RT_TYPES_DEFINED
typedef struct salam_file salam_file;
typedef struct salam_map salam_map;
typedef struct salam_map_iter salam_map_iter;
typedef struct {
    void *data;
    int64_t len;
} salam_slice;
extern int64_t salam_idx(int64_t, int64_t);
extern void salam_panic(const char *msg);
void *_Salam_mem_Allocate_u64(uint64_t size);
void *_Salam_mem_Reallocate_void_ptr_u64(void *ptr, uint64_t size);
void _Salam_mem_Free_void_ptr(void *ptr);
static inline salam_slice salam_slice_new(void *b, int64_t lo, int64_t hi, int64_t esz)
{
    salam_slice s;
    s.data = (void *)((char *)b + (lo) * (esz));
    s.len = (hi) - (lo);
    return s;
}
static inline salam_slice salam_slice_sub(salam_slice b, int64_t lo, int64_t hi,
                                          int has_hi, int64_t esz)
{
    salam_slice s;
    if (!has_hi) hi = b.len;
    s.data = (void *)((char *)b.data + (lo) * (esz));
    s.len = (hi) - (lo);
    return s;
}
static inline void *salam_slice_at(salam_slice s, int64_t i, int64_t esz, int sf)
{
    return (void *)((char *)s.data + (sf ? salam_idx(i, s.len) : i) * (esz));
}
typedef void (*salam_thread_fn)(void);
#  endif
#  ifndef SALAM_RT_STR_DEFINED
#    define SALAM_RT_STR_DEFINED
extern uint64_t strlen(const char *s);
extern int32_t strcmp(const char *a, const char *b);
extern const char *strstr(const char *haystack, const char *needle);
extern int64_t strtol(const char *s, void *endptr, int32_t base);
extern double strtod(const char *s, void *endptr);
extern const char *salam_strcat(const char *a, const char *b);
extern const char *salam_char_from_code(int32_t c);
extern const char *salam_str_substr(const char *s, int32_t start, int32_t n);
extern const char *salam_str_trim(const char *s);
extern void *salam_str_split(const char *s, const char *delim, void *out_count);
#  endif
#  ifndef SALAM_FN_ATTRS_DEFINED
#    define SALAM_FN_ATTRS_DEFINED
#    if defined(__GNUC__) || defined(__clang__)
#      define SALAM_NOINLINE __attribute__((noinline))
#      define SALAM_PURE __attribute__((pure))
#      define SALAM_NORET __attribute__((noreturn))
#      define SALAM_DEPRECATED __attribute__((deprecated))
#    else
#      define SALAM_NOINLINE
#      define SALAM_PURE
#      define SALAM_NORET
#      define SALAM_DEPRECATED
#    endif
#  endif
#  include "salam_mod_core.h"
#  include "salam_mod_math.h"
#  include "salam_mod_sort.h"
#  include "salam_mod_collections.h"

#  ifndef SALAM_FWD_Vector__f64
#    define SALAM_FWD_Vector__f64
typedef struct Vector__f64 Vector__f64;
#  endif
#  ifndef SALAM_FWD_VectorIter__f64
#    define SALAM_FWD_VectorIter__f64
typedef struct VectorIter__f64 VectorIter__f64;
#  endif
#  ifndef SALAM_FWD_Vector__i32
#    define SALAM_FWD_Vector__i32
typedef struct Vector__i32 Vector__i32;
#  endif
#  ifndef SALAM_FWD_VectorIter__i32
#    define SALAM_FWD_VectorIter__i32
typedef struct VectorIter__i32 VectorIter__i32;
#  endif
#  ifndef SALAM_FWD_Vector__i64
#    define SALAM_FWD_Vector__i64
typedef struct Vector__i64 Vector__i64;
#  endif
#  ifndef SALAM_FWD_VectorIter__i64
#    define SALAM_FWD_VectorIter__i64
typedef struct VectorIter__i64 VectorIter__i64;
#  endif
#  ifndef SALAM_FWD_str__StringBuilder
#    define SALAM_FWD_str__StringBuilder
typedef struct str__StringBuilder str__StringBuilder;
#  endif
#  ifndef SALAM_FWD_Vector__str
#    define SALAM_FWD_Vector__str
typedef struct Vector__str Vector__str;
#  endif
#  ifndef SALAM_FWD_VectorIter__str
#    define SALAM_FWD_VectorIter__str
typedef struct VectorIter__str VectorIter__str;
#  endif
#  ifndef SALAM_FWD_thread__Thread
#    define SALAM_FWD_thread__Thread
typedef struct thread__Thread thread__Thread;
#  endif
#  ifndef SALAM_FWD_Vector__thread__Thread
#    define SALAM_FWD_Vector__thread__Thread
typedef struct Vector__thread__Thread Vector__thread__Thread;
#  endif
#  ifndef SALAM_FWD_VectorIter__thread__Thread
#    define SALAM_FWD_VectorIter__thread__Thread
typedef struct VectorIter__thread__Thread VectorIter__thread__Thread;
#  endif
#  ifndef SALAM_FWD_crypto__U64Pair
#    define SALAM_FWD_crypto__U64Pair
typedef struct crypto__U64Pair crypto__U64Pair;
#  endif
#  ifndef SALAM_FWD_crypto__Sha256State
#    define SALAM_FWD_crypto__Sha256State
typedef struct crypto__Sha256State crypto__Sha256State;
#  endif
#  ifndef SALAM_FWD_Vector__str
#    define SALAM_FWD_Vector__str
typedef struct Vector__str Vector__str;
#  endif
#  ifndef SALAM_FWD_VectorIter__str
#    define SALAM_FWD_VectorIter__str
typedef struct VectorIter__str VectorIter__str;
#  endif

#  ifndef SALAM_DEF_Vector__f64
#    define SALAM_DEF_Vector__f64
struct Vector__f64 {
    double *data;
    int32_t __len;
    int32_t __cap;
};
#  endif
#  ifndef SALAM_DEF_VectorIter__f64
#    define SALAM_DEF_VectorIter__f64
struct VectorIter__f64 {
    double *data;
    int32_t len;
    int32_t pos;
};
#  endif

double _Salam_stats_Sum_Vector__f64(Vector__f64 v);
double _Salam_stats_Mean_Vector__f64(Vector__f64 v);
double _Salam_stats_Min_Vector__f64(Vector__f64 v);
double _Salam_stats_Max_Vector__f64(Vector__f64 v);
void _Salam_stats___sorted_Vector__f64(Vector__f64 v, Vector__f64 *__ret);
double _Salam_stats_Median_Vector__f64(Vector__f64 v);
double _Salam_stats___variance_Vector__f64_bool(Vector__f64 v, bool sample);
double _Salam_stats_Variance_Vector__f64(Vector__f64 v);
double _Salam_stats_PopVariance_Vector__f64(Vector__f64 v);
double _Salam_stats_Stdev_Vector__f64(Vector__f64 v);
double _Salam_stats_PopStdev_Vector__f64(Vector__f64 v);
double _Salam_stats_Geomean_Vector__f64(Vector__f64 v);
double _Salam_stats_Percentile_Vector__f64_f64(Vector__f64 v, double p);
#  ifndef SALAM_IP__Salam_g_S_Vector__f64_reserve_i32
#    define SALAM_IP__Salam_g_S_Vector__f64_reserve_i32
static inline void _Salam_g_S_Vector__f64_reserve_i32(Vector__f64 *this, int32_t need);
#  endif
#  ifndef SALAM_IP__Salam_g_S_Vector__f64_push_f64
#    define SALAM_IP__Salam_g_S_Vector__f64_push_f64
static inline void _Salam_g_S_Vector__f64_push_f64(Vector__f64 *this, double x);
#  endif
#  ifndef SALAM_IP__Salam_g_S_Vector__f64_pop
#    define SALAM_IP__Salam_g_S_Vector__f64_pop
static inline double _Salam_g_S_Vector__f64_pop(Vector__f64 *this);
#  endif
#  ifndef SALAM_IP__Salam_g_S_Vector__f64_get_i32
#    define SALAM_IP__Salam_g_S_Vector__f64_get_i32
static inline double *_Salam_g_S_Vector__f64_get_i32(Vector__f64 *this, int32_t i);
#  endif
#  ifndef SALAM_IP__Salam_g_S_Vector__f64_set_i32_f64
#    define SALAM_IP__Salam_g_S_Vector__f64_set_i32_f64
static inline void _Salam_g_S_Vector__f64_set_i32_f64(Vector__f64 *this, int32_t i,
                                                      double x);
#  endif
#  ifndef SALAM_IP__Salam_g_S_Vector__f64_len
#    define SALAM_IP__Salam_g_S_Vector__f64_len
static inline int32_t _Salam_g_S_Vector__f64_len(Vector__f64 *this);
#  endif
#  ifndef SALAM_IP__Salam_g_S_Vector__f64_cap
#    define SALAM_IP__Salam_g_S_Vector__f64_cap
static inline int32_t _Salam_g_S_Vector__f64_cap(Vector__f64 *this);
#  endif
#  ifndef SALAM_IP__Salam_g_S_Vector__f64_is__empty
#    define SALAM_IP__Salam_g_S_Vector__f64_is__empty
static inline bool _Salam_g_S_Vector__f64_is__empty(Vector__f64 *this);
#  endif
#  ifndef SALAM_IP__Salam_g_S_Vector__f64_first
#    define SALAM_IP__Salam_g_S_Vector__f64_first
static inline double _Salam_g_S_Vector__f64_first(Vector__f64 *this);
#  endif
#  ifndef SALAM_IP__Salam_g_S_Vector__f64_last
#    define SALAM_IP__Salam_g_S_Vector__f64_last
static inline double _Salam_g_S_Vector__f64_last(Vector__f64 *this);
#  endif
#  ifndef SALAM_IP__Salam_g_S_Vector__f64_clear
#    define SALAM_IP__Salam_g_S_Vector__f64_clear
static inline void _Salam_g_S_Vector__f64_clear(Vector__f64 *this);
#  endif
#  ifndef SALAM_IP__Salam_g_S_Vector__f64_insert_i32_f64
#    define SALAM_IP__Salam_g_S_Vector__f64_insert_i32_f64
static inline void _Salam_g_S_Vector__f64_insert_i32_f64(Vector__f64 *this, int32_t i,
                                                         double x);
#  endif
#  ifndef SALAM_IP__Salam_g_S_Vector__f64_remove__at_i32
#    define SALAM_IP__Salam_g_S_Vector__f64_remove__at_i32
static inline double _Salam_g_S_Vector__f64_remove__at_i32(Vector__f64 *this, int32_t i);
#  endif
#  ifndef SALAM_IP__Salam_g_S_Vector__f64_operator__index_i32
#    define SALAM_IP__Salam_g_S_Vector__f64_operator__index_i32
static inline double _Salam_g_S_Vector__f64_operator__index_i32(Vector__f64 *this,
                                                                int32_t i);
#  endif
#  ifndef SALAM_IP__Salam_g_S_Vector__f64_operator__index__set_i32_f64
#    define SALAM_IP__Salam_g_S_Vector__f64_operator__index__set_i32_f64
static inline void _Salam_g_S_Vector__f64_operator__index__set_i32_f64(Vector__f64 *this,
                                                                       int32_t i,
                                                                       double x);
#  endif
#  ifndef SALAM_IP__Salam_g_S_Vector__f64_iter
#    define SALAM_IP__Salam_g_S_Vector__f64_iter
static inline void _Salam_g_S_Vector__f64_iter(Vector__f64 *this, VectorIter__f64 *__ret);
#  endif
#  ifndef SALAM_IP__Salam_g_S_Vector__f64_free
#    define SALAM_IP__Salam_g_S_Vector__f64_free
static inline void _Salam_g_S_Vector__f64_free(Vector__f64 *this);
#  endif
#  ifndef SALAM_IP__Salam_g_S_VectorIter__f64_has__next
#    define SALAM_IP__Salam_g_S_VectorIter__f64_has__next
static inline bool _Salam_g_S_VectorIter__f64_has__next(VectorIter__f64 *this);
#  endif
#  ifndef SALAM_IP__Salam_g_S_VectorIter__f64_value
#    define SALAM_IP__Salam_g_S_VectorIter__f64_value
static inline double _Salam_g_S_VectorIter__f64_value(VectorIter__f64 *this);
#  endif
#  ifndef SALAM_IP__Salam_g_S_VectorIter__f64_next
#    define SALAM_IP__Salam_g_S_VectorIter__f64_next
static inline void _Salam_g_S_VectorIter__f64_next(VectorIter__f64 *this);
#  endif
#  ifndef SALAM_IP__Salam_g_clone__f64_Vector__f64
#    define SALAM_IP__Salam_g_clone__f64_Vector__f64
static inline void _Salam_g_clone__f64_Vector__f64(Vector__f64 v, Vector__f64 *__ret);
#  endif
#  ifndef SALAM_IP__Salam_g_Sort__f64_Vector__f64
#    define SALAM_IP__Salam_g_Sort__f64_Vector__f64
static inline void _Salam_g_Sort__f64_Vector__f64(Vector__f64 v);
#  endif
#  ifndef SALAM_IP__Salam_g_IntroSort__f64_Vector__f64
#    define SALAM_IP__Salam_g_IntroSort__f64_Vector__f64
static inline void _Salam_g_IntroSort__f64_Vector__f64(Vector__f64 v);
#  endif
#  ifndef SALAM_IP__Salam_g___intro__f64_Vector__f64_i32_i32_i32
#    define SALAM_IP__Salam_g___intro__f64_Vector__f64_i32_i32_i32
static inline void _Salam_g___intro__f64_Vector__f64_i32_i32_i32(Vector__f64 v,
                                                                 int32_t lo__in,
                                                                 int32_t hi__in,
                                                                 int32_t depth__in);
#  endif
#  ifndef SALAM_IP__Salam_g___insertion__range__f64_Vector__f64_i32_i32
#    define SALAM_IP__Salam_g___insertion__range__f64_Vector__f64_i32_i32
static inline void _Salam_g___insertion__range__f64_Vector__f64_i32_i32(Vector__f64 v,
                                                                        int32_t lo,
                                                                        int32_t hi);
#  endif
#  ifndef SALAM_IP__Salam_g___heapsort__range__f64_Vector__f64_i32_i32
#    define SALAM_IP__Salam_g___heapsort__range__f64_Vector__f64_i32_i32
static inline void _Salam_g___heapsort__range__f64_Vector__f64_i32_i32(Vector__f64 v,
                                                                       int32_t lo,
                                                                       int32_t hi);
#  endif
#  ifndef SALAM_IP__Salam_g___partition__f64_Vector__f64_i32_i32
#    define SALAM_IP__Salam_g___partition__f64_Vector__f64_i32_i32
static inline int32_t
_Salam_g___partition__f64_Vector__f64_i32_i32(Vector__f64 v, int32_t lo, int32_t hi);
#  endif
#  ifndef SALAM_IP__Salam_g___sift__down__f64_Vector__f64_i32_i32_i32
#    define SALAM_IP__Salam_g___sift__down__f64_Vector__f64_i32_i32_i32
static inline void _Salam_g___sift__down__f64_Vector__f64_i32_i32_i32(Vector__f64 v,
                                                                      int32_t lo,
                                                                      int32_t n,
                                                                      int32_t start);
#  endif
#  ifndef SALAM_IP__Salam_g_Swap__f64_Vector__f64_i32_i32
#    define SALAM_IP__Salam_g_Swap__f64_Vector__f64_i32_i32
static inline void _Salam_g_Swap__f64_Vector__f64_i32_i32(Vector__f64 v, int32_t i,
                                                          int32_t j);
#  endif
#  ifndef SALAM_IP__Salam_g_swap__f64_Vector__f64_i32_i32
#    define SALAM_IP__Salam_g_swap__f64_Vector__f64_i32_i32
static inline void _Salam_g_swap__f64_Vector__f64_i32_i32(Vector__f64 v, int32_t i,
                                                          int32_t j);
#  endif
#  ifndef SALAM_IB__Salam_g_S_Vector__f64_reserve_i32
#    define SALAM_IB__Salam_g_S_Vector__f64_reserve_i32
static inline void _Salam_g_S_Vector__f64_reserve_i32(Vector__f64 *this, int32_t need)
{
    if ((need <= this->__cap)) {
        return;
    }
    int32_t c = this->__cap;
    if ((c == 0)) {
        c = 4;
    }
    while ((c < need)) {
        c = (c * 2);
    }
    this->data = ((double *)(_Salam_mem_Reallocate_void_ptr_u64(
        ((void *)(this->data)), (((uint64_t)(c)) * (uint64_t)sizeof(double)))));
    this->__cap = c;
}

#  endif
#  ifndef SALAM_IB__Salam_g_S_Vector__f64_push_f64
#    define SALAM_IB__Salam_g_S_Vector__f64_push_f64
static inline void _Salam_g_S_Vector__f64_push_f64(Vector__f64 *this, double x)
{
    _Salam_g_S_Vector__f64_reserve_i32(this, (this->__len + 1));
    double *d = this->data;
    d[this->__len] = x;
    this->__len += 1;
}

#  endif
#  ifndef SALAM_IB__Salam_g_S_Vector__f64_pop
#    define SALAM_IB__Salam_g_S_Vector__f64_pop
static inline double _Salam_g_S_Vector__f64_pop(Vector__f64 *this)
{
    if ((this->__len <= 0)) {
        salam_panic("Vector.pop: empty");
    }
    this->__len -= 1;
    double *d = this->data;
    return d[this->__len];
}

#  endif
#  ifndef SALAM_IB__Salam_g_S_Vector__f64_get_i32
#    define SALAM_IB__Salam_g_S_Vector__f64_get_i32
static inline double *_Salam_g_S_Vector__f64_get_i32(Vector__f64 *this, int32_t i)
{
    if (((i < 0) || (i >= this->__len))) {
        salam_panic("Vector.get: index out of bounds");
    }
    return (
        (double *)(intptr_t)((((int64_t)(intptr_t)(this->data)) +
                              (((int64_t)(i)) * ((int64_t)((uint64_t)sizeof(double)))))));
}

#  endif
#  ifndef SALAM_IB__Salam_g_S_Vector__f64_set_i32_f64
#    define SALAM_IB__Salam_g_S_Vector__f64_set_i32_f64
static inline void _Salam_g_S_Vector__f64_set_i32_f64(Vector__f64 *this, int32_t i,
                                                      double x)
{
    if ((i < 0)) {
        salam_panic("Vector.set: negative index");
    }
    _Salam_g_S_Vector__f64_reserve_i32(this, (i + 1));
    if ((i >= this->__len)) {
        this->__len = (i + 1);
    }
    double *d = this->data;
    d[i] = x;
}

#  endif
#  ifndef SALAM_IB__Salam_g_S_Vector__f64_len
#    define SALAM_IB__Salam_g_S_Vector__f64_len
static inline int32_t _Salam_g_S_Vector__f64_len(Vector__f64 *this)
{
    return this->__len;
}

#  endif
#  ifndef SALAM_IB__Salam_g_S_Vector__f64_cap
#    define SALAM_IB__Salam_g_S_Vector__f64_cap
static inline int32_t _Salam_g_S_Vector__f64_cap(Vector__f64 *this)
{
    return this->__cap;
}

#  endif
#  ifndef SALAM_IB__Salam_g_S_Vector__f64_is__empty
#    define SALAM_IB__Salam_g_S_Vector__f64_is__empty
static inline bool _Salam_g_S_Vector__f64_is__empty(Vector__f64 *this)
{
    return (this->__len == 0);
}

#  endif
#  ifndef SALAM_IB__Salam_g_S_Vector__f64_first
#    define SALAM_IB__Salam_g_S_Vector__f64_first
static inline double _Salam_g_S_Vector__f64_first(Vector__f64 *this)
{
    return _Salam_g_S_Vector__f64_get_i32(this, 0)[0];
}

#  endif
#  ifndef SALAM_IB__Salam_g_S_Vector__f64_last
#    define SALAM_IB__Salam_g_S_Vector__f64_last
static inline double _Salam_g_S_Vector__f64_last(Vector__f64 *this)
{
    return _Salam_g_S_Vector__f64_get_i32(this, (this->__len - 1))[0];
}

#  endif
#  ifndef SALAM_IB__Salam_g_S_Vector__f64_clear
#    define SALAM_IB__Salam_g_S_Vector__f64_clear
static inline void _Salam_g_S_Vector__f64_clear(Vector__f64 *this)
{
    this->__len = 0;
}

#  endif
#  ifndef SALAM_IB__Salam_g_S_Vector__f64_insert_i32_f64
#    define SALAM_IB__Salam_g_S_Vector__f64_insert_i32_f64
static inline void _Salam_g_S_Vector__f64_insert_i32_f64(Vector__f64 *this, int32_t i,
                                                         double x)
{
    if (((i < 0) || (i > this->__len))) {
        salam_panic("Vector.insert: index out of bounds");
    }
    _Salam_g_S_Vector__f64_reserve_i32(this, (this->__len + 1));
    double *d = this->data;
    int32_t j = this->__len;
    while ((j > i)) {
        d[j] = d[(j - 1)];
        j -= 1;
    }
    d[i] = x;
    this->__len += 1;
}

#  endif
#  ifndef SALAM_IB__Salam_g_S_Vector__f64_remove__at_i32
#    define SALAM_IB__Salam_g_S_Vector__f64_remove__at_i32
static inline double _Salam_g_S_Vector__f64_remove__at_i32(Vector__f64 *this, int32_t i)
{
    if (((i < 0) || (i >= this->__len))) {
        salam_panic("Vector.remove_at: index out of bounds");
    }
    double *d = this->data;
    const double out = d[i];
    int32_t j = i;
    while ((j < (this->__len - 1))) {
        d[j] = d[(j + 1)];
        j += 1;
    }
    this->__len -= 1;
    return out;
}

#  endif
#  ifndef SALAM_IB__Salam_g_S_Vector__f64_operator__index_i32
#    define SALAM_IB__Salam_g_S_Vector__f64_operator__index_i32
static inline double _Salam_g_S_Vector__f64_operator__index_i32(Vector__f64 *this,
                                                                int32_t i)
{
    return _Salam_g_S_Vector__f64_get_i32(this, i)[0];
}

#  endif
#  ifndef SALAM_IB__Salam_g_S_Vector__f64_operator__index__set_i32_f64
#    define SALAM_IB__Salam_g_S_Vector__f64_operator__index__set_i32_f64
static inline void _Salam_g_S_Vector__f64_operator__index__set_i32_f64(Vector__f64 *this,
                                                                       int32_t i,
                                                                       double x)
{
    _Salam_g_S_Vector__f64_set_i32_f64(this, i, x);
}

#  endif
#  ifndef SALAM_IB__Salam_g_S_Vector__f64_iter
#    define SALAM_IB__Salam_g_S_Vector__f64_iter
static inline void _Salam_g_S_Vector__f64_iter(Vector__f64 *this, VectorIter__f64 *__ret)
{
    VectorIter__f64 it = (VectorIter__f64){.data = NULL, .len = 0, .pos = 0};
    it.data = this->data;
    it.len = this->__len;
    it.pos = 0;
    *__ret = (it);
    return;
}

#  endif
#  ifndef SALAM_IB__Salam_g_S_Vector__f64_free
#    define SALAM_IB__Salam_g_S_Vector__f64_free
static inline void _Salam_g_S_Vector__f64_free(Vector__f64 *this)
{
    if ((this->data != NULL)) {
        _Salam_mem_Free_void_ptr(((void *)(this->data)));
    }
    this->data = NULL;
    this->__len = 0;
    this->__cap = 0;
}

#  endif
#  ifndef SALAM_IB__Salam_g_S_VectorIter__f64_has__next
#    define SALAM_IB__Salam_g_S_VectorIter__f64_has__next
static inline bool _Salam_g_S_VectorIter__f64_has__next(VectorIter__f64 *this)
{
    return (this->pos < this->len);
}

#  endif
#  ifndef SALAM_IB__Salam_g_S_VectorIter__f64_value
#    define SALAM_IB__Salam_g_S_VectorIter__f64_value
static inline double _Salam_g_S_VectorIter__f64_value(VectorIter__f64 *this)
{
    double *d = this->data;
    return d[this->pos];
}

#  endif
#  ifndef SALAM_IB__Salam_g_S_VectorIter__f64_next
#    define SALAM_IB__Salam_g_S_VectorIter__f64_next
static inline void _Salam_g_S_VectorIter__f64_next(VectorIter__f64 *this)
{
    this->pos = (this->pos + 1);
}

#  endif
#  ifndef SALAM_IB__Salam_g_clone__f64_Vector__f64
#    define SALAM_IB__Salam_g_clone__f64_Vector__f64
static inline void _Salam_g_clone__f64_Vector__f64(Vector__f64 v, Vector__f64 *__ret)
{
    Vector__f64 out = (Vector__f64){.data = NULL, .__len = 0, .__cap = 0};
    int32_t i = 0;
    while ((i < _Salam_g_S_Vector__f64_len(&(v)))) {
        _Salam_g_S_Vector__f64_push_f64(&(out),
                                        _Salam_g_S_Vector__f64_get_i32(&(v), i)[0]);
        i += 1;
    }
    *__ret = (out);
    return;
}

#  endif
#  ifndef SALAM_IB__Salam_g_Sort__f64_Vector__f64
#    define SALAM_IB__Salam_g_Sort__f64_Vector__f64
static inline void _Salam_g_Sort__f64_Vector__f64(Vector__f64 v)
{
    _Salam_g_IntroSort__f64_Vector__f64(v);
}

#  endif
#  ifndef SALAM_IB__Salam_g_IntroSort__f64_Vector__f64
#    define SALAM_IB__Salam_g_IntroSort__f64_Vector__f64
static inline void _Salam_g_IntroSort__f64_Vector__f64(Vector__f64 v)
{
    const int32_t n = _Salam_g_S_Vector__f64_len(&(v));
    if ((n < 2)) {
        return;
    }
    int32_t lg = 0;
    int32_t m = n;
    while ((m > 1)) {
        m /= 2;
        lg += 1;
    }
    _Salam_g___intro__f64_Vector__f64_i32_i32_i32(v, 0, (n - 1), (2 * lg));
    _Salam_g___insertion__range__f64_Vector__f64_i32_i32(v, 0, (n - 1));
}

#  endif
#  ifndef SALAM_IB__Salam_g___intro__f64_Vector__f64_i32_i32_i32
#    define SALAM_IB__Salam_g___intro__f64_Vector__f64_i32_i32_i32
static inline void _Salam_g___intro__f64_Vector__f64_i32_i32_i32(Vector__f64 v,
                                                                 int32_t lo__in,
                                                                 int32_t hi__in,
                                                                 int32_t depth__in)
{
    int32_t lo = lo__in;
    int32_t hi = hi__in;
    int32_t depth = depth__in;
    while (((hi - lo) > 16)) {
        if ((depth == 0)) {
            _Salam_g___heapsort__range__f64_Vector__f64_i32_i32(v, lo, hi);
            return;
        }
        depth -= 1;
        const int32_t p = _Salam_g___partition__f64_Vector__f64_i32_i32(v, lo, hi);
        _Salam_g___intro__f64_Vector__f64_i32_i32_i32(v, (p + 1), hi, depth);
        hi = (p - 1);
    }
}

#  endif
#  ifndef SALAM_IB__Salam_g___insertion__range__f64_Vector__f64_i32_i32
#    define SALAM_IB__Salam_g___insertion__range__f64_Vector__f64_i32_i32
static inline void _Salam_g___insertion__range__f64_Vector__f64_i32_i32(Vector__f64 v,
                                                                        int32_t lo,
                                                                        int32_t hi)
{
    int32_t i = (lo + 1);
    while ((i <= hi)) {
        const double key = _Salam_g_S_Vector__f64_get_i32(&(v), i)[0];
        int32_t j = (i - 1);
        bool go = true;
        while (go) {
            if ((j < lo)) {
                go = false;
            } else {
                if ((key < _Salam_g_S_Vector__f64_get_i32(&(v), j)[0])) {
                    _Salam_g_S_Vector__f64_set_i32_f64(
                        &(v), (j + 1), _Salam_g_S_Vector__f64_get_i32(&(v), j)[0]);
                    j -= 1;
                } else {
                    go = false;
                }
            }
        }
        _Salam_g_S_Vector__f64_set_i32_f64(&(v), (j + 1), key);
        i += 1;
    }
}

#  endif
#  ifndef SALAM_IB__Salam_g___heapsort__range__f64_Vector__f64_i32_i32
#    define SALAM_IB__Salam_g___heapsort__range__f64_Vector__f64_i32_i32
static inline void
_Salam_g___heapsort__range__f64_Vector__f64_i32_i32(Vector__f64 v, int32_t lo, int32_t hi)
{
    const int32_t n = ((hi - lo) + 1);
    if ((n < 2)) {
        return;
    }
    int32_t i = ((n / 2) - 1);
    while ((i >= 0)) {
        _Salam_g___sift__down__f64_Vector__f64_i32_i32_i32(v, lo, n, i);
        i -= 1;
    }
    int32_t e = (n - 1);
    while ((e > 0)) {
        _Salam_g_Swap__f64_Vector__f64_i32_i32(v, lo, (lo + e));
        _Salam_g___sift__down__f64_Vector__f64_i32_i32_i32(v, lo, e, 0);
        e -= 1;
    }
}

#  endif
#  ifndef SALAM_IB__Salam_g___partition__f64_Vector__f64_i32_i32
#    define SALAM_IB__Salam_g___partition__f64_Vector__f64_i32_i32
static inline int32_t
_Salam_g___partition__f64_Vector__f64_i32_i32(Vector__f64 v, int32_t lo, int32_t hi)
{
    const int32_t mid = (lo + ((hi - lo) / 2));
    if ((_Salam_g_S_Vector__f64_get_i32(&(v), mid)[0] <
         _Salam_g_S_Vector__f64_get_i32(&(v), lo)[0])) {
        _Salam_g_Swap__f64_Vector__f64_i32_i32(v, lo, mid);
    }
    if ((_Salam_g_S_Vector__f64_get_i32(&(v), hi)[0] <
         _Salam_g_S_Vector__f64_get_i32(&(v), lo)[0])) {
        _Salam_g_Swap__f64_Vector__f64_i32_i32(v, lo, hi);
    }
    if ((_Salam_g_S_Vector__f64_get_i32(&(v), hi)[0] <
         _Salam_g_S_Vector__f64_get_i32(&(v), mid)[0])) {
        _Salam_g_Swap__f64_Vector__f64_i32_i32(v, mid, hi);
    }
    _Salam_g_Swap__f64_Vector__f64_i32_i32(v, mid, hi);
    const double pivot = _Salam_g_S_Vector__f64_get_i32(&(v), hi)[0];
    int32_t i = (lo - 1);
    int32_t j = lo;
    while ((j < hi)) {
        if ((_Salam_g_S_Vector__f64_get_i32(&(v), j)[0] < pivot)) {
            i += 1;
            _Salam_g_Swap__f64_Vector__f64_i32_i32(v, i, j);
        }
        j += 1;
    }
    _Salam_g_Swap__f64_Vector__f64_i32_i32(v, (i + 1), hi);
    return (i + 1);
}

#  endif
#  ifndef SALAM_IB__Salam_g___sift__down__f64_Vector__f64_i32_i32_i32
#    define SALAM_IB__Salam_g___sift__down__f64_Vector__f64_i32_i32_i32
static inline void _Salam_g___sift__down__f64_Vector__f64_i32_i32_i32(Vector__f64 v,
                                                                      int32_t lo,
                                                                      int32_t n,
                                                                      int32_t start)
{
    int32_t i = start;
    bool go = true;
    while (go) {
        const int32_t c = ((2 * i) + 1);
        if ((c >= n)) {
            go = false;
        } else {
            int32_t big = c;
            if (((c + 1) < n)) {
                if ((_Salam_g_S_Vector__f64_get_i32(&(v), (lo + c))[0] <
                     _Salam_g_S_Vector__f64_get_i32(&(v), ((lo + c) + 1))[0])) {
                    big = (c + 1);
                }
            }
            if ((_Salam_g_S_Vector__f64_get_i32(&(v), (lo + i))[0] <
                 _Salam_g_S_Vector__f64_get_i32(&(v), (lo + big))[0])) {
                _Salam_g_Swap__f64_Vector__f64_i32_i32(v, (lo + i), (lo + big));
                i = big;
            } else {
                go = false;
            }
        }
    }
}

#  endif
#  ifndef SALAM_IB__Salam_g_Swap__f64_Vector__f64_i32_i32
#    define SALAM_IB__Salam_g_Swap__f64_Vector__f64_i32_i32
static inline void _Salam_g_Swap__f64_Vector__f64_i32_i32(Vector__f64 v, int32_t i,
                                                          int32_t j)
{
    _Salam_g_swap__f64_Vector__f64_i32_i32(v, i, j);
}

#  endif
#  ifndef SALAM_IB__Salam_g_swap__f64_Vector__f64_i32_i32
#    define SALAM_IB__Salam_g_swap__f64_Vector__f64_i32_i32
static inline void _Salam_g_swap__f64_Vector__f64_i32_i32(Vector__f64 v, int32_t i,
                                                          int32_t j)
{
    const double t = _Salam_g_S_Vector__f64_get_i32(&(v), i)[0];
    _Salam_g_S_Vector__f64_set_i32_f64(&(v), i,
                                       _Salam_g_S_Vector__f64_get_i32(&(v), j)[0]);
    _Salam_g_S_Vector__f64_set_i32_f64(&(v), j, t);
}

#  endif

#endif
