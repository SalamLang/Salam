#include "salam_mod_mem.h"

static const char* _Salam_mem___itos_i64(int64_t n);

void* salam_alloc(uint64_t size) {
    return malloc(size);
}

void* salam_realloc(void* ptr, uint64_t size) {
    return realloc(ptr, size);
}

void salam_free(void* ptr) {
    free(ptr);
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

void* _Salam_mem_Allocate_u64(uint64_t size) {
    return salam_alloc(size);
}

void* _Salam_mem_Reallocate_void_ptr_u64(void* ptr, uint64_t size) {
    return salam_realloc(ptr, size);
}

void _Salam_mem_Free_void_ptr(void* ptr) {
    salam_free(ptr);
}

void _Salam_mem_Set_void_ptr_i32_u64(void* ptr, int32_t value, uint64_t n) {
    memset(ptr, value, n);
}

void _Salam_mem_Copy_void_ptr_void_ptr_u64(void* dst, void* src, uint64_t n) {
    memcpy(dst, src, n);
}

void* _Salam_mem_AllocateZeroed_u64(uint64_t size) {
    void* p = salam_alloc(size);
    memset(p, 0, size);
    return p;
}

void* _Salam_mem_AllocateArray_u64_u64(uint64_t count, uint64_t size) {
    return _Salam_mem_AllocateZeroed_u64((count * size));
}

bool _Salam_mem_MemDebugActive(void) {
    return (_Salam_mem_IsMemDebug() || _Salam_mem_IsAsanActive());
}

static const char* _Salam_mem___itos_i64(int64_t n) {
    if ((n == 0)) {
        return "0";
    }
    int64_t v = n;
    const char* r = "";
    while ((v > 0)) {
        r = salam_strcat(salam_char_from_code((((int32_t)((v % 10))) + 48)), r);
        v = (v / 10);
    }
    return r;
}

void _Salam_mem_CheckLeaks(void) {
    if (_Salam_mem_IsAsanActive()) {
        ({ salam_emit("[salam memcheck] ASAN is active - leak report will appear at process exit.", 1); salam_emit("\n", 1); });
    } else if (_Salam_mem_IsMemDebug()) {
        const int64_t a = _Salam_mem_AllocCount();
        const int64_t f = _Salam_mem_FreeCount();
        const int64_t lb = _Salam_mem_LiveBytes();
        const int64_t pb = _Salam_mem_PeakBytes();
        ({ salam_emit("[salam memcheck] allocs=", 1); });
        ({ salam_emit(_Salam_mem___itos_i64(a), 1); });
        ({ salam_emit("  frees=", 1); });
        ({ salam_emit(_Salam_mem___itos_i64(f), 1); });
        ({ salam_emit("  live_bytes=", 1); });
        ({ salam_emit(_Salam_mem___itos_i64(lb), 1); });
        ({ salam_emit("  peak_bytes=", 1); });
        ({ salam_emit(_Salam_mem___itos_i64(pb), 1); salam_emit("\n", 1); });
        if ((lb > 0)) {
            ({ salam_emit("[salam memcheck] WARNING: ", 1); });
            ({ salam_emit(_Salam_mem___itos_i64(lb), 1); });
            ({ salam_emit(" bytes still allocated (possible leak)", 1); salam_emit("\n", 1); });
        }
    }
}

void* salam_memcpy(void* dst, void* src, uint64_t n) {
    int8_t* d = ((int8_t*)(dst));
    int8_t* s = ((int8_t*)(src));
    const int32_t ni = ((int32_t)(n));
    int32_t i = 0;
    while ((i < ni)) {
        d[i] = s[i];
        i = (i + 1);
    }
    return dst;
}

void* salam_memmove(void* dst, void* src, uint64_t n) {
    int8_t* d = ((int8_t*)(dst));
    int8_t* s = ((int8_t*)(src));
    const int32_t ni = ((int32_t)(n));
    if ((((int64_t)(intptr_t)(dst)) < ((int64_t)(intptr_t)(src)))) {
        int32_t i = 0;
        while ((i < ni)) {
            d[i] = s[i];
            i = (i + 1);
        }
    } else {
        int32_t i = (ni - 1);
        while ((i >= 0)) {
            d[i] = s[i];
            i = (i - 1);
        }
    }
    return dst;
}
