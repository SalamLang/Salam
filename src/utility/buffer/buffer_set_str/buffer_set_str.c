#include <utility/buffer/buffer_set_str/buffer_set_str.h>

/**
 *
 * @function buffer_set_str
 * @brief Set the value of a buffer
 * @params {buffer_t*} str - buffer
 * @params {const char*} value - Value
 * @returns {void}
 *
 */
void buffer_set_str(buffer_t* str, const char* value) {
    DEBUG_ME;
    buffer_append_str(str, value);
}
