#include <utility/buffer/buffer_append_str_begin/buffer_append_str_begin.h>

/**
 *
 * @function buffer_append_str_begin
 * @brief Append a buffer to the beginning of the buffer
 * @params {buffer_t*} str - buffer
 * @params {const char*} prefix - Prefix
 * @returns {void}
 *
 */
void buffer_append_str_begin(buffer_t* str, const char* prefix) {
    DEBUG_ME;
    size_t prefix_len = strlen(prefix);

    while (str->length + prefix_len >= str->capacity) {
        str->capacity *= 2;
        str->data = memory_realloc(str->data, str->capacity * sizeof(char));
    }

    memory_move(str->data + prefix_len, str->data, str->length + 1);

    memcpy(str->data, prefix, prefix_len);
    str->length += prefix_len;
}
