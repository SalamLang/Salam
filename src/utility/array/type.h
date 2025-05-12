#ifndef _UTILITY_ARRAY_TYPE_H_
#define _UTILITY_ARRAY_TYPE_H_

#include <stddef.h>  // for size_t

typedef void (*array_free_func_t)(void*);

typedef struct {
    void** items;             // actual data
    size_t capacity;          // total capacity
    size_t size;              // current number of elements
    array_free_func_t freer;    // custom free function (NULL means no-op)
} array_t;

#endif
