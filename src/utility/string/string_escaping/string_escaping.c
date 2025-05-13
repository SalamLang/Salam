#include <utility/string/string_escaping/string_escaping.h>

char* string_escaping(const char* input)
{
    return string_replace_all(input, "\"", "\\\\");
}