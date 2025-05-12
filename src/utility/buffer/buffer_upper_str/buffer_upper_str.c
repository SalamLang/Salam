/**
 *
 * @function buffer_upper_str
 * @brief Convert a buffer to uppercase
 * @params {const char*} str - buffer
 * @returns {char*} - Uppercase buffer
 *
 */
char *buffer_upper_str(const char *str) {
    DEBUG_ME;
    char *buffer = buffer_strdup(str);

    for (size_t i = 0; i < strlen(buffer); i++) {
        buffer[i] = tolower(buffer[i]);
    }

    return buffer;
}
