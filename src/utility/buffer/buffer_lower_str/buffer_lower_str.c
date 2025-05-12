/**
 *
 * @function buffer_lower_str
 * @brief Convert a buffer to lowercase
 * @params {const char*} str - buffer
 * @returns {char*} - Lowercase buffer
 *
 */
char *buffer_lower_str(const char *str) {
    DEBUG_ME;
    char *buffer = buffer_strdup(str);

    for (size_t i = 0; i < strlen(buffer); i++) {
        buffer[i] = tolower(buffer[i]);
    }

    return buffer;
}
