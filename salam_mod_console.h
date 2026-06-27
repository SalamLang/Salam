#pragma once
#ifndef SALAM_MODULE_console_H
#define SALAM_MODULE_console_H
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
#include "salam_mod_mem.h"

extern int32_t sprintf(void* buf, const char* fmt, ...);
extern int32_t printf(const char* fmt, ...);
extern const char* salam_tostr_i32(int32_t x);
extern const char* salam_tostr_i64(int64_t x);
extern const char* salam_tostr_u32(uint32_t x);
extern const char* salam_tostr_u64(uint64_t x);
extern const char* salam_tostr_f64(double x);
extern const char* salam_tostr_bool(bool b);
extern const char* salam_tostr_char(char c);
extern int64_t write(int32_t fd, void* buf, uint64_t n);
extern void salam_emit(const char* s, int32_t err);
extern void salam_emit_owned(const char* s, int32_t err);

#endif
