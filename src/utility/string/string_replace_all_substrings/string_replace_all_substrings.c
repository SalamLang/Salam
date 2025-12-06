#include <utility/string/string_replace_all_substrings/string_replace_all_substrings.h>

/**
 *
 * @function replace_all_substrings
 * @brief Replace all substrings in a string
 * @params {const char*} str - String
 * @params {const char*} old_substr - Old substring
 * @params {const char*} new_substr - New substring
 * @returns {char*} - New string
 *
 */
char* string_replace_all_substrings(const char* str, const char* old_substr,
                                    const char* new_substr) {
    DEBUG_ME;
    if (str == NULL || old_substr == NULL || new_substr == NULL) {
        return NULL;
    }

    size_t str_len = string_length(str);
    size_t old_len = string_length(old_substr);
    size_t new_len = string_length(new_substr);

    if (old_len == 0) {
        char* result = memory_allocation(str_len + 1);

        strcpy(result, str);

        return result;
    }

    size_t max_result_len = str_len;
    const char* tmp = str;
    while ((tmp = strstr(tmp, old_substr)) != NULL) {
        max_result_len += (new_len - old_len);
        tmp += old_len;
    }

    char* result = memory_allocation(max_result_len + 1);

    char* result_ptr = result;
    const char* search_start = str;
    const char* pos;

    while ((pos = strstr(search_start, old_substr)) != NULL) {
        size_t bytes_to_copy = pos - search_start;

        strncpy(result_ptr, search_start, bytes_to_copy);
        result_ptr += bytes_to_copy;

        strcpy(result_ptr, new_substr);
        result_ptr += new_len;

        search_start = pos + old_len;
    }

    strcpy(result_ptr, search_start);

    return result;
}
