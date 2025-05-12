#include <utility/buffer/buffer_destroy_and_get/buffer_destroy_and_get.h>

/**
 *
 * @function buffer_destroy_and_get
 * @brief Free a buffer and return the buffer data
 * @params {buffer_t*} str - buffer
 * @returns {char*}
 *
 */
char *buffer_destroy_and_get(buffer_t *str)
{
    DEBUG_ME;
    char *res = string_duplicate(str->data);
    buffer_destroy(str);

    if (res == NULL) {
        log_fatal("Failed to allocate memory for buffer data");

        return NULL;
    }

    return res;
}
