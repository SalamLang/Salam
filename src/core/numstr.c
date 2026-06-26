/*
 * Salam Programming Language (2024–2026)
 *
 *   +-------------------+
 *   |     S A L A M     |
 *   +-------------------+
 *
 * Designed by Seyyed Ali Mohammadiyeh and the Salam Team
 * Born from a decade of language design experience (since 2018)
 *
 * Repository: https://github.com/SalamLang/Salam
 *
 */

#include "core/numstr.h"

char *sal_u64toa(uint64_t v, char *buf)
{
    char tmp[SAL_NUMSTR_MAX];
    int i = 0;
    int j = 0;
    
    do {
        tmp[i++] = (char)('0' + (int)(v % 10u));
        v /= 10u;
    } while (v != 0);
    while (i > 0) buf[j++] = tmp[--i];
    buf[j] = '\0';
    return buf;
}

char *sal_i64toa(int64_t v, char *buf)
{
    if (v < 0) {
        
        uint64_t u = (uint64_t)(-(v + 1)) + 1u;
        buf[0] = '-';
        sal_u64toa(u, buf + 1);
    } else {
        sal_u64toa((uint64_t)v, buf);
    }
    return buf;
}
