#include <stage/value/value_destroy/value_destroy.h>

void value_destroy(value_t* value)
{
    DEBUG_ME;
    if (! value) return;

    switch (value->type) {
        case VALUE_TYPE_STRING:
            if (value->raw.string_value != NULL) {
                memory_destroy(value->raw.string_value);
            }
            break;
        case VALUE_TYPE_NUMBER_INT:
            // No need to free int_value
            break;
        case VALUE_TYPE_NUMBER_FLOAT:
            // No need to free float_value
            break;
    }

    memory_destroy(value);
}
