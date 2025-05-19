#include <utility/memory/memory_calloc/memory_calloc.h>

void* memory_calloc(size_t count, size_t size)
{
    DEBUG_ME;
    void* ptr = calloc(count, size);
    if (ptr == NULL) {
        log_fatal("Memory calloc failed for count=%zu, size=%zu\n", count, size);
    }
    return ptr;
}
