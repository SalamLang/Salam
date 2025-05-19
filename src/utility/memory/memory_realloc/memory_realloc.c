#include <utility/memory/memory_realloc/memory_realloc.h>

void* memory_realloc(void* ptr, size_t new_size)
{
    DEBUG_ME;
    void* new_ptr = realloc(ptr, new_size);
    if (!new_ptr && new_size > 0) {
        log_fatal("memory_realloc failed for size: %zu\n", new_size);
    }
    return new_ptr;
}
