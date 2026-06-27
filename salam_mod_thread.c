#include "salam_mod_thread.h"

void _Salam_thread_New_i64(int64_t h, thread__Thread* __ret) {
    *__ret = ((thread__Thread){ .handle = h }); return;
}

void _Salam_thread_Join_thread__Thread(thread__Thread t) {
    salam_thread_join(t.handle);
}

void _Salam_thread_JoinAll_Vector__thread__Thread(Vector__thread__Thread threads) {
    int32_t i = 0;
    while ((i < _Salam_g_S_Vector__thread__Thread_len(&(threads)))) {
        salam_thread_join(_Salam_g_S_Vector__thread__Thread_get_i32(&(threads), i)[0].handle);
        i = (i + 1);
    }
}

bool _Salam_thread_Valid_thread__Thread(thread__Thread t) {
    return (t.handle != 0);
}

int64_t _Salam_thread_Handle_thread__Thread(thread__Thread t) {
    return t.handle;
}

void _Salam_thread_Collect_Vector__thread__Thread_i64(Vector__thread__Thread threads, int64_t h, Vector__thread__Thread* __ret) {
    _Salam_g_S_Vector__thread__Thread_push_thread__Thread(&(threads), (thread__Thread){ .handle = h });
    *__ret = (threads); return;
}

int64_t salam_thread_spawn(void* fn) {
    if ((fn == NULL)) {
        return 0;
    }
    void* t = _Salam_mem_Allocate_u64(((uint64_t)(8)));
    if ((pthread_create(t, NULL, fn, NULL) != 0)) {
        _Salam_mem_Free_void_ptr(t);
        return 0;
    }
    return ((int64_t)(intptr_t)(t));
}

void salam_thread_join(int64_t h) {
    if ((h == 0)) {
        return;
    }
    int64_t* tp = ((int64_t*)(((void*)(intptr_t)(h))));
    pthread_join(((uint64_t)(tp[0])), NULL);
    _Salam_mem_Free_void_ptr(((void*)(intptr_t)(h)));
}
