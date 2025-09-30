#include <utility/array/array_resize/array_resize.h>

int array_resize(array_t* array, size_t new_capacity) {
    DEBUG_ME;
    void** new_items =
        memory_realloc(array->items, sizeof(void*) * new_capacity);
    array->items = new_items;
    array->capacity = new_capacity;

    return 1;
}
