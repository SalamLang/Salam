#ifndef _SRC_UTILITY_UTF8_UTF8_DECODE_UTF8_DECODE_H_
#define _SRC_UTILITY_UTF8_UTF8_DECODE_UTF8_DECODE_H_

#include <stdint.h> // for uint32_t
#include <stddef.h> // for size_t

// base
#include <base.h>

// utf8
#include <utility/utf8/utf8_char_length/utf8_char_length.h>

uint32_t utf8_decode(const char *source, size_t *index);

#endif // _SRC_UTILITY_UTF8_UTF8_DECODE_UTF8_DECODE_H_
