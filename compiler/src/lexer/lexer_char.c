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

#include "core/prelude.h"
#include "lexer/lexer_internal.h"

bool lx_is_digit(char c)
{
    unsigned char u = (unsigned char)c;
    return u >= '0' && u <= '9';
}

bool lx_is_alpha(char c)
{
    unsigned char u = (unsigned char)c;
    return (u >= 'a' && u <= 'z') || (u >= 'A' && u <= 'Z');
}

bool lx_is_ident_start(char c)
{
    unsigned char u = (unsigned char)c;
    return lx_is_alpha(c) || c == '_' || u >= 0x80;
}

bool lx_is_ident_cont(char c)
{
    return lx_is_ident_start(c) || lx_is_digit(c);
}

int lx_hex(char c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

bool lx_is_base_digit(char c, char base)
{
    switch (base) {
    case 'x':
    case 'X':
        return lx_hex(c) >= 0;
    case 'b':
    case 'B':
        return c == '0' || c == '1';
    case 'o':
    case 'O':
        return c >= '0' && c <= '7';
    default:
        return false;
    }
}

int lx_decimal_digit(const char *s, size_t n, int *val)
{
    if (n == 0) return 0;
    unsigned char b0 = (unsigned char)s[0];
    if (b0 >= '0' && b0 <= '9') {
        *val = b0 - '0';
        return 1;
    }
    if (n < 2) return 0;
    unsigned char b1 = (unsigned char)s[1];
    if (b0 == 0xD9 && b1 >= 0xA0 && b1 <= 0xA9) {
        *val = b1 - 0xA0;
        return 2;
    }
    if (b0 == 0xDB && b1 >= 0xB0 && b1 <= 0xB9) {
        *val = b1 - 0xB0;
        return 2;
    }
    return 0;
}

size_t lx_utf8_encode(uint32_t cp, char *out)
{
    if (cp < 0x80) {
        out[0] = (char)cp;
        return 1;
    }
    if (cp < 0x800) {
        out[0] = (char)(0xC0 | (cp >> 6));
        out[1] = (char)(0x80 | (cp & 0x3F));
        return 2;
    }
    if (cp < 0x10000) {
        out[0] = (char)(0xE0 | (cp >> 12));
        out[1] = (char)(0x80 | ((cp >> 6) & 0x3F));
        out[2] = (char)(0x80 | (cp & 0x3F));
        return 3;
    }
    out[0] = (char)(0xF0 | (cp >> 18));
    out[1] = (char)(0x80 | ((cp >> 12) & 0x3F));
    out[2] = (char)(0x80 | ((cp >> 6) & 0x3F));
    out[3] = (char)(0x80 | (cp & 0x3F));
    return 4;
}

int lx_utf8_seq_len(const char *s, size_t n)
{
    if (n == 0) return 0;
    unsigned char b = (unsigned char)s[0];
    if (b < 0x80) return 1;
    if ((b & 0xE0) == 0xC0 && n >= 2) return 2;
    if ((b & 0xF0) == 0xE0 && n >= 3) return 3;
    if ((b & 0xF8) == 0xF0 && n >= 4) return 4;
    return 0;
}

uint64_t lx_parse_uint(const char *d, int base, bool *overflow)
{
    uint64_t v = 0;
    uint64_t b = (uint64_t)base;
    if (overflow) *overflow = false;
    for (; *d; d++) {
        int digit = lx_hex((char)*d);
        if (digit < 0 || digit >= base) break;
        if (v > (UINT64_MAX - (uint64_t)digit) / b) {
            if (overflow) *overflow = true;
            v = UINT64_MAX;
        } else {
            v = v * b + (uint64_t)digit;
        }
    }
    return v;
}
