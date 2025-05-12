#include <utility/array/array_size/array_size.h>

size_t array_size(array_t* array)
{
    return array ? array->size : 0;
}
