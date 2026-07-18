#include "salam_mod_sort.h"

int32_t _Salam_sort___bucket__of_i32_i32_i64_i32(int32_t x, int32_t lo, int64_t span, int32_t n);

void _Salam_sort_BucketSort_Vector__i32(Vector__i32 v) {
    const int32_t n = _Salam_g_S_Vector__i32_len(&(v));
    if ((n < 2)) {
        return;
    }
    int32_t lo = _Salam_g_S_Vector__i32_get_i32(&(v), 0)[0];
    int32_t hi = lo;
    int32_t i = 1;
    while ((i < n)) {
        const int32_t x = _Salam_g_S_Vector__i32_get_i32(&(v), i)[0];
        if ((x < lo)) {
            lo = x;
        }
        if ((hi < x)) {
            hi = x;
        }
        i += 1;
    }
    if ((lo == hi)) {
        return;
    }
    const int64_t span = ((((int64_t)(hi)) - ((int64_t)(lo))) + 1);
    Vector__i32 count = (Vector__i32){ .data = NULL, .__len = 0, .__cap = 0 };
    int32_t b = 0;
    while ((b <= n)) {
        _Salam_g_S_Vector__i32_push_i32(&(count), 0);
        b += 1;
    }
    i = 0;
    while ((i < n)) {
        const int32_t bi = _Salam_sort___bucket__of_i32_i32_i64_i32(_Salam_g_S_Vector__i32_get_i32(&(v), i)[0], lo, span, n);
        _Salam_g_S_Vector__i32_set_i32_i32(&(count), bi, (_Salam_g_S_Vector__i32_get_i32(&(count), bi)[0] + 1));
        i += 1;
    }
    int32_t s = 1;
    while ((s <= n)) {
        _Salam_g_S_Vector__i32_set_i32_i32(&(count), s, (_Salam_g_S_Vector__i32_get_i32(&(count), s)[0] + _Salam_g_S_Vector__i32_get_i32(&(count), (s - 1))[0]));
        s += 1;
    }
    Vector__i32 starts = (Vector__i32){ .data = NULL, .__len = 0, .__cap = 0 };
    int32_t k = 0;
    while ((k < n)) {
        if ((k == 0)) {
            _Salam_g_S_Vector__i32_push_i32(&(starts), 0);
        } else {
            _Salam_g_S_Vector__i32_push_i32(&(starts), _Salam_g_S_Vector__i32_get_i32(&(count), (k - 1))[0]);
        }
        k += 1;
    }
    Vector__i32 cursor = (Vector__i32){ .data = NULL, .__len = 0, .__cap = 0 };
    k = 0;
    while ((k < n)) {
        _Salam_g_S_Vector__i32_push_i32(&(cursor), _Salam_g_S_Vector__i32_get_i32(&(starts), k)[0]);
        k += 1;
    }
    Vector__i32 out = (Vector__i32){ .data = NULL, .__len = 0, .__cap = 0 };
    i = 0;
    while ((i < n)) {
        _Salam_g_S_Vector__i32_push_i32(&(out), 0);
        i += 1;
    }
    i = 0;
    while ((i < n)) {
        const int32_t bi = _Salam_sort___bucket__of_i32_i32_i64_i32(_Salam_g_S_Vector__i32_get_i32(&(v), i)[0], lo, span, n);
        const int32_t pos = _Salam_g_S_Vector__i32_get_i32(&(cursor), bi)[0];
        _Salam_g_S_Vector__i32_set_i32_i32(&(out), pos, _Salam_g_S_Vector__i32_get_i32(&(v), i)[0]);
        _Salam_g_S_Vector__i32_set_i32_i32(&(cursor), bi, (pos + 1));
        i += 1;
    }
    k = 0;
    while ((k < n)) {
        const int32_t bstart = _Salam_g_S_Vector__i32_get_i32(&(starts), k)[0];
        int32_t bend = n;
        if ((k < (n - 1))) {
            bend = _Salam_g_S_Vector__i32_get_i32(&(starts), (k + 1))[0];
        }
        if (((bend - bstart) > 1)) {
            _Salam_g___insertion__range__i32_Vector__i32_i32_i32(out, bstart, (bend - 1));
        }
        k += 1;
    }
    i = 0;
    while ((i < n)) {
        _Salam_g_S_Vector__i32_set_i32_i32(&(v), i, _Salam_g_S_Vector__i32_get_i32(&(out), i)[0]);
        i += 1;
    }
    _Salam_g_S_Vector__i32_free(&(out));
    _Salam_g_S_Vector__i32_free(&(cursor));
    _Salam_g_S_Vector__i32_free(&(starts));
    _Salam_g_S_Vector__i32_free(&(count));
}

int32_t _Salam_sort___bucket__of_i32_i32_i64_i32(int32_t x, int32_t lo, int64_t span, int32_t n) {
    const int64_t off = (((int64_t)(x)) - ((int64_t)(lo)));
    const int64_t bi = ((off * ((int64_t)(n))) / span);
    if ((bi >= ((int64_t)(n)))) {
        return (n - 1);
    }
    return ((int32_t)(bi));
}

void _Salam_sort_CountingSort_Vector__i32(Vector__i32 v) {
    const int32_t n = _Salam_g_S_Vector__i32_len(&(v));
    if ((n < 2)) {
        return;
    }
    int32_t lo = _Salam_g_S_Vector__i32_get_i32(&(v), 0)[0];
    int32_t hi = lo;
    int32_t i = 1;
    while ((i < n)) {
        const int32_t x = _Salam_g_S_Vector__i32_get_i32(&(v), i)[0];
        if ((x < lo)) {
            lo = x;
        }
        if ((hi < x)) {
            hi = x;
        }
        i += 1;
    }
    const int32_t range = ((hi - lo) + 1);
    Vector__i32 count = (Vector__i32){ .data = NULL, .__len = 0, .__cap = 0 };
    int32_t c = 0;
    while ((c < range)) {
        _Salam_g_S_Vector__i32_push_i32(&(count), 0);
        c += 1;
    }
    i = 0;
    while ((i < n)) {
        const int32_t idx = (_Salam_g_S_Vector__i32_get_i32(&(v), i)[0] - lo);
        _Salam_g_S_Vector__i32_set_i32_i32(&(count), idx, (_Salam_g_S_Vector__i32_get_i32(&(count), idx)[0] + 1));
        i += 1;
    }
    int32_t o = 0;
    int32_t k = 0;
    while ((k < range)) {
        int32_t cnt = _Salam_g_S_Vector__i32_get_i32(&(count), k)[0];
        while ((cnt > 0)) {
            _Salam_g_S_Vector__i32_set_i32_i32(&(v), o, (lo + k));
            o += 1;
            cnt -= 1;
        }
        k += 1;
    }
    _Salam_g_S_Vector__i32_free(&(count));
}

void _Salam_sort_RadixSort_Vector__i32(Vector__i32 v) {
    const int32_t n = _Salam_g_S_Vector__i32_len(&(v));
    if ((n < 2)) {
        return;
    }
    int64_t lo = ((int64_t)(_Salam_g_S_Vector__i32_get_i32(&(v), 0)[0]));
    int32_t i = 1;
    while ((i < n)) {
        const int64_t x = ((int64_t)(_Salam_g_S_Vector__i32_get_i32(&(v), i)[0]));
        if ((x < lo)) {
            lo = x;
        }
        i += 1;
    }
    Vector__i64 a = (Vector__i64){ .data = NULL, .__len = 0, .__cap = 0 };
    Vector__i64 out = (Vector__i64){ .data = NULL, .__len = 0, .__cap = 0 };
    int64_t maxv = 0;
    i = 0;
    while ((i < n)) {
        const int64_t d = (((int64_t)(_Salam_g_S_Vector__i32_get_i32(&(v), i)[0])) - lo);
        _Salam_g_S_Vector__i64_push_i64(&(a), d);
        _Salam_g_S_Vector__i64_push_i64(&(out), 0);
        if ((maxv < d)) {
            maxv = d;
        }
        i += 1;
    }
    Vector__i32 count = (Vector__i32){ .data = NULL, .__len = 0, .__cap = 0 };
    int32_t z = 0;
    while ((z < 256)) {
        _Salam_g_S_Vector__i32_push_i32(&(count), 0);
        z += 1;
    }
    int64_t exp = 1;
    while (((maxv / exp) > 0)) {
        int32_t b = 0;
        while ((b < 256)) {
            _Salam_g_S_Vector__i32_set_i32_i32(&(count), b, 0);
            b += 1;
        }
        i = 0;
        while ((i < n)) {
            const int32_t dig = ((int32_t)(((_Salam_g_S_Vector__i64_get_i32(&(a), i)[0] / exp) % 256)));
            _Salam_g_S_Vector__i32_set_i32_i32(&(count), dig, (_Salam_g_S_Vector__i32_get_i32(&(count), dig)[0] + 1));
            i += 1;
        }
        int32_t s = 1;
        while ((s < 256)) {
            _Salam_g_S_Vector__i32_set_i32_i32(&(count), s, (_Salam_g_S_Vector__i32_get_i32(&(count), s)[0] + _Salam_g_S_Vector__i32_get_i32(&(count), (s - 1))[0]));
            s += 1;
        }
        i = (n - 1);
        while ((i >= 0)) {
            const int32_t dig = ((int32_t)(((_Salam_g_S_Vector__i64_get_i32(&(a), i)[0] / exp) % 256)));
            _Salam_g_S_Vector__i32_set_i32_i32(&(count), dig, (_Salam_g_S_Vector__i32_get_i32(&(count), dig)[0] - 1));
            _Salam_g_S_Vector__i64_set_i32_i64(&(out), _Salam_g_S_Vector__i32_get_i32(&(count), dig)[0], _Salam_g_S_Vector__i64_get_i32(&(a), i)[0]);
            i -= 1;
        }
        i = 0;
        while ((i < n)) {
            _Salam_g_S_Vector__i64_set_i32_i64(&(a), i, _Salam_g_S_Vector__i64_get_i32(&(out), i)[0]);
            i += 1;
        }
        exp *= 256;
    }
    i = 0;
    while ((i < n)) {
        _Salam_g_S_Vector__i32_set_i32_i32(&(v), i, ((int32_t)((_Salam_g_S_Vector__i64_get_i32(&(a), i)[0] + lo))));
        i += 1;
    }
    _Salam_g_S_Vector__i32_free(&(count));
    _Salam_g_S_Vector__i64_free(&(out));
    _Salam_g_S_Vector__i64_free(&(a));
}
