#include <utility/memory/memory_destroy/memory_destroy.h>

void memory_destroy(void* ptr)
{
    free(ptr);
}
