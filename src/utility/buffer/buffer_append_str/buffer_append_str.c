#include <utility/buffer/buffer_append_str/buffer_append_str.h>

/**
 *
 * @function buffer_append_str
 * @brief Append a buffer to the end of the buffer
 * @params {buffer_t*} str - buffer
 * @params {const char*} suffix - Suffix
 * @returns {void}
 *
 */
void buffer_append_str(buffer_t *str, const char *suffix)
{
    DEBUG_ME;
    if (suffix == NULL) {
        return;
    }

    size_t suffix_len = string_length(suffix);
    if (suffix_len == 0) {
        return;
    }

    while (str->length + suffix_len >= str->capacity) {
        str->capacity *= 2;
        str->data = (char*) memory_realloc(str->data, str->capacity * sizeof(char));
    }

    strcpy(str->data + str->length, suffix);

    str->length += suffix_len;
}
