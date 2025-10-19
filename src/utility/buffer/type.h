#ifndef _SRC_UTILITY_BUFFER_TYPE_H_
#define _SRC_UTILITY_BUFFER_TYPE_H_

#include <stddef.h> // for size_t

// base
#include <base.h>

struct buffer_t;

typedef void (*buffer_destroy_t)(struct buffer_t *);

typedef struct buffer_t {
    size_t capacity;
    size_t length;
    char *data;

    void (*print)(struct buffer_t *);
    buffer_destroy_t destroyer;
} buffer_t;

#endif // _SRC_UTILITY_BUFFER_TYPE_H_
