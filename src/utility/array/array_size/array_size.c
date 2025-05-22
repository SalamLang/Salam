#include <utility/array/array_size/array_size.h>

size_t array_size(array_t* array)
{
    DEBUG_ME;
    return array ? array->size : 0;
}
