#include <utility/memory/memory_destroy/memory_destroy.h>

void memory_destroy(void* ptr)
{
    DEBUG_ME;
    // if (! ptr) return;
    free(ptr);
}
