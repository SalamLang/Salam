#include <utility/array/array_prepend/array_prepend.h>

int array_prepend(array_t* array, void* item)
{
    if (!array) return 0;
    if (array->size == array->capacity) {
        if (!array_resize(array, array->capacity * 2)) return 0;
    }
    memory_move(&array->items[1], &array->items[0], sizeof(void*) * array->size);
    array->items[0] = item;
    array->size++;
    return 1;
}
