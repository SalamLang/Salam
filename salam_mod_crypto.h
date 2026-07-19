#pragma once
#ifndef SALAM_MODULE_crypto_H
#define SALAM_MODULE_crypto_H
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
#include "salam_mod_mem.h"
#include "salam_mod_str.h"
#include "salam_mod_conv.h"

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

#ifndef SALAM_DEF_crypto__U64Pair
#define SALAM_DEF_crypto__U64Pair
struct crypto__U64Pair {
    int64_t hi;
    int64_t lo;
};
#endif
#ifndef SALAM_DEF_crypto__Sha256State
#define SALAM_DEF_crypto__Sha256State
struct crypto__Sha256State {
    int64_t h0;
    int64_t h1;
    int64_t h2;
    int64_t h3;
    int64_t h4;
    int64_t h5;
    int64_t h6;
    int64_t h7;
};
#endif
#ifndef SALAM_DEF_Vector__str
#define SALAM_DEF_Vector__str
struct Vector__str {
    const char** data;
    int32_t __len;
    int32_t __cap;
};
#endif
#ifndef SALAM_DEF_VectorIter__str
#define SALAM_DEF_VectorIter__str
struct VectorIter__str {
    const char** data;
    int32_t len;
    int32_t pos;
};
#endif

extern uint64_t salam_hash_bytes(void* p, uint64_t n);
extern uint64_t salam_str_hash(const char* s);
extern uint64_t salam_hash_int(uint64_t x);
extern void* fopen(const char* path, const char* mode);
extern uint64_t fread(void* ptr, uint64_t size, uint64_t nmemb, void* stream);
extern int32_t fclose(void* stream);
extern void abort(void);

extern const int64_t POW2[32];
extern const int32_t XOR4[256];
extern const int32_t AND4[256];
extern const int32_t PBKDF2__DEFAULT__ITERATIONS;
extern const int64_t SHA256__K[64];
extern const int64_t SHA512__K__HI[80];
extern const int64_t SHA512__K__LO[80];

uint64_t _Salam_crypto_HashBytes_void_ptr_u64(void* p, uint64_t n);
uint64_t _Salam_crypto_HashStr_str(const char* s);
uint64_t _Salam_crypto_HashInt_u64(uint64_t x);
int64_t _Salam_crypto___xor32_i64_i64(int64_t a0, int64_t b0);
int64_t _Salam_crypto___and32_i64_i64(int64_t a0, int64_t b0);
int64_t _Salam_crypto___not32_i64(int64_t a0);
int64_t _Salam_crypto___rotr32_i64_i32(int64_t x0, int32_t n);
int64_t _Salam_crypto___shr32_i64_i32(int64_t x0, int32_t n);
int64_t _Salam_crypto___rotr32__2_i64(int64_t x0);
int64_t _Salam_crypto___rotr32__6_i64(int64_t x0);
int64_t _Salam_crypto___rotr32__7_i64(int64_t x0);
int64_t _Salam_crypto___rotr32__11_i64(int64_t x0);
int64_t _Salam_crypto___rotr32__13_i64(int64_t x0);
int64_t _Salam_crypto___rotr32__17_i64(int64_t x0);
int64_t _Salam_crypto___rotr32__18_i64(int64_t x0);
int64_t _Salam_crypto___rotr32__19_i64(int64_t x0);
int64_t _Salam_crypto___rotr32__22_i64(int64_t x0);
int64_t _Salam_crypto___rotr32__25_i64(int64_t x0);
int64_t _Salam_crypto___shr32__3_i64(int64_t x0);
int64_t _Salam_crypto___shr32__10_i64(int64_t x0);
int32_t _Salam_crypto___xor8_i32_i32(int32_t a0, int32_t b0);
int32_t _Salam_crypto___bget_void_ptr_i32(void* p, int32_t i);
void _Salam_crypto___bput_void_ptr_i32_i32(void* p, int32_t i, int32_t v);
void _Salam_crypto___put__word32_void_ptr_i32_i64(void* p, int32_t off, int64_t v);
const char* _Salam_crypto___hex32_i64(int64_t v);
const char* _Salam_crypto___hex__byte_i32(int32_t b);
void _Salam_crypto___u64_i64_i64(int64_t hi, int64_t lo, crypto__U64Pair* __ret);
void _Salam_crypto___add64_crypto__U64Pair_crypto__U64Pair(crypto__U64Pair a, crypto__U64Pair b, crypto__U64Pair* __ret);
void _Salam_crypto___xor64_crypto__U64Pair_crypto__U64Pair(crypto__U64Pair a, crypto__U64Pair b, crypto__U64Pair* __ret);
void _Salam_crypto___and64_crypto__U64Pair_crypto__U64Pair(crypto__U64Pair a, crypto__U64Pair b, crypto__U64Pair* __ret);
void _Salam_crypto___not64_crypto__U64Pair(crypto__U64Pair a, crypto__U64Pair* __ret);
void _Salam_crypto___rotr64_crypto__U64Pair_i32(crypto__U64Pair a, int32_t n0, crypto__U64Pair* __ret);
void _Salam_crypto___shr64_crypto__U64Pair_i32(crypto__U64Pair a, int32_t n, crypto__U64Pair* __ret);
void _Salam_crypto___hmac__sha256__prep_void_ptr_i32_void_ptr_void_ptr(void* key, int32_t klen, void* ipad, void* opad);
void _Salam_crypto___hmac__sha256__apply_void_ptr_void_ptr_void_ptr_i32_void_ptr(void* ipad, void* opad, void* msg, int32_t mlen, void* outp);
void _Salam_crypto___hmac__fixed96__block2__init_void_ptr(void* blk2);
void _Salam_crypto___hmac__sha256__apply32__fast_crypto__Sha256State_crypto__Sha256State_void_ptr_void_ptr_void_ptr(crypto__Sha256State mid__ipad, crypto__Sha256State mid__opad, void* blk2, void* msg32, void* outp);
void _Salam_crypto___hmac__sha256__core_void_ptr_i32_void_ptr_i32_void_ptr(void* key, int32_t klen, void* msg, int32_t mlen, void* outp);
const char* _Salam_crypto_HmacSha256Hex_str_str(const char* key, const char* msg);
void _Salam_crypto___pbkdf2__hmac__sha256__core_void_ptr_i32_void_ptr_i32_i32_void_ptr_i32(void* password, int32_t plen, void* salt, int32_t slen, int32_t iterations, void* outp, int32_t dklen);
const char* _Salam_crypto_Pbkdf2HmacSha256Hex_str_str_i32_i32(const char* password, const char* salt, int32_t iterations, int32_t keylen);
bool _Salam_crypto___consttime__eq_str_str(const char* a, const char* b);
const char* _Salam_crypto_HashPassword_str(const char* password);
bool _Salam_crypto_VerifyPassword_str_str(const char* password, const char* encoded);
void _Salam_crypto___crypto__fatal(void);
void _Salam_crypto___os__random__fill_void_ptr_i32(void* buf, int32_t n);
const char* _Salam_crypto_RandomHex_i32(int32_t n);
const char* _Salam_crypto_RandomToken_i32(int32_t n);
int32_t _Salam_crypto___sha256__pad__byte_void_ptr_i32_i32_i32(void* data, int32_t dlen, int32_t total, int32_t i);
void _Salam_crypto___sha256__iv(crypto__Sha256State* __ret);
void _Salam_crypto___sha256__write__state_crypto__Sha256State_void_ptr(crypto__Sha256State st, void* outp);
void _Salam_crypto___sha256__compress_crypto__Sha256State_i64_ptr(crypto__Sha256State* st, int64_t* w);
void _Salam_crypto___sha256__compress__block_crypto__Sha256State_void_ptr(crypto__Sha256State st, void* block, crypto__Sha256State* __ret);
void _Salam_crypto___sha256__core_void_ptr_i32_void_ptr(void* data, int32_t dlen, void* outp);
const char* _Salam_crypto_Sha256Hex_str(const char* data);
int32_t _Salam_crypto___sha512__pad__byte_void_ptr_i32_i32_i32(void* data, int32_t dlen, int32_t total, int32_t i);
void _Salam_crypto___sha512__core_void_ptr_i32_void_ptr(void* data, int32_t dlen, void* outp);
const char* _Salam_crypto_Sha512Hex_str(const char* data);
#ifndef SALAM_IP__Salam_g_S_Vector__str_reserve_i32
#define SALAM_IP__Salam_g_S_Vector__str_reserve_i32
static inline void _Salam_g_S_Vector__str_reserve_i32(Vector__str* this, int32_t need);
#endif
#ifndef SALAM_IP__Salam_g_S_Vector__str_push_str
#define SALAM_IP__Salam_g_S_Vector__str_push_str
static inline void _Salam_g_S_Vector__str_push_str(Vector__str* this, const char* x);
#endif
#ifndef SALAM_IP__Salam_g_S_Vector__str_pop
#define SALAM_IP__Salam_g_S_Vector__str_pop
static inline const char* _Salam_g_S_Vector__str_pop(Vector__str* this);
#endif
#ifndef SALAM_IP__Salam_g_S_Vector__str_get_i32
#define SALAM_IP__Salam_g_S_Vector__str_get_i32
static inline const char** _Salam_g_S_Vector__str_get_i32(Vector__str* this, int32_t i);
#endif
#ifndef SALAM_IP__Salam_g_S_Vector__str_set_i32_str
#define SALAM_IP__Salam_g_S_Vector__str_set_i32_str
static inline void _Salam_g_S_Vector__str_set_i32_str(Vector__str* this, int32_t i, const char* x);
#endif
#ifndef SALAM_IP__Salam_g_S_Vector__str_len
#define SALAM_IP__Salam_g_S_Vector__str_len
static inline int32_t _Salam_g_S_Vector__str_len(Vector__str* this);
#endif
#ifndef SALAM_IP__Salam_g_S_Vector__str_cap
#define SALAM_IP__Salam_g_S_Vector__str_cap
static inline int32_t _Salam_g_S_Vector__str_cap(Vector__str* this);
#endif
#ifndef SALAM_IP__Salam_g_S_Vector__str_is__empty
#define SALAM_IP__Salam_g_S_Vector__str_is__empty
static inline bool _Salam_g_S_Vector__str_is__empty(Vector__str* this);
#endif
#ifndef SALAM_IP__Salam_g_S_Vector__str_first
#define SALAM_IP__Salam_g_S_Vector__str_first
static inline const char* _Salam_g_S_Vector__str_first(Vector__str* this);
#endif
#ifndef SALAM_IP__Salam_g_S_Vector__str_last
#define SALAM_IP__Salam_g_S_Vector__str_last
static inline const char* _Salam_g_S_Vector__str_last(Vector__str* this);
#endif
#ifndef SALAM_IP__Salam_g_S_Vector__str_clear
#define SALAM_IP__Salam_g_S_Vector__str_clear
static inline void _Salam_g_S_Vector__str_clear(Vector__str* this);
#endif
#ifndef SALAM_IP__Salam_g_S_Vector__str_insert_i32_str
#define SALAM_IP__Salam_g_S_Vector__str_insert_i32_str
static inline void _Salam_g_S_Vector__str_insert_i32_str(Vector__str* this, int32_t i, const char* x);
#endif
#ifndef SALAM_IP__Salam_g_S_Vector__str_remove__at_i32
#define SALAM_IP__Salam_g_S_Vector__str_remove__at_i32
static inline const char* _Salam_g_S_Vector__str_remove__at_i32(Vector__str* this, int32_t i);
#endif
#ifndef SALAM_IP__Salam_g_S_Vector__str_operator__index_i32
#define SALAM_IP__Salam_g_S_Vector__str_operator__index_i32
static inline const char* _Salam_g_S_Vector__str_operator__index_i32(Vector__str* this, int32_t i);
#endif
#ifndef SALAM_IP__Salam_g_S_Vector__str_operator__index__set_i32_str
#define SALAM_IP__Salam_g_S_Vector__str_operator__index__set_i32_str
static inline void _Salam_g_S_Vector__str_operator__index__set_i32_str(Vector__str* this, int32_t i, const char* x);
#endif
#ifndef SALAM_IP__Salam_g_S_Vector__str_iter
#define SALAM_IP__Salam_g_S_Vector__str_iter
static inline void _Salam_g_S_Vector__str_iter(Vector__str* this, VectorIter__str* __ret);
#endif
#ifndef SALAM_IP__Salam_g_S_Vector__str_free
#define SALAM_IP__Salam_g_S_Vector__str_free
static inline void _Salam_g_S_Vector__str_free(Vector__str* this);
#endif
#ifndef SALAM_IP__Salam_g_S_VectorIter__str_has__next
#define SALAM_IP__Salam_g_S_VectorIter__str_has__next
static inline bool _Salam_g_S_VectorIter__str_has__next(VectorIter__str* this);
#endif
#ifndef SALAM_IP__Salam_g_S_VectorIter__str_value
#define SALAM_IP__Salam_g_S_VectorIter__str_value
static inline const char* _Salam_g_S_VectorIter__str_value(VectorIter__str* this);
#endif
#ifndef SALAM_IP__Salam_g_S_VectorIter__str_next
#define SALAM_IP__Salam_g_S_VectorIter__str_next
static inline void _Salam_g_S_VectorIter__str_next(VectorIter__str* this);
#endif
#ifndef SALAM_IB__Salam_g_S_Vector__str_reserve_i32
#define SALAM_IB__Salam_g_S_Vector__str_reserve_i32
static inline void _Salam_g_S_Vector__str_reserve_i32(Vector__str* this, int32_t need) {
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
    this->data = ((const char**)(_Salam_mem_Reallocate_void_ptr_u64(((void*)(this->data)), (((uint64_t)(c)) * (uint64_t)sizeof(const char*)))));
    this->__cap = c;
}

#endif
#ifndef SALAM_IB__Salam_g_S_Vector__str_push_str
#define SALAM_IB__Salam_g_S_Vector__str_push_str
static inline void _Salam_g_S_Vector__str_push_str(Vector__str* this, const char* x) {
    _Salam_g_S_Vector__str_reserve_i32(this, (this->__len + 1));
    const char** d = this->data;
    d[this->__len] = x;
    this->__len += 1;
}

#endif
#ifndef SALAM_IB__Salam_g_S_Vector__str_pop
#define SALAM_IB__Salam_g_S_Vector__str_pop
static inline const char* _Salam_g_S_Vector__str_pop(Vector__str* this) {
    if ((this->__len <= 0)) {
        salam_panic("Vector.pop: empty");
    }
    this->__len -= 1;
    const char** d = this->data;
    return d[this->__len];
}

#endif
#ifndef SALAM_IB__Salam_g_S_Vector__str_get_i32
#define SALAM_IB__Salam_g_S_Vector__str_get_i32
static inline const char** _Salam_g_S_Vector__str_get_i32(Vector__str* this, int32_t i) {
    if (((i < 0) || (i >= this->__len))) {
        salam_panic("Vector.get: index out of bounds");
    }
    return ((const char**)(intptr_t)((((int64_t)(intptr_t)(this->data)) + (((int64_t)(i)) * ((int64_t)((uint64_t)sizeof(const char*)))))));
}

#endif
#ifndef SALAM_IB__Salam_g_S_Vector__str_set_i32_str
#define SALAM_IB__Salam_g_S_Vector__str_set_i32_str
static inline void _Salam_g_S_Vector__str_set_i32_str(Vector__str* this, int32_t i, const char* x) {
    if ((i < 0)) {
        salam_panic("Vector.set: negative index");
    }
    _Salam_g_S_Vector__str_reserve_i32(this, (i + 1));
    if ((i >= this->__len)) {
        this->__len = (i + 1);
    }
    const char** d = this->data;
    d[i] = x;
}

#endif
#ifndef SALAM_IB__Salam_g_S_Vector__str_len
#define SALAM_IB__Salam_g_S_Vector__str_len
static inline int32_t _Salam_g_S_Vector__str_len(Vector__str* this) {
    return this->__len;
}

#endif
#ifndef SALAM_IB__Salam_g_S_Vector__str_cap
#define SALAM_IB__Salam_g_S_Vector__str_cap
static inline int32_t _Salam_g_S_Vector__str_cap(Vector__str* this) {
    return this->__cap;
}

#endif
#ifndef SALAM_IB__Salam_g_S_Vector__str_is__empty
#define SALAM_IB__Salam_g_S_Vector__str_is__empty
static inline bool _Salam_g_S_Vector__str_is__empty(Vector__str* this) {
    return (this->__len == 0);
}

#endif
#ifndef SALAM_IB__Salam_g_S_Vector__str_first
#define SALAM_IB__Salam_g_S_Vector__str_first
static inline const char* _Salam_g_S_Vector__str_first(Vector__str* this) {
    return _Salam_g_S_Vector__str_get_i32(this, 0)[0];
}

#endif
#ifndef SALAM_IB__Salam_g_S_Vector__str_last
#define SALAM_IB__Salam_g_S_Vector__str_last
static inline const char* _Salam_g_S_Vector__str_last(Vector__str* this) {
    return _Salam_g_S_Vector__str_get_i32(this, (this->__len - 1))[0];
}

#endif
#ifndef SALAM_IB__Salam_g_S_Vector__str_clear
#define SALAM_IB__Salam_g_S_Vector__str_clear
static inline void _Salam_g_S_Vector__str_clear(Vector__str* this) {
    this->__len = 0;
}

#endif
#ifndef SALAM_IB__Salam_g_S_Vector__str_insert_i32_str
#define SALAM_IB__Salam_g_S_Vector__str_insert_i32_str
static inline void _Salam_g_S_Vector__str_insert_i32_str(Vector__str* this, int32_t i, const char* x) {
    if (((i < 0) || (i > this->__len))) {
        salam_panic("Vector.insert: index out of bounds");
    }
    _Salam_g_S_Vector__str_reserve_i32(this, (this->__len + 1));
    const char** d = this->data;
    int32_t j = this->__len;
    while ((j > i)) {
        d[j] = d[(j - 1)];
        j -= 1;
    }
    d[i] = x;
    this->__len += 1;
}

#endif
#ifndef SALAM_IB__Salam_g_S_Vector__str_remove__at_i32
#define SALAM_IB__Salam_g_S_Vector__str_remove__at_i32
static inline const char* _Salam_g_S_Vector__str_remove__at_i32(Vector__str* this, int32_t i) {
    if (((i < 0) || (i >= this->__len))) {
        salam_panic("Vector.remove_at: index out of bounds");
    }
    const char** d = this->data;
    const char* out = d[i];
    int32_t j = i;
    while ((j < (this->__len - 1))) {
        d[j] = d[(j + 1)];
        j += 1;
    }
    this->__len -= 1;
    return out;
}

#endif
#ifndef SALAM_IB__Salam_g_S_Vector__str_operator__index_i32
#define SALAM_IB__Salam_g_S_Vector__str_operator__index_i32
static inline const char* _Salam_g_S_Vector__str_operator__index_i32(Vector__str* this, int32_t i) {
    return _Salam_g_S_Vector__str_get_i32(this, i)[0];
}

#endif
#ifndef SALAM_IB__Salam_g_S_Vector__str_operator__index__set_i32_str
#define SALAM_IB__Salam_g_S_Vector__str_operator__index__set_i32_str
static inline void _Salam_g_S_Vector__str_operator__index__set_i32_str(Vector__str* this, int32_t i, const char* x) {
    _Salam_g_S_Vector__str_set_i32_str(this, i, x);
}

#endif
#ifndef SALAM_IB__Salam_g_S_Vector__str_iter
#define SALAM_IB__Salam_g_S_Vector__str_iter
static inline void _Salam_g_S_Vector__str_iter(Vector__str* this, VectorIter__str* __ret) {
    VectorIter__str it = (VectorIter__str){ .data = NULL, .len = 0, .pos = 0 };
    it.data = this->data;
    it.len = this->__len;
    it.pos = 0;
    *__ret = (it); return;
}

#endif
#ifndef SALAM_IB__Salam_g_S_Vector__str_free
#define SALAM_IB__Salam_g_S_Vector__str_free
static inline void _Salam_g_S_Vector__str_free(Vector__str* this) {
    if ((this->data != NULL)) {
        _Salam_mem_Free_void_ptr(((void*)(this->data)));
    }
    this->data = NULL;
    this->__len = 0;
    this->__cap = 0;
}

#endif
#ifndef SALAM_IB__Salam_g_S_VectorIter__str_has__next
#define SALAM_IB__Salam_g_S_VectorIter__str_has__next
static inline bool _Salam_g_S_VectorIter__str_has__next(VectorIter__str* this) {
    return (this->pos < this->len);
}

#endif
#ifndef SALAM_IB__Salam_g_S_VectorIter__str_value
#define SALAM_IB__Salam_g_S_VectorIter__str_value
static inline const char* _Salam_g_S_VectorIter__str_value(VectorIter__str* this) {
    const char** d = this->data;
    return d[this->pos];
}

#endif
#ifndef SALAM_IB__Salam_g_S_VectorIter__str_next
#define SALAM_IB__Salam_g_S_VectorIter__str_next
static inline void _Salam_g_S_VectorIter__str_next(VectorIter__str* this) {
    this->pos = (this->pos + 1);
}

#endif

#endif
