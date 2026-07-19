#include "salam_mod_conv.h"

SALAM_PURE int8_t* _Salam_conv___bp_str(const char* s);
SALAM_PURE int32_t _Salam_conv___ub_i8_ptr_i32(int8_t* bp, int32_t i);
SALAM_PURE bool _Salam_conv___is__space_i32(int32_t c);
SALAM_PURE bool _Salam_conv___kw_i8_ptr_i32_i32_str(int8_t* bp, int32_t i, int32_t w, const char* kw);
const char* _Salam_conv___digits6_i64(int64_t d);
const char* _Salam_conv___exp__str_i32(int32_t e);

SALAM_PURE int8_t* _Salam_conv___bp_str(const char* s) {
    return ((int8_t*)(((void*)(intptr_t)(s))));
}

SALAM_PURE int32_t _Salam_conv___ub_i8_ptr_i32(int8_t* bp, int32_t i) {
    int32_t v = ((int32_t)(bp[i]));
    if ((v < 0)) {
        v = (v + 256);
    }
    return v;
}

SALAM_PURE bool _Salam_conv___is__space_i32(int32_t c) {
    return ((((c == 32) || (c == 9)) || (c == 10)) || (c == 13));
}

SALAM_PURE bool _Salam_conv___kw_i8_ptr_i32_i32_str(int8_t* bp, int32_t i, int32_t w, const char* kw) {
    if ((w != (int32_t)strlen(kw))) {
        return false;
    }
    int8_t* kp = _Salam_conv___bp_str(kw);
    int32_t k = 0;
    while ((k < w)) {
        if ((bp[(i + k)] != kp[k])) {
            return false;
        }
        k += 1;
    }
    return true;
}

const char* _Salam_conv_FormatUint_u64(uint64_t x) {
    int32_t nd = 1;
    uint64_t t = x;
    while ((t >= 10)) {
        t /= 10;
        nd += 1;
    }
    void* buf = malloc(((uint64_t)((nd + 1))));
    int8_t* p = ((int8_t*)(buf));
    p[nd] = 0;
    uint64_t v = x;
    int32_t i = (nd - 1);
    while ((i >= 0)) {
        p[i] = ((int8_t)((((int32_t)((v % 10))) + 48)));
        v /= 10;
        i -= 1;
    }
    return ((const char*)(buf));
}

const char* _Salam_conv_FormatInt_i64(int64_t n) {
    if ((n >= 0)) {
        return _Salam_conv_FormatUint_u64(((uint64_t)(n)));
    }
    const uint64_t mag = (0 - ((uint64_t)(n)));
    return salam_strcat("-", _Salam_conv_FormatUint_u64(mag));
}

SALAM_PURE bool _Salam_conv_ParseBool_str(const char* s) {
    const int32_t n = (int32_t)strlen(s);
    int8_t* bp = _Salam_conv___bp_str(s);
    int32_t i = 0;
    while (((i < n) && _Salam_conv___is__space_i32(_Salam_conv___ub_i8_ptr_i32(bp, i)))) {
        i += 1;
    }
    int32_t e = n;
    while (((e > i) && _Salam_conv___is__space_i32(_Salam_conv___ub_i8_ptr_i32(bp, (e - 1))))) {
        e -= 1;
    }
    const int32_t w = (e - i);
    if (_Salam_conv___kw_i8_ptr_i32_i32_str(bp, i, w, "true")) {
        return true;
    } else if (_Salam_conv___kw_i8_ptr_i32_i32_str(bp, i, w, "1")) {
        return true;
    } else {
        if (_Salam_conv___kw_i8_ptr_i32_i32_str(bp, i, w, "yes")) {
            return true;
        } else if (_Salam_conv___kw_i8_ptr_i32_i32_str(bp, i, w, "on")) {
            return true;
        } else {
            return false;
        }
    }
}

SALAM_PURE int64_t _Salam_conv_ParseInt_str(const char* s) {
    const int32_t n = (int32_t)strlen(s);
    int8_t* bp = _Salam_conv___bp_str(s);
    int32_t i = 0;
    while (((i < n) && _Salam_conv___is__space_i32(_Salam_conv___ub_i8_ptr_i32(bp, i)))) {
        i += 1;
    }
    bool neg = false;
    if ((i < n)) {
        const int32_t c = _Salam_conv___ub_i8_ptr_i32(bp, i);
        if ((c == 45)) {
            neg = true;
            i += 1;
        } else if ((c == 43)) {
            i += 1;
        }
    }
    int64_t v = 0;
    while ((i < n)) {
        const int32_t c = _Salam_conv___ub_i8_ptr_i32(bp, i);
        if (((c < 48) || (c > 57))) {
            break;
        }
        v = ((v * 10) + ((int64_t)((c - 48))));
        i += 1;
    }
    if (neg) {
        return (0 - v);
    }
    return v;
}

SALAM_PURE double _Salam_conv_ParseFloat_str(const char* s) {
    const int32_t n = (int32_t)strlen(s);
    int8_t* bp = _Salam_conv___bp_str(s);
    int32_t i = 0;
    while (((i < n) && _Salam_conv___is__space_i32(_Salam_conv___ub_i8_ptr_i32(bp, i)))) {
        i += 1;
    }
    bool neg = false;
    if ((i < n)) {
        const int32_t c = _Salam_conv___ub_i8_ptr_i32(bp, i);
        if ((c == 45)) {
            neg = true;
            i += 1;
        } else if ((c == 43)) {
            i += 1;
        }
    }
    double v = 0.0;
    while ((i < n)) {
        const int32_t c = _Salam_conv___ub_i8_ptr_i32(bp, i);
        if (((c < 48) || (c > 57))) {
            break;
        }
        v = ((v * 10.0) + ((double)((c - 48))));
        i += 1;
    }
    if (((i < n) && (_Salam_conv___ub_i8_ptr_i32(bp, i) == 46))) {
        i += 1;
        double scale = 1.0;
        while ((i < n)) {
            const int32_t c = _Salam_conv___ub_i8_ptr_i32(bp, i);
            if (((c < 48) || (c > 57))) {
                break;
            }
            scale /= 10.0;
            v += (((double)((c - 48))) * scale);
            i += 1;
        }
    }
    if (((i < n) && ((_Salam_conv___ub_i8_ptr_i32(bp, i) == 101) || (_Salam_conv___ub_i8_ptr_i32(bp, i) == 69)))) {
        i += 1;
        bool eneg = false;
        if ((i < n)) {
            const int32_t c = _Salam_conv___ub_i8_ptr_i32(bp, i);
            if ((c == 45)) {
                eneg = true;
                i += 1;
            } else if ((c == 43)) {
                i += 1;
            }
        }
        int32_t ev = 0;
        while ((i < n)) {
            const int32_t c = _Salam_conv___ub_i8_ptr_i32(bp, i);
            if (((c < 48) || (c > 57))) {
                break;
            }
            ev = ((ev * 10) + (c - 48));
            i += 1;
        }
        while ((ev > 0)) {
            if (eneg) {
                v = (v / 10.0);
            } else {
                v *= 10.0;
            }
            ev -= 1;
        }
    }
    if (neg) {
        return (0.0 - v);
    }
    return v;
}

const char* _Salam_conv___digits6_i64(int64_t d) {
    const char* ds = "";
    int64_t t = d;
    while ((t > 0)) {
        ds = salam_strcat(salam_char_from_code((((int32_t)((t % 10))) + 48)), ds);
        t /= 10;
    }
    while ((((int32_t)strlen(ds) > 1) && (strcmp(salam_str_substr(ds, ((int32_t)strlen(ds) - 1), 1), "0") == 0))) {
        ds = salam_str_substr(ds, 0, ((int32_t)strlen(ds) - 1));
    }
    return ds;
}

const char* _Salam_conv___exp__str_i32(int32_t e) {
    const char* es = "e+";
    int32_t ea = e;
    if ((e < 0)) {
        es = "e-";
        ea = (0 - e);
    }
    const char* en = _Salam_conv_FormatInt_i64(((int64_t)(ea)));
    if ((ea < 10)) {
        return salam_strcat(salam_strcat(es, "0"), en);
    }
    return salam_strcat(es, en);
}

const char* _Salam_conv_FormatFloat_f64(double x) {
    if ((x != x)) {
        return "nan";
    }
    double v = x;
    const char* sign = "";
    if ((v < 0.0)) {
        sign = "-";
        v = (0.0 - v);
    }
    if ((v == 0.0)) {
        return salam_strcat(sign, "0");
    }
    if (((v / 2.0) == v)) {
        return salam_strcat(sign, "inf");
    }
    int32_t e = 0;
    while ((v >= 10.0)) {
        v /= 10.0;
        e += 1;
    }
    while ((v < 1.0)) {
        v *= 10.0;
        e -= 1;
    }
    int64_t d = ((int64_t)(((v * 100000.0) + 0.5)));
    if ((d >= 1000000)) {
        d /= 10;
        e += 1;
    }
    const char* ds = _Salam_conv___digits6_i64(d);
    if (((e < (-4LL)) || (e >= 6))) {
        const char* m = salam_str_substr(ds, 0, 1);
        if (((int32_t)strlen(ds) > 1)) {
            m = salam_strcat(salam_strcat(m, "."), salam_str_substr(ds, 1, ((int32_t)strlen(ds) - 1)));
        }
        return salam_strcat(salam_strcat(sign, m), _Salam_conv___exp__str_i32(e));
    } else if ((e >= 0)) {
        const int32_t k = (e + 1);
        if (((int32_t)strlen(ds) <= k)) {
            const char* out = ds;
            int32_t pad = (k - (int32_t)strlen(ds));
            while ((pad > 0)) {
                out = salam_strcat(out, "0");
                pad -= 1;
            }
            return salam_strcat(sign, out);
        }
        return salam_strcat(salam_strcat(salam_strcat(sign, salam_str_substr(ds, 0, k)), "."), salam_str_substr(ds, k, ((int32_t)strlen(ds) - k)));
    } else {
        const char* zeros = "";
        int32_t z = ((0 - e) - 1);
        while ((z > 0)) {
            zeros = salam_strcat(zeros, "0");
            z -= 1;
        }
        return salam_strcat(salam_strcat(salam_strcat(sign, "0."), zeros), ds);
    }
}

const char* _Salam_conv_FormatFloatPrec_f64_i32(double x, int32_t prec) {
    if ((x != x)) {
        return "nan";
    }
    double v = x;
    const char* sign = "";
    if ((v < 0.0)) {
        sign = "-";
        v = (0.0 - v);
    }
    if ((((v / 2.0) == v) && (v > 0.0))) {
        return salam_strcat(sign, "inf");
    }
    int32_t p = prec;
    if ((p < 0)) {
        p = 0;
    }
    if ((p > 15)) {
        p = 15;
    }
    if ((v > 900000000000000.0)) {
        return salam_strcat(sign, _Salam_conv_FormatFloat_f64(v));
    }
    int64_t scale = 1;
    int32_t k = 0;
    while ((k < p)) {
        scale *= 10;
        k += 1;
    }
    const int64_t total = ((int64_t)(((v * ((double)(scale))) + 0.5)));
    const int64_t ip = (total / scale);
    if ((p == 0)) {
        return salam_strcat(sign, _Salam_conv_FormatUint_u64(((uint64_t)(ip))));
    }
    const int64_t fp = (total % scale);
    const char* fs = _Salam_conv_FormatUint_u64(((uint64_t)(fp)));
    while (((int32_t)strlen(fs) < p)) {
        fs = salam_strcat("0", fs);
    }
    return salam_strcat(salam_strcat(salam_strcat(sign, _Salam_conv_FormatUint_u64(((uint64_t)(ip)))), "."), fs);
}

const char* _Salam_conv_FormatHex_u64(uint64_t x) {
    int32_t nd = 1;
    uint64_t t = x;
    while ((t >= 16)) {
        t /= 16;
        nd += 1;
    }
    void* buf = malloc(((uint64_t)((nd + 1))));
    int8_t* p = ((int8_t*)(buf));
    p[nd] = 0;
    uint64_t v = x;
    int32_t i = (nd - 1);
    while ((i >= 0)) {
        const int32_t d = ((int32_t)((v % 16)));
        if ((d < 10)) {
            p[i] = ((int8_t)((d + 48)));
        } else {
            p[i] = ((int8_t)((d + 87)));
        }
        v /= 16;
        i -= 1;
    }
    return ((const char*)(buf));
}

SALAM_PURE int64_t _Salam_conv_ParseHex_str(const char* s) {
    const int32_t n = (int32_t)strlen(s);
    int8_t* bp = _Salam_conv___bp_str(s);
    int32_t i = 0;
    while (((i < n) && _Salam_conv___is__space_i32(_Salam_conv___ub_i8_ptr_i32(bp, i)))) {
        i += 1;
    }
    if (((((i + 1) < n) && (_Salam_conv___ub_i8_ptr_i32(bp, i) == 48)) && (_Salam_conv___ub_i8_ptr_i32(bp, (i + 1)) == 120))) {
        i += 2;
    }
    int64_t v = 0;
    while ((i < n)) {
        const int32_t c = _Salam_conv___ub_i8_ptr_i32(bp, i);
        int32_t d = (-1LL);
        if (((c >= 48) && (c <= 57))) {
            d = (c - 48);
        } else if (((c >= 97) && (c <= 102))) {
            d = (c - 87);
        } else {
            if (((c >= 65) && (c <= 70))) {
                d = (c - 55);
            }
        }
        if ((d < 0)) {
            break;
        }
        v = ((v * 16) + ((int64_t)(d)));
        i += 1;
    }
    return v;
}
