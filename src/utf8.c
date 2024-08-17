#include <stdio.h>
#include <wchar.h>
#include <locale.h>
#include <stdint.h>
#include <wctype.h>
#include <stdlib.h>
#include <stdbool.h>

/**
 * @function is_utf8_continuation_byte
 * @brief Check if a byte is a UTF-8 continuation byte
 * @params {char} c The byte to check
 * @returns {bool} True if the byte is a continuation byte, false otherwise
 */
bool is_utf8_continuation_byte(char c) {
    return (c & 0xC0) == 0x80;
}

/**
 * @function is_valid_utf8
 * @brief Check if a string is a valid UTF-8 sequence
 * @params {const char*} str The string to check
 * @returns {bool} True if the string is a valid UTF-8 sequence, false otherwise
 */
bool is_valid_utf8(const char* str) {
    const unsigned char* bytes = (const unsigned char *)str;

    while (*bytes) {
        if (*bytes <= 0x7F) {
            // ASCII byte
            bytes += 1;
        } else if ((*bytes & 0xE0) == 0xC0) {
            // 2-byte sequence
            if ((bytes[1] & 0xC0) != 0x80) return false;
            bytes += 2;
        } else if ((*bytes & 0xF0) == 0xE0) {
            // 3-byte sequence
            if ((bytes[1] & 0xC0) != 0x80 || (bytes[2] & 0xC0) != 0x80) return false;
            bytes += 3;
        } else if ((*bytes & 0xF8) == 0xF0) {
            // 4-byte sequence
            if ((bytes[1] & 0xC0) != 0x80 || (bytes[2] & 0xC0) != 0x80 || (bytes[3] & 0xC0) != 0x80) return false;
            bytes += 4;
        } else {
            return false;
        }
    }

    return true;
}

/**
 * @function is_schar_alpha
 * @brief Check if a UTF-8 character is alphabetic
 * @params {const char*} c Pointer to the UTF-8 character
 * @returns {bool} True if the character is alphabetic, false otherwise
 */
bool is_schar_alpha(const char* c) {
    wchar_t wc;
    int len = mbtowc(&wc, c, MB_CUR_MAX);

    if (len <= 0) {
        return false;
    }

    return iswalpha(wc);
}

/**
 * @function is_char_digit
 * @brief Check if a character is a digit
 * @params {char} c - Character
 * @returns {bool}
 */
bool is_char_digit(char c) {
    return c >= '0' && c <= '9';
}

/**
 * @function is_char_alpha
 * @brief Check if a character is an alphabet
 * @params {char} c - Character
 * @returns {bool}
 */
bool is_char_alpha(const char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

/**
 * @function is_char_alnum
 * @brief Check if a character is an alphabet or a digit
 * @params {char} c - Character
 * @returns {bool}
 */
bool is_char_alnum(char c) {
    return is_char_alpha(c) || is_char_digit(c);
}

/**
 * @function is_char_whitespace
 * @brief Check if a character is a whitespace
 * @params {char} c - Character
 * @returns {bool}
 */
bool is_char_whitespace(char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

/**
 * @function is_wchar_alpha
 * @brief Check if a wide character is alphabetic
 * @params {uint32_t} codepoint - Wide character
 * @returns {bool} True if the character is alphabetic, false otherwise
 */
bool is_wchar_alpha(uint32_t codepoint) {
    return iswalpha(codepoint);
}

/**
 * @function is_wchar_digits
 * @brief Check if a wide character is a digit
 * @params {uint32_t} codepoint - Wide character
 * @returns {bool} True if the character is a digit, false otherwise
 */
bool is_wchar_digit(uint32_t codepoint) {
    return iswdigit(codepoint);
}

wchar_t read_token(const char* input, int* index)
{
	wchar_t current_char;
	int char_size = mbtowc(&current_char, &input[*index], MB_CUR_MAX);
	if (char_size < 0) {
		printf("MESSAGE_LEXER_TOKEN_READ_UNICODE\n");

		exit(EXIT_FAILURE);
		return 0;
	}

	*index += char_size;

	return current_char;
}

void lexer(const char* input)
{
    int index = 0;

	while (input[index] != 0) {
		char current_char = input[index];

		if (current_char == '\a' || current_char == '\r' || current_char == ' ' || current_char == '\t') {
			index++;
			continue;
		}
		else if (current_char == '\n') {
			index++;
			continue;
		}

		wchar_t current_wchar = read_token(input, &index);

		if (current_wchar == L'\u200C') {
			index++;
			continue;
		}
        else if (iswalpha(current_wchar)) {
            printf("Alphabetic character\n");
            printf("Current wchar: %lc\n", current_wchar);
        }
        else if (iswdigit(current_wchar)) {
            printf("Digit character\n");
            printf("Current wchar: %lc\n", current_wchar);
        }
        else {
            printf("Unknown character\n");
            printf("Current wchar: %lc\n", current_wchar);
        }
    }
}

int main() {
    setlocale(LC_ALL, "");

    // const char* test_str = "سلام چطوری?";
    // if (is_valid_utf8(test_str)) {
    //     printf("The string is valid UTF-8.\n");
    // } else {
    //     printf("The string is not valid UTF-8.\n");
    // }

    const char* input = "عملکرد سلام + 123";
    lexer(input);

    return 0;
}
