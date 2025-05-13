#include <stage/value/value_destroy/value_destroy.h>

void value_destroy(value_t* value)
{
    if (value == NULL) {
        return;
    }

    value_direct_destroy(*value);
    
    memory_destroy(value);
}
