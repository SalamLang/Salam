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

#ifndef SALAM_CORE_SAL_TYPES_H
#define SALAM_CORE_SAL_TYPES_H

#include <stddef.h>

#if defined(__has_include)
# if __has_include(<stdint.h>)
#  define SAL_HAVE_STDINT_H 1
# endif
#endif

#if (defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)) \
    || (defined(_MSC_VER) && (_MSC_VER >= 1600)) \
    || defined(SAL_HAVE_STDINT_H)
# include <stdint.h>
# include <stdbool.h>
#else
# include <limits.h>

  typedef signed   char  int8_t;
  typedef unsigned char  uint8_t;
  typedef short          int16_t;
  typedef unsigned short uint16_t;
  typedef int            int32_t;
  typedef unsigned int   uint32_t;

# if defined(_MSC_VER)
    typedef          __int64 int64_t;
    typedef unsigned __int64 uint64_t;
# elif defined(__GNUC__) || defined(__clang__) || defined(__TINYC__) \
       || defined(__WATCOMC__) || defined(__LCC__)
    __extension__ typedef          long long int64_t;
    __extension__ typedef unsigned long long uint64_t;
# else
    typedef          long long int64_t;
    typedef unsigned long long uint64_t;
# endif

# if defined(_WIN64) || defined(__LLP64__)
    typedef int64_t  intptr_t;
    typedef uint64_t uintptr_t;
# else
    typedef long          intptr_t;
    typedef unsigned long uintptr_t;
# endif

# ifndef __cplusplus
#   ifndef __bool_true_false_are_defined
      typedef unsigned char _sal_bool;
#     define bool  _sal_bool
#     define true  1
#     define false 0
#     define __bool_true_false_are_defined 1
#   endif
# endif
#endif

#ifndef SIZE_MAX
# define SIZE_MAX ((size_t)-1)
#endif

#include <stdarg.h>

#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
# define SAL_INLINE static inline
#elif defined(__GNUC__)
# define SAL_INLINE static __inline__
#elif defined(_MSC_VER)
# define SAL_INLINE static __inline
#else
# define SAL_INLINE static
#endif

#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
# define SAL_VA_COPY(d, s) va_copy((d), (s))
#elif defined(__GNUC__)
# define SAL_VA_COPY(d, s) __builtin_va_copy((d), (s))
#else
# define SAL_VA_COPY(d, s) ((void)memcpy(&(d), &(s), sizeof(va_list)))
#endif

#endif /* SALAM_CORE_SAL_TYPES_H */
