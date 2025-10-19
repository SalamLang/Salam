#ifndef _SRC_UTILITY_MEMORY_MEMORY_MOVE_MEMORY_MOVE_H_
#define _SRC_UTILITY_MEMORY_MEMORY_MOVE_MEMORY_MOVE_H_

#include <stddef.h>  // for size_t
#include <string.h> // for memmove

// base
#include <base.h>

// log
#include <utility/log/log_fatal/log_fatal.h>

void memory_move(void* dest, const void* src, size_t size);

#endif // _SRC_UTILITY_MEMORY_MEMORY_MOVE_MEMORY_MOVE_H_
