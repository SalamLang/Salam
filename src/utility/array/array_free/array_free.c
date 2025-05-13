#include <utility/array/array_destroy/array_destroy.h>

void array_destroy(array_t* array)
{
    if (!array) return;
    if (array->destroyer) {
        for (size_t i = 0; i < array->size; ++i) {
            if (array->items[i]) array->destroyer(array->items[i]);
        }
    }
    memory_destroy(array->items);
    memory_destroy(array);
}
