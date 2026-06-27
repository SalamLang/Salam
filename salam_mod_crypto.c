#include "salam_mod_crypto.h"

uint64_t salam_hash_bytes(void* p, uint64_t n) {
    int8_t* bp = ((int8_t*)(p));
    const int32_t ni = ((int32_t)(n));
    uint64_t h = ((uint64_t)(14695981039346656037ULL));
    int32_t i = 0;
    while ((i < ni)) {
        int32_t c = ((int32_t)(bp[i]));
        if ((c < 0)) {
            c = (c + 256);
        }
        h = (h * ((uint64_t)(1099511628211LL)));
        h = (h + ((uint64_t)(c)));
        i = (i + 1);
    }
    return h;
}

uint64_t salam_str_hash(const char* s) {
    int8_t* bp = ((int8_t*)(((void*)(intptr_t)(s))));
    uint64_t h = ((uint64_t)(14695981039346656037ULL));
    int32_t j = 0;
    while ((bp[j] != 0)) {
        int32_t c = ((int32_t)(bp[j]));
        if ((c < 0)) {
            c = (c + 256);
        }
        h = (h * ((uint64_t)(1099511628211LL)));
        h = (h + ((uint64_t)(c)));
        j = (j + 1);
    }
    return h;
}

uint64_t salam_hash_int(uint64_t x) {
    uint64_t h = (x + ((uint64_t)(14695981039346656037ULL)));
    h = (h * ((uint64_t)(1099511628211LL)));
    h = (h + ((uint64_t)(1099511628211LL)));
    h = (h * ((uint64_t)(1099511628211LL)));
    return h;
}

uint64_t _Salam_crypto_HashBytes_void_ptr_u64(void* p, uint64_t n) {
    return salam_hash_bytes(p, n);
}

uint64_t _Salam_crypto_HashStr_str(const char* s) {
    return salam_str_hash(s);
}

uint64_t _Salam_crypto_HashInt_u64(uint64_t x) {
    return salam_hash_int(x);
}
