/**
 *
 * @function buffer_append_wchar
 * @brief Append a wide character to the end of the buffer
 * @params {buffer_t*} str - buffer
 * @params {wchar_t} c - Wide character
 * @returns {void}
 *
 */
void buffer_append_wchar(buffer_t *str, wchar_t c)
{
    DEBUG_ME;
    char buffer[30];
    int len = wctomb(buffer, c);

    if (len <= 0) {
        error(2, "Failed to convert wide character to multibyte character");
        return;
    }

    buffer[len] = '\0';
    buffer_append_str(str, buffer);
}
