#include <utility/memory/memory_copy/memory_copy.h>

void* memory_copy(void* destination, const void* source, size_t size)
{
    return memcpy(destination, source, size);
}
