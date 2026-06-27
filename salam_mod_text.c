#include "salam_mod_text.h"

static int64_t _Salam_text___strptr_str(const char* s);
static bool _Salam_text___is__ws_i32(int32_t ch);

static int64_t _Salam_text___strptr_str(const char* s) {
    void* v = ((void*)(intptr_t)(s));
    return ((int64_t)(intptr_t)(v));
}

const char* salam_strcat(const char* a, const char* b) {
    const int64_t la = ((int64_t)((int32_t)strlen(a)));
    const int64_t lb = ((int64_t)((int32_t)strlen(b)));
    void* r = _Salam_mem_AllocateZeroed_u64(((uint64_t)(((la + lb) + 1))));
    _Salam_mem_Copy_void_ptr_void_ptr_u64(r, ((void*)(intptr_t)(a)), ((uint64_t)(la)));
    _Salam_mem_Copy_void_ptr_void_ptr_u64(((void*)(intptr_t)((((int64_t)(intptr_t)(r)) + la))), ((void*)(intptr_t)(b)), ((uint64_t)(lb)));
    return ((const char*)(r));
}

const char* salam_char_from_code(int32_t c) {
    void* r = _Salam_mem_AllocateZeroed_u64(((uint64_t)(2)));
    _Salam_mem_Set_void_ptr_i32_u64(r, c, ((uint64_t)(1)));
    return ((const char*)(r));
}

const char* salam_str_substr(const char* s, int32_t start, int32_t n) {
    const int32_t slen = (int32_t)strlen(s);
    int32_t st = start;
    int32_t ln = n;
    if ((st < 0)) {
        st = 0;
    }
    if ((st > slen)) {
        st = slen;
    }
    if ((ln < 0)) {
        ln = 0;
    }
    if (((st + ln) > slen)) {
        ln = (slen - st);
    }
    void* r = _Salam_mem_AllocateZeroed_u64(((uint64_t)((ln + 1))));
    _Salam_mem_Copy_void_ptr_void_ptr_u64(r, ((void*)(intptr_t)((_Salam_text___strptr_str(s) + ((int64_t)(st))))), ((uint64_t)(ln)));
    return ((const char*)(r));
}

static bool _Salam_text___is__ws_i32(int32_t ch) {
    if ((ch == 32)) {
        return true;
    }
    if ((ch == 9)) {
        return true;
    }
    if ((ch == 10)) {
        return true;
    }
    if ((ch == 13)) {
        return true;
    }
    return false;
}

const char* salam_str_trim(const char* s) {
    int8_t* bp = ((int8_t*)(((void*)(intptr_t)(s))));
    const int32_t slen = (int32_t)strlen(s);
    int32_t a = 0;
    bool go = true;
    while (((a < slen) && go)) {
        if (_Salam_text___is__ws_i32(((int32_t)(bp[a])))) {
            a = (a + 1);
        } else {
            go = false;
        }
    }
    int32_t e = slen;
    go = true;
    while (((e > a) && go)) {
        if (_Salam_text___is__ws_i32(((int32_t)(bp[(e - 1)])))) {
            e = (e - 1);
        } else {
            go = false;
        }
    }
    const int32_t ln = (e - a);
    void* r = _Salam_mem_AllocateZeroed_u64(((uint64_t)((ln + 1))));
    _Salam_mem_Copy_void_ptr_void_ptr_u64(r, ((void*)(intptr_t)((_Salam_text___strptr_str(s) + ((int64_t)(a))))), ((uint64_t)(ln)));
    return ((const char*)(r));
}

void* salam_str_split(const char* s, const char* delim, void* out__count) {
    int32_t* oc = ((int32_t*)(out__count));
    const int32_t dl = (int32_t)strlen(delim);
    if ((dl == 0)) {
        void* arr = _Salam_mem_Allocate_u64(((uint64_t)(8)));
        int64_t* a64 = ((int64_t*)(arr));
        a64[0] = _Salam_text___strptr_str(salam_str_substr(s, 0, (int32_t)strlen(s)));
        oc[0] = 1;
        return arr;
    }
    int32_t count = 1;
    const char* rest = s;
    int32_t idx = ({ const char *__fh1=(rest); const char *__fn1=(delim); const char *__fp1=(__fh1&&__fn1)?strstr(__fh1,__fn1):0; __fp1?(int32_t)(__fp1-__fh1):-1; });
    while ((idx >= 0)) {
        count = (count + 1);
        rest = salam_str_substr(rest, (idx + dl), (((int32_t)strlen(rest) - idx) - dl));
        idx = ({ const char *__fh2=(rest); const char *__fn2=(delim); const char *__fp2=(__fh2&&__fn2)?strstr(__fh2,__fn2):0; __fp2?(int32_t)(__fp2-__fh2):-1; });
    }
    void* arr = _Salam_mem_Allocate_u64(((uint64_t)((count * 8))));
    int64_t* a64 = ((int64_t*)(arr));
    int32_t k = 0;
    rest = s;
    idx = ({ const char *__fh3=(rest); const char *__fn3=(delim); const char *__fp3=(__fh3&&__fn3)?strstr(__fh3,__fn3):0; __fp3?(int32_t)(__fp3-__fh3):-1; });
    while ((idx >= 0)) {
        a64[k] = _Salam_text___strptr_str(salam_str_substr(rest, 0, idx));
        k = (k + 1);
        rest = salam_str_substr(rest, (idx + dl), (((int32_t)strlen(rest) - idx) - dl));
        idx = ({ const char *__fh4=(rest); const char *__fn4=(delim); const char *__fp4=(__fh4&&__fn4)?strstr(__fh4,__fn4):0; __fp4?(int32_t)(__fp4-__fh4):-1; });
    }
    a64[k] = _Salam_text___strptr_str(salam_str_substr(rest, 0, (int32_t)strlen(rest)));
    oc[0] = count;
    return arr;
}
