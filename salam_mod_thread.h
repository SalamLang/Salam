#pragma once
#ifndef SALAM_MODULE_thread_H
#define SALAM_MODULE_thread_H
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

#ifndef SALAM_DEF_thread__Thread
#define SALAM_DEF_thread__Thread
struct thread__Thread {
    int64_t handle;
};
#endif
#ifndef SALAM_DEF_Vector__thread__Thread
#define SALAM_DEF_Vector__thread__Thread
struct Vector__thread__Thread {
    thread__Thread* data;
    int32_t __len;
    int32_t __cap;
};
#endif
#ifndef SALAM_DEF_VectorIter__thread__Thread
#define SALAM_DEF_VectorIter__thread__Thread
struct VectorIter__thread__Thread {
    thread__Thread* data;
    int32_t len;
    int32_t pos;
};
#endif

extern int32_t pthread_create(void* thread, void* attr, void* start, void* arg);
extern int32_t pthread_join(uint64_t thread, void* retval);
extern int64_t salam_thread_spawn(void* fn);
extern void salam_thread_join(int64_t h);

void _Salam_thread_New_i64(int64_t h, thread__Thread* __ret);
void _Salam_thread_Join_thread__Thread(thread__Thread t);
void _Salam_thread_JoinAll_Vector__thread__Thread(Vector__thread__Thread threads);
bool _Salam_thread_Valid_thread__Thread(thread__Thread t);
int64_t _Salam_thread_Handle_thread__Thread(thread__Thread t);
void _Salam_thread_Collect_Vector__thread__Thread_i64(Vector__thread__Thread threads, int64_t h, Vector__thread__Thread* __ret);
#ifndef SALAM_IP__Salam_g_S_Vector__thread__Thread_reserve_i32
#define SALAM_IP__Salam_g_S_Vector__thread__Thread_reserve_i32
static inline void _Salam_g_S_Vector__thread__Thread_reserve_i32(Vector__thread__Thread* this, int32_t need);
#endif
#ifndef SALAM_IP__Salam_g_S_Vector__thread__Thread_push_thread__Thread
#define SALAM_IP__Salam_g_S_Vector__thread__Thread_push_thread__Thread
static inline void _Salam_g_S_Vector__thread__Thread_push_thread__Thread(Vector__thread__Thread* this, thread__Thread x);
#endif
#ifndef SALAM_IP__Salam_g_S_Vector__thread__Thread_pop
#define SALAM_IP__Salam_g_S_Vector__thread__Thread_pop
static inline void _Salam_g_S_Vector__thread__Thread_pop(Vector__thread__Thread* this, thread__Thread* __ret);
#endif
#ifndef SALAM_IP__Salam_g_S_Vector__thread__Thread_get_i32
#define SALAM_IP__Salam_g_S_Vector__thread__Thread_get_i32
static inline thread__Thread* _Salam_g_S_Vector__thread__Thread_get_i32(Vector__thread__Thread* this, int32_t i);
#endif
#ifndef SALAM_IP__Salam_g_S_Vector__thread__Thread_set_i32_thread__Thread
#define SALAM_IP__Salam_g_S_Vector__thread__Thread_set_i32_thread__Thread
static inline void _Salam_g_S_Vector__thread__Thread_set_i32_thread__Thread(Vector__thread__Thread* this, int32_t i, thread__Thread x);
#endif
#ifndef SALAM_IP__Salam_g_S_Vector__thread__Thread_len
#define SALAM_IP__Salam_g_S_Vector__thread__Thread_len
static inline int32_t _Salam_g_S_Vector__thread__Thread_len(Vector__thread__Thread* this);
#endif
#ifndef SALAM_IP__Salam_g_S_Vector__thread__Thread_cap
#define SALAM_IP__Salam_g_S_Vector__thread__Thread_cap
static inline int32_t _Salam_g_S_Vector__thread__Thread_cap(Vector__thread__Thread* this);
#endif
#ifndef SALAM_IP__Salam_g_S_Vector__thread__Thread_is__empty
#define SALAM_IP__Salam_g_S_Vector__thread__Thread_is__empty
static inline bool _Salam_g_S_Vector__thread__Thread_is__empty(Vector__thread__Thread* this);
#endif
#ifndef SALAM_IP__Salam_g_S_Vector__thread__Thread_first
#define SALAM_IP__Salam_g_S_Vector__thread__Thread_first
static inline void _Salam_g_S_Vector__thread__Thread_first(Vector__thread__Thread* this, thread__Thread* __ret);
#endif
#ifndef SALAM_IP__Salam_g_S_Vector__thread__Thread_last
#define SALAM_IP__Salam_g_S_Vector__thread__Thread_last
static inline void _Salam_g_S_Vector__thread__Thread_last(Vector__thread__Thread* this, thread__Thread* __ret);
#endif
#ifndef SALAM_IP__Salam_g_S_Vector__thread__Thread_clear
#define SALAM_IP__Salam_g_S_Vector__thread__Thread_clear
static inline void _Salam_g_S_Vector__thread__Thread_clear(Vector__thread__Thread* this);
#endif
#ifndef SALAM_IP__Salam_g_S_Vector__thread__Thread_insert_i32_thread__Thread
#define SALAM_IP__Salam_g_S_Vector__thread__Thread_insert_i32_thread__Thread
static inline void _Salam_g_S_Vector__thread__Thread_insert_i32_thread__Thread(Vector__thread__Thread* this, int32_t i, thread__Thread x);
#endif
#ifndef SALAM_IP__Salam_g_S_Vector__thread__Thread_remove__at_i32
#define SALAM_IP__Salam_g_S_Vector__thread__Thread_remove__at_i32
static inline void _Salam_g_S_Vector__thread__Thread_remove__at_i32(Vector__thread__Thread* this, int32_t i, thread__Thread* __ret);
#endif
#ifndef SALAM_IP__Salam_g_S_Vector__thread__Thread_operator__index_i32
#define SALAM_IP__Salam_g_S_Vector__thread__Thread_operator__index_i32
static inline void _Salam_g_S_Vector__thread__Thread_operator__index_i32(Vector__thread__Thread* this, int32_t i, thread__Thread* __ret);
#endif
#ifndef SALAM_IP__Salam_g_S_Vector__thread__Thread_operator__index__set_i32_thread__Thread
#define SALAM_IP__Salam_g_S_Vector__thread__Thread_operator__index__set_i32_thread__Thread
static inline void _Salam_g_S_Vector__thread__Thread_operator__index__set_i32_thread__Thread(Vector__thread__Thread* this, int32_t i, thread__Thread x);
#endif
#ifndef SALAM_IP__Salam_g_S_Vector__thread__Thread_iter
#define SALAM_IP__Salam_g_S_Vector__thread__Thread_iter
static inline void _Salam_g_S_Vector__thread__Thread_iter(Vector__thread__Thread* this, VectorIter__thread__Thread* __ret);
#endif
#ifndef SALAM_IP__Salam_g_S_Vector__thread__Thread_free
#define SALAM_IP__Salam_g_S_Vector__thread__Thread_free
static inline void _Salam_g_S_Vector__thread__Thread_free(Vector__thread__Thread* this);
#endif
#ifndef SALAM_IP__Salam_g_S_VectorIter__thread__Thread_has__next
#define SALAM_IP__Salam_g_S_VectorIter__thread__Thread_has__next
static inline bool _Salam_g_S_VectorIter__thread__Thread_has__next(VectorIter__thread__Thread* this);
#endif
#ifndef SALAM_IP__Salam_g_S_VectorIter__thread__Thread_value
#define SALAM_IP__Salam_g_S_VectorIter__thread__Thread_value
static inline void _Salam_g_S_VectorIter__thread__Thread_value(VectorIter__thread__Thread* this, thread__Thread* __ret);
#endif
#ifndef SALAM_IP__Salam_g_S_VectorIter__thread__Thread_next
#define SALAM_IP__Salam_g_S_VectorIter__thread__Thread_next
static inline void _Salam_g_S_VectorIter__thread__Thread_next(VectorIter__thread__Thread* this);
#endif
#ifndef SALAM_IB__Salam_g_S_Vector__thread__Thread_reserve_i32
#define SALAM_IB__Salam_g_S_Vector__thread__Thread_reserve_i32
static inline void _Salam_g_S_Vector__thread__Thread_reserve_i32(Vector__thread__Thread* this, int32_t need) {
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
    this->data = ((thread__Thread*)(_Salam_mem_Reallocate_void_ptr_u64(((void*)(this->data)), (((uint64_t)(c)) * (uint64_t)sizeof(thread__Thread)))));
    this->__cap = c;
}

#endif
#ifndef SALAM_IB__Salam_g_S_Vector__thread__Thread_push_thread__Thread
#define SALAM_IB__Salam_g_S_Vector__thread__Thread_push_thread__Thread
static inline void _Salam_g_S_Vector__thread__Thread_push_thread__Thread(Vector__thread__Thread* this, thread__Thread x) {
    _Salam_g_S_Vector__thread__Thread_reserve_i32(this, (this->__len + 1));
    thread__Thread* d = this->data;
    d[this->__len] = x;
    this->__len = (this->__len + 1);
}

#endif
#ifndef SALAM_IB__Salam_g_S_Vector__thread__Thread_pop
#define SALAM_IB__Salam_g_S_Vector__thread__Thread_pop
static inline void _Salam_g_S_Vector__thread__Thread_pop(Vector__thread__Thread* this, thread__Thread* __ret) {
    if ((this->__len <= 0)) {
        salam_panic("Vector.pop: empty");
    }
    this->__len = (this->__len - 1);
    thread__Thread* d = this->data;
    *__ret = (d[this->__len]); return;
}

#endif
#ifndef SALAM_IB__Salam_g_S_Vector__thread__Thread_get_i32
#define SALAM_IB__Salam_g_S_Vector__thread__Thread_get_i32
static inline thread__Thread* _Salam_g_S_Vector__thread__Thread_get_i32(Vector__thread__Thread* this, int32_t i) {
    if (((i < 0) || (i >= this->__len))) {
        salam_panic("Vector.get: index out of bounds");
    }
    return ((thread__Thread*)(intptr_t)((((int64_t)(intptr_t)(this->data)) + (((int64_t)(i)) * ((int64_t)((uint64_t)sizeof(thread__Thread)))))));
}

#endif
#ifndef SALAM_IB__Salam_g_S_Vector__thread__Thread_set_i32_thread__Thread
#define SALAM_IB__Salam_g_S_Vector__thread__Thread_set_i32_thread__Thread
static inline void _Salam_g_S_Vector__thread__Thread_set_i32_thread__Thread(Vector__thread__Thread* this, int32_t i, thread__Thread x) {
    if ((i < 0)) {
        salam_panic("Vector.set: negative index");
    }
    _Salam_g_S_Vector__thread__Thread_reserve_i32(this, (i + 1));
    if ((i >= this->__len)) {
        this->__len = (i + 1);
    }
    thread__Thread* d = this->data;
    d[i] = x;
}

#endif
#ifndef SALAM_IB__Salam_g_S_Vector__thread__Thread_len
#define SALAM_IB__Salam_g_S_Vector__thread__Thread_len
static inline int32_t _Salam_g_S_Vector__thread__Thread_len(Vector__thread__Thread* this) {
    return this->__len;
}

#endif
#ifndef SALAM_IB__Salam_g_S_Vector__thread__Thread_cap
#define SALAM_IB__Salam_g_S_Vector__thread__Thread_cap
static inline int32_t _Salam_g_S_Vector__thread__Thread_cap(Vector__thread__Thread* this) {
    return this->__cap;
}

#endif
#ifndef SALAM_IB__Salam_g_S_Vector__thread__Thread_is__empty
#define SALAM_IB__Salam_g_S_Vector__thread__Thread_is__empty
static inline bool _Salam_g_S_Vector__thread__Thread_is__empty(Vector__thread__Thread* this) {
    return (this->__len == 0);
}

#endif
#ifndef SALAM_IB__Salam_g_S_Vector__thread__Thread_first
#define SALAM_IB__Salam_g_S_Vector__thread__Thread_first
static inline void _Salam_g_S_Vector__thread__Thread_first(Vector__thread__Thread* this, thread__Thread* __ret) {
    *__ret = (_Salam_g_S_Vector__thread__Thread_get_i32(this, 0)[0]); return;
}

#endif
#ifndef SALAM_IB__Salam_g_S_Vector__thread__Thread_last
#define SALAM_IB__Salam_g_S_Vector__thread__Thread_last
static inline void _Salam_g_S_Vector__thread__Thread_last(Vector__thread__Thread* this, thread__Thread* __ret) {
    *__ret = (_Salam_g_S_Vector__thread__Thread_get_i32(this, (this->__len - 1))[0]); return;
}

#endif
#ifndef SALAM_IB__Salam_g_S_Vector__thread__Thread_clear
#define SALAM_IB__Salam_g_S_Vector__thread__Thread_clear
static inline void _Salam_g_S_Vector__thread__Thread_clear(Vector__thread__Thread* this) {
    this->__len = 0;
}

#endif
#ifndef SALAM_IB__Salam_g_S_Vector__thread__Thread_insert_i32_thread__Thread
#define SALAM_IB__Salam_g_S_Vector__thread__Thread_insert_i32_thread__Thread
static inline void _Salam_g_S_Vector__thread__Thread_insert_i32_thread__Thread(Vector__thread__Thread* this, int32_t i, thread__Thread x) {
    if (((i < 0) || (i > this->__len))) {
        salam_panic("Vector.insert: index out of bounds");
    }
    _Salam_g_S_Vector__thread__Thread_reserve_i32(this, (this->__len + 1));
    thread__Thread* d = this->data;
    int32_t j = this->__len;
    while ((j > i)) {
        d[j] = d[(j - 1)];
        j = (j - 1);
    }
    d[i] = x;
    this->__len = (this->__len + 1);
}

#endif
#ifndef SALAM_IB__Salam_g_S_Vector__thread__Thread_remove__at_i32
#define SALAM_IB__Salam_g_S_Vector__thread__Thread_remove__at_i32
static inline void _Salam_g_S_Vector__thread__Thread_remove__at_i32(Vector__thread__Thread* this, int32_t i, thread__Thread* __ret) {
    if (((i < 0) || (i >= this->__len))) {
        salam_panic("Vector.remove_at: index out of bounds");
    }
    thread__Thread* d = this->data;
    const thread__Thread out = d[i];
    int32_t j = i;
    while ((j < (this->__len - 1))) {
        d[j] = d[(j + 1)];
        j = (j + 1);
    }
    this->__len = (this->__len - 1);
    *__ret = (out); return;
}

#endif
#ifndef SALAM_IB__Salam_g_S_Vector__thread__Thread_operator__index_i32
#define SALAM_IB__Salam_g_S_Vector__thread__Thread_operator__index_i32
static inline void _Salam_g_S_Vector__thread__Thread_operator__index_i32(Vector__thread__Thread* this, int32_t i, thread__Thread* __ret) {
    *__ret = (_Salam_g_S_Vector__thread__Thread_get_i32(this, i)[0]); return;
}

#endif
#ifndef SALAM_IB__Salam_g_S_Vector__thread__Thread_operator__index__set_i32_thread__Thread
#define SALAM_IB__Salam_g_S_Vector__thread__Thread_operator__index__set_i32_thread__Thread
static inline void _Salam_g_S_Vector__thread__Thread_operator__index__set_i32_thread__Thread(Vector__thread__Thread* this, int32_t i, thread__Thread x) {
    _Salam_g_S_Vector__thread__Thread_set_i32_thread__Thread(this, i, x);
}

#endif
#ifndef SALAM_IB__Salam_g_S_Vector__thread__Thread_iter
#define SALAM_IB__Salam_g_S_Vector__thread__Thread_iter
static inline void _Salam_g_S_Vector__thread__Thread_iter(Vector__thread__Thread* this, VectorIter__thread__Thread* __ret) {
    VectorIter__thread__Thread it = (VectorIter__thread__Thread){ .data = NULL, .len = 0, .pos = 0 };
    it.data = this->data;
    it.len = this->__len;
    it.pos = 0;
    *__ret = (it); return;
}

#endif
#ifndef SALAM_IB__Salam_g_S_Vector__thread__Thread_free
#define SALAM_IB__Salam_g_S_Vector__thread__Thread_free
static inline void _Salam_g_S_Vector__thread__Thread_free(Vector__thread__Thread* this) {
    if ((this->data != NULL)) {
        _Salam_mem_Free_void_ptr(((void*)(this->data)));
    }
    this->data = NULL;
    this->__len = 0;
    this->__cap = 0;
}

#endif
#ifndef SALAM_IB__Salam_g_S_VectorIter__thread__Thread_has__next
#define SALAM_IB__Salam_g_S_VectorIter__thread__Thread_has__next
static inline bool _Salam_g_S_VectorIter__thread__Thread_has__next(VectorIter__thread__Thread* this) {
    return (this->pos < this->len);
}

#endif
#ifndef SALAM_IB__Salam_g_S_VectorIter__thread__Thread_value
#define SALAM_IB__Salam_g_S_VectorIter__thread__Thread_value
static inline void _Salam_g_S_VectorIter__thread__Thread_value(VectorIter__thread__Thread* this, thread__Thread* __ret) {
    thread__Thread* d = this->data;
    *__ret = (d[this->pos]); return;
}

#endif
#ifndef SALAM_IB__Salam_g_S_VectorIter__thread__Thread_next
#define SALAM_IB__Salam_g_S_VectorIter__thread__Thread_next
static inline void _Salam_g_S_VectorIter__thread__Thread_next(VectorIter__thread__Thread* this) {
    this->pos = (this->pos + 1);
}

#endif

#endif
