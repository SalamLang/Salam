#include <utility/buffer/buffer_is/buffer_is.h>

/**
 *
 * @function buffer_is
 * @brief Check if two buffers are equal
 * @params {buffer_t*} str1 - buffer 1
 * @params {const char*} str2 - buffer 2
 * @returns {bool}
 *
 */
bool buffer_is(buffer_t *str1, const char *str2)
{
    DEBUG_ME;
    if (str1 == NULL || str2 == NULL) {
        return false;
    }

    if (str1->length != string_length(str2)) {
        return false;
    }

    return strcmp(str1->data, str2) == 0;
}
