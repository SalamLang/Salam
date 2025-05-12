#ifndef _UTILITY_BUFFER_TYPE_H_
#define _UTILITY_BUFFER_TYPE_H_

#include <stddef.h> // for size_t

typedef struct {
    size_t capacity;
    size_t length;
    char *data;

    void (*print)(void *);
    void (*destroy)(void *);
} buffer_t;

#endif
