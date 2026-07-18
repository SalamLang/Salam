#pragma once
#ifndef SALAM_MODULE_sort_H
#  define SALAM_MODULE_sort_H
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
#  include "salam_mod_collections.h"

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

#  ifndef SALAM_DEF_Vector__i32
#    define SALAM_DEF_Vector__i32
struct Vector__i32 {
    int32_t *data;
    int32_t __len;
    int32_t __cap;
};
#  endif
#  ifndef SALAM_DEF_VectorIter__i32
#    define SALAM_DEF_VectorIter__i32
struct VectorIter__i32 {
    int32_t *data;
    int32_t len;
    int32_t pos;
};
#  endif
#  ifndef SALAM_DEF_Vector__i64
#    define SALAM_DEF_Vector__i64
struct Vector__i64 {
    int64_t *data;
    int32_t __len;
    int32_t __cap;
};
#  endif
#  ifndef SALAM_DEF_VectorIter__i64
#    define SALAM_DEF_VectorIter__i64
struct VectorIter__i64 {
    int64_t *data;
    int32_t len;
    int32_t pos;
};
#  endif

void _Salam_sort_BucketSort_Vector__i32(Vector__i32 v);
int32_t _Salam_sort___bucket__of_i32_i32_i64_i32(int32_t x, int32_t lo, int64_t span,
                                                 int32_t n);
void _Salam_sort_CountingSort_Vector__i32(Vector__i32 v);
void _Salam_sort_RadixSort_Vector__i32(Vector__i32 v);
#  ifndef SALAM_IP__Salam_g_S_Vector__i32_reserve_i32
#    define SALAM_IP__Salam_g_S_Vector__i32_reserve_i32
static inline void _Salam_g_S_Vector__i32_reserve_i32(Vector__i32 *this, int32_t need);
#  endif
#  ifndef SALAM_IP__Salam_g_S_Vector__i32_push_i32
#    define SALAM_IP__Salam_g_S_Vector__i32_push_i32
static inline void _Salam_g_S_Vector__i32_push_i32(Vector__i32 *this, int32_t x);
#  endif
#  ifndef SALAM_IP__Salam_g_S_Vector__i32_pop
#    define SALAM_IP__Salam_g_S_Vector__i32_pop
static inline int32_t _Salam_g_S_Vector__i32_pop(Vector__i32 *this);
#  endif
#  ifndef SALAM_IP__Salam_g_S_Vector__i32_get_i32
#    define SALAM_IP__Salam_g_S_Vector__i32_get_i32
static inline int32_t *_Salam_g_S_Vector__i32_get_i32(Vector__i32 *this, int32_t i);
#  endif
#  ifndef SALAM_IP__Salam_g_S_Vector__i32_set_i32_i32
#    define SALAM_IP__Salam_g_S_Vector__i32_set_i32_i32
static inline void _Salam_g_S_Vector__i32_set_i32_i32(Vector__i32 *this, int32_t i,
                                                      int32_t x);
#  endif
#  ifndef SALAM_IP__Salam_g_S_Vector__i32_len
#    define SALAM_IP__Salam_g_S_Vector__i32_len
static inline int32_t _Salam_g_S_Vector__i32_len(Vector__i32 *this);
#  endif
#  ifndef SALAM_IP__Salam_g_S_Vector__i32_cap
#    define SALAM_IP__Salam_g_S_Vector__i32_cap
static inline int32_t _Salam_g_S_Vector__i32_cap(Vector__i32 *this);
#  endif
#  ifndef SALAM_IP__Salam_g_S_Vector__i32_is__empty
#    define SALAM_IP__Salam_g_S_Vector__i32_is__empty
static inline bool _Salam_g_S_Vector__i32_is__empty(Vector__i32 *this);
#  endif
#  ifndef SALAM_IP__Salam_g_S_Vector__i32_first
#    define SALAM_IP__Salam_g_S_Vector__i32_first
static inline int32_t _Salam_g_S_Vector__i32_first(Vector__i32 *this);
#  endif
#  ifndef SALAM_IP__Salam_g_S_Vector__i32_last
#    define SALAM_IP__Salam_g_S_Vector__i32_last
static inline int32_t _Salam_g_S_Vector__i32_last(Vector__i32 *this);
#  endif
#  ifndef SALAM_IP__Salam_g_S_Vector__i32_clear
#    define SALAM_IP__Salam_g_S_Vector__i32_clear
static inline void _Salam_g_S_Vector__i32_clear(Vector__i32 *this);
#  endif
#  ifndef SALAM_IP__Salam_g_S_Vector__i32_insert_i32_i32
#    define SALAM_IP__Salam_g_S_Vector__i32_insert_i32_i32
static inline void _Salam_g_S_Vector__i32_insert_i32_i32(Vector__i32 *this, int32_t i,
                                                         int32_t x);
#  endif
#  ifndef SALAM_IP__Salam_g_S_Vector__i32_remove__at_i32
#    define SALAM_IP__Salam_g_S_Vector__i32_remove__at_i32
static inline int32_t _Salam_g_S_Vector__i32_remove__at_i32(Vector__i32 *this, int32_t i);
#  endif
#  ifndef SALAM_IP__Salam_g_S_Vector__i32_operator__index_i32
#    define SALAM_IP__Salam_g_S_Vector__i32_operator__index_i32
static inline int32_t _Salam_g_S_Vector__i32_operator__index_i32(Vector__i32 *this,
                                                                 int32_t i);
#  endif
#  ifndef SALAM_IP__Salam_g_S_Vector__i32_operator__index__set_i32_i32
#    define SALAM_IP__Salam_g_S_Vector__i32_operator__index__set_i32_i32
static inline void _Salam_g_S_Vector__i32_operator__index__set_i32_i32(Vector__i32 *this,
                                                                       int32_t i,
                                                                       int32_t x);
#  endif
#  ifndef SALAM_IP__Salam_g_S_Vector__i32_iter
#    define SALAM_IP__Salam_g_S_Vector__i32_iter
static inline void _Salam_g_S_Vector__i32_iter(Vector__i32 *this, VectorIter__i32 *__ret);
#  endif
#  ifndef SALAM_IP__Salam_g_S_Vector__i32_free
#    define SALAM_IP__Salam_g_S_Vector__i32_free
static inline void _Salam_g_S_Vector__i32_free(Vector__i32 *this);
#  endif
#  ifndef SALAM_IP__Salam_g_S_VectorIter__i32_has__next
#    define SALAM_IP__Salam_g_S_VectorIter__i32_has__next
static inline bool _Salam_g_S_VectorIter__i32_has__next(VectorIter__i32 *this);
#  endif
#  ifndef SALAM_IP__Salam_g_S_VectorIter__i32_value
#    define SALAM_IP__Salam_g_S_VectorIter__i32_value
static inline int32_t _Salam_g_S_VectorIter__i32_value(VectorIter__i32 *this);
#  endif
#  ifndef SALAM_IP__Salam_g_S_VectorIter__i32_next
#    define SALAM_IP__Salam_g_S_VectorIter__i32_next
static inline void _Salam_g_S_VectorIter__i32_next(VectorIter__i32 *this);
#  endif
#  ifndef SALAM_IP__Salam_g___insertion__range__i32_Vector__i32_i32_i32
#    define SALAM_IP__Salam_g___insertion__range__i32_Vector__i32_i32_i32
static inline void _Salam_g___insertion__range__i32_Vector__i32_i32_i32(Vector__i32 v,
                                                                        int32_t lo,
                                                                        int32_t hi);
#  endif
#  ifndef SALAM_IP__Salam_g_S_Vector__i64_reserve_i32
#    define SALAM_IP__Salam_g_S_Vector__i64_reserve_i32
static inline void _Salam_g_S_Vector__i64_reserve_i32(Vector__i64 *this, int32_t need);
#  endif
#  ifndef SALAM_IP__Salam_g_S_Vector__i64_push_i64
#    define SALAM_IP__Salam_g_S_Vector__i64_push_i64
static inline void _Salam_g_S_Vector__i64_push_i64(Vector__i64 *this, int64_t x);
#  endif
#  ifndef SALAM_IP__Salam_g_S_Vector__i64_pop
#    define SALAM_IP__Salam_g_S_Vector__i64_pop
static inline int64_t _Salam_g_S_Vector__i64_pop(Vector__i64 *this);
#  endif
#  ifndef SALAM_IP__Salam_g_S_Vector__i64_get_i32
#    define SALAM_IP__Salam_g_S_Vector__i64_get_i32
static inline int64_t *_Salam_g_S_Vector__i64_get_i32(Vector__i64 *this, int32_t i);
#  endif
#  ifndef SALAM_IP__Salam_g_S_Vector__i64_set_i32_i64
#    define SALAM_IP__Salam_g_S_Vector__i64_set_i32_i64
static inline void _Salam_g_S_Vector__i64_set_i32_i64(Vector__i64 *this, int32_t i,
                                                      int64_t x);
#  endif
#  ifndef SALAM_IP__Salam_g_S_Vector__i64_len
#    define SALAM_IP__Salam_g_S_Vector__i64_len
static inline int32_t _Salam_g_S_Vector__i64_len(Vector__i64 *this);
#  endif
#  ifndef SALAM_IP__Salam_g_S_Vector__i64_cap
#    define SALAM_IP__Salam_g_S_Vector__i64_cap
static inline int32_t _Salam_g_S_Vector__i64_cap(Vector__i64 *this);
#  endif
#  ifndef SALAM_IP__Salam_g_S_Vector__i64_is__empty
#    define SALAM_IP__Salam_g_S_Vector__i64_is__empty
static inline bool _Salam_g_S_Vector__i64_is__empty(Vector__i64 *this);
#  endif
#  ifndef SALAM_IP__Salam_g_S_Vector__i64_first
#    define SALAM_IP__Salam_g_S_Vector__i64_first
static inline int64_t _Salam_g_S_Vector__i64_first(Vector__i64 *this);
#  endif
#  ifndef SALAM_IP__Salam_g_S_Vector__i64_last
#    define SALAM_IP__Salam_g_S_Vector__i64_last
static inline int64_t _Salam_g_S_Vector__i64_last(Vector__i64 *this);
#  endif
#  ifndef SALAM_IP__Salam_g_S_Vector__i64_clear
#    define SALAM_IP__Salam_g_S_Vector__i64_clear
static inline void _Salam_g_S_Vector__i64_clear(Vector__i64 *this);
#  endif
#  ifndef SALAM_IP__Salam_g_S_Vector__i64_insert_i32_i64
#    define SALAM_IP__Salam_g_S_Vector__i64_insert_i32_i64
static inline void _Salam_g_S_Vector__i64_insert_i32_i64(Vector__i64 *this, int32_t i,
                                                         int64_t x);
#  endif
#  ifndef SALAM_IP__Salam_g_S_Vector__i64_remove__at_i32
#    define SALAM_IP__Salam_g_S_Vector__i64_remove__at_i32
static inline int64_t _Salam_g_S_Vector__i64_remove__at_i32(Vector__i64 *this, int32_t i);
#  endif
#  ifndef SALAM_IP__Salam_g_S_Vector__i64_operator__index_i32
#    define SALAM_IP__Salam_g_S_Vector__i64_operator__index_i32
static inline int64_t _Salam_g_S_Vector__i64_operator__index_i32(Vector__i64 *this,
                                                                 int32_t i);
#  endif
#  ifndef SALAM_IP__Salam_g_S_Vector__i64_operator__index__set_i32_i64
#    define SALAM_IP__Salam_g_S_Vector__i64_operator__index__set_i32_i64
static inline void _Salam_g_S_Vector__i64_operator__index__set_i32_i64(Vector__i64 *this,
                                                                       int32_t i,
                                                                       int64_t x);
#  endif
#  ifndef SALAM_IP__Salam_g_S_Vector__i64_iter
#    define SALAM_IP__Salam_g_S_Vector__i64_iter
static inline void _Salam_g_S_Vector__i64_iter(Vector__i64 *this, VectorIter__i64 *__ret);
#  endif
#  ifndef SALAM_IP__Salam_g_S_Vector__i64_free
#    define SALAM_IP__Salam_g_S_Vector__i64_free
static inline void _Salam_g_S_Vector__i64_free(Vector__i64 *this);
#  endif
#  ifndef SALAM_IP__Salam_g_S_VectorIter__i64_has__next
#    define SALAM_IP__Salam_g_S_VectorIter__i64_has__next
static inline bool _Salam_g_S_VectorIter__i64_has__next(VectorIter__i64 *this);
#  endif
#  ifndef SALAM_IP__Salam_g_S_VectorIter__i64_value
#    define SALAM_IP__Salam_g_S_VectorIter__i64_value
static inline int64_t _Salam_g_S_VectorIter__i64_value(VectorIter__i64 *this);
#  endif
#  ifndef SALAM_IP__Salam_g_S_VectorIter__i64_next
#    define SALAM_IP__Salam_g_S_VectorIter__i64_next
static inline void _Salam_g_S_VectorIter__i64_next(VectorIter__i64 *this);
#  endif
#  ifndef SALAM_IB__Salam_g_S_Vector__i32_reserve_i32
#    define SALAM_IB__Salam_g_S_Vector__i32_reserve_i32
static inline void _Salam_g_S_Vector__i32_reserve_i32(Vector__i32 *this, int32_t need)
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
    this->data = ((int32_t *)(_Salam_mem_Reallocate_void_ptr_u64(
        ((void *)(this->data)), (((uint64_t)(c)) * (uint64_t)sizeof(int32_t)))));
    this->__cap = c;
}

#  endif
#  ifndef SALAM_IB__Salam_g_S_Vector__i32_push_i32
#    define SALAM_IB__Salam_g_S_Vector__i32_push_i32
static inline void _Salam_g_S_Vector__i32_push_i32(Vector__i32 *this, int32_t x)
{
    _Salam_g_S_Vector__i32_reserve_i32(this, (this->__len + 1));
    int32_t *d = this->data;
    d[this->__len] = x;
    this->__len += 1;
}

#  endif
#  ifndef SALAM_IB__Salam_g_S_Vector__i32_pop
#    define SALAM_IB__Salam_g_S_Vector__i32_pop
static inline int32_t _Salam_g_S_Vector__i32_pop(Vector__i32 *this)
{
    if ((this->__len <= 0)) {
        salam_panic("Vector.pop: empty");
    }
    this->__len -= 1;
    int32_t *d = this->data;
    return d[this->__len];
}

#  endif
#  ifndef SALAM_IB__Salam_g_S_Vector__i32_get_i32
#    define SALAM_IB__Salam_g_S_Vector__i32_get_i32
static inline int32_t *_Salam_g_S_Vector__i32_get_i32(Vector__i32 *this, int32_t i)
{
    if (((i < 0) || (i >= this->__len))) {
        salam_panic("Vector.get: index out of bounds");
    }
    return ((int32_t *)(intptr_t)((
        ((int64_t)(intptr_t)(this->data)) +
        (((int64_t)(i)) * ((int64_t)((uint64_t)sizeof(int32_t)))))));
}

#  endif
#  ifndef SALAM_IB__Salam_g_S_Vector__i32_set_i32_i32
#    define SALAM_IB__Salam_g_S_Vector__i32_set_i32_i32
static inline void _Salam_g_S_Vector__i32_set_i32_i32(Vector__i32 *this, int32_t i,
                                                      int32_t x)
{
    if ((i < 0)) {
        salam_panic("Vector.set: negative index");
    }
    _Salam_g_S_Vector__i32_reserve_i32(this, (i + 1));
    if ((i >= this->__len)) {
        this->__len = (i + 1);
    }
    int32_t *d = this->data;
    d[i] = x;
}

#  endif
#  ifndef SALAM_IB__Salam_g_S_Vector__i32_len
#    define SALAM_IB__Salam_g_S_Vector__i32_len
static inline int32_t _Salam_g_S_Vector__i32_len(Vector__i32 *this)
{
    return this->__len;
}

#  endif
#  ifndef SALAM_IB__Salam_g_S_Vector__i32_cap
#    define SALAM_IB__Salam_g_S_Vector__i32_cap
static inline int32_t _Salam_g_S_Vector__i32_cap(Vector__i32 *this)
{
    return this->__cap;
}

#  endif
#  ifndef SALAM_IB__Salam_g_S_Vector__i32_is__empty
#    define SALAM_IB__Salam_g_S_Vector__i32_is__empty
static inline bool _Salam_g_S_Vector__i32_is__empty(Vector__i32 *this)
{
    return (this->__len == 0);
}

#  endif
#  ifndef SALAM_IB__Salam_g_S_Vector__i32_first
#    define SALAM_IB__Salam_g_S_Vector__i32_first
static inline int32_t _Salam_g_S_Vector__i32_first(Vector__i32 *this)
{
    return _Salam_g_S_Vector__i32_get_i32(this, 0)[0];
}

#  endif
#  ifndef SALAM_IB__Salam_g_S_Vector__i32_last
#    define SALAM_IB__Salam_g_S_Vector__i32_last
static inline int32_t _Salam_g_S_Vector__i32_last(Vector__i32 *this)
{
    return _Salam_g_S_Vector__i32_get_i32(this, (this->__len - 1))[0];
}

#  endif
#  ifndef SALAM_IB__Salam_g_S_Vector__i32_clear
#    define SALAM_IB__Salam_g_S_Vector__i32_clear
static inline void _Salam_g_S_Vector__i32_clear(Vector__i32 *this)
{
    this->__len = 0;
}

#  endif
#  ifndef SALAM_IB__Salam_g_S_Vector__i32_insert_i32_i32
#    define SALAM_IB__Salam_g_S_Vector__i32_insert_i32_i32
static inline void _Salam_g_S_Vector__i32_insert_i32_i32(Vector__i32 *this, int32_t i,
                                                         int32_t x)
{
    if (((i < 0) || (i > this->__len))) {
        salam_panic("Vector.insert: index out of bounds");
    }
    _Salam_g_S_Vector__i32_reserve_i32(this, (this->__len + 1));
    int32_t *d = this->data;
    int32_t j = this->__len;
    while ((j > i)) {
        d[j] = d[(j - 1)];
        j -= 1;
    }
    d[i] = x;
    this->__len += 1;
}

#  endif
#  ifndef SALAM_IB__Salam_g_S_Vector__i32_remove__at_i32
#    define SALAM_IB__Salam_g_S_Vector__i32_remove__at_i32
static inline int32_t _Salam_g_S_Vector__i32_remove__at_i32(Vector__i32 *this, int32_t i)
{
    if (((i < 0) || (i >= this->__len))) {
        salam_panic("Vector.remove_at: index out of bounds");
    }
    int32_t *d = this->data;
    const int32_t out = d[i];
    int32_t j = i;
    while ((j < (this->__len - 1))) {
        d[j] = d[(j + 1)];
        j += 1;
    }
    this->__len -= 1;
    return out;
}

#  endif
#  ifndef SALAM_IB__Salam_g_S_Vector__i32_operator__index_i32
#    define SALAM_IB__Salam_g_S_Vector__i32_operator__index_i32
static inline int32_t _Salam_g_S_Vector__i32_operator__index_i32(Vector__i32 *this,
                                                                 int32_t i)
{
    return _Salam_g_S_Vector__i32_get_i32(this, i)[0];
}

#  endif
#  ifndef SALAM_IB__Salam_g_S_Vector__i32_operator__index__set_i32_i32
#    define SALAM_IB__Salam_g_S_Vector__i32_operator__index__set_i32_i32
static inline void _Salam_g_S_Vector__i32_operator__index__set_i32_i32(Vector__i32 *this,
                                                                       int32_t i,
                                                                       int32_t x)
{
    _Salam_g_S_Vector__i32_set_i32_i32(this, i, x);
}

#  endif
#  ifndef SALAM_IB__Salam_g_S_Vector__i32_iter
#    define SALAM_IB__Salam_g_S_Vector__i32_iter
static inline void _Salam_g_S_Vector__i32_iter(Vector__i32 *this, VectorIter__i32 *__ret)
{
    VectorIter__i32 it = (VectorIter__i32){.data = NULL, .len = 0, .pos = 0};
    it.data = this->data;
    it.len = this->__len;
    it.pos = 0;
    *__ret = (it);
    return;
}

#  endif
#  ifndef SALAM_IB__Salam_g_S_Vector__i32_free
#    define SALAM_IB__Salam_g_S_Vector__i32_free
static inline void _Salam_g_S_Vector__i32_free(Vector__i32 *this)
{
    if ((this->data != NULL)) {
        _Salam_mem_Free_void_ptr(((void *)(this->data)));
    }
    this->data = NULL;
    this->__len = 0;
    this->__cap = 0;
}

#  endif
#  ifndef SALAM_IB__Salam_g_S_VectorIter__i32_has__next
#    define SALAM_IB__Salam_g_S_VectorIter__i32_has__next
static inline bool _Salam_g_S_VectorIter__i32_has__next(VectorIter__i32 *this)
{
    return (this->pos < this->len);
}

#  endif
#  ifndef SALAM_IB__Salam_g_S_VectorIter__i32_value
#    define SALAM_IB__Salam_g_S_VectorIter__i32_value
static inline int32_t _Salam_g_S_VectorIter__i32_value(VectorIter__i32 *this)
{
    int32_t *d = this->data;
    return d[this->pos];
}

#  endif
#  ifndef SALAM_IB__Salam_g_S_VectorIter__i32_next
#    define SALAM_IB__Salam_g_S_VectorIter__i32_next
static inline void _Salam_g_S_VectorIter__i32_next(VectorIter__i32 *this)
{
    this->pos = (this->pos + 1);
}

#  endif
#  ifndef SALAM_IB__Salam_g___insertion__range__i32_Vector__i32_i32_i32
#    define SALAM_IB__Salam_g___insertion__range__i32_Vector__i32_i32_i32
static inline void _Salam_g___insertion__range__i32_Vector__i32_i32_i32(Vector__i32 v,
                                                                        int32_t lo,
                                                                        int32_t hi)
{
    int32_t i = (lo + 1);
    while ((i <= hi)) {
        const int32_t key = _Salam_g_S_Vector__i32_get_i32(&(v), i)[0];
        int32_t j = (i - 1);
        bool go = true;
        while (go) {
            if ((j < lo)) {
                go = false;
            } else {
                if ((key < _Salam_g_S_Vector__i32_get_i32(&(v), j)[0])) {
                    _Salam_g_S_Vector__i32_set_i32_i32(
                        &(v), (j + 1), _Salam_g_S_Vector__i32_get_i32(&(v), j)[0]);
                    j -= 1;
                } else {
                    go = false;
                }
            }
        }
        _Salam_g_S_Vector__i32_set_i32_i32(&(v), (j + 1), key);
        i += 1;
    }
}

#  endif
#  ifndef SALAM_IB__Salam_g_S_Vector__i64_reserve_i32
#    define SALAM_IB__Salam_g_S_Vector__i64_reserve_i32
static inline void _Salam_g_S_Vector__i64_reserve_i32(Vector__i64 *this, int32_t need)
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
    this->data = ((int64_t *)(_Salam_mem_Reallocate_void_ptr_u64(
        ((void *)(this->data)), (((uint64_t)(c)) * (uint64_t)sizeof(int64_t)))));
    this->__cap = c;
}

#  endif
#  ifndef SALAM_IB__Salam_g_S_Vector__i64_push_i64
#    define SALAM_IB__Salam_g_S_Vector__i64_push_i64
static inline void _Salam_g_S_Vector__i64_push_i64(Vector__i64 *this, int64_t x)
{
    _Salam_g_S_Vector__i64_reserve_i32(this, (this->__len + 1));
    int64_t *d = this->data;
    d[this->__len] = x;
    this->__len += 1;
}

#  endif
#  ifndef SALAM_IB__Salam_g_S_Vector__i64_pop
#    define SALAM_IB__Salam_g_S_Vector__i64_pop
static inline int64_t _Salam_g_S_Vector__i64_pop(Vector__i64 *this)
{
    if ((this->__len <= 0)) {
        salam_panic("Vector.pop: empty");
    }
    this->__len -= 1;
    int64_t *d = this->data;
    return d[this->__len];
}

#  endif
#  ifndef SALAM_IB__Salam_g_S_Vector__i64_get_i32
#    define SALAM_IB__Salam_g_S_Vector__i64_get_i32
static inline int64_t *_Salam_g_S_Vector__i64_get_i32(Vector__i64 *this, int32_t i)
{
    if (((i < 0) || (i >= this->__len))) {
        salam_panic("Vector.get: index out of bounds");
    }
    return ((int64_t *)(intptr_t)((
        ((int64_t)(intptr_t)(this->data)) +
        (((int64_t)(i)) * ((int64_t)((uint64_t)sizeof(int64_t)))))));
}

#  endif
#  ifndef SALAM_IB__Salam_g_S_Vector__i64_set_i32_i64
#    define SALAM_IB__Salam_g_S_Vector__i64_set_i32_i64
static inline void _Salam_g_S_Vector__i64_set_i32_i64(Vector__i64 *this, int32_t i,
                                                      int64_t x)
{
    if ((i < 0)) {
        salam_panic("Vector.set: negative index");
    }
    _Salam_g_S_Vector__i64_reserve_i32(this, (i + 1));
    if ((i >= this->__len)) {
        this->__len = (i + 1);
    }
    int64_t *d = this->data;
    d[i] = x;
}

#  endif
#  ifndef SALAM_IB__Salam_g_S_Vector__i64_len
#    define SALAM_IB__Salam_g_S_Vector__i64_len
static inline int32_t _Salam_g_S_Vector__i64_len(Vector__i64 *this)
{
    return this->__len;
}

#  endif
#  ifndef SALAM_IB__Salam_g_S_Vector__i64_cap
#    define SALAM_IB__Salam_g_S_Vector__i64_cap
static inline int32_t _Salam_g_S_Vector__i64_cap(Vector__i64 *this)
{
    return this->__cap;
}

#  endif
#  ifndef SALAM_IB__Salam_g_S_Vector__i64_is__empty
#    define SALAM_IB__Salam_g_S_Vector__i64_is__empty
static inline bool _Salam_g_S_Vector__i64_is__empty(Vector__i64 *this)
{
    return (this->__len == 0);
}

#  endif
#  ifndef SALAM_IB__Salam_g_S_Vector__i64_first
#    define SALAM_IB__Salam_g_S_Vector__i64_first
static inline int64_t _Salam_g_S_Vector__i64_first(Vector__i64 *this)
{
    return _Salam_g_S_Vector__i64_get_i32(this, 0)[0];
}

#  endif
#  ifndef SALAM_IB__Salam_g_S_Vector__i64_last
#    define SALAM_IB__Salam_g_S_Vector__i64_last
static inline int64_t _Salam_g_S_Vector__i64_last(Vector__i64 *this)
{
    return _Salam_g_S_Vector__i64_get_i32(this, (this->__len - 1))[0];
}

#  endif
#  ifndef SALAM_IB__Salam_g_S_Vector__i64_clear
#    define SALAM_IB__Salam_g_S_Vector__i64_clear
static inline void _Salam_g_S_Vector__i64_clear(Vector__i64 *this)
{
    this->__len = 0;
}

#  endif
#  ifndef SALAM_IB__Salam_g_S_Vector__i64_insert_i32_i64
#    define SALAM_IB__Salam_g_S_Vector__i64_insert_i32_i64
static inline void _Salam_g_S_Vector__i64_insert_i32_i64(Vector__i64 *this, int32_t i,
                                                         int64_t x)
{
    if (((i < 0) || (i > this->__len))) {
        salam_panic("Vector.insert: index out of bounds");
    }
    _Salam_g_S_Vector__i64_reserve_i32(this, (this->__len + 1));
    int64_t *d = this->data;
    int32_t j = this->__len;
    while ((j > i)) {
        d[j] = d[(j - 1)];
        j -= 1;
    }
    d[i] = x;
    this->__len += 1;
}

#  endif
#  ifndef SALAM_IB__Salam_g_S_Vector__i64_remove__at_i32
#    define SALAM_IB__Salam_g_S_Vector__i64_remove__at_i32
static inline int64_t _Salam_g_S_Vector__i64_remove__at_i32(Vector__i64 *this, int32_t i)
{
    if (((i < 0) || (i >= this->__len))) {
        salam_panic("Vector.remove_at: index out of bounds");
    }
    int64_t *d = this->data;
    const int64_t out = d[i];
    int32_t j = i;
    while ((j < (this->__len - 1))) {
        d[j] = d[(j + 1)];
        j += 1;
    }
    this->__len -= 1;
    return out;
}

#  endif
#  ifndef SALAM_IB__Salam_g_S_Vector__i64_operator__index_i32
#    define SALAM_IB__Salam_g_S_Vector__i64_operator__index_i32
static inline int64_t _Salam_g_S_Vector__i64_operator__index_i32(Vector__i64 *this,
                                                                 int32_t i)
{
    return _Salam_g_S_Vector__i64_get_i32(this, i)[0];
}

#  endif
#  ifndef SALAM_IB__Salam_g_S_Vector__i64_operator__index__set_i32_i64
#    define SALAM_IB__Salam_g_S_Vector__i64_operator__index__set_i32_i64
static inline void _Salam_g_S_Vector__i64_operator__index__set_i32_i64(Vector__i64 *this,
                                                                       int32_t i,
                                                                       int64_t x)
{
    _Salam_g_S_Vector__i64_set_i32_i64(this, i, x);
}

#  endif
#  ifndef SALAM_IB__Salam_g_S_Vector__i64_iter
#    define SALAM_IB__Salam_g_S_Vector__i64_iter
static inline void _Salam_g_S_Vector__i64_iter(Vector__i64 *this, VectorIter__i64 *__ret)
{
    VectorIter__i64 it = (VectorIter__i64){.data = NULL, .len = 0, .pos = 0};
    it.data = this->data;
    it.len = this->__len;
    it.pos = 0;
    *__ret = (it);
    return;
}

#  endif
#  ifndef SALAM_IB__Salam_g_S_Vector__i64_free
#    define SALAM_IB__Salam_g_S_Vector__i64_free
static inline void _Salam_g_S_Vector__i64_free(Vector__i64 *this)
{
    if ((this->data != NULL)) {
        _Salam_mem_Free_void_ptr(((void *)(this->data)));
    }
    this->data = NULL;
    this->__len = 0;
    this->__cap = 0;
}

#  endif
#  ifndef SALAM_IB__Salam_g_S_VectorIter__i64_has__next
#    define SALAM_IB__Salam_g_S_VectorIter__i64_has__next
static inline bool _Salam_g_S_VectorIter__i64_has__next(VectorIter__i64 *this)
{
    return (this->pos < this->len);
}

#  endif
#  ifndef SALAM_IB__Salam_g_S_VectorIter__i64_value
#    define SALAM_IB__Salam_g_S_VectorIter__i64_value
static inline int64_t _Salam_g_S_VectorIter__i64_value(VectorIter__i64 *this)
{
    int64_t *d = this->data;
    return d[this->pos];
}

#  endif
#  ifndef SALAM_IB__Salam_g_S_VectorIter__i64_next
#    define SALAM_IB__Salam_g_S_VectorIter__i64_next
static inline void _Salam_g_S_VectorIter__i64_next(VectorIter__i64 *this)
{
    this->pos = (this->pos + 1);
}

#  endif

#endif
