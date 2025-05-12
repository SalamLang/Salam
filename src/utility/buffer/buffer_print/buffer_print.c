/**
 *
 * @function buffer_print
 * @brief Print a buffer
 * @params {buffer_t*} str - buffer
 * @returns {void}
 *
 */
void buffer_print(buffer_t *str)
{
    DEBUG_ME;
    if (str == NULL || str->data == NULL) {
        printf("NULL\n");
    } else {
        printf("%s\n", str->data);
    }
}
