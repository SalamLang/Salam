#include "salam_mod_console.h"

static const char* _Salam_console___fmt_str_u64_bool_i64_f64(const char* fmt, uint64_t cap, bool write__int, int64_t iv, double fv);
static const char* _Salam_console___dup_str(const char* src);
static void _Salam_console___err__raw_str(const char* s);

static const char* _Salam_console___fmt_str_u64_bool_i64_f64(const char* fmt, uint64_t cap, bool write__int, int64_t iv, double fv) {
    void* b = _Salam_mem_Allocate_u64(cap);
    if (write__int) {
        sprintf(b, fmt, iv);
    } else {
        sprintf(b, fmt, fv);
    }
    return ((const char*)(b));
}

static const char* _Salam_console___dup_str(const char* src) {
    const int32_t n = (int32_t)strlen(src);
    void* d = _Salam_mem_AllocateZeroed_u64(((uint64_t)((n + 1))));
    _Salam_mem_Copy_void_ptr_void_ptr_u64(d, ((void*)(intptr_t)(src)), ((uint64_t)(n)));
    return ((const char*)(d));
}

const char* salam_tostr_i32(int32_t x) {
    return _Salam_console___fmt_str_u64_bool_i64_f64("%d", ((uint64_t)(16)), true, ((int64_t)(x)), 0.0);
}

const char* salam_tostr_i64(int64_t x) {
    return _Salam_console___fmt_str_u64_bool_i64_f64("%lld", ((uint64_t)(24)), true, x, 0.0);
}

const char* salam_tostr_u32(uint32_t x) {
    return _Salam_console___fmt_str_u64_bool_i64_f64("%u", ((uint64_t)(16)), true, ((int64_t)(x)), 0.0);
}

const char* salam_tostr_u64(uint64_t x) {
    return _Salam_console___fmt_str_u64_bool_i64_f64("%llu", ((uint64_t)(24)), true, ((int64_t)(x)), 0.0);
}

const char* salam_tostr_f64(double x) {
    return _Salam_console___fmt_str_u64_bool_i64_f64("%g", ((uint64_t)(40)), false, 0, x);
}

const char* salam_tostr_bool(bool b) {
    if (b) {
        return _Salam_console___dup_str("true");
    }
    return _Salam_console___dup_str("false");
}

const char* salam_tostr_char(char c) {
    return salam_char_from_code(((int32_t)(c)));
}

static void _Salam_console___err__raw_str(const char* s) {
    write(2, ((void*)(intptr_t)(s)), ((uint64_t)((int32_t)strlen(s))));
}

void salam_emit(const char* s, int32_t err) {
    if ((err != 0)) {
        _Salam_console___err__raw_str(s);
    } else {
        printf("%s", s);
    }
}

void salam_emit_owned(const char* s, int32_t err) {
    salam_emit(s, err);
    _Salam_mem_Free_void_ptr(((void*)(intptr_t)(s)));
}
