#pragma once
#ifndef SALAM_MODULE_mem_H
#define SALAM_MODULE_mem_H
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

extern void* malloc(uint64_t size);
extern void* realloc(void* ptr, uint64_t size);
extern void free(void* ptr);
extern void* salam_alloc(uint64_t size);
extern void* salam_realloc(void* ptr, uint64_t size);
extern void salam_free(void* ptr);
extern void* memset(void* ptr, int32_t value, uint64_t n);
extern void* memcpy(void* dst, void* src, uint64_t n);
extern void* salam_memcpy(void* dst, void* src, uint64_t n);
extern void* salam_memmove(void* dst, void* src, uint64_t n);

int64_t _Salam_mem_AllocCount(void);
int64_t _Salam_mem_FreeCount(void);
int64_t _Salam_mem_LiveBytes(void);
int64_t _Salam_mem_PeakBytes(void);
bool _Salam_mem_IsMemDebug(void);
bool _Salam_mem_IsAsanActive(void);
void* _Salam_mem_Allocate_u64(uint64_t size);
void* _Salam_mem_Reallocate_void_ptr_u64(void* ptr, uint64_t size);
void _Salam_mem_Free_void_ptr(void* ptr);
void _Salam_mem_Set_void_ptr_i32_u64(void* ptr, int32_t value, uint64_t n);
void _Salam_mem_Copy_void_ptr_void_ptr_u64(void* dst, void* src, uint64_t n);
void* _Salam_mem_AllocateZeroed_u64(uint64_t size);
void* _Salam_mem_AllocateArray_u64_u64(uint64_t count, uint64_t size);
bool _Salam_mem_MemDebugActive(void);
void _Salam_mem_CheckLeaks(void);

#endif
