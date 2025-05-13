#ifndef _STAGE_VALUE_TYPE_H_
#define _STAGE_VALUE_TYPE_H_

#include <stddef.h> // for size_t

// base
#include <base.h>

typedef enum value_type_t
{
    VALUE_TYPE_STRING = 0,
    VALUE_TYPE_NUMBER_INT,
    VALUE_TYPE_NUMBER_FLOAT,
} value_type_t;

typedef struct value_t
{
    value_type_t type;                    // Type of the value
    union {
        char* string_value;               // String value
        int int_value;                    // Integer value
        double float_value;               // Float value
    } raw;                              // Value of the token
} value_t;

#endif
