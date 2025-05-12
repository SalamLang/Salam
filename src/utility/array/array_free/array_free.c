#include <utility/array/array_free/array_free.h>

void array_free(array_t* array)
{
    if (!array) return;
    if (array->freer) {
        for (size_t i = 0; i < array->size; ++i) {
            if (array->items[i]) array->freer(array->items[i]);
        }
    }
    memory_free(array->items);
    memory_free(array);
}
