#include <stddef.h>  // for size_t

typedef void (*array_free_func)(void*);

typedef struct {
    void** items;             // actual data
    size_t capacity;          // total capacity
    size_t size;              // current number of elements
    array_free_func freer;    // custom free function (NULL means no-op)
} array_t;
