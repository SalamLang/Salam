#ifndef _UTILITY_BUFFER_BUFFER_CREATE_BUFFER_CREATE_H_
#define _UTILITY_BUFFER_BUFFER_CREATE_BUFFER_CREATE_H_

// base
#include <base.h>

// buffer
#include <utility/buffer/buffer_destroy/buffer_destroy.h>
#include <utility/buffer/buffer_print/buffer_print.h>
#include <utility/buffer/type.h>

// memory
#include <utility/memory/memory_allocation/memory_allocation.h>

buffer_t* buffer_create(size_t initial_capacity);

#endif  // _UTILITY_BUFFER_BUFFER_CREATE_BUFFER_CREATE_H_
