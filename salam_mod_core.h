#pragma once
#ifndef SALAM_MODULE_core_H
#define SALAM_MODULE_core_H
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
#include "salam_mod_mem.h"
#include "salam_mod_map.h"
#include "salam_mod_text.h"
#include "salam_mod_console.h"
#include "salam_mod_fs.h"
#include "salam_mod_thread.h"
#include "salam_mod_crypto.h"

extern int32_t getchar(void);
extern int32_t strcmp(const char* a, const char* b);
extern const char* strstr(const char* haystack, const char* needle);
extern int64_t strtol(const char* s, void* endptr, int32_t base);
extern double strtod(const char* s, void* endptr);
extern uint64_t strlen(const char* s);
extern void abort(void);
extern int64_t salam_idx(int64_t idx, int64_t len);
extern void salam_panic(const char* msg);
extern const char* salam_input(void);
extern void salam_set_args(int32_t argc, void* argv);
extern void* salam_args(void* out__count);

void _Salam_core_Panic_str(const char* msg);

#endif
