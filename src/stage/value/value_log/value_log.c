#include <stage/value/value_log/value_log.h>

void value_log(value_t* value)
{
    if (value == NULL) {
        log_info("Value: NULL\n");
        return;
    }

    switch (value->type) {
        case VALUE_TYPE_STRING:
            log_info("Value: String('%s')\n", value->raw.string_value);
            break;
        case VALUE_TYPE_NUMBER_INT:
            log_info("Value: Int(%d)\n", value->raw.int_value);
            break;
        case VALUE_TYPE_NUMBER_FLOAT:
            log_info("Value: Float(%f)\n", value->raw.float_value);
            break;
        case VALUE_TYPE_BOOL:
            log_info("Value: Bool(%s)\n", value->raw.bool_value ? "true" : "false");
            break;
        case VALUE_TYPE_NULL:
            log_info("Value: Null\n");
            break;
    }
}
