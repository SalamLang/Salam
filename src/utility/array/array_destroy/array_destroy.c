#include <utility/array/array_destroy/array_destroy.h>

void array_destroy(array_t* array)
{
    if (!array) return;

    for (size_t i = 0; i < array->count; i++) {
        if (array->items[i]) {
            array->destroyer(array->items[i]);
        }
    }

    memory_destroy(array);
}
