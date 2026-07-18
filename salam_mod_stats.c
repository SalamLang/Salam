#include "salam_mod_stats.h"

void _Salam_stats___sorted_Vector__f64(Vector__f64 v, Vector__f64 *__ret);
double _Salam_stats___variance_Vector__f64_bool(Vector__f64 v, bool sample);

double _Salam_stats_Sum_Vector__f64(Vector__f64 v)
{
    double s = 0.0;
    for (int64_t __rep1 = 0, __repn1 = (int64_t)(_Salam_g_S_Vector__f64_len(&(v)));
         __rep1 < __repn1; __rep1++) {
        const int32_t i = (int32_t)__rep1;
        s += _Salam_g_S_Vector__f64_get_i32(&(v), i)[0];
    }
    return s;
}

double _Salam_stats_Mean_Vector__f64(Vector__f64 v)
{
    if ((_Salam_g_S_Vector__f64_len(&(v)) == 0)) {
        return 0.0;
    }
    return (_Salam_stats_Sum_Vector__f64(v) /
            ((double)(_Salam_g_S_Vector__f64_len(&(v)))));
}

double _Salam_stats_Min_Vector__f64(Vector__f64 v)
{
    if ((_Salam_g_S_Vector__f64_len(&(v)) == 0)) {
        return 0.0;
    }
    double m = _Salam_g_S_Vector__f64_get_i32(&(v), 0)[0];
    for (int64_t __rep2 = 0, __repn2 = (int64_t)(_Salam_g_S_Vector__f64_len(&(v)));
         __rep2 < __repn2; __rep2++) {
        const int32_t i = (int32_t)__rep2;
        const double x = _Salam_g_S_Vector__f64_get_i32(&(v), i)[0];
        if ((x < m)) {
            m = x;
        }
    }
    return m;
}

double _Salam_stats_Max_Vector__f64(Vector__f64 v)
{
    if ((_Salam_g_S_Vector__f64_len(&(v)) == 0)) {
        return 0.0;
    }
    double m = _Salam_g_S_Vector__f64_get_i32(&(v), 0)[0];
    for (int64_t __rep3 = 0, __repn3 = (int64_t)(_Salam_g_S_Vector__f64_len(&(v)));
         __rep3 < __repn3; __rep3++) {
        const int32_t i = (int32_t)__rep3;
        const double x = _Salam_g_S_Vector__f64_get_i32(&(v), i)[0];
        if ((x > m)) {
            m = x;
        }
    }
    return m;
}

void _Salam_stats___sorted_Vector__f64(Vector__f64 v, Vector__f64 *__ret)
{
    Vector__f64 c = ({
        Vector__f64 __s4;
        _Salam_g_clone__f64_Vector__f64(v, &__s4);
        __s4;
    });
    _Salam_g_Sort__f64_Vector__f64(c);
    *__ret = (c);
    return;
}

double _Salam_stats_Median_Vector__f64(Vector__f64 v)
{
    const int32_t n = _Salam_g_S_Vector__f64_len(&(v));
    if ((n == 0)) {
        return 0.0;
    }
    Vector__f64 c = ({
        Vector__f64 __s5;
        _Salam_stats___sorted_Vector__f64(v, &__s5);
        __s5;
    });
    double m = 0.0;
    if (((n % 2) == 1)) {
        m = _Salam_g_S_Vector__f64_get_i32(&(c), (n / 2))[0];
    } else {
        m = ((_Salam_g_S_Vector__f64_get_i32(&(c), ((n / 2) - 1))[0] +
              _Salam_g_S_Vector__f64_get_i32(&(c), (n / 2))[0]) /
             2.0);
    }
    _Salam_g_S_Vector__f64_free(&(c));
    return m;
    _Salam_g_S_Vector__f64_free(&(c));
}

double _Salam_stats___variance_Vector__f64_bool(Vector__f64 v, bool sample)
{
    const int32_t n = _Salam_g_S_Vector__f64_len(&(v));
    int32_t denom = n;
    if (sample) {
        denom = (n - 1);
    }
    if ((denom <= 0)) {
        return 0.0;
    }
    const double mu = _Salam_stats_Mean_Vector__f64(v);
    double acc = 0.0;
    for (int64_t __rep6 = 0, __repn6 = (int64_t)(n); __rep6 < __repn6; __rep6++) {
        const int32_t i = (int32_t)__rep6;
        const double d = (_Salam_g_S_Vector__f64_get_i32(&(v), i)[0] - mu);
        acc += (d * d);
    }
    return (acc / ((double)(denom)));
}

double _Salam_stats_Variance_Vector__f64(Vector__f64 v)
{
    return _Salam_stats___variance_Vector__f64_bool(v, true);
}

double _Salam_stats_PopVariance_Vector__f64(Vector__f64 v)
{
    return _Salam_stats___variance_Vector__f64_bool(v, false);
}

double _Salam_stats_Stdev_Vector__f64(Vector__f64 v)
{
    return _Salam_math_Sqrt_f64(_Salam_stats___variance_Vector__f64_bool(v, true));
}

double _Salam_stats_PopStdev_Vector__f64(Vector__f64 v)
{
    return _Salam_math_Sqrt_f64(_Salam_stats___variance_Vector__f64_bool(v, false));
}

double _Salam_stats_Geomean_Vector__f64(Vector__f64 v)
{
    const int32_t n = _Salam_g_S_Vector__f64_len(&(v));
    if ((n == 0)) {
        return 0.0;
    }
    double acc = 0.0;
    for (int64_t __rep7 = 0, __repn7 = (int64_t)(n); __rep7 < __repn7; __rep7++) {
        const int32_t i = (int32_t)__rep7;
        const double x = _Salam_g_S_Vector__f64_get_i32(&(v), i)[0];
        if ((x <= 0.0)) {
            return 0.0;
        }
        acc += _Salam_math_Log_f64(x);
    }
    return _Salam_math_Exp_f64((acc / ((double)(n))));
}

double _Salam_stats_Percentile_Vector__f64_f64(Vector__f64 v, double p)
{
    const int32_t n = _Salam_g_S_Vector__f64_len(&(v));
    if ((n == 0)) {
        return 0.0;
    }
    Vector__f64 c = ({
        Vector__f64 __s8;
        _Salam_stats___sorted_Vector__f64(v, &__s8);
        __s8;
    });
    double q = p;
    if ((q < 0.0)) {
        q = 0.0;
    }
    if ((q > 100.0)) {
        q = 100.0;
    }
    const double pos = ((q / 100.0) * ((double)((n - 1))));
    const int32_t lo = ((int32_t)(_Salam_math_Floor_f64(pos)));
    int32_t hi = (lo + 1);
    if ((hi >= n)) {
        hi = (n - 1);
    }
    const double frac = (pos - ((double)(lo)));
    const double out = (_Salam_g_S_Vector__f64_get_i32(&(c), lo)[0] +
                        ((_Salam_g_S_Vector__f64_get_i32(&(c), hi)[0] -
                          _Salam_g_S_Vector__f64_get_i32(&(c), lo)[0]) *
                         frac));
    _Salam_g_S_Vector__f64_free(&(c));
    return out;
    _Salam_g_S_Vector__f64_free(&(c));
}
