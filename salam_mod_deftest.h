#pragma once
#ifndef SALAM_MODULE_deftest_H
#define SALAM_MODULE_deftest_H
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

#endif
