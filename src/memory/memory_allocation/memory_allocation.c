#include "memory_allocation.h"

void* memory_alloc(size_t siz)
{
    void* ptr = malloc(siz);
    if (ptr == NULL) {
        log_fatal("Memory allocation failed\n");
    }
    return ptr;
}
