#include "salam_mod_crypto.h"

int64_t _Salam_crypto___xor32_i64_i64(int64_t a0, int64_t b0);
int64_t _Salam_crypto___and32_i64_i64(int64_t a0, int64_t b0);
int64_t _Salam_crypto___not32_i64(int64_t a0);
int64_t _Salam_crypto___rotr32_i64_i32(int64_t x0, int32_t n);
int64_t _Salam_crypto___shr32_i64_i32(int64_t x0, int32_t n);
int64_t _Salam_crypto___rotr32__2_i64(int64_t x0);
int64_t _Salam_crypto___rotr32__6_i64(int64_t x0);
int64_t _Salam_crypto___rotr32__7_i64(int64_t x0);
int64_t _Salam_crypto___rotr32__11_i64(int64_t x0);
int64_t _Salam_crypto___rotr32__13_i64(int64_t x0);
int64_t _Salam_crypto___rotr32__17_i64(int64_t x0);
int64_t _Salam_crypto___rotr32__18_i64(int64_t x0);
int64_t _Salam_crypto___rotr32__19_i64(int64_t x0);
int64_t _Salam_crypto___rotr32__22_i64(int64_t x0);
int64_t _Salam_crypto___rotr32__25_i64(int64_t x0);
int64_t _Salam_crypto___shr32__3_i64(int64_t x0);
int64_t _Salam_crypto___shr32__10_i64(int64_t x0);
int32_t _Salam_crypto___xor8_i32_i32(int32_t a0, int32_t b0);
int32_t _Salam_crypto___bget_void_ptr_i32(void* p, int32_t i);
void _Salam_crypto___bput_void_ptr_i32_i32(void* p, int32_t i, int32_t v);
void _Salam_crypto___put__word32_void_ptr_i32_i64(void* p, int32_t off, int64_t v);
const char* _Salam_crypto___hex32_i64(int64_t v);
const char* _Salam_crypto___hex__byte_i32(int32_t b);
void _Salam_crypto___u64_i64_i64(int64_t hi, int64_t lo, crypto__U64Pair* __ret);
void _Salam_crypto___add64_crypto__U64Pair_crypto__U64Pair(crypto__U64Pair a, crypto__U64Pair b, crypto__U64Pair* __ret);
void _Salam_crypto___xor64_crypto__U64Pair_crypto__U64Pair(crypto__U64Pair a, crypto__U64Pair b, crypto__U64Pair* __ret);
void _Salam_crypto___and64_crypto__U64Pair_crypto__U64Pair(crypto__U64Pair a, crypto__U64Pair b, crypto__U64Pair* __ret);
void _Salam_crypto___not64_crypto__U64Pair(crypto__U64Pair a, crypto__U64Pair* __ret);
void _Salam_crypto___rotr64_crypto__U64Pair_i32(crypto__U64Pair a, int32_t n0, crypto__U64Pair* __ret);
void _Salam_crypto___shr64_crypto__U64Pair_i32(crypto__U64Pair a, int32_t n, crypto__U64Pair* __ret);
void _Salam_crypto___hmac__sha256__prep_void_ptr_i32_void_ptr_void_ptr(void* key, int32_t klen, void* ipad, void* opad);
void _Salam_crypto___hmac__sha256__apply_void_ptr_void_ptr_void_ptr_i32_void_ptr(void* ipad, void* opad, void* msg, int32_t mlen, void* outp);
void _Salam_crypto___hmac__fixed96__block2__init_void_ptr(void* blk2);
void _Salam_crypto___hmac__sha256__apply32__fast_crypto__Sha256State_crypto__Sha256State_void_ptr_void_ptr_void_ptr(crypto__Sha256State mid__ipad, crypto__Sha256State mid__opad, void* blk2, void* msg32, void* outp);
void _Salam_crypto___hmac__sha256__core_void_ptr_i32_void_ptr_i32_void_ptr(void* key, int32_t klen, void* msg, int32_t mlen, void* outp);
void _Salam_crypto___pbkdf2__hmac__sha256__core_void_ptr_i32_void_ptr_i32_i32_void_ptr_i32(void* password, int32_t plen, void* salt, int32_t slen, int32_t iterations, void* outp, int32_t dklen);
bool _Salam_crypto___consttime__eq_str_str(const char* a, const char* b);
void _Salam_crypto___crypto__fatal(void);
void _Salam_crypto___os__random__fill_void_ptr_i32(void* buf, int32_t n);
int32_t _Salam_crypto___sha256__pad__byte_void_ptr_i32_i32_i32(void* data, int32_t dlen, int32_t total, int32_t i);
void _Salam_crypto___sha256__iv(crypto__Sha256State* __ret);
void _Salam_crypto___sha256__write__state_crypto__Sha256State_void_ptr(crypto__Sha256State st, void* outp);
void _Salam_crypto___sha256__compress_crypto__Sha256State_i64_ptr(crypto__Sha256State* st, int64_t* w);
void _Salam_crypto___sha256__compress__block_crypto__Sha256State_void_ptr(crypto__Sha256State st, void* block, crypto__Sha256State* __ret);
void _Salam_crypto___sha256__core_void_ptr_i32_void_ptr(void* data, int32_t dlen, void* outp);
int32_t _Salam_crypto___sha512__pad__byte_void_ptr_i32_i32_i32(void* data, int32_t dlen, int32_t total, int32_t i);
void _Salam_crypto___sha512__core_void_ptr_i32_void_ptr(void* data, int32_t dlen, void* outp);

const int64_t POW2[32] = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768, 65536, 131072, 262144, 524288, 1048576, 2097152, 4194304, 8388608, 16777216, 33554432, 67108864, 134217728, 268435456, 536870912, 1073741824, 2147483648LL};
const int32_t XOR4[256] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 1, 0, 3, 2, 5, 4, 7, 6, 9, 8, 11, 10, 13, 12, 15, 14, 2, 3, 0, 1, 6, 7, 4, 5, 10, 11, 8, 9, 14, 15, 12, 13, 3, 2, 1, 0, 7, 6, 5, 4, 11, 10, 9, 8, 15, 14, 13, 12, 4, 5, 6, 7, 0, 1, 2, 3, 12, 13, 14, 15, 8, 9, 10, 11, 5, 4, 7, 6, 1, 0, 3, 2, 13, 12, 15, 14, 9, 8, 11, 10, 6, 7, 4, 5, 2, 3, 0, 1, 14, 15, 12, 13, 10, 11, 8, 9, 7, 6, 5, 4, 3, 2, 1, 0, 15, 14, 13, 12, 11, 10, 9, 8, 8, 9, 10, 11, 12, 13, 14, 15, 0, 1, 2, 3, 4, 5, 6, 7, 9, 8, 11, 10, 13, 12, 15, 14, 1, 0, 3, 2, 5, 4, 7, 6, 10, 11, 8, 9, 14, 15, 12, 13, 2, 3, 0, 1, 6, 7, 4, 5, 11, 10, 9, 8, 15, 14, 13, 12, 3, 2, 1, 0, 7, 6, 5, 4, 12, 13, 14, 15, 8, 9, 10, 11, 4, 5, 6, 7, 0, 1, 2, 3, 13, 12, 15, 14, 9, 8, 11, 10, 5, 4, 7, 6, 1, 0, 3, 2, 14, 15, 12, 13, 10, 11, 8, 9, 6, 7, 4, 5, 2, 3, 0, 1, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0};
const int32_t AND4[256] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 2, 2, 0, 0, 2, 2, 0, 0, 2, 2, 0, 0, 2, 2, 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3, 0, 0, 0, 0, 4, 4, 4, 4, 0, 0, 0, 0, 4, 4, 4, 4, 0, 1, 0, 1, 4, 5, 4, 5, 0, 1, 0, 1, 4, 5, 4, 5, 0, 0, 2, 2, 4, 4, 6, 6, 0, 0, 2, 2, 4, 4, 6, 6, 0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7, 0, 0, 0, 0, 0, 0, 0, 0, 8, 8, 8, 8, 8, 8, 8, 8, 0, 1, 0, 1, 0, 1, 0, 1, 8, 9, 8, 9, 8, 9, 8, 9, 0, 0, 2, 2, 0, 0, 2, 2, 8, 8, 10, 10, 8, 8, 10, 10, 0, 1, 2, 3, 0, 1, 2, 3, 8, 9, 10, 11, 8, 9, 10, 11, 0, 0, 0, 0, 4, 4, 4, 4, 8, 8, 8, 8, 12, 12, 12, 12, 0, 1, 0, 1, 4, 5, 4, 5, 8, 9, 8, 9, 12, 13, 12, 13, 0, 0, 2, 2, 4, 4, 6, 6, 8, 8, 10, 10, 12, 12, 14, 14, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
const int32_t PBKDF2__DEFAULT__ITERATIONS = 1000;
const int64_t SHA256__K[64] = {1116352408, 1899447441, 3049323471LL, 3921009573LL, 961987163, 1508970993, 2453635748LL, 2870763221LL, 3624381080LL, 310598401, 607225278, 1426881987, 1925078388, 2162078206LL, 2614888103LL, 3248222580LL, 3835390401LL, 4022224774LL, 264347078, 604807628, 770255983, 1249150122, 1555081692, 1996064986, 2554220882LL, 2821834349LL, 2952996808LL, 3210313671LL, 3336571891LL, 3584528711LL, 113926993, 338241895, 666307205, 773529912, 1294757372, 1396182291, 1695183700, 1986661051, 2177026350LL, 2456956037LL, 2730485921LL, 2820302411LL, 3259730800LL, 3345764771LL, 3516065817LL, 3600352804LL, 4094571909LL, 275423344, 430227734, 506948616, 659060556, 883997877, 958139571, 1322822218, 1537002063, 1747873779, 1955562222, 2024104815, 2227730452LL, 2361852424LL, 2428436474LL, 2756734187LL, 3204031479LL, 3329325298LL};
const int64_t SHA512__K__HI[80] = {1116352408, 1899447441, 3049323471LL, 3921009573LL, 961987163, 1508970993, 2453635748LL, 2870763221LL, 3624381080LL, 310598401, 607225278, 1426881987, 1925078388, 2162078206LL, 2614888103LL, 3248222580LL, 3835390401LL, 4022224774LL, 264347078, 604807628, 770255983, 1249150122, 1555081692, 1996064986, 2554220882LL, 2821834349LL, 2952996808LL, 3210313671LL, 3336571891LL, 3584528711LL, 113926993, 338241895, 666307205, 773529912, 1294757372, 1396182291, 1695183700, 1986661051, 2177026350LL, 2456956037LL, 2730485921LL, 2820302411LL, 3259730800LL, 3345764771LL, 3516065817LL, 3600352804LL, 4094571909LL, 275423344, 430227734, 506948616, 659060556, 883997877, 958139571, 1322822218, 1537002063, 1747873779, 1955562222, 2024104815, 2227730452LL, 2361852424LL, 2428436474LL, 2756734187LL, 3204031479LL, 3329325298LL, 3391569614LL, 3515267271LL, 3940187606LL, 4118630271LL, 116418474, 174292421, 289380356, 460393269, 685471733, 852142971, 1017036298, 1126000580, 1288033470, 1501505948, 1607167915, 1816402316};
const int64_t SHA512__K__LO[80] = {3609767458LL, 602891725, 3964484399LL, 2173295548LL, 4081628472LL, 3053834265LL, 2937671579LL, 3664609560LL, 2734883394LL, 1164996542, 1323610764, 3590304994LL, 4068182383LL, 991336113, 633803317, 3479774868LL, 2666613458LL, 944711139, 2341262773LL, 2007800933, 1495990901, 1856431235, 3175218132LL, 2198950837LL, 3999719339LL, 766784016, 2566594879LL, 3203337956LL, 1034457026, 2466948901LL, 3758326383LL, 168717936, 1188179964, 1546045734, 1522805485, 2643833823LL, 2343527390LL, 1014477480, 1206759142, 344077627, 1290863460, 3158454273LL, 3505952657LL, 106217008, 3606008344LL, 1432725776, 1467031594, 851169720, 3100823752LL, 1363258195, 3750685593LL, 3785050280LL, 3318307427LL, 3812723403LL, 2003034995, 3602036899LL, 1575990012, 1125592928, 2716904306LL, 442776044, 593698344, 3733110249LL, 2999351573LL, 3815920427LL, 3928383900LL, 566280711, 3454069534LL, 4000239992LL, 1914138554, 2731055270LL, 3203993006LL, 320620315, 587496836, 1086792851, 365543100, 2618297676LL, 3409855158LL, 4234509866LL, 987167468, 1246189591};

uint64_t salam_hash_bytes(void* p, uint64_t n) {
    int8_t* bp = ((int8_t*)(p));
    const int32_t ni = ((int32_t)(n));
    uint64_t h = 14695981039346656037ULL;
    int32_t i = 0;
    while ((i < ni)) {
        int32_t c = ((int32_t)(bp[i]));
        if ((c < 0)) {
            c = (c + 256);
        }
        h *= 1099511628211LL;
        h += ((uint64_t)(c));
        i += 1;
    }
    return h;
}

uint64_t salam_str_hash(const char* s) {
    int8_t* bp = ((int8_t*)(((void*)(intptr_t)(s))));
    uint64_t h = 14695981039346656037ULL;
    int32_t j = 0;
    while ((bp[j] != 0)) {
        int32_t c = ((int32_t)(bp[j]));
        if ((c < 0)) {
            c = (c + 256);
        }
        h *= 1099511628211LL;
        h += ((uint64_t)(c));
        j += 1;
    }
    return h;
}

uint64_t salam_hash_int(uint64_t x) {
    uint64_t h = (x + 14695981039346656037ULL);
    h *= 1099511628211LL;
    h += 1099511628211LL;
    h *= 1099511628211LL;
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

int64_t _Salam_crypto___xor32_i64_i64(int64_t a0, int64_t b0) {
    int64_t a = (a0 % 4294967296LL);
    int64_t b = (b0 % 4294967296LL);
    int64_t r = 0;
    int64_t p = 1;
    int32_t i = 0;
    while ((i < 8)) {
        const int64_t na = (a % 16);
        const int64_t nb = (b % 16);
        r += (((int64_t)(XOR4[((int32_t)(((na * 16) + nb)))])) * p);
        a /= 16;
        b /= 16;
        p *= 16;
        i += 1;
    }
    return r;
}

int64_t _Salam_crypto___and32_i64_i64(int64_t a0, int64_t b0) {
    int64_t a = (a0 % 4294967296LL);
    int64_t b = (b0 % 4294967296LL);
    int64_t r = 0;
    int64_t p = 1;
    int32_t i = 0;
    while ((i < 8)) {
        const int64_t na = (a % 16);
        const int64_t nb = (b % 16);
        r += (((int64_t)(AND4[((int32_t)(((na * 16) + nb)))])) * p);
        a /= 16;
        b /= 16;
        p *= 16;
        i += 1;
    }
    return r;
}

int64_t _Salam_crypto___not32_i64(int64_t a0) {
    return (4294967295LL - (a0 % 4294967296LL));
}

int64_t _Salam_crypto___rotr32_i64_i32(int64_t x0, int32_t n) {
    const int64_t x = (x0 % 4294967296LL);
    if ((n == 0)) {
        return x;
    }
    const int64_t hi = (x % POW2[n]);
    const int64_t lo = (x / POW2[n]);
    return (lo + (hi * POW2[(32 - n)]));
}

int64_t _Salam_crypto___shr32_i64_i32(int64_t x0, int32_t n) {
    const int64_t x = (x0 % 4294967296LL);
    if ((n == 0)) {
        return x;
    }
    if ((n >= 32)) {
        return 0;
    }
    return (x / POW2[n]);
}

int64_t _Salam_crypto___rotr32__2_i64(int64_t x0) {
    const int64_t x = (x0 % 4294967296LL);
    return ((x / 4) + ((x % 4) * 1073741824));
}

int64_t _Salam_crypto___rotr32__6_i64(int64_t x0) {
    const int64_t x = (x0 % 4294967296LL);
    return ((x / 64) + ((x % 64) * 67108864));
}

int64_t _Salam_crypto___rotr32__7_i64(int64_t x0) {
    const int64_t x = (x0 % 4294967296LL);
    return ((x / 128) + ((x % 128) * 33554432));
}

int64_t _Salam_crypto___rotr32__11_i64(int64_t x0) {
    const int64_t x = (x0 % 4294967296LL);
    return ((x / 2048) + ((x % 2048) * 2097152));
}

int64_t _Salam_crypto___rotr32__13_i64(int64_t x0) {
    const int64_t x = (x0 % 4294967296LL);
    return ((x / 8192) + ((x % 8192) * 524288));
}

int64_t _Salam_crypto___rotr32__17_i64(int64_t x0) {
    const int64_t x = (x0 % 4294967296LL);
    return ((x / 131072) + ((x % 131072) * 32768));
}

int64_t _Salam_crypto___rotr32__18_i64(int64_t x0) {
    const int64_t x = (x0 % 4294967296LL);
    return ((x / 262144) + ((x % 262144) * 16384));
}

int64_t _Salam_crypto___rotr32__19_i64(int64_t x0) {
    const int64_t x = (x0 % 4294967296LL);
    return ((x / 524288) + ((x % 524288) * 8192));
}

int64_t _Salam_crypto___rotr32__22_i64(int64_t x0) {
    const int64_t x = (x0 % 4294967296LL);
    return ((x / 4194304) + ((x % 4194304) * 1024));
}

int64_t _Salam_crypto___rotr32__25_i64(int64_t x0) {
    const int64_t x = (x0 % 4294967296LL);
    return ((x / 33554432) + ((x % 33554432) * 128));
}

int64_t _Salam_crypto___shr32__3_i64(int64_t x0) {
    return ((x0 % 4294967296LL) / 8);
}

int64_t _Salam_crypto___shr32__10_i64(int64_t x0) {
    return ((x0 % 4294967296LL) / 1024);
}

int32_t _Salam_crypto___xor8_i32_i32(int32_t a0, int32_t b0) {
    const int32_t na = (a0 % 16);
    const int32_t nb = (b0 % 16);
    const int32_t ha = (a0 / 16);
    const int32_t hb = (b0 / 16);
    return ((XOR4[((ha * 16) + hb)] * 16) + XOR4[((na * 16) + nb)]);
}

int32_t _Salam_crypto___bget_void_ptr_i32(void* p, int32_t i) {
    int8_t* bp = ((int8_t*)(p));
    int32_t v = ((int32_t)(bp[i]));
    if ((v < 0)) {
        v += 256;
    }
    return v;
}

void _Salam_crypto___bput_void_ptr_i32_i32(void* p, int32_t i, int32_t v) {
    int8_t* bp = ((int8_t*)(p));
    bp[i] = ((int8_t)(v));
}

void _Salam_crypto___put__word32_void_ptr_i32_i64(void* p, int32_t off, int64_t v) {
    _Salam_crypto___bput_void_ptr_i32_i32(p, off, ((int32_t)(((v / 16777216) % 256))));
    _Salam_crypto___bput_void_ptr_i32_i32(p, (off + 1), ((int32_t)(((v / 65536) % 256))));
    _Salam_crypto___bput_void_ptr_i32_i32(p, (off + 2), ((int32_t)(((v / 256) % 256))));
    _Salam_crypto___bput_void_ptr_i32_i32(p, (off + 3), ((int32_t)((v % 256))));
}

const char* _Salam_crypto___hex32_i64(int64_t v) {
    const char* digits = "0123456789abcdef";
    const char* result = "";
    int32_t k = 0;
    while ((k < 8)) {
        const int64_t d = ((v / POW2[(4 * (7 - k))]) % 16);
        result = salam_strcat(result, salam_str_substr(digits, ((int32_t)(d)), 1));
        k += 1;
    }
    return result;
}

const char* _Salam_crypto___hex__byte_i32(int32_t b) {
    const char* digits = "0123456789abcdef";
    return salam_strcat(salam_str_substr(digits, (b / 16), 1), salam_str_substr(digits, (b % 16), 1));
}

void _Salam_crypto___u64_i64_i64(int64_t hi, int64_t lo, crypto__U64Pair* __ret) {
    *__ret = ((crypto__U64Pair){ .hi = hi, .lo = lo }); return;
}

void _Salam_crypto___add64_crypto__U64Pair_crypto__U64Pair(crypto__U64Pair a, crypto__U64Pair b, crypto__U64Pair* __ret) {
    const int64_t lo__sum = (a.lo + b.lo);
    const int64_t carry = (lo__sum / 4294967296LL);
    const int64_t lo = (lo__sum % 4294967296LL);
    const int64_t hi = (((a.hi + b.hi) + carry) % 4294967296LL);
    *__ret = (({ crypto__U64Pair __s1; _Salam_crypto___u64_i64_i64(hi, lo, &__s1); __s1; })); return;
}

void _Salam_crypto___xor64_crypto__U64Pair_crypto__U64Pair(crypto__U64Pair a, crypto__U64Pair b, crypto__U64Pair* __ret) {
    *__ret = (({ crypto__U64Pair __s2; _Salam_crypto___u64_i64_i64(_Salam_crypto___xor32_i64_i64(a.hi, b.hi), _Salam_crypto___xor32_i64_i64(a.lo, b.lo), &__s2); __s2; })); return;
}

void _Salam_crypto___and64_crypto__U64Pair_crypto__U64Pair(crypto__U64Pair a, crypto__U64Pair b, crypto__U64Pair* __ret) {
    *__ret = (({ crypto__U64Pair __s3; _Salam_crypto___u64_i64_i64(_Salam_crypto___and32_i64_i64(a.hi, b.hi), _Salam_crypto___and32_i64_i64(a.lo, b.lo), &__s3); __s3; })); return;
}

void _Salam_crypto___not64_crypto__U64Pair(crypto__U64Pair a, crypto__U64Pair* __ret) {
    *__ret = (({ crypto__U64Pair __s4; _Salam_crypto___u64_i64_i64(_Salam_crypto___not32_i64(a.hi), _Salam_crypto___not32_i64(a.lo), &__s4); __s4; })); return;
}

void _Salam_crypto___rotr64_crypto__U64Pair_i32(crypto__U64Pair a, int32_t n0, crypto__U64Pair* __ret) {
    int32_t n = (n0 % 64);
    if ((n < 0)) {
        n += 64;
    }
    if ((n == 0)) {
        *__ret = (a); return;
    }
    if ((n == 32)) {
        *__ret = (({ crypto__U64Pair __s5; _Salam_crypto___u64_i64_i64(a.lo, a.hi, &__s5); __s5; })); return;
    }
    if ((n < 32)) {
        const int64_t nlo = ((a.lo / POW2[n]) + ((a.hi % POW2[n]) * POW2[(32 - n)]));
        const int64_t nhi = ((a.hi / POW2[n]) + ((a.lo % POW2[n]) * POW2[(32 - n)]));
        *__ret = (({ crypto__U64Pair __s6; _Salam_crypto___u64_i64_i64((nhi % 4294967296LL), (nlo % 4294967296LL), &__s6); __s6; })); return;
    }
    const int32_t m = (n - 32);
    const int64_t hi2 = a.lo;
    const int64_t lo2 = a.hi;
    if ((m == 0)) {
        *__ret = (({ crypto__U64Pair __s7; _Salam_crypto___u64_i64_i64(hi2, lo2, &__s7); __s7; })); return;
    }
    const int64_t nlo = ((lo2 / POW2[m]) + ((hi2 % POW2[m]) * POW2[(32 - m)]));
    const int64_t nhi = ((hi2 / POW2[m]) + ((lo2 % POW2[m]) * POW2[(32 - m)]));
    *__ret = (({ crypto__U64Pair __s8; _Salam_crypto___u64_i64_i64((nhi % 4294967296LL), (nlo % 4294967296LL), &__s8); __s8; })); return;
}

void _Salam_crypto___shr64_crypto__U64Pair_i32(crypto__U64Pair a, int32_t n, crypto__U64Pair* __ret) {
    if ((n == 0)) {
        *__ret = (a); return;
    }
    if ((n < 32)) {
        const int64_t nlo = ((a.lo / POW2[n]) + ((a.hi % POW2[n]) * POW2[(32 - n)]));
        const int64_t nhi = (a.hi / POW2[n]);
        *__ret = (({ crypto__U64Pair __s9; _Salam_crypto___u64_i64_i64((nhi % 4294967296LL), (nlo % 4294967296LL), &__s9); __s9; })); return;
    }
    if ((n < 64)) {
        const int32_t m = (n - 32);
        int64_t nlo = a.hi;
        if ((m > 0)) {
            nlo = (a.hi / POW2[m]);
        }
        *__ret = (({ crypto__U64Pair __s10; _Salam_crypto___u64_i64_i64(0, (nlo % 4294967296LL), &__s10); __s10; })); return;
    }
    *__ret = (({ crypto__U64Pair __s11; _Salam_crypto___u64_i64_i64(0, 0, &__s11); __s11; })); return;
}

void _Salam_crypto___hmac__sha256__prep_void_ptr_i32_void_ptr_void_ptr(void* key, int32_t klen, void* ipad, void* opad) {
    void* keysrc = key;
    int32_t effklen = klen;
    void* khash = NULL;
    if ((klen > 64)) {
        khash = _Salam_mem_Allocate_u64(32);
        _Salam_crypto___sha256__core_void_ptr_i32_void_ptr(key, klen, khash);
        keysrc = khash;
        effklen = 32;
    }
    void* keyblock = _Salam_mem_Allocate_u64(64);
    int32_t fi = 0;
    while ((fi < effklen)) {
        _Salam_crypto___bput_void_ptr_i32_i32(keyblock, fi, _Salam_crypto___bget_void_ptr_i32(keysrc, fi));
        fi += 1;
    }
    while ((fi < 64)) {
        _Salam_crypto___bput_void_ptr_i32_i32(keyblock, fi, 0);
        fi += 1;
    }
    if ((khash != NULL)) {
        _Salam_mem_Free_void_ptr(khash);
    }
    int32_t pi = 0;
    while ((pi < 64)) {
        const int32_t kb = _Salam_crypto___bget_void_ptr_i32(keyblock, pi);
        _Salam_crypto___bput_void_ptr_i32_i32(ipad, pi, _Salam_crypto___xor8_i32_i32(kb, 54));
        _Salam_crypto___bput_void_ptr_i32_i32(opad, pi, _Salam_crypto___xor8_i32_i32(kb, 92));
        pi += 1;
    }
    _Salam_mem_Free_void_ptr(keyblock);
}

void _Salam_crypto___hmac__sha256__apply_void_ptr_void_ptr_void_ptr_i32_void_ptr(void* ipad, void* opad, void* msg, int32_t mlen, void* outp) {
    const int32_t inner__len = (64 + mlen);
    void* inner__buf = _Salam_mem_Allocate_u64(((uint64_t)(inner__len)));
    _Salam_mem_Copy_void_ptr_void_ptr_u64(inner__buf, ipad, 64);
    void* inner__msg__dst = ((void*)(intptr_t)((((int64_t)(intptr_t)(inner__buf)) + 64)));
    _Salam_mem_Copy_void_ptr_void_ptr_u64(inner__msg__dst, msg, ((uint64_t)(mlen)));
    void* inner__hash = _Salam_mem_Allocate_u64(32);
    _Salam_crypto___sha256__core_void_ptr_i32_void_ptr(inner__buf, inner__len, inner__hash);
    void* outer__buf = _Salam_mem_Allocate_u64(96);
    _Salam_mem_Copy_void_ptr_void_ptr_u64(outer__buf, opad, 64);
    void* outer__hash__dst = ((void*)(intptr_t)((((int64_t)(intptr_t)(outer__buf)) + 64)));
    _Salam_mem_Copy_void_ptr_void_ptr_u64(outer__hash__dst, inner__hash, 32);
    _Salam_crypto___sha256__core_void_ptr_i32_void_ptr(outer__buf, 96, outp);
    _Salam_mem_Free_void_ptr(outer__buf);
    _Salam_mem_Free_void_ptr(inner__hash);
    _Salam_mem_Free_void_ptr(inner__buf);
}

void _Salam_crypto___hmac__fixed96__block2__init_void_ptr(void* blk2) {
    _Salam_crypto___bput_void_ptr_i32_i32(blk2, 32, 128);
    int32_t z = 33;
    while ((z < 62)) {
        _Salam_crypto___bput_void_ptr_i32_i32(blk2, z, 0);
        z += 1;
    }
    _Salam_crypto___bput_void_ptr_i32_i32(blk2, 62, 3);
    _Salam_crypto___bput_void_ptr_i32_i32(blk2, 63, 0);
}

void _Salam_crypto___hmac__sha256__apply32__fast_crypto__Sha256State_crypto__Sha256State_void_ptr_void_ptr_void_ptr(crypto__Sha256State mid__ipad, crypto__Sha256State mid__opad, void* blk2, void* msg32, void* outp) {
    _Salam_mem_Copy_void_ptr_void_ptr_u64(blk2, msg32, 32);
    crypto__Sha256State inner__final = ({ crypto__Sha256State __s12; _Salam_crypto___sha256__compress__block_crypto__Sha256State_void_ptr(mid__ipad, blk2, &__s12); __s12; });
    _Salam_crypto___sha256__write__state_crypto__Sha256State_void_ptr(inner__final, blk2);
    crypto__Sha256State outer__final = ({ crypto__Sha256State __s13; _Salam_crypto___sha256__compress__block_crypto__Sha256State_void_ptr(mid__opad, blk2, &__s13); __s13; });
    _Salam_crypto___sha256__write__state_crypto__Sha256State_void_ptr(outer__final, outp);
}

void _Salam_crypto___hmac__sha256__core_void_ptr_i32_void_ptr_i32_void_ptr(void* key, int32_t klen, void* msg, int32_t mlen, void* outp) {
    void* ipad = _Salam_mem_Allocate_u64(64);
    void* opad = _Salam_mem_Allocate_u64(64);
    _Salam_crypto___hmac__sha256__prep_void_ptr_i32_void_ptr_void_ptr(key, klen, ipad, opad);
    _Salam_crypto___hmac__sha256__apply_void_ptr_void_ptr_void_ptr_i32_void_ptr(ipad, opad, msg, mlen, outp);
    _Salam_mem_Free_void_ptr(opad);
    _Salam_mem_Free_void_ptr(ipad);
}

const char* _Salam_crypto_HmacSha256Hex_str_str(const char* key, const char* msg) {
    const int32_t klen = (int32_t)strlen(key);
    const int32_t mlen = (int32_t)strlen(msg);
    void* out = _Salam_mem_Allocate_u64(32);
    _Salam_crypto___hmac__sha256__core_void_ptr_i32_void_ptr_i32_void_ptr(((void*)(intptr_t)(key)), klen, ((void*)(intptr_t)(msg)), mlen, out);
    const char* result = "";
    int32_t i = 0;
    while ((i < 32)) {
        result = salam_strcat(result, _Salam_crypto___hex__byte_i32(_Salam_crypto___bget_void_ptr_i32(out, i)));
        i += 1;
    }
    _Salam_mem_Free_void_ptr(out);
    return result;
    _Salam_mem_Free_void_ptr(out);
}

void _Salam_crypto___pbkdf2__hmac__sha256__core_void_ptr_i32_void_ptr_i32_i32_void_ptr_i32(void* password, int32_t plen, void* salt, int32_t slen, int32_t iterations, void* outp, int32_t dklen) {
    void* ipad = _Salam_mem_Allocate_u64(64);
    void* opad = _Salam_mem_Allocate_u64(64);
    _Salam_crypto___hmac__sha256__prep_void_ptr_i32_void_ptr_void_ptr(password, plen, ipad, opad);
    crypto__Sha256State mid__ipad = ({ crypto__Sha256State __s15; _Salam_crypto___sha256__compress__block_crypto__Sha256State_void_ptr(({ crypto__Sha256State __s14; _Salam_crypto___sha256__iv(&__s14); __s14; }), ipad, &__s15); __s15; });
    crypto__Sha256State mid__opad = ({ crypto__Sha256State __s17; _Salam_crypto___sha256__compress__block_crypto__Sha256State_void_ptr(({ crypto__Sha256State __s16; _Salam_crypto___sha256__iv(&__s16); __s16; }), opad, &__s17); __s17; });
    void* blk2 = _Salam_mem_Allocate_u64(64);
    _Salam_crypto___hmac__fixed96__block2__init_void_ptr(blk2);
    void* block__salt = _Salam_mem_Allocate_u64(((uint64_t)((slen + 4))));
    _Salam_mem_Copy_void_ptr_void_ptr_u64(block__salt, salt, ((uint64_t)(slen)));
    void* u = _Salam_mem_Allocate_u64(32);
    void* acc = _Salam_mem_Allocate_u64(32);
    int32_t produced = 0;
    int32_t blocknum = 1;
    while ((produced < dklen)) {
        _Salam_crypto___bput_void_ptr_i32_i32(block__salt, slen, ((blocknum / 16777216) % 256));
        _Salam_crypto___bput_void_ptr_i32_i32(block__salt, (slen + 1), ((blocknum / 65536) % 256));
        _Salam_crypto___bput_void_ptr_i32_i32(block__salt, (slen + 2), ((blocknum / 256) % 256));
        _Salam_crypto___bput_void_ptr_i32_i32(block__salt, (slen + 3), (blocknum % 256));
        _Salam_crypto___hmac__sha256__apply_void_ptr_void_ptr_void_ptr_i32_void_ptr(ipad, opad, block__salt, (slen + 4), u);
        int32_t ci = 0;
        while ((ci < 32)) {
            _Salam_crypto___bput_void_ptr_i32_i32(acc, ci, _Salam_crypto___bget_void_ptr_i32(u, ci));
            ci += 1;
        }
        int32_t iter = 1;
        while ((iter < iterations)) {
            _Salam_crypto___hmac__sha256__apply32__fast_crypto__Sha256State_crypto__Sha256State_void_ptr_void_ptr_void_ptr(mid__ipad, mid__opad, blk2, u, u);
            int32_t cj = 0;
            while ((cj < 32)) {
                _Salam_crypto___bput_void_ptr_i32_i32(acc, cj, _Salam_crypto___xor8_i32_i32(_Salam_crypto___bget_void_ptr_i32(acc, cj), _Salam_crypto___bget_void_ptr_i32(u, cj)));
                cj += 1;
            }
            iter += 1;
        }
        int32_t take = 32;
        if (((dklen - produced) < 32)) {
            take = (dklen - produced);
        }
        int32_t ck = 0;
        while ((ck < take)) {
            _Salam_crypto___bput_void_ptr_i32_i32(outp, (produced + ck), _Salam_crypto___bget_void_ptr_i32(acc, ck));
            ck += 1;
        }
        produced += take;
        blocknum += 1;
    }
    _Salam_mem_Free_void_ptr(acc);
    _Salam_mem_Free_void_ptr(u);
    _Salam_mem_Free_void_ptr(block__salt);
    _Salam_mem_Free_void_ptr(blk2);
    _Salam_mem_Free_void_ptr(opad);
    _Salam_mem_Free_void_ptr(ipad);
}

const char* _Salam_crypto_Pbkdf2HmacSha256Hex_str_str_i32_i32(const char* password, const char* salt, int32_t iterations, int32_t keylen) {
    const int32_t plen = (int32_t)strlen(password);
    const int32_t slen = (int32_t)strlen(salt);
    void* outp = _Salam_mem_Allocate_u64(((uint64_t)(keylen)));
    _Salam_crypto___pbkdf2__hmac__sha256__core_void_ptr_i32_void_ptr_i32_i32_void_ptr_i32(((void*)(intptr_t)(password)), plen, ((void*)(intptr_t)(salt)), slen, iterations, outp, keylen);
    const char* result = "";
    int32_t i = 0;
    while ((i < keylen)) {
        result = salam_strcat(result, _Salam_crypto___hex__byte_i32(_Salam_crypto___bget_void_ptr_i32(outp, i)));
        i += 1;
    }
    _Salam_mem_Free_void_ptr(outp);
    return result;
    _Salam_mem_Free_void_ptr(outp);
}

bool _Salam_crypto___consttime__eq_str_str(const char* a, const char* b) {
    if (((int32_t)strlen(a) != (int32_t)strlen(b))) {
        return false;
    }
    const int32_t n = (int32_t)strlen(a);
    void* ap = ((void*)(intptr_t)(a));
    void* bp = ((void*)(intptr_t)(b));
    int32_t diff = 0;
    int32_t i = 0;
    while ((i < n)) {
        diff += _Salam_crypto___xor8_i32_i32(_Salam_crypto___bget_void_ptr_i32(ap, i), _Salam_crypto___bget_void_ptr_i32(bp, i));
        i += 1;
    }
    return (diff == 0);
}

const char* _Salam_crypto_HashPassword_str(const char* password) {
    const char* salt = _Salam_crypto_RandomHex_i32(16);
    const char* dk = _Salam_crypto_Pbkdf2HmacSha256Hex_str_str_i32_i32(password, salt, PBKDF2__DEFAULT__ITERATIONS, 32);
    return salam_strcat(salam_strcat(salam_strcat(salam_strcat(salam_strcat("pbkdf2-sha256$", _Salam_conv_FormatInt_i64(((int64_t)(PBKDF2__DEFAULT__ITERATIONS)))), "$"), salt), "$"), dk);
}

bool _Salam_crypto_VerifyPassword_str_str(const char* password, const char* encoded) {
    Vector__str parts = ({ Vector__str __s18; _Salam_str_Split_str_str(encoded, "$", &__s18); __s18; });
    if ((_Salam_g_S_Vector__str_len(&(parts)) != 4)) {
        return false;
    }
    if ((strcmp(_Salam_g_S_Vector__str_get_i32(&(parts), 0)[0], "pbkdf2-sha256") != 0)) {
        return false;
    }
    const int32_t iterations = ((int32_t)(_Salam_conv_ParseInt_str(_Salam_g_S_Vector__str_get_i32(&(parts), 1)[0])));
    if ((iterations <= 0)) {
        return false;
    }
    const char* salt = _Salam_g_S_Vector__str_get_i32(&(parts), 2)[0];
    const char* stored = _Salam_g_S_Vector__str_get_i32(&(parts), 3)[0];
    const int32_t keylen = ((int32_t)strlen(stored) / 2);
    const char* computed = _Salam_crypto_Pbkdf2HmacSha256Hex_str_str_i32_i32(password, salt, iterations, keylen);
    return _Salam_crypto___consttime__eq_str_str(computed, stored);
}

void _Salam_crypto___crypto__fatal(void) {
    abort();
}

void _Salam_crypto___os__random__fill_void_ptr_i32(void* buf, int32_t n) {
    void* f = fopen("/dev/urandom", "rb");
    if ((f == NULL)) {
        _Salam_crypto___crypto__fatal();
    }
    int32_t total = 0;
    while ((total < n)) {
        void* dst = ((void*)(intptr_t)((((int64_t)(intptr_t)(buf)) + total)));
        const uint64_t got = fread(dst, 1, ((uint64_t)((n - total))), f);
        if ((got == 0)) {
            fclose(f);
            _Salam_crypto___crypto__fatal();
        }
        total += ((int32_t)(got));
    }
    fclose(f);
}

const char* _Salam_crypto_RandomHex_i32(int32_t n) {
    if ((n <= 0)) {
        return "";
    }
    void* buf = _Salam_mem_Allocate_u64(((uint64_t)(n)));
    _Salam_crypto___os__random__fill_void_ptr_i32(buf, n);
    const char* result = "";
    int32_t i = 0;
    while ((i < n)) {
        result = salam_strcat(result, _Salam_crypto___hex__byte_i32(_Salam_crypto___bget_void_ptr_i32(buf, i)));
        i += 1;
    }
    _Salam_mem_Free_void_ptr(buf);
    return result;
    _Salam_mem_Free_void_ptr(buf);
}

const char* _Salam_crypto_RandomToken_i32(int32_t n) {
    return _Salam_crypto_RandomHex_i32(n);
}

int32_t _Salam_crypto___sha256__pad__byte_void_ptr_i32_i32_i32(void* data, int32_t dlen, int32_t total, int32_t i) {
    if ((i < dlen)) {
        return _Salam_crypto___bget_void_ptr_i32(data, i);
    }
    if ((i == dlen)) {
        return 128;
    }
    if ((i >= (total - 8))) {
        const int64_t ml = (((int64_t)(dlen)) * 8);
        const int32_t shift = ((total - 1) - i);
        int64_t d = ml;
        int32_t j = 0;
        while ((j < shift)) {
            d /= 256;
            j += 1;
        }
        return ((int32_t)((d % 256)));
    }
    return 0;
}

void _Salam_crypto___sha256__iv(crypto__Sha256State* __ret) {
    *__ret = ((crypto__Sha256State){ .h0 = 1779033703, .h1 = 3144134277LL, .h2 = 1013904242, .h3 = 2773480762LL, .h4 = 1359893119, .h5 = 2600822924LL, .h6 = 528734635, .h7 = 1541459225 }); return;
}

void _Salam_crypto___sha256__write__state_crypto__Sha256State_void_ptr(crypto__Sha256State st, void* outp) {
    _Salam_crypto___put__word32_void_ptr_i32_i64(outp, 0, st.h0);
    _Salam_crypto___put__word32_void_ptr_i32_i64(outp, 4, st.h1);
    _Salam_crypto___put__word32_void_ptr_i32_i64(outp, 8, st.h2);
    _Salam_crypto___put__word32_void_ptr_i32_i64(outp, 12, st.h3);
    _Salam_crypto___put__word32_void_ptr_i32_i64(outp, 16, st.h4);
    _Salam_crypto___put__word32_void_ptr_i32_i64(outp, 20, st.h5);
    _Salam_crypto___put__word32_void_ptr_i32_i64(outp, 24, st.h6);
    _Salam_crypto___put__word32_void_ptr_i32_i64(outp, 28, st.h7);
}

void _Salam_crypto___sha256__compress_crypto__Sha256State_i64_ptr(crypto__Sha256State* st, int64_t* w) {
    int64_t a = (*st).h0;
    int64_t b = (*st).h1;
    int64_t c = (*st).h2;
    int64_t d = (*st).h3;
    int64_t e = (*st).h4;
    int64_t f = (*st).h5;
    int64_t g = (*st).h6;
    int64_t h = (*st).h7;
    int32_t t = 0;
    while ((t < 64)) {
        const int64_t s1cap = _Salam_crypto___xor32_i64_i64(_Salam_crypto___xor32_i64_i64(_Salam_crypto___rotr32__6_i64(e), _Salam_crypto___rotr32__11_i64(e)), _Salam_crypto___rotr32__25_i64(e));
        const int64_t ch = _Salam_crypto___xor32_i64_i64(_Salam_crypto___and32_i64_i64(e, f), _Salam_crypto___and32_i64_i64(_Salam_crypto___not32_i64(e), g));
        const int64_t temp1 = (((((h + s1cap) + ch) + SHA256__K[t]) + w[t]) % 4294967296LL);
        const int64_t s0cap = _Salam_crypto___xor32_i64_i64(_Salam_crypto___xor32_i64_i64(_Salam_crypto___rotr32__2_i64(a), _Salam_crypto___rotr32__13_i64(a)), _Salam_crypto___rotr32__22_i64(a));
        const int64_t maj = _Salam_crypto___xor32_i64_i64(_Salam_crypto___xor32_i64_i64(_Salam_crypto___and32_i64_i64(a, b), _Salam_crypto___and32_i64_i64(a, c)), _Salam_crypto___and32_i64_i64(b, c));
        const int64_t temp2 = ((s0cap + maj) % 4294967296LL);
        h = g;
        g = f;
        f = e;
        e = ((d + temp1) % 4294967296LL);
        d = c;
        c = b;
        b = a;
        a = ((temp1 + temp2) % 4294967296LL);
        t += 1;
    }
    (*st).h0 = (((*st).h0 + a) % 4294967296LL);
    (*st).h1 = (((*st).h1 + b) % 4294967296LL);
    (*st).h2 = (((*st).h2 + c) % 4294967296LL);
    (*st).h3 = (((*st).h3 + d) % 4294967296LL);
    (*st).h4 = (((*st).h4 + e) % 4294967296LL);
    (*st).h5 = (((*st).h5 + f) % 4294967296LL);
    (*st).h6 = (((*st).h6 + g) % 4294967296LL);
    (*st).h7 = (((*st).h7 + h) % 4294967296LL);
}

void _Salam_crypto___sha256__compress__block_crypto__Sha256State_void_ptr(crypto__Sha256State st, void* block, crypto__Sha256State* __ret) {
    int64_t* w = ((int64_t*)(_Salam_mem_Allocate_u64((64 * 8))));
    int32_t t = 0;
    while ((t < 16)) {
        const int32_t base = (t * 4);
        w[t] = ((((((int64_t)(_Salam_crypto___bget_void_ptr_i32(block, base))) * 16777216) + (((int64_t)(_Salam_crypto___bget_void_ptr_i32(block, (base + 1)))) * 65536)) + (((int64_t)(_Salam_crypto___bget_void_ptr_i32(block, (base + 2)))) * 256)) + ((int64_t)(_Salam_crypto___bget_void_ptr_i32(block, (base + 3)))));
        t += 1;
    }
    while ((t < 64)) {
        const int64_t s0 = _Salam_crypto___xor32_i64_i64(_Salam_crypto___xor32_i64_i64(_Salam_crypto___rotr32__7_i64(w[(t - 15)]), _Salam_crypto___rotr32__18_i64(w[(t - 15)])), _Salam_crypto___shr32__3_i64(w[(t - 15)]));
        const int64_t s1 = _Salam_crypto___xor32_i64_i64(_Salam_crypto___xor32_i64_i64(_Salam_crypto___rotr32__17_i64(w[(t - 2)]), _Salam_crypto___rotr32__19_i64(w[(t - 2)])), _Salam_crypto___shr32__10_i64(w[(t - 2)]));
        w[t] = ((((w[(t - 16)] + s0) + w[(t - 7)]) + s1) % 4294967296LL);
        t += 1;
    }
    crypto__Sha256State next = st;
    _Salam_crypto___sha256__compress_crypto__Sha256State_i64_ptr((crypto__Sha256State*)&(next), w);
    _Salam_mem_Free_void_ptr(((void*)(w)));
    *__ret = (next); return;
    _Salam_mem_Free_void_ptr(((void*)(w)));
}

void _Salam_crypto___sha256__core_void_ptr_i32_void_ptr(void* data, int32_t dlen, void* outp) {
    const int32_t total = ((((dlen + 8) / 64) + 1) * 64);
    crypto__Sha256State st = ({ crypto__Sha256State __s19; _Salam_crypto___sha256__iv(&__s19); __s19; });
    int64_t* w = ((int64_t*)(_Salam_mem_Allocate_u64((64 * 8))));
    int32_t blk = 0;
    while ((blk < total)) {
        int32_t t = 0;
        while ((t < 16)) {
            const int32_t base = (blk + (t * 4));
            const int64_t b0 = ((int64_t)(_Salam_crypto___sha256__pad__byte_void_ptr_i32_i32_i32(data, dlen, total, base)));
            const int64_t b1 = ((int64_t)(_Salam_crypto___sha256__pad__byte_void_ptr_i32_i32_i32(data, dlen, total, (base + 1))));
            const int64_t b2 = ((int64_t)(_Salam_crypto___sha256__pad__byte_void_ptr_i32_i32_i32(data, dlen, total, (base + 2))));
            const int64_t b3 = ((int64_t)(_Salam_crypto___sha256__pad__byte_void_ptr_i32_i32_i32(data, dlen, total, (base + 3))));
            w[t] = ((((b0 * 16777216) + (b1 * 65536)) + (b2 * 256)) + b3);
            t += 1;
        }
        while ((t < 64)) {
            const int64_t s0 = _Salam_crypto___xor32_i64_i64(_Salam_crypto___xor32_i64_i64(_Salam_crypto___rotr32__7_i64(w[(t - 15)]), _Salam_crypto___rotr32__18_i64(w[(t - 15)])), _Salam_crypto___shr32__3_i64(w[(t - 15)]));
            const int64_t s1 = _Salam_crypto___xor32_i64_i64(_Salam_crypto___xor32_i64_i64(_Salam_crypto___rotr32__17_i64(w[(t - 2)]), _Salam_crypto___rotr32__19_i64(w[(t - 2)])), _Salam_crypto___shr32__10_i64(w[(t - 2)]));
            w[t] = ((((w[(t - 16)] + s0) + w[(t - 7)]) + s1) % 4294967296LL);
            t += 1;
        }
        _Salam_crypto___sha256__compress_crypto__Sha256State_i64_ptr((crypto__Sha256State*)&(st), w);
        blk += 64;
    }
    _Salam_crypto___sha256__write__state_crypto__Sha256State_void_ptr(st, outp);
    _Salam_mem_Free_void_ptr(((void*)(w)));
}

const char* _Salam_crypto_Sha256Hex_str(const char* data) {
    const int32_t dlen = (int32_t)strlen(data);
    void* out = _Salam_mem_Allocate_u64(32);
    _Salam_crypto___sha256__core_void_ptr_i32_void_ptr(((void*)(intptr_t)(data)), dlen, out);
    const char* result = "";
    int32_t i = 0;
    while ((i < 8)) {
        const int32_t base = (i * 4);
        const int64_t w = ((((((int64_t)(_Salam_crypto___bget_void_ptr_i32(out, base))) * 16777216) + (((int64_t)(_Salam_crypto___bget_void_ptr_i32(out, (base + 1)))) * 65536)) + (((int64_t)(_Salam_crypto___bget_void_ptr_i32(out, (base + 2)))) * 256)) + ((int64_t)(_Salam_crypto___bget_void_ptr_i32(out, (base + 3)))));
        result = salam_strcat(result, _Salam_crypto___hex32_i64(w));
        i += 1;
    }
    _Salam_mem_Free_void_ptr(out);
    return result;
    _Salam_mem_Free_void_ptr(out);
}

int32_t _Salam_crypto___sha512__pad__byte_void_ptr_i32_i32_i32(void* data, int32_t dlen, int32_t total, int32_t i) {
    if ((i < dlen)) {
        return _Salam_crypto___bget_void_ptr_i32(data, i);
    }
    if ((i == dlen)) {
        return 128;
    }
    if ((i >= (total - 8))) {
        const int64_t ml = (((int64_t)(dlen)) * 8);
        const int32_t shift = ((total - 1) - i);
        int64_t d = ml;
        int32_t j = 0;
        while ((j < shift)) {
            d /= 256;
            j += 1;
        }
        return ((int32_t)((d % 256)));
    }
    return 0;
}

void _Salam_crypto___sha512__core_void_ptr_i32_void_ptr(void* data, int32_t dlen, void* outp) {
    const int32_t total = ((((dlen + 16) / 128) + 1) * 128);
    crypto__U64Pair h0 = ({ crypto__U64Pair __s20; _Salam_crypto___u64_i64_i64(1779033703, 4089235720LL, &__s20); __s20; });
    crypto__U64Pair h1 = ({ crypto__U64Pair __s21; _Salam_crypto___u64_i64_i64(3144134277LL, 2227873595LL, &__s21); __s21; });
    crypto__U64Pair h2 = ({ crypto__U64Pair __s22; _Salam_crypto___u64_i64_i64(1013904242, 4271175723LL, &__s22); __s22; });
    crypto__U64Pair h3 = ({ crypto__U64Pair __s23; _Salam_crypto___u64_i64_i64(2773480762LL, 1595750129, &__s23); __s23; });
    crypto__U64Pair h4 = ({ crypto__U64Pair __s24; _Salam_crypto___u64_i64_i64(1359893119, 2917565137LL, &__s24); __s24; });
    crypto__U64Pair h5 = ({ crypto__U64Pair __s25; _Salam_crypto___u64_i64_i64(2600822924LL, 725511199, &__s25); __s25; });
    crypto__U64Pair h6 = ({ crypto__U64Pair __s26; _Salam_crypto___u64_i64_i64(528734635, 4215389547LL, &__s26); __s26; });
    crypto__U64Pair h7 = ({ crypto__U64Pair __s27; _Salam_crypto___u64_i64_i64(1541459225, 327033209, &__s27); __s27; });
    int64_t* w__hi = ((int64_t*)(_Salam_mem_Allocate_u64((80 * 8))));
    int64_t* w__lo = ((int64_t*)(_Salam_mem_Allocate_u64((80 * 8))));
    int32_t blk = 0;
    while ((blk < total)) {
        int32_t t = 0;
        while ((t < 16)) {
            const int32_t base = (blk + (t * 8));
            const int64_t b0 = ((int64_t)(_Salam_crypto___sha512__pad__byte_void_ptr_i32_i32_i32(data, dlen, total, base)));
            const int64_t b1 = ((int64_t)(_Salam_crypto___sha512__pad__byte_void_ptr_i32_i32_i32(data, dlen, total, (base + 1))));
            const int64_t b2 = ((int64_t)(_Salam_crypto___sha512__pad__byte_void_ptr_i32_i32_i32(data, dlen, total, (base + 2))));
            const int64_t b3 = ((int64_t)(_Salam_crypto___sha512__pad__byte_void_ptr_i32_i32_i32(data, dlen, total, (base + 3))));
            const int64_t b4 = ((int64_t)(_Salam_crypto___sha512__pad__byte_void_ptr_i32_i32_i32(data, dlen, total, (base + 4))));
            const int64_t b5 = ((int64_t)(_Salam_crypto___sha512__pad__byte_void_ptr_i32_i32_i32(data, dlen, total, (base + 5))));
            const int64_t b6 = ((int64_t)(_Salam_crypto___sha512__pad__byte_void_ptr_i32_i32_i32(data, dlen, total, (base + 6))));
            const int64_t b7 = ((int64_t)(_Salam_crypto___sha512__pad__byte_void_ptr_i32_i32_i32(data, dlen, total, (base + 7))));
            w__hi[t] = ((((b0 * 16777216) + (b1 * 65536)) + (b2 * 256)) + b3);
            w__lo[t] = ((((b4 * 16777216) + (b5 * 65536)) + (b6 * 256)) + b7);
            t += 1;
        }
        while ((t < 80)) {
            crypto__U64Pair wt15 = ({ crypto__U64Pair __s28; _Salam_crypto___u64_i64_i64(w__hi[(t - 15)], w__lo[(t - 15)], &__s28); __s28; });
            crypto__U64Pair wt2 = ({ crypto__U64Pair __s29; _Salam_crypto___u64_i64_i64(w__hi[(t - 2)], w__lo[(t - 2)], &__s29); __s29; });
            crypto__U64Pair w16 = ({ crypto__U64Pair __s30; _Salam_crypto___u64_i64_i64(w__hi[(t - 16)], w__lo[(t - 16)], &__s30); __s30; });
            crypto__U64Pair w7 = ({ crypto__U64Pair __s31; _Salam_crypto___u64_i64_i64(w__hi[(t - 7)], w__lo[(t - 7)], &__s31); __s31; });
            crypto__U64Pair s0 = ({ crypto__U64Pair __s36; _Salam_crypto___xor64_crypto__U64Pair_crypto__U64Pair(({ crypto__U64Pair __s34; _Salam_crypto___xor64_crypto__U64Pair_crypto__U64Pair(({ crypto__U64Pair __s32; _Salam_crypto___rotr64_crypto__U64Pair_i32(wt15, 1, &__s32); __s32; }), ({ crypto__U64Pair __s33; _Salam_crypto___rotr64_crypto__U64Pair_i32(wt15, 8, &__s33); __s33; }), &__s34); __s34; }), ({ crypto__U64Pair __s35; _Salam_crypto___shr64_crypto__U64Pair_i32(wt15, 7, &__s35); __s35; }), &__s36); __s36; });
            crypto__U64Pair s1 = ({ crypto__U64Pair __s41; _Salam_crypto___xor64_crypto__U64Pair_crypto__U64Pair(({ crypto__U64Pair __s39; _Salam_crypto___xor64_crypto__U64Pair_crypto__U64Pair(({ crypto__U64Pair __s37; _Salam_crypto___rotr64_crypto__U64Pair_i32(wt2, 19, &__s37); __s37; }), ({ crypto__U64Pair __s38; _Salam_crypto___rotr64_crypto__U64Pair_i32(wt2, 61, &__s38); __s38; }), &__s39); __s39; }), ({ crypto__U64Pair __s40; _Salam_crypto___shr64_crypto__U64Pair_i32(wt2, 6, &__s40); __s40; }), &__s41); __s41; });
            crypto__U64Pair wt = ({ crypto__U64Pair __s44; _Salam_crypto___add64_crypto__U64Pair_crypto__U64Pair(({ crypto__U64Pair __s42; _Salam_crypto___add64_crypto__U64Pair_crypto__U64Pair(w16, s0, &__s42); __s42; }), ({ crypto__U64Pair __s43; _Salam_crypto___add64_crypto__U64Pair_crypto__U64Pair(w7, s1, &__s43); __s43; }), &__s44); __s44; });
            w__hi[t] = wt.hi;
            w__lo[t] = wt.lo;
            t += 1;
        }
        crypto__U64Pair a = h0;
        crypto__U64Pair b = h1;
        crypto__U64Pair c = h2;
        crypto__U64Pair d = h3;
        crypto__U64Pair e = h4;
        crypto__U64Pair f = h5;
        crypto__U64Pair g = h6;
        crypto__U64Pair h = h7;
        t = 0;
        while ((t < 80)) {
            crypto__U64Pair s1cap = ({ crypto__U64Pair __s49; _Salam_crypto___xor64_crypto__U64Pair_crypto__U64Pair(({ crypto__U64Pair __s47; _Salam_crypto___xor64_crypto__U64Pair_crypto__U64Pair(({ crypto__U64Pair __s45; _Salam_crypto___rotr64_crypto__U64Pair_i32(e, 14, &__s45); __s45; }), ({ crypto__U64Pair __s46; _Salam_crypto___rotr64_crypto__U64Pair_i32(e, 18, &__s46); __s46; }), &__s47); __s47; }), ({ crypto__U64Pair __s48; _Salam_crypto___rotr64_crypto__U64Pair_i32(e, 41, &__s48); __s48; }), &__s49); __s49; });
            crypto__U64Pair ch = ({ crypto__U64Pair __s53; _Salam_crypto___xor64_crypto__U64Pair_crypto__U64Pair(({ crypto__U64Pair __s50; _Salam_crypto___and64_crypto__U64Pair_crypto__U64Pair(e, f, &__s50); __s50; }), ({ crypto__U64Pair __s52; _Salam_crypto___and64_crypto__U64Pair_crypto__U64Pair(({ crypto__U64Pair __s51; _Salam_crypto___not64_crypto__U64Pair(e, &__s51); __s51; }), g, &__s52); __s52; }), &__s53); __s53; });
            crypto__U64Pair k = ({ crypto__U64Pair __s54; _Salam_crypto___u64_i64_i64(SHA512__K__HI[t], SHA512__K__LO[t], &__s54); __s54; });
            crypto__U64Pair wtc = ({ crypto__U64Pair __s55; _Salam_crypto___u64_i64_i64(w__hi[t], w__lo[t], &__s55); __s55; });
            crypto__U64Pair temp1 = ({ crypto__U64Pair __s59; _Salam_crypto___add64_crypto__U64Pair_crypto__U64Pair(({ crypto__U64Pair __s57; _Salam_crypto___add64_crypto__U64Pair_crypto__U64Pair(({ crypto__U64Pair __s56; _Salam_crypto___add64_crypto__U64Pair_crypto__U64Pair(h, s1cap, &__s56); __s56; }), ch, &__s57); __s57; }), ({ crypto__U64Pair __s58; _Salam_crypto___add64_crypto__U64Pair_crypto__U64Pair(k, wtc, &__s58); __s58; }), &__s59); __s59; });
            crypto__U64Pair s0cap = ({ crypto__U64Pair __s64; _Salam_crypto___xor64_crypto__U64Pair_crypto__U64Pair(({ crypto__U64Pair __s62; _Salam_crypto___xor64_crypto__U64Pair_crypto__U64Pair(({ crypto__U64Pair __s60; _Salam_crypto___rotr64_crypto__U64Pair_i32(a, 28, &__s60); __s60; }), ({ crypto__U64Pair __s61; _Salam_crypto___rotr64_crypto__U64Pair_i32(a, 34, &__s61); __s61; }), &__s62); __s62; }), ({ crypto__U64Pair __s63; _Salam_crypto___rotr64_crypto__U64Pair_i32(a, 39, &__s63); __s63; }), &__s64); __s64; });
            crypto__U64Pair maj = ({ crypto__U64Pair __s69; _Salam_crypto___xor64_crypto__U64Pair_crypto__U64Pair(({ crypto__U64Pair __s67; _Salam_crypto___xor64_crypto__U64Pair_crypto__U64Pair(({ crypto__U64Pair __s65; _Salam_crypto___and64_crypto__U64Pair_crypto__U64Pair(a, b, &__s65); __s65; }), ({ crypto__U64Pair __s66; _Salam_crypto___and64_crypto__U64Pair_crypto__U64Pair(a, c, &__s66); __s66; }), &__s67); __s67; }), ({ crypto__U64Pair __s68; _Salam_crypto___and64_crypto__U64Pair_crypto__U64Pair(b, c, &__s68); __s68; }), &__s69); __s69; });
            crypto__U64Pair temp2 = ({ crypto__U64Pair __s70; _Salam_crypto___add64_crypto__U64Pair_crypto__U64Pair(s0cap, maj, &__s70); __s70; });
            h = g;
            g = f;
            f = e;
            e = ({ crypto__U64Pair __s71; _Salam_crypto___add64_crypto__U64Pair_crypto__U64Pair(d, temp1, &__s71); __s71; });
            d = c;
            c = b;
            b = a;
            a = ({ crypto__U64Pair __s72; _Salam_crypto___add64_crypto__U64Pair_crypto__U64Pair(temp1, temp2, &__s72); __s72; });
            t += 1;
        }
        h0 = ({ crypto__U64Pair __s73; _Salam_crypto___add64_crypto__U64Pair_crypto__U64Pair(h0, a, &__s73); __s73; });
        h1 = ({ crypto__U64Pair __s74; _Salam_crypto___add64_crypto__U64Pair_crypto__U64Pair(h1, b, &__s74); __s74; });
        h2 = ({ crypto__U64Pair __s75; _Salam_crypto___add64_crypto__U64Pair_crypto__U64Pair(h2, c, &__s75); __s75; });
        h3 = ({ crypto__U64Pair __s76; _Salam_crypto___add64_crypto__U64Pair_crypto__U64Pair(h3, d, &__s76); __s76; });
        h4 = ({ crypto__U64Pair __s77; _Salam_crypto___add64_crypto__U64Pair_crypto__U64Pair(h4, e, &__s77); __s77; });
        h5 = ({ crypto__U64Pair __s78; _Salam_crypto___add64_crypto__U64Pair_crypto__U64Pair(h5, f, &__s78); __s78; });
        h6 = ({ crypto__U64Pair __s79; _Salam_crypto___add64_crypto__U64Pair_crypto__U64Pair(h6, g, &__s79); __s79; });
        h7 = ({ crypto__U64Pair __s80; _Salam_crypto___add64_crypto__U64Pair_crypto__U64Pair(h7, h, &__s80); __s80; });
        blk += 128;
    }
    _Salam_crypto___put__word32_void_ptr_i32_i64(outp, 0, h0.hi);
    _Salam_crypto___put__word32_void_ptr_i32_i64(outp, 4, h0.lo);
    _Salam_crypto___put__word32_void_ptr_i32_i64(outp, 8, h1.hi);
    _Salam_crypto___put__word32_void_ptr_i32_i64(outp, 12, h1.lo);
    _Salam_crypto___put__word32_void_ptr_i32_i64(outp, 16, h2.hi);
    _Salam_crypto___put__word32_void_ptr_i32_i64(outp, 20, h2.lo);
    _Salam_crypto___put__word32_void_ptr_i32_i64(outp, 24, h3.hi);
    _Salam_crypto___put__word32_void_ptr_i32_i64(outp, 28, h3.lo);
    _Salam_crypto___put__word32_void_ptr_i32_i64(outp, 32, h4.hi);
    _Salam_crypto___put__word32_void_ptr_i32_i64(outp, 36, h4.lo);
    _Salam_crypto___put__word32_void_ptr_i32_i64(outp, 40, h5.hi);
    _Salam_crypto___put__word32_void_ptr_i32_i64(outp, 44, h5.lo);
    _Salam_crypto___put__word32_void_ptr_i32_i64(outp, 48, h6.hi);
    _Salam_crypto___put__word32_void_ptr_i32_i64(outp, 52, h6.lo);
    _Salam_crypto___put__word32_void_ptr_i32_i64(outp, 56, h7.hi);
    _Salam_crypto___put__word32_void_ptr_i32_i64(outp, 60, h7.lo);
    _Salam_mem_Free_void_ptr(((void*)(w__lo)));
    _Salam_mem_Free_void_ptr(((void*)(w__hi)));
}

const char* _Salam_crypto_Sha512Hex_str(const char* data) {
    const int32_t dlen = (int32_t)strlen(data);
    void* out = _Salam_mem_Allocate_u64(64);
    _Salam_crypto___sha512__core_void_ptr_i32_void_ptr(((void*)(intptr_t)(data)), dlen, out);
    const char* result = "";
    int32_t i = 0;
    while ((i < 16)) {
        const int32_t base = (i * 4);
        const int64_t w = ((((((int64_t)(_Salam_crypto___bget_void_ptr_i32(out, base))) * 16777216) + (((int64_t)(_Salam_crypto___bget_void_ptr_i32(out, (base + 1)))) * 65536)) + (((int64_t)(_Salam_crypto___bget_void_ptr_i32(out, (base + 2)))) * 256)) + ((int64_t)(_Salam_crypto___bget_void_ptr_i32(out, (base + 3)))));
        result = salam_strcat(result, _Salam_crypto___hex32_i64(w));
        i += 1;
    }
    _Salam_mem_Free_void_ptr(out);
    return result;
    _Salam_mem_Free_void_ptr(out);
}
