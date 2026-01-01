#ifndef _UTILITY_UTF8_UTF8_DECODE_UTF8_DECODE_H_
#define _UTILITY_UTF8_UTF8_DECODE_UTF8_DECODE_H_

#include <stddef.h>  // for size_t
#include <stdint.h>  // for uint32_t

// base
#include <base.h>

// utf8
#include <utility/utf8/utf8_char_length/utf8_char_length.h>

uint32_t utf8_decode(const char* source, size_t* index);

#endif  // _UTILITY_UTF8_UTF8_DECODE_UTF8_DECODE_H_
