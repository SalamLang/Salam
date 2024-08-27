#include "string.h"

/**
 *
 * @function string_create
 * @brief Create a string
 * @params {size_t} initial_capacity - Initial size of the string
 * @returns {string_t*}
 *
 */
string_t *string_create(size_t initial_capacity)
{
	DEBUG_ME;
	string_t *str = memory_allocate(sizeof(string_t));

	str->capacity = initial_capacity;
	str->length = 0;

	str->data = memory_allocate(initial_capacity * sizeof(char));
	str->data[0] = '\0';

	str->print = cast(void (*)(void *), string_print);
	str->destroy = cast(void (*)(void *), string_destroy);

	return str;
}

/**
 *
 * @function string_append_char_begin
 * @brief Append a character to the beginning of the string
 * @params {string_t*} str - String
 * @params {char} c - Character
 * @returns {void}
 *
 */
void string_append_char_begin(string_t *str, char c)
{
	DEBUG_ME;
	if (str->length + 1 >= str->capacity)
	{
		str->capacity *= 2;
		str->data = memory_reallocate(str->data, str->capacity * sizeof(char));
	}

	memmove(str->data + 1, str->data, str->length + 1);

	str->data[0] = c;
	str->length++;
}

/**
 *
 * @function string_append_str_begin
 * @brief Append a string to the beginning of the string
 * @params {string_t*} str - String
 * @params {const char*} prefix - Prefix
 * @returns {void}
 *
 */
void string_append_str_begin(string_t *str, const char *prefix)
{
	DEBUG_ME;
	size_t prefix_len = strlen(prefix);

	while (str->length + prefix_len >= str->capacity)
	{
		str->capacity *= 2;
		str->data = memory_reallocate(str->data, str->capacity * sizeof(char));
	}

	memmove(str->data + prefix_len, str->data, str->length + 1);

	memcpy(str->data, prefix, prefix_len);
	str->length += prefix_len;
}

/**
 *
 * @function string_append_char
 * @brief Append a character to the end of the string
 * @params {string_t*} str - String
 * @params {char} c - Character
 * @returns {void}
 *
 */
void string_append_char(string_t *str, char c)
{
	DEBUG_ME;
	if (str->length + 1 >= str->capacity)
	{
		str->capacity *= 2;
		str->data = memory_reallocate(str->data, str->capacity * sizeof(char));
	}

	str->data[str->length] = c;
	str->length++;
	str->data[str->length] = '\0';
}

/**
 *
 * @function string_append_wchar
 * @brief Append a wide character to the end of the string
 * @params {string_t*} str - String
 * @params {wchar_t} c - Wide character
 * @returns {void}
 *
 */
void string_append_wchar(string_t *str, wchar_t c)
{
	DEBUG_ME;
	char buffer[30];
	int len = wctomb(buffer, c);

	if (len <= 0)
	{
		error(2, "Failed to convert wide character to multibyte character");
		return;
	}

	buffer[len] = '\0';
	string_append_str(str, buffer);
}

/**
 *
 * @function string_append_str
 * @brief Append a string to the end of the string
 * @params {string_t*} str - String
 * @params {const char*} suffix - Suffix
 * @returns {void}
 *
 */
void string_append_str(string_t *str, const char *suffix)
{
	DEBUG_ME;
	size_t suffix_len = strlen(suffix);
	if (suffix_len == 0)
	{
		return;
	}

	while (str->length + suffix_len >= str->capacity)
	{
		str->capacity *= 2;
		str->data = memory_reallocate(str->data, str->capacity * sizeof(char));
	}

	strcpy(str->data + str->length, suffix);

	str->length += suffix_len;
}

/**
 *
 * @function string_destroy
 * @brief Free a string
 * @params {string_t*} str - String
 * @returns {void}
 *
 */
void string_destroy(string_t *str)
{
	DEBUG_ME;
	if (str != NULL)
	{
		if (str->data != NULL)
		{
			memory_destroy(str->data);
		}

		str->capacity = 0;
		str->length = 0;

		memory_destroy(str);
	}
}

/**
 *
 * @function string_destroy_and_get
 * @brief Free a string and return the string data
 * @params {string_t*} str - String
 * @returns {char*}
 *
 */
char *string_destroy_and_get(string_t *str)
{
	DEBUG_ME;
	char *res = strdup(str->data);
	string_destroy(str);

	if (res == NULL)
	{
		error(2, "Failed to allocate memory for string data");

		return NULL;
	}

	return res;
}

/**
 *
 * @function string_print
 * @brief Print a string
 * @params {string_t*} str - String
 * @returns {void}
 *
 */
void string_print(string_t *str)
{
	DEBUG_ME;
	if (str == NULL || str->data == NULL)
	{
		printf("NULL\n");
	}
	else
	{
		printf("%s\n", str->data);
	}
}

/**
 *
 * @function string_append
 * @brief Append a string to the end of the string
 * @params {string_t*} str - String
 * @params {string_t*} value - Value
 * @returns {void}
 *
 */
void string_append(string_t *str, const string_t *value)
{
	DEBUG_ME;
	if (value->length == 0)
	{
		return;
	}

	string_append_str(str, value->data);
}

/**
 *
 * @function string_set
 * @brief Set the value of a string
 * @params {string_t*} str - String
 * @params {string_t*} value - Value
 * @returns {void}
 *
 */
void string_set(string_t *str, string_t *value)
{
	DEBUG_ME;
	return string_append_str(str, value->data);
}

/**
 *
 * @function string_set_str
 * @brief Set the value of a string
 * @params {string_t*} str - String
 * @params {const char*} value - Value
 * @returns {void}
 *
 */
void string_set_str(string_t *str, const char *value)
{
	DEBUG_ME;
	return string_append_str(str, value);
}

/**
 *
 * @function string_lower_str
 * @brief Convert a string to lowercase
 * @params {const char*} str - String
 * @returns {char*} - Lowercase string
 *
 */
char *string_lower_str(const char *str)
{
	DEBUG_ME;
	char *buffer = strdup(str);

	for (size_t i = 0; i < strlen(buffer); i++)
	{
		buffer[i] = tolower(buffer[i]);
	}

	return buffer;
}

/**
 *
 * @function string_upper_str
 * @brief Convert a string to uppercase
 * @params {const char*} str - String
 * @returns {char*} - Uppercase string
 *
 */
char *string_upper_str(const char *str)
{
	DEBUG_ME;
	char *buffer = strdup(str);

	for (size_t i = 0; i < strlen(buffer); i++)
	{
		buffer[i] = tolower(buffer[i]);
	}

	return buffer;
}

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
char *replace_all_substrings(const char *str, const char *old_substr, const char *new_substr)
{
	DEBUG_ME;
	if (str == NULL || old_substr == NULL || new_substr == NULL)
	{
		return NULL;
	}

	size_t str_len = strlen(str);
	size_t old_len = strlen(old_substr);
	size_t new_len = strlen(new_substr);

	if (old_len == 0)
	{
		char *result = memory_allocate(str_len + 1);

		strcpy(result, str);

		return result;
	}

	size_t max_result_len = str_len;
	const char *tmp = str;
	while ((tmp = strstr(tmp, old_substr)) != NULL)
	{
		max_result_len += (new_len - old_len);
		tmp += old_len;
	}

	char *result = memory_allocate(max_result_len + 1);

	char *result_ptr = result;
	const char *search_start = str;
	const char *pos;

	while ((pos = strstr(search_start, old_substr)) != NULL)
	{
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

/**
 *
 * @function string_is
 * @brief Check if two strings are equal
 * @params {string_t*} str1 - String 1
 * @params {const char*} str2 - String 2
 * @returns {bool}
 *
 */
bool string_is(string_t *str1, const char *str2)
{
	DEBUG_ME;
	if (str1 == NULL || str2 == NULL)
	{
		return false;
	}

	if (str1->length != strlen(str2))
	{
		return false;
	}

	return strcmp(str1->data, str2) == 0;
}

/**
 *
 * @function string_equals
 * @brief Check if two strings are equal
 * @params {string_t*} str1 - String 1
 * @params {string_t*} str2 - String 2
 * @returns {bool}
 *
 */
bool string_equals(string_t *str1, string_t *str2)
{
	DEBUG_ME;
	if (str1 == NULL || str2 == NULL)
	{
		return false;
	}
	else if (str1->length != str2->length)
	{
		return false;
	}

	return strcmp(str1->data, str2->data) == 0;
}

/**
 *
 * @function string_compare
 * @brief Compare two strings
 * @params {string_t*} str1 - String 1
 * @params {string_t*} str2 - String 2
 * @returns {bool}
 *
 */
bool string_compare(string_t *str1, string_t *str2)
{
	DEBUG_ME;
	if (str1 == NULL || str2 == NULL)
	{
		return false;
	}
	else if (str1->length != str2->length)
	{
		return false;
	}

	return strcmp(str1->data, str2->data) < 0;
}

/**
 *
 * @function is_utf8_continuation_byte
 * @brief Check if a byte is a UTF-8 continuation byte
 * @params {char} c The byte to check
 * @returns {bool} True if the byte is a continuation byte, false otherwise
 *
 */
bool is_utf8_continuation_byte(char c)
{
	return (c & 0xC0) == 0x80;
}

/**
 *
 * @function is_valid_utf8
 * @brief Check if a string is a valid UTF-8 sequence
 * @params {const char*} str The string to check
 * @returns {bool} True if the string is a valid UTF-8 sequence, false otherwise
 *
 */
bool is_valid_utf8(const char *str)
{
	const unsigned char *bytes = (const unsigned char *)str;

	while (*bytes)
	{
		if (*bytes <= 0x7F)
		{
			// ASCII byte
			bytes += 1;
		}
		else if ((*bytes & 0xE0) == 0xC0)
		{
			// 2-byte sequence
			if ((bytes[1] & 0xC0) != 0x80)
			{
				return false;
			}

			bytes += 2;
		}
		else if ((*bytes & 0xF0) == 0xE0)
		{
			// 3-byte sequence
			if ((bytes[1] & 0xC0) != 0x80 || (bytes[2] & 0xC0) != 0x80)
			{
				return false;
			}

			bytes += 3;
		}
		else if ((*bytes & 0xF8) == 0xF0)
		{
			// 4-byte sequence
			if ((bytes[1] & 0xC0) != 0x80 || (bytes[2] & 0xC0) != 0x80 || (bytes[3] & 0xC0) != 0x80)
			{
				return false;
			}

			bytes += 4;
		}
		else
		{
			return false;
		}
	}

	return true;
}

/**
 *
 * @function is_schar_alpha
 * @brief Check if a UTF-8 character is alphabetic
 * @params {const char*} c Pointer to the UTF-8 character
 * @returns {bool} True if the character is alphabetic, false otherwise
 *
 */
bool is_schar_alpha(const char *c)
{
	DEBUG_ME;
	wchar_t wc;
	int len = mbtowc(&wc, c, MB_CUR_MAX);

	if (len <= 0)
	{
		return false;
	}

	return iswalpha(wc);
}

/**
 *
 * @function is_char_digit
 * @brief Check if a character is a digit
 * @params {char} c - Character
 * @returns {bool}
 *
 */
bool is_char_digit(char c)
{
	DEBUG_ME;
	return c >= '0' && c <= '9';
}

/**
 *
 * @function is_char_alpha
 * @brief Check if a character is an alphabet
 * @params {char} c - Character
 * @returns {bool}
 *
 */
bool is_char_alpha(const char c)
{
	DEBUG_ME;
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

/**
 *
 * @function is_char_alnum
 * @brief Check if a character is an alphabet or a digit
 * @params {char} c - Character
 * @returns {bool}
 *
 */
bool is_char_alnum(char c)
{
	DEBUG_ME;
	return is_char_alpha(c) || is_char_digit(c);
}

/**
 *
 * @function is_char_whitespace
 * @brief Check if a character is a whitespace
 * @params {char} c - Character
 * @returns {bool}
 *
 */
bool is_char_whitespace(char c)
{
	DEBUG_ME;
	return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

/**
 *
 * @function is_wchar_alpha
 * @brief Check if a wide character is alphabetic
 * @params {uint32_t} codepoint - Wide character
 * @returns {bool} True if the character is alphabetic, false otherwise
 *
 */
bool is_wchar_alpha(uint32_t codepoint)
{
	DEBUG_ME;
	return iswalpha(codepoint);
}

/**
 *
 * @function is_wchar_digits
 * @brief Check if a wide character is a digit
 * @params {uint32_t} codepoint - Wide character
 * @returns {bool} True if the character is a digit, false otherwise
 *
 */
bool is_wchar_digit(uint32_t codepoint)
{
	return is_english_digit(codepoint) || is_persian_digit(codepoint) || is_arabic_digit(codepoint) || iswdigit(codepoint);
}

/**
 *
 * @function is_utf8_alpha
 * @brief Check if a UTF-8 string is an alphabet
 * @params {char*} utf8 - UTF-8 string
 * @returns {bool} True if the string is an alphabet, false otherwise
 *
 */
bool is_utf8_alpha(char *utf8)
{
	wchar_t wc;
	int wcl = mbtowc(&wc, utf8, MB_CUR_MAX);
	if (wcl <= 0)
	{
		return false;
	}

	return iswalpha(wc);
}

/**
 *
 * @function is_english_digit
 * @brief Check if a UTF-8 string is an Persian/Arabic/English digit
 * @params {char*} utf8 - UTF-8 string
 * @returns {bool} True if the string is a Persian/Arabic/English digit, false otherwise
 *
 */
bool is_utf8_digit(char *utf8)
{
	// Persian digits
	if (
		strcmp(utf8, "۰") == 0 || // Persian 0
		strcmp(utf8, "۱") == 0 || // Persian 1
		strcmp(utf8, "۲") == 0 || // Persian 2
		strcmp(utf8, "۳") == 0 || // Persian 3
		strcmp(utf8, "۴") == 0 || // Persian 4
		strcmp(utf8, "۵") == 0 || // Persian 5
		strcmp(utf8, "۶") == 0 || // Persian 6
		strcmp(utf8, "۷") == 0 || // Persian 7
		strcmp(utf8, "۸") == 0 || // Persian 8
		strcmp(utf8, "۹") == 0	  // Persian 9
	)
	{
		return true;
	}
	// Arabic digits
	else if (
		strcmp(utf8, "٠") == 0 || // Arabic 0
		strcmp(utf8, "١") == 0 || // Arabic 1
		strcmp(utf8, "٢") == 0 || // Arabic 2
		strcmp(utf8, "٣") == 0 || // Arabic 3
		strcmp(utf8, "٤") == 0 || // Arabic 4
		strcmp(utf8, "٥") == 0 || // Arabic 5
		strcmp(utf8, "٦") == 0 || // Arabic 6
		strcmp(utf8, "٧") == 0 || // Arabic 7
		strcmp(utf8, "٨") == 0 || // Arabic 8
		strcmp(utf8, "٩") == 0	  // Arabic 9
	)
	{
		return true;
	}
	// English digits
	else if (
		strcmp(utf8, "0") == 0 || // English 0
		strcmp(utf8, "1") == 0 || // English 1
		strcmp(utf8, "2") == 0 || // English 2
		strcmp(utf8, "3") == 0 || // English 3
		strcmp(utf8, "4") == 0 || // English 4
		strcmp(utf8, "5") == 0 || // English 5
		strcmp(utf8, "6") == 0 || // English 6
		strcmp(utf8, "7") == 0 || // English 7
		strcmp(utf8, "8") == 0 || // English 8
		strcmp(utf8, "9") == 0	  // English 9
	)
	{
		return true;
	}

	return false;
}

/**
 *
 * @function convert_to_english_digit
 * @brief Convert a wide character to an English digit
 * @params {wchar_t} ch - Wide character
 * @returns {char} English digit
 *
 */
char convert_to_english_digit(wchar_t ch)
{
	DEBUG_ME;
	if (is_english_digit(ch))
	{
		return (char)ch;
	}
	else if (is_persian_digit(ch))
	{
		return '0' + (ch - 0x06F0);
	}
	else if (is_arabic_digit(ch))
	{
		return '0' + (ch - 0x0660);
	}

	return ch;
}

/**
 *
 * @function convert_utf8_to_english_digit
 * @brief Convert a UTF-8 string to an English digit
 * @params {char*} uc - UTF-8 string
 * @returns {char} English digit
 *
 */
char convert_utf8_to_english_digit(char *uc)
{
	if (strcmp(uc, "۰") == 0 || strcmp(uc, "٠") == 0 || strcmp(uc, "0") == 0)
	{
		return '0';
	}
	else if (strcmp(uc, "۱") == 0 || strcmp(uc, "١") == 0 || strcmp(uc, "1") == 0)
	{
		return '1';
	}
	else if (strcmp(uc, "۲") == 0 || strcmp(uc, "٢") == 0 || strcmp(uc, "2") == 0)
	{
		return '2';
	}
	else if (strcmp(uc, "۳") == 0 || strcmp(uc, "٣") == 0 || strcmp(uc, "3") == 0)
	{
		return '3';
	}
	else if (strcmp(uc, "۴") == 0 || strcmp(uc, "٤") == 0 || strcmp(uc, "4") == 0)
	{
		return '4';
	}
	else if (strcmp(uc, "۵") == 0 || strcmp(uc, "٥") == 0 || strcmp(uc, "5") == 0)
	{
		return '5';
	}
	else if (strcmp(uc, "۶") == 0 || strcmp(uc, "٦") == 0 || strcmp(uc, "6") == 0)
	{
		return '6';
	}
	else if (strcmp(uc, "۷") == 0 || strcmp(uc, "٧") == 0 || strcmp(uc, "7") == 0)
	{
		return '7';
	}
	else if (strcmp(uc, "۸") == 0 || strcmp(uc, "٨") == 0 || strcmp(uc, "8") == 0)
	{
		return '8';
	}
	else if (strcmp(uc, "۹") == 0 || strcmp(uc, "٩") == 0 || strcmp(uc, "9") == 0)
	{
		return '9';
	}

	return '\0';
}

/**
 *
 * @function string_number2number
 * @brief Convert digits in a string to English digits
 * @params {char*} str - Input string
 * @returns {void}
 *
 */
void string_number2number(char *str)
{
	DEBUG_ME;
	if (str == NULL)
	{
		return;
	}

	size_t len = strlen(str);
	wchar_t wc;
	int wcl;
	char *temp_str = (char *)malloc((len + 1) * sizeof(char));

	char *src_ptr = str;
	char *dest_ptr = temp_str;

	while (*src_ptr != '\0')
	{
		wcl = mbtowc(&wc, src_ptr, MB_CUR_MAX);
		if (wcl <= 0)
		{
			// Error or end of multibyte sequence
			break;
		}
		*dest_ptr = convert_to_english_digit(wc);
		src_ptr += wcl;
		dest_ptr++;
	}

	*dest_ptr = '\0';

	strcpy(str, temp_str);

	memory_destroy(temp_str);
}

/**
 *
 * @function int2string
 * @brief Convert an integer to a string
 * @params {int} value - Integer value
 * @returns {char*}
 *
 */
char *int2string(int value)
{
	DEBUG_ME;
	static char buffer[256];
	snprintf(buffer, sizeof(buffer), "%d", value);

	return buffer;
}

/**
 *
 * @function float2string
 * @brief Convert a float to a string
 * @params {float} value - Float value
 * @returns {char*}
 *
 */
char *float2string(float value)
{
	DEBUG_ME;
	static char buffer[256];
	snprintf(buffer, sizeof(buffer), "%f", value);

	return buffer;
}

/**
 *
 * @function double2string
 * @brief Convert a double to a string
 * @params {double} value - Double value
 * @returns {char*}
 *
 */
char *double2string(double value)
{
	DEBUG_ME;
	static char buffer[256];
	snprintf(buffer, sizeof(buffer), "%f", value);

	return buffer;
}

/**
 *
 * @function utf8_char_length
 * @brief Get the length of a UTF-8 character
 * @params {char} c - UTF-8 character
 * @returns {size_t} Length of the character
 *
 */
size_t utf8_char_length(char c)
{
	DEBUG_ME;
	if ((c & 0x80) == 0)
	{
		return 1; // 0xxxxxxx
	}
	else if ((c & 0xE0) == 0xC0)
	{
		return 2; // 110xxxxx
	}
	else if ((c & 0xF0) == 0xE0)
	{
		return 3; // 1110xxxx
	}
	else if ((c & 0xF8) == 0xF0)
	{
		return 4; // 11110xxx
	}

	return 0; // Invalid UTF-8
}

/**
 *
 * @function utf8_decode
 * @brief Decode a UTF-8 character
 * @params {const char*} source - Source string
 * @params {size_t*} index - Index of the character
 * @returns {uint32_t} Decoded character
 *
 */
uint32_t utf8_decode(const char *source, size_t *index)
{
	DEBUG_ME;
	size_t length = utf8_char_length(source[*index]);
	uint32_t codepoint = 0;

	switch (length)
	{
	case 1:
		codepoint = source[*index];
		break;
	case 2:
		codepoint = ((source[*index] & 0x1F) << 6) |
					(source[*index + 1] & 0x3F);
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

/**
 *
 * @function utf8_strlen
 * @brief Get the length of a UTF-8 string
 * @params {const char*} str - UTF-8 string
 * @returns {size_t} Length of the string
 *
 */
size_t utf8_strlen(const char *str)
{
	size_t len = 0;
	mbstate_t state;
	memset(&state, 0, sizeof(state));

	while (*str)
	{
		wchar_t wc;
		int bytes = mbrtowc(&wc, str, MB_CUR_MAX, &state);

		if (bytes > 0)
		{
			str += bytes;
			len++;
		}
		else if (bytes == -1)
		{
			perror("mbrtowc");

			exit(EXIT_FAILURE);
		}
		else
		{
			break;
		}
	}

	return len;
}

/**
 *
 * @function mb2strlen
 * @brief Get the length of a multibyte string
 * @params {const char*} identifier - Multibyte string
 * @returns {size_t} Length of the string
 *
 */
size_t mb2strlen(const char *identifier)
{
	size_t wcs_len = mbstowcs(NULL, identifier, 0);
	if (wcs_len == (size_t)-1)
	{
		printf("MESSAGE_LEXER_STRING_GET_LENGTH_UNICODE\n");

		exit(EXIT_FAILURE);
	}

	return wcs_len;
}
