#pragma once
#ifndef SALAM_MODULE_text_H
#define SALAM_MODULE_text_H
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

extern const char* salam_strcat(const char* a, const char* b);
extern const char* salam_char_from_code(int32_t c);
extern const char* salam_str_substr(const char* s, int32_t start, int32_t n);
extern const char* salam_str_trim(const char* s);
extern void* salam_str_split(const char* s, const char* delim, void* out__count);

#endif
