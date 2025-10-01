#ifndef _UTILITY_UTF8_UTF8_IS_ALPHA_UTF8_IS_ALPHA_H_
#define _UTILITY_UTF8_UTF8_IS_ALPHA_UTF8_IS_ALPHA_H_

#include <stdbool.h>  // for bool
#include <stdint.h>   // for uint32_t
#include <string.h>   // for iswalpha
#include <wctype.h>   // for iswalpha, wchar_t

// base
#include <base.h>

bool utf8_is_alpha(char* utf8);

#endif  // _UTILITY_UTF8_UTF8_IS_ALPHA_UTF8_IS_ALPHA_H_
