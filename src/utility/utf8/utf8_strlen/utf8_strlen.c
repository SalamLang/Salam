#include <utility/utf8/utf8_strlen/utf8_strlen.h>

/*
 *
 * @function utf8_strlen
 * @brief Get the length of a UTF-8 string
 * @params {const char*} str - UTF-8 string
 * @returns {size_t} Length of the string
 *
 */
size_t utf8_strlen(const char *str)
{
    DEBUG_ME;
    size_t len = 0;
    mbstate_t state;

    memory_set(&state, sizeof(state), 0);

    while (*str) {
        wchar_t wc;
        int bytes = mbrtowc(&wc, str, MB_CUR_MAX, &state);

        if (bytes > 0) {
            str += bytes;
            len++;
        } else if (bytes == -1) {
            log_fatal("mbrtowc");
        } else {
            break;
        }
    }

    return len;
}
