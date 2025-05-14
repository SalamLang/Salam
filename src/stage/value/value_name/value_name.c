#include <stage/value/value_name/value_name.h>

char* value_name(value_type_t type)
{
    switch (type) {
        case VALUE_TYPE_STRING: return "STRING";
        case VALUE_TYPE_NUMBER_INT: return "NUMBER_INT";
        case VALUE_TYPE_NUMBER_FLOAT: return "NUMBER_FLOAT";
        case VALUE_TYPE_BOOL: return "BOOL";
        case VALUE_TYPE_NULL: return "NULL";
    }
    return "UNKNOWN";
}
