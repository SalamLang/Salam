#pragma once
#ifndef SALAM_MODULE_fs_H
#define SALAM_MODULE_fs_H
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

extern void* fopen(const char* path, const char* mode);
extern int32_t fclose(void* fp);
extern uint64_t fread(void* ptr, uint64_t size, uint64_t nmemb, void* stream);
extern uint64_t fwrite(void* ptr, uint64_t size, uint64_t nmemb, void* stream);
extern int32_t fgetc(void* stream);
extern int32_t fseek(void* fp, int32_t off, int32_t whence);
extern salam_file* salam_open(const char* path, const char* mode);
extern const char* salam_file_read(salam_file* f, uint64_t size);
extern const char* salam_file_readline(salam_file* f);
extern int32_t salam_file_write(salam_file* f, const char* data);
extern void salam_file_close(salam_file* f);
extern int32_t salam_file_seek(salam_file* f, int64_t off, int32_t whence);
extern void* opendir(const char* path);
extern void* readdir(void* d);
extern int32_t closedir(void* d);
extern void* salam_os_listdir(const char* path, void* out__count);

#endif
