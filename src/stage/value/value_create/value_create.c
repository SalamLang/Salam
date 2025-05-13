#include <stage/value/value_create/value_create.h>

value_t* value_create(value_type_t type)
{
    value_t* value = memory_allocation(sizeof(value_t));
    value->type = type;
    value->value.string_value = NULL;
    value->value.int_value = 0;
    value->value.float_value = 0.0f;
    return value;
}
