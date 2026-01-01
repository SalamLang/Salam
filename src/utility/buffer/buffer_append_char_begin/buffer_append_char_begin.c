#include <utility/buffer/buffer_append_char_begin/buffer_append_char_begin.h>

/**
 *
 * @function buffer_append_char_begin
 * @brief Append a character to the beginning of the buffer
 * @params {buffer_t*} str - buffer
 * @params {char} c - Character
 * @returns {void}
 *
 */
void buffer_append_char_begin(buffer_t* str, char c) {
    DEBUG_ME;
    if (str->length + 1 >= str->capacity) {
        str->capacity *= 2;
        str->data = memory_realloc(str->data, str->capacity * sizeof(char));
    }

    memory_move(str->data + 1, str->data, str->length + 1);

    str->data[0] = c;
    str->length++;
}
