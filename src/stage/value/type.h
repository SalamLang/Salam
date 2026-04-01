#ifndef _STAGE_VALUE_TYPE_H_
#define _STAGE_VALUE_TYPE_H_

#include <stdbool.h>  // for bool, true, false
#include <stddef.h>   // for size_t

// base
// #include <base.h>
// TODO

typedef enum value_type_t {
    VALUE_TYPE_STRING = 0,
    VALUE_TYPE_NUMBER_INT,
    VALUE_TYPE_NUMBER_FLOAT,
    VALUE_TYPE_BOOL,
    VALUE_TYPE_SIZE,
    VALUE_TYPE_NULL,
    // TODO: VALUE_TYPE_ARRAY,
} value_type_t;

typedef struct value_t {
    value_type_t type;  // Type of the value
    union {
        char* string_value;  // String value
        int int_value;       // Integer value
        double float_value;  // Float value
        bool bool_value;     // Boolean value
        size_t size_value;   // Boolean value
    } raw;                   // Value of the token
} value_t;

#endif  // _STAGE_VALUE_TYPE_H_
