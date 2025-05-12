#include <utility/memory/memory_allocation/memory_allocation.h>

void* memory_allocation_soft(size_t siz)
{
    void* ptr = malloc(siz);
    if (ptr == NULL) {
        log_error("Memory allocation failed (soft)\n");
    }
    return ptr;
}
