#include <utility/buffer/buffer_destroy/buffer_destroy.h>

/**
 *
 * @function buffer_destroy
 * @brief Free a buffer
 * @params {buffer_t*} str - buffer
 * @returns {void}
 *
 */
void buffer_destroy(buffer_t* str) {
    DEBUG_ME;
    if (str != NULL) {
        if (str->data != NULL) {
            memory_destroy(str->data);
        }

        str->capacity = 0;
        str->length = 0;

        memory_destroy(str);
    }
}
