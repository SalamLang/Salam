#include <memory/memory_free/memory_free.h>

void memory_free(void* ptr)
{
    free(ptr);
}
