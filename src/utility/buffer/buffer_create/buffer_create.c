#include <utility/buffer/buffer_create/buffer_create.h>

/**
 *
 * @function buffer_create
 * @brief Create a buffer
 * @params {size_t} initial_capacity - Initial size of the buffer
 * @returns {buffer_t*}
 *
 */
buffer_t *buffer_create(size_t initial_capacity)
{
    DEBUG_ME;
    buffer_t *str = memory_allocation(sizeof(buffer_t));

    str->capacity = initial_capacity;
    str->length = 0;

    str->data = memory_allocation(initial_capacity * sizeof(char));
    str->data[0] = '\0';

    str->print = buffer_print;
    str->destroyer = buffer_destroy;

    return str;
}
