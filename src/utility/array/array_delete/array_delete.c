#include <utility/array/array_delete/array_delete.h>

int array_delete(array_t* array, size_t index) {
    DEBUG_ME;
    if (!array || index >= array->size) return 0;
    if (array->destroyer && array->items[index]) {
        array->destroyer(array->items[index]);
    }
    memory_move(&array->items[index], &array->items[index + 1],
                sizeof(void*) * (array->size - index - 1));
    array->size--;
    return 1;
}
