#include <utility/array/array_create/array_create.h>

array_t* array_create(array_destroy_t destroyer) {
    DEBUG_ME;
    const size_t INITIAL_CAPACITY = 4;
    array_t* array = memory_allocation(sizeof(array_t));
    array->capacity = INITIAL_CAPACITY;
    array->size = 0;
    array->destroyer = destroyer;
    array->items = memory_allocation_soft(sizeof(void*) * array->capacity);
    if (!array->items) {
        memory_destroy(array);
        return NULL;
    }
    return array;
}
