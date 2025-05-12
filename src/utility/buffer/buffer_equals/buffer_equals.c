/**
 *
 * @function buffer_equals
 * @brief Check if two buffers are equal
 * @params {buffer_t*} str1 - buffer 1
 * @params {buffer_t*} str2 - buffer 2
 * @returns {bool}
 *
 */
bool buffer_equals(buffer_t *str1, buffer_t *str2)
{
    DEBUG_ME;
    if (str1 == NULL || str2 == NULL) {
        return false;
    } else if (str1->length != str2->length) {
        return false;
    }

    return strcmp(str1->data, str2->data) == 0;
}
