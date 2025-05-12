/**
 *
 * @function buffer_append
 * @brief Append a buffer to the end of the buffer
 * @params {buffer_t*} str - buffer
 * @params {buffer_t*} value - Value
 * @returns {void}
 *
 */
void buffer_append(buffer_t *str, const buffer_t *value)
{
    DEBUG_ME;
    if (value->length == 0) {
        return;
    }

    buffer_append_str(str, value->data);
}
