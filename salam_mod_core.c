#include "salam_mod_core.h"

int64_t _Salam_core___strptr_str(const char* s);

int32_t __argc = 0;
void* __argv = NULL;

int64_t _Salam_core___strptr_str(const char* s) {
    void* v = ((void*)(intptr_t)(s));
    return ((int64_t)(intptr_t)(v));
}

int64_t salam_idx(int64_t idx, int64_t len) {
    if (((idx < 0) || (idx >= len))) {
        ({ salam_emit(salam_strcat(salam_strcat(salam_strcat(salam_strcat("salam: array index ", _Salam_conv_FormatInt_i64(idx)), " out of bounds [0,"), _Salam_conv_FormatInt_i64(len)), ")"), 1); salam_emit("\n", 1); });
        abort();
    }
    return idx;
}

void salam_panic(const char* msg) {
    ({ salam_emit(salam_strcat("salam panic: ", msg), 1); salam_emit("\n", 1); });
    abort();
}

void _Salam_core_Panic_str(const char* msg) {
    salam_panic(msg);
}

const char* salam_input(void) {
    str__StringBuilder sb = ({ str__StringBuilder __s1; _Salam_str_NewBuilder(&__s1); __s1; });
    int32_t c = getchar();
    while (((c != (-1LL)) && (c != 10))) {
        _Salam_str_BufAppendChar_str__StringBuilder_i32((str__StringBuilder*)&(sb), c);
        c = getchar();
    }
    const char* out = _Salam_str_BufStr_str__StringBuilder((str__StringBuilder*)&(sb));
    _Salam_str_BufFree_str__StringBuilder((str__StringBuilder*)&(sb));
    return out;
    _Salam_str_BufFree_str__StringBuilder((str__StringBuilder*)&(sb));
}

void salam_set_args(int32_t argc, void* argv) {
    __argc = argc;
    __argv = argv;
}

void* salam_args(void* out__count) {
    int32_t* oc = ((int32_t*)(out__count));
    const int32_t n = __argc;
    int32_t slots = n;
    if ((slots < 1)) {
        slots = 1;
    }
    void* arr = _Salam_mem_Allocate_u64(((uint64_t)((slots * 8))));
    int64_t* a64 = ((int64_t*)(arr));
    int64_t* av = ((int64_t*)(__argv));
    int32_t i = 0;
    while ((i < n)) {
        void* cp = ((void*)(intptr_t)(av[i]));
        const char* s = ((const char*)(cp));
        a64[i] = _Salam_core___strptr_str(salam_str_substr(s, 0, (int32_t)strlen(s)));
        i += 1;
    }
    oc[0] = n;
    return arr;
}
