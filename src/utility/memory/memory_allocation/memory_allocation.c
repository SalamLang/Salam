#include <utility/memory/memory_allocation/memory_allocation.h>

void* memory_allocation(size_t siz) {
    DEBUG_ME;
    void* ptr = malloc(siz);
    if (ptr == NULL) {
        log_fatal("Memory allocation failed (soft)\n");
    }
    return ptr;
}
