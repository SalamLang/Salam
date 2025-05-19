#include <utility/memory/memory_allocation_soft/memory_allocation_soft.h>

void* memory_allocation_soft(size_t siz)
{
    DEBUG_ME;
    void* ptr = malloc(siz);
    if (ptr == NULL) {
        log_error("Memory allocation failed (soft)\n");
    }
    return ptr;
}
