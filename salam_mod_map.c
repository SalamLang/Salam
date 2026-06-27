#include "salam_mod_map.h"

static const char* _Salam_map___ptrstr_i64(int64_t p);
static int32_t _Salam_map___m__keykind_void_ptr(void* m);
static int64_t _Salam_map___m__cap_void_ptr(void* m);
static int64_t _Salam_map___m__count_void_ptr(void* m);
static int32_t* _Salam_map___m__state_void_ptr(void* m);
static int64_t* _Salam_map___m__keys_void_ptr(void* m);
static int64_t* _Salam_map___m__vals_void_ptr(void* m);
static void _Salam_map___m__set__cap_void_ptr_i64(void* m, int64_t v);
static void _Salam_map___m__set__count_void_ptr_i64(void* m, int64_t v);
static void _Salam_map___m__set__state_void_ptr_void_ptr(void* m, void* v);
static void _Salam_map___m__set__keys_void_ptr_void_ptr(void* m, void* v);
static void _Salam_map___m__set__vals_void_ptr_void_ptr(void* m, void* v);
static uint64_t _Salam_map___m__hash_void_ptr_i64(void* m, int64_t key);
static bool _Salam_map___m__keq_void_ptr_i64_i64(void* m, int64_t a, int64_t b);
static int64_t _Salam_map___m__find_void_ptr_i64_void_ptr(void* m, int64_t key, void* found);
static void _Salam_map___m__raw__insert_void_ptr_i64_i64(void* m, int64_t key, int64_t val);
static void _Salam_map___m__grow_void_ptr(void* m);
static void* _Salam_map___it__map_void_ptr(void* it);
static int64_t _Salam_map___it__i_void_ptr(void* it);
static void _Salam_map___it__set__i_void_ptr_i64(void* it, int64_t v);
static void _Salam_map___it__seek_void_ptr(void* it);

static const char* _Salam_map___ptrstr_i64(int64_t p) {
    void* v = ((void*)(intptr_t)(p));
    return ((const char*)(v));
}

static int32_t _Salam_map___m__keykind_void_ptr(void* m) {
    int32_t* p = ((int32_t*)(m));
    return p[0];
}

static int64_t _Salam_map___m__cap_void_ptr(void* m) {
    int64_t* p = ((int64_t*)(m));
    return p[1];
}

static int64_t _Salam_map___m__count_void_ptr(void* m) {
    int64_t* p = ((int64_t*)(m));
    return p[2];
}

static int32_t* _Salam_map___m__state_void_ptr(void* m) {
    int64_t* p = ((int64_t*)(m));
    void* sp = ((void*)(intptr_t)(p[3]));
    return ((int32_t*)(sp));
}

static int64_t* _Salam_map___m__keys_void_ptr(void* m) {
    int64_t* p = ((int64_t*)(m));
    void* kp = ((void*)(intptr_t)(p[4]));
    return ((int64_t*)(kp));
}

static int64_t* _Salam_map___m__vals_void_ptr(void* m) {
    int64_t* p = ((int64_t*)(m));
    void* vp = ((void*)(intptr_t)(p[5]));
    return ((int64_t*)(vp));
}

static void _Salam_map___m__set__cap_void_ptr_i64(void* m, int64_t v) {
    int64_t* p = ((int64_t*)(m));
    p[1] = v;
}

static void _Salam_map___m__set__count_void_ptr_i64(void* m, int64_t v) {
    int64_t* p = ((int64_t*)(m));
    p[2] = v;
}

static void _Salam_map___m__set__state_void_ptr_void_ptr(void* m, void* v) {
    int64_t* p = ((int64_t*)(m));
    p[3] = ((int64_t)(intptr_t)(v));
}

static void _Salam_map___m__set__keys_void_ptr_void_ptr(void* m, void* v) {
    int64_t* p = ((int64_t*)(m));
    p[4] = ((int64_t)(intptr_t)(v));
}

static void _Salam_map___m__set__vals_void_ptr_void_ptr(void* m, void* v) {
    int64_t* p = ((int64_t*)(m));
    p[5] = ((int64_t)(intptr_t)(v));
}

static uint64_t _Salam_map___m__hash_void_ptr_i64(void* m, int64_t key) {
    if ((_Salam_map___m__keykind_void_ptr(m) == 1)) {
        int8_t* bp = ((int8_t*)(((void*)(intptr_t)(key))));
        uint64_t h = ((uint64_t)(5381));
        int32_t j = 0;
        while ((bp[j] != 0)) {
            int32_t c = ((int32_t)(bp[j]));
            if ((c < 0)) {
                c = (c + 256);
            }
            h = ((h * ((uint64_t)(33))) + ((uint64_t)(c)));
            j = (j + 1);
        }
        return h;
    }
    return (((uint64_t)(key)) * ((uint64_t)(2654435761LL)));
}

static bool _Salam_map___m__keq_void_ptr_i64_i64(void* m, int64_t a, int64_t b) {
    if ((_Salam_map___m__keykind_void_ptr(m) == 1)) {
        return (strcmp(_Salam_map___ptrstr_i64(a), _Salam_map___ptrstr_i64(b)) == 0);
    }
    return (a == b);
}

static int64_t _Salam_map___m__find_void_ptr_i64_void_ptr(void* m, int64_t key, void* found) {
    int32_t* fp = ((int32_t*)(found));
    const int64_t cap = _Salam_map___m__cap_void_ptr(m);
    int32_t* st = _Salam_map___m__state_void_ptr(m);
    int64_t* keys = _Salam_map___m__keys_void_ptr(m);
    const uint64_t h = _Salam_map___m__hash_void_ptr_i64(m, key);
    int64_t i = ((int64_t)((h % ((uint64_t)(cap)))));
    int64_t free__slot = (0 - 1);
    int64_t n = 0;
    while ((n < cap)) {
        const int32_t s = st[i];
        if ((s == 0)) {
            fp[0] = 0;
            if ((free__slot != (0 - 1))) {
                return free__slot;
            }
            return i;
        }
        if ((s == 2)) {
            if ((free__slot == (0 - 1))) {
                free__slot = i;
            }
        } else {
            if (_Salam_map___m__keq_void_ptr_i64_i64(m, keys[i], key)) {
                fp[0] = 1;
                return i;
            }
        }
        i = (i + 1);
        if ((i >= cap)) {
            i = 0;
        }
        n = (n + 1);
    }
    fp[0] = 0;
    return free__slot;
}

static void _Salam_map___m__raw__insert_void_ptr_i64_i64(void* m, int64_t key, int64_t val) {
    void* found = _Salam_mem_AllocateZeroed_u64(((uint64_t)(4)));
    const int64_t i = _Salam_map___m__find_void_ptr_i64_void_ptr(m, key, found);
    _Salam_mem_Free_void_ptr(found);
    int32_t* st = _Salam_map___m__state_void_ptr(m);
    int64_t* keys = _Salam_map___m__keys_void_ptr(m);
    int64_t* vals = _Salam_map___m__vals_void_ptr(m);
    st[i] = 1;
    keys[i] = key;
    vals[i] = val;
    _Salam_map___m__set__count_void_ptr_i64(m, (_Salam_map___m__count_void_ptr(m) + 1));
}

static void _Salam_map___m__grow_void_ptr(void* m) {
    const int64_t oc = _Salam_map___m__cap_void_ptr(m);
    int32_t* ost = _Salam_map___m__state_void_ptr(m);
    int64_t* okeys = _Salam_map___m__keys_void_ptr(m);
    int64_t* ovals = _Salam_map___m__vals_void_ptr(m);
    const int64_t ncap = (oc * 2);
    _Salam_map___m__set__cap_void_ptr_i64(m, ncap);
    _Salam_map___m__set__count_void_ptr_i64(m, 0);
    _Salam_map___m__set__state_void_ptr_void_ptr(m, _Salam_mem_AllocateZeroed_u64(((uint64_t)((ncap * 4)))));
    _Salam_map___m__set__keys_void_ptr_void_ptr(m, _Salam_mem_AllocateZeroed_u64(((uint64_t)((ncap * 8)))));
    _Salam_map___m__set__vals_void_ptr_void_ptr(m, _Salam_mem_AllocateZeroed_u64(((uint64_t)((ncap * 8)))));
    int64_t idx = 0;
    while ((idx < oc)) {
        if ((ost[idx] == 1)) {
            _Salam_map___m__raw__insert_void_ptr_i64_i64(m, okeys[idx], ovals[idx]);
        }
        idx = (idx + 1);
    }
    _Salam_mem_Free_void_ptr(((void*)(ost)));
    _Salam_mem_Free_void_ptr(((void*)(okeys)));
    _Salam_mem_Free_void_ptr(((void*)(ovals)));
}

void* salam_map_new(int32_t key__kind, int32_t val__size) {
    void* m = _Salam_mem_AllocateZeroed_u64(((uint64_t)(48)));
    int32_t* mi32 = ((int32_t*)(m));
    mi32[0] = key__kind;
    int32_t vs = val__size;
    if ((vs <= 0)) {
        vs = 1;
    }
    mi32[1] = vs;
    _Salam_map___m__set__cap_void_ptr_i64(m, 16);
    _Salam_map___m__set__count_void_ptr_i64(m, 0);
    _Salam_map___m__set__state_void_ptr_void_ptr(m, _Salam_mem_AllocateZeroed_u64(((uint64_t)((16 * 4)))));
    _Salam_map___m__set__keys_void_ptr_void_ptr(m, _Salam_mem_AllocateZeroed_u64(((uint64_t)((16 * 8)))));
    _Salam_map___m__set__vals_void_ptr_void_ptr(m, _Salam_mem_AllocateZeroed_u64(((uint64_t)((16 * 8)))));
    return m;
}

void salam_map_put(void* m, int64_t key, int64_t val) {
    if ((((_Salam_map___m__count_void_ptr(m) + 1) * 10) >= (_Salam_map___m__cap_void_ptr(m) * 7))) {
        _Salam_map___m__grow_void_ptr(m);
    }
    void* found = _Salam_mem_AllocateZeroed_u64(((uint64_t)(4)));
    const int64_t i = _Salam_map___m__find_void_ptr_i64_void_ptr(m, key, found);
    int32_t* fp = ((int32_t*)(found));
    const int32_t f = fp[0];
    _Salam_mem_Free_void_ptr(found);
    int32_t* st = _Salam_map___m__state_void_ptr(m);
    int64_t* keys = _Salam_map___m__keys_void_ptr(m);
    int64_t* vals = _Salam_map___m__vals_void_ptr(m);
    if ((f == 0)) {
        st[i] = 1;
        keys[i] = key;
        _Salam_map___m__set__count_void_ptr_i64(m, (_Salam_map___m__count_void_ptr(m) + 1));
    }
    vals[i] = val;
}

int64_t salam_map_get(void* m, int64_t key) {
    void* found = _Salam_mem_AllocateZeroed_u64(((uint64_t)(4)));
    const int64_t i = _Salam_map___m__find_void_ptr_i64_void_ptr(m, key, found);
    int32_t* fp = ((int32_t*)(found));
    const int32_t f = fp[0];
    _Salam_mem_Free_void_ptr(found);
    if ((f == 0)) {
        return 0;
    }
    int64_t* vals = _Salam_map___m__vals_void_ptr(m);
    return vals[i];
}

int32_t salam_map_has(void* m, int64_t key) {
    void* found = _Salam_mem_AllocateZeroed_u64(((uint64_t)(4)));
    _Salam_map___m__find_void_ptr_i64_void_ptr(m, key, found);
    int32_t* fp = ((int32_t*)(found));
    const int32_t f = fp[0];
    _Salam_mem_Free_void_ptr(found);
    return f;
}

int32_t salam_map_remove(void* m, int64_t key) {
    void* found = _Salam_mem_AllocateZeroed_u64(((uint64_t)(4)));
    const int64_t i = _Salam_map___m__find_void_ptr_i64_void_ptr(m, key, found);
    int32_t* fp = ((int32_t*)(found));
    const int32_t f = fp[0];
    _Salam_mem_Free_void_ptr(found);
    if ((f == 0)) {
        return 0;
    }
    int32_t* st = _Salam_map___m__state_void_ptr(m);
    st[i] = 2;
    _Salam_map___m__set__count_void_ptr_i64(m, (_Salam_map___m__count_void_ptr(m) - 1));
    return 1;
}

int32_t salam_map_size(void* m) {
    return ((int32_t)(_Salam_map___m__count_void_ptr(m)));
}

static void* _Salam_map___it__map_void_ptr(void* it) {
    int64_t* p = ((int64_t*)(it));
    return ((void*)(intptr_t)(p[0]));
}

static int64_t _Salam_map___it__i_void_ptr(void* it) {
    int64_t* p = ((int64_t*)(it));
    return p[1];
}

static void _Salam_map___it__set__i_void_ptr_i64(void* it, int64_t v) {
    int64_t* p = ((int64_t*)(it));
    p[1] = v;
}

static void _Salam_map___it__seek_void_ptr(void* it) {
    void* m = _Salam_map___it__map_void_ptr(it);
    const int64_t cap = _Salam_map___m__cap_void_ptr(m);
    int32_t* st = _Salam_map___m__state_void_ptr(m);
    int64_t i = _Salam_map___it__i_void_ptr(it);
    while ((i < cap)) {
        if ((st[i] == 1)) {
            _Salam_map___it__set__i_void_ptr_i64(it, i);
            return;
        }
        i = (i + 1);
    }
    _Salam_map___it__set__i_void_ptr_i64(it, cap);
}

void* salam_map_iter_new(void* m) {
    void* it = _Salam_mem_AllocateZeroed_u64(((uint64_t)(16)));
    int64_t* p = ((int64_t*)(it));
    p[0] = ((int64_t)(intptr_t)(m));
    p[1] = 0;
    _Salam_map___it__seek_void_ptr(it);
    return it;
}

int32_t salam_map_iter_valid(void* it) {
    void* m = _Salam_map___it__map_void_ptr(it);
    if ((_Salam_map___it__i_void_ptr(it) < _Salam_map___m__cap_void_ptr(m))) {
        return 1;
    }
    return 0;
}

int64_t salam_map_iter_key(void* it) {
    void* m = _Salam_map___it__map_void_ptr(it);
    int64_t* keys = _Salam_map___m__keys_void_ptr(m);
    return keys[_Salam_map___it__i_void_ptr(it)];
}

int64_t salam_map_iter_value(void* it) {
    void* m = _Salam_map___it__map_void_ptr(it);
    int64_t* vals = _Salam_map___m__vals_void_ptr(m);
    return vals[_Salam_map___it__i_void_ptr(it)];
}

void salam_map_iter_next(void* it) {
    _Salam_map___it__set__i_void_ptr_i64(it, (_Salam_map___it__i_void_ptr(it) + 1));
    _Salam_map___it__seek_void_ptr(it);
}
