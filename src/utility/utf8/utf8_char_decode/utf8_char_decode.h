#ifndef _UTILITY_UTF8_UTF8_CHAR_DECODE_UTF8_CHAR_DECODE_H_
#define _UTILITY_UTF8_UTF8_CHAR_DECODE_UTF8_CHAR_DECODE_H_

#include <stddef.h>  // for size_t
#include <stdio.h>   // for NULL

// base
#include <base.h>

// log
#include <utility/log/log_fatal/log_fatal.h>

// memory
#include <utility/memory/memory_allocation/memory_allocation.h>

char* utf8_char_decode(char* source, size_t* index, size_t* num_bytes);

#endif  // _UTILITY_UTF8_UTF8_CHAR_DECODE_UTF8_CHAR_DECODE_H_
