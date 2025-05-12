/**
 *
 * @function buffer_append_char
 * @brief Append a character to the end of the buffer
 * @params {buffer_t*} str - buffer
 * @params {char} c - Character
 * @returns {void}
 *
 */
void buffer_append_char(buffer_t *str, char c)
{
    DEBUG_ME;
    if (str->length + 1 >= str->capacity) {
        str->capacity *= 2;
        str->data = memory_reallocate(str->data, str->capacity * sizeof(char));
    }

    str->data[str->length] = c;
    str->length++;
    str->data[str->length] = '\0';
}
