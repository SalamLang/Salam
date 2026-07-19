#pragma once
#ifndef SALAM_MODULE_smoke_H
#define SALAM_MODULE_smoke_H
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <stddef.h>
#ifndef SALAM_OUT_DEFINED
#define SALAM_OUT_DEFINED
#define SALAM_OB_SZ 65536
extern void *stdout;
extern int fflush(void *);
#if defined(_WIN32)
extern int _write(int, void *, unsigned);
#define SALAM_RAW_WRITE(fd, buf, n) _write((fd), (void *)(buf), (unsigned)(n))
#else
extern int64_t write(int32_t, void *, uint64_t);
#define SALAM_RAW_WRITE(fd, buf, n) write((fd), (void *)(buf), (uint64_t)(n))
#endif
#if (defined(__GNUC__) || defined(__clang__)) && !defined(__TINYC__)
__attribute__((weak)) char salam_ob[SALAM_OB_SZ];
__attribute__((weak)) uint64_t salam_obn;
__attribute__((weak)) void salam_out_flush(void) {
    if (salam_obn) { int64_t r = (int64_t)SALAM_RAW_WRITE(1, salam_ob, salam_obn); (void)r; salam_obn = 0; }
}
__attribute__((weak, destructor)) void salam_out_fini(void) { salam_out_flush(); }
static inline void salam_out_write_(const void *s, uint64_t n) {
    if (salam_obn + n > SALAM_OB_SZ) salam_out_flush();
    if (n >= SALAM_OB_SZ) { int64_t r = (int64_t)SALAM_RAW_WRITE(1, s, n); (void)r; return; }
    __builtin_memcpy(salam_ob + salam_obn, s, (size_t)n); salam_obn += n;
}
#else
static void salam_out_flush(void) { fflush(stdout); }
static void salam_out_write_(const void *s, uint64_t n) {
    int64_t r = (int64_t)SALAM_RAW_WRITE(1, s, n); (void)r;
}
#endif
#define SALAM_OUT_LIT(s, n) salam_out_write_((s), (uint64_t)(n))
#endif
#ifndef SALAM_RT_TYPES_DEFINED
#define SALAM_RT_TYPES_DEFINED
typedef struct salam_file salam_file;
typedef struct salam_map salam_map;
typedef struct salam_map_iter salam_map_iter;
typedef struct { void* data; int64_t len; } salam_slice;
extern int64_t salam_idx(int64_t, int64_t);
extern void salam_panic(const char* msg);
void* _Salam_mem_Allocate_u64(uint64_t size);
void* _Salam_mem_Reallocate_void_ptr_u64(void* ptr, uint64_t size);
void _Salam_mem_Free_void_ptr(void* ptr);
static inline void salam_slice_new(salam_slice* out, void* b, int64_t lo, int64_t hi, int64_t esz){ out->data=(void*)((char*)b+(lo)*(esz)); out->len=(hi)-(lo); }
static inline void salam_slice_sub(salam_slice* out, salam_slice b, int64_t lo, int64_t hi, int has_hi, int64_t esz){ if(!has_hi) hi=b.len; out->data=(void*)((char*)b.data+(lo)*(esz)); out->len=(hi)-(lo); }
static inline void* salam_slice_at(salam_slice s, int64_t i, int64_t esz, int sf){ return (void*)((char*)s.data+(sf?salam_idx(i,s.len):i)*(esz)); }
typedef void (*salam_thread_fn)(void);
#endif
#ifndef SALAM_RT_STR_DEFINED
#define SALAM_RT_STR_DEFINED
extern uint64_t strlen(const char* s);
extern int32_t strcmp(const char* a, const char* b);
extern const char* strstr(const char* haystack, const char* needle);
extern int64_t strtol(const char* s, void* endptr, int32_t base);
extern double strtod(const char* s, void* endptr);
extern const char* salam_strcat(const char* a, const char* b);
extern const char* salam_char_from_code(int32_t c);
extern const char* salam_str_substr(const char* s, int32_t start, int32_t n);
extern const char* salam_str_trim(const char* s);
extern void* salam_str_split(const char* s, const char* delim, void* out_count);
#endif
#ifndef SALAM_FN_ATTRS_DEFINED
#define SALAM_FN_ATTRS_DEFINED
#if defined(__GNUC__) || defined(__clang__)
#define SALAM_NOINLINE __attribute__((noinline))
#define SALAM_PURE __attribute__((pure))
#define SALAM_NORET __attribute__((noreturn))
#define SALAM_DEPRECATED __attribute__((deprecated))
#else
#define SALAM_NOINLINE
#define SALAM_PURE
#define SALAM_NORET
#define SALAM_DEPRECATED
#endif
#endif
#include "salam_mod_core.h"

#ifndef SALAM_FWD_str__StringBuilder
#define SALAM_FWD_str__StringBuilder
typedef struct str__StringBuilder str__StringBuilder;
#endif
#ifndef SALAM_FWD_Vector__str
#define SALAM_FWD_Vector__str
typedef struct Vector__str Vector__str;
#endif
#ifndef SALAM_FWD_VectorIter__str
#define SALAM_FWD_VectorIter__str
typedef struct VectorIter__str VectorIter__str;
#endif
#ifndef SALAM_FWD_thread__Thread
#define SALAM_FWD_thread__Thread
typedef struct thread__Thread thread__Thread;
#endif
#ifndef SALAM_FWD_Vector__thread__Thread
#define SALAM_FWD_Vector__thread__Thread
typedef struct Vector__thread__Thread Vector__thread__Thread;
#endif
#ifndef SALAM_FWD_VectorIter__thread__Thread
#define SALAM_FWD_VectorIter__thread__Thread
typedef struct VectorIter__thread__Thread VectorIter__thread__Thread;
#endif
#ifndef SALAM_FWD_crypto__U64Pair
#define SALAM_FWD_crypto__U64Pair
typedef struct crypto__U64Pair crypto__U64Pair;
#endif
#ifndef SALAM_FWD_crypto__Sha256State
#define SALAM_FWD_crypto__Sha256State
typedef struct crypto__Sha256State crypto__Sha256State;
#endif
#ifndef SALAM_FWD_Vector__str
#define SALAM_FWD_Vector__str
typedef struct Vector__str Vector__str;
#endif
#ifndef SALAM_FWD_VectorIter__str
#define SALAM_FWD_VectorIter__str
typedef struct VectorIter__str VectorIter__str;
#endif

#endif
