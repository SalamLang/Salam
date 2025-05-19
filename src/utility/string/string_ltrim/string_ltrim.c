#include <utility/string/string_ltrim/string_ltrim.h>

char* string_ltrim(const char* input)
{
    DEBUG_ME;
    if (!input) return NULL;

    while (isspace((unsigned char)*input)) {
        input++;
    }

    return string_duplicate(input);
}
