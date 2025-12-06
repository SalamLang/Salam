#include <utility/buffer/buffer_set/buffer_set.h>

/**
 *
 * @function buffer_set
 * @brief Set the value of a buffer
 * @params {buffer_t*} str - buffer
 * @params {buffer_t*} value - Value
 * @returns {void}
 *
 */
void buffer_set(buffer_t* str, buffer_t* value) {
    DEBUG_ME;
    buffer_append_str(str, value->data);
}
