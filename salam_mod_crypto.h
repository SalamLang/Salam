#pragma once
#ifndef SALAM_MODULE_crypto_H
#define SALAM_MODULE_crypto_H
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <stddef.h>
#ifndef SALAM_RT_TYPES_DEFINED
#define SALAM_RT_TYPES_DEFINED
typedef struct salam_file salam_file;
typedef struct salam_map salam_map;
typedef struct salam_map_iter salam_map_iter;
typedef void (*salam_thread_fn)(void);
extern void* salam_alloc(uint64_t size);
extern void* salam_realloc(void* ptr, uint64_t size);
extern void salam_free(void* ptr);
#endif
#include "salam_mod_core.h"

extern uint64_t salam_hash_bytes(void* p, uint64_t n);
extern uint64_t salam_str_hash(const char* s);
extern uint64_t salam_hash_int(uint64_t x);

uint64_t _Salam_crypto_HashBytes_void_ptr_u64(void* p, uint64_t n);
uint64_t _Salam_crypto_HashStr_str(const char* s);
uint64_t _Salam_crypto_HashInt_u64(uint64_t x);

#endif
