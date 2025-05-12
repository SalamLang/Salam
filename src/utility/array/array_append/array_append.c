#include <utility/array/array_append/array_append.h>

int array_append(array_t* array, void* item)
{
    if (!array) return 0;
    if (array->size == array->capacity) {
        if (!array_resize(array, array->capacity * 2)) return 0;
    }
    array->items[array->size++] = item;
    return 1;
}
