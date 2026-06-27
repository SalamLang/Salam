#pragma once
#ifndef SALAM_MODULE_map_H
#define SALAM_MODULE_map_H
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

extern void* salam_map_new(int32_t key__kind, int32_t val__size);
extern void salam_map_put(void* m, int64_t key, int64_t val);
extern int64_t salam_map_get(void* m, int64_t key);
extern int32_t salam_map_has(void* m, int64_t key);
extern int32_t salam_map_remove(void* m, int64_t key);
extern int32_t salam_map_size(void* m);
extern void* salam_map_iter_new(void* m);
extern int32_t salam_map_iter_valid(void* it);
extern int64_t salam_map_iter_key(void* it);
extern int64_t salam_map_iter_value(void* it);
extern void salam_map_iter_next(void* it);

#endif
