#include <utility/string/string_escaping/string_escaping.h>

char* string_escaping(const char* input)
{
    DEBUG_ME;
    if (!input) return NULL;

    char* temp = string_replace_all(input, "\\", "\\\\");
    char* next;

    next = string_replace_all(temp, "\"", "\\\"");
    free(temp); temp = next;

    next = string_replace_all(temp, "\n", "\\n");
    free(temp); temp = next;

    next = string_replace_all(temp, "\t", "\\t");
    free(temp); temp = next;

    next = string_replace_all(temp, "\r", "\\r");
    free(temp); temp = next;

    next = string_replace_all(temp, "\b", "\\b");
    free(temp); temp = next;

    next = string_replace_all(temp, "\f", "\\f");
    free(temp); temp = next;

    next = string_replace_all(temp, "\a", "\\a");
    free(temp); temp = next;

    next = string_replace_all(temp, "\v", "\\v");
    free(temp); temp = next;

    return temp;
}
