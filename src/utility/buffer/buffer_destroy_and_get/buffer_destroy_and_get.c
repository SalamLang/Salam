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
    char *res = buffer_strdup(str->data);
    buffer_destroy(str);

    if (res == NULL) {
        error(2, "Failed to allocate memory for buffer data");

        return NULL;
    }

    return res;
}
