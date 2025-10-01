#ifndef _UTILITY_BUFFER_BUFFER_DESTROY_AND_GET_BUFFER_DESTROY_AND_GET_H_
#define _UTILITY_BUFFER_BUFFER_DESTROY_AND_GET_BUFFER_DESTROY_AND_GET_H_

// base
#include <base.h>

// buffer
#include <utility/buffer/buffer_destroy/buffer_destroy.h>
#include <utility/buffer/type.h>

// log
#include <utility/log/log_fatal/log_fatal.h>

// string
#include <utility/string/string_duplicate/string_duplicate.h>

char* buffer_destroy_and_get(buffer_t* str);

#endif  // _UTILITY_BUFFER_BUFFER_DESTROY_AND_GET_BUFFER_DESTROY_AND_GET_H_
