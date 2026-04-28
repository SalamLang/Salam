#include <utility/utf8/utf8_decode/utf8_decode.h>

uint32_t utf8_decode(const char *source, size_t *index)
{
    DEBUG_ME;
    size_t length = utf8_char_length(source[*index]);
    uint32_t codepoint = 0;

    switch (length) {
        case 1:
            codepoint = source[*index];
            break;
        case 2:
            codepoint =
                ((source[*index] & 0x1F) << 6) | (source[*index + 1] & 0x3F);
            break;
        case 3:
            codepoint = ((source[*index] & 0x0F) << 12) |
                        ((source[*index + 1] & 0x3F) << 6) |
                        (source[*index + 2] & 0x3F);
            break;
        case 4:
            codepoint = ((source[*index] & 0x07) << 18) |
                        ((source[*index + 1] & 0x3F) << 12) |
                        ((source[*index + 2] & 0x3F) << 6) |
                        (source[*index + 3] & 0x3F);
            break;
    }

    *index += length;

    return codepoint;
}
