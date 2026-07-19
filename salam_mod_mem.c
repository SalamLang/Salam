#include "salam_mod_mem.h"

const uint64_t HDR__SIZE = 8;
int64_t __md__allocs = 0;
int64_t __md__frees = 0;
int64_t __md__live = 0;
int64_t __md__peak = 0;

void* _Salam_mem_Allocate_u64(uint64_t size) {
    return malloc(size);
}

void* _Salam_mem_Reallocate_void_ptr_u64(void* ptr, uint64_t size) {
    return realloc(ptr, size);
}

void _Salam_mem_Free_void_ptr(void* ptr) {
    if ((ptr == NULL)) {
        return;
    }
    free(ptr);
}

void _Salam_mem_Set_void_ptr_i32_u64(void* ptr, int32_t value, uint64_t n) {
    if (((ptr == NULL) || (n == 0))) {
        return;
    }
    memset(ptr, value, n);
}

void _Salam_mem_Copy_void_ptr_void_ptr_u64(void* dst, void* src, uint64_t n) {
    if (((dst == src) || (n == 0))) {
        return;
    }
    memcpy(dst, src, n);
}

void* _Salam_mem_MemMove_void_ptr_void_ptr_u64(void* dst, void* src, uint64_t n) {
    if (((dst == src) || (n == 0))) {
        return dst;
    }
    return memmove(dst, src, n);
}

void* _Salam_mem_AllocateZeroed_u64(uint64_t size) {
    void* p = _Salam_mem_Allocate_u64(size);
    if ((p != NULL)) {
        _Salam_mem_Set_void_ptr_i32_u64(p, 0, size);
    }
    return p;
}

void* _Salam_mem_AllocateArray_u64_u64(uint64_t count, uint64_t size) {
    if (((count == 0) || (size == 0))) {
        return NULL;
    }
    const uint64_t total = (count * size);
    if (((total / count) != size)) {
        return NULL;
    }
    return _Salam_mem_AllocateZeroed_u64(total);
}

int64_t _Salam_mem_AllocCount(void) {
    return 0;
}

int64_t _Salam_mem_FreeCount(void) {
    return 0;
}

int64_t _Salam_mem_LiveBytes(void) {
    return 0;
}

int64_t _Salam_mem_PeakBytes(void) {
    return 0;
}

bool _Salam_mem_IsMemDebug(void) {
    return false;
}

bool _Salam_mem_IsAsanActive(void) {
    return false;
}

bool _Salam_mem_MemDebugActive(void) {
    return (_Salam_mem_IsMemDebug() || _Salam_mem_IsAsanActive());
}

void _Salam_mem_CheckLeaks(void) {
    if (_Salam_mem_IsAsanActive()) {
        ({ salam_out_flush(); ({ salam_emit("[salam memcheck] ASAN active - report at exit", 1); salam_emit("\n", 1); }); });
    } else if (_Salam_mem_IsMemDebug()) {
        ({ salam_out_flush(); ({ salam_emit("[salam memcheck] allocs=", 1); }); });
        ({ salam_out_flush(); ({ salam_emit(_Salam_conv_FormatInt_i64(_Salam_mem_AllocCount()), 1); }); });
        ({ salam_out_flush(); ({ salam_emit(" frees=", 1); }); });
        ({ salam_out_flush(); ({ salam_emit(_Salam_conv_FormatInt_i64(_Salam_mem_FreeCount()), 1); }); });
        ({ salam_out_flush(); ({ salam_emit(" live=", 1); }); });
        ({ salam_out_flush(); ({ salam_emit(_Salam_conv_FormatInt_i64(_Salam_mem_LiveBytes()), 1); }); });
        ({ salam_out_flush(); ({ salam_emit(" peak=", 1); }); });
        ({ salam_out_flush(); ({ salam_emit(_Salam_conv_FormatInt_i64(_Salam_mem_PeakBytes()), 1); salam_emit("\n", 1); }); });
        if ((_Salam_mem_LiveBytes() > 0)) {
            ({ salam_out_flush(); ({ salam_emit("[WARNING] leak bytes=", 1); }); });
            ({ salam_out_flush(); ({ salam_emit(_Salam_conv_FormatInt_i64(_Salam_mem_LiveBytes()), 1); salam_emit("\n", 1); }); });
        }
    }
}
