#include <utility/array/array_get/array_get.h>

void* array_get(array_t* array, size_t index)
{
    DEBUG_ME;
    if (!array || index >= array->size) return NULL;

    return array->items[index];
}
