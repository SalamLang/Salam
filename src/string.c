#include "string.h"

/**
 * 
 * @function string_create
 * @brief Create a string
 * @params {size_t} initial_size - Initial size of the string
 * @returns {string_t*}
 * 
 */
string_t* string_create(size_t initial_size)
{
	string_t* str = memory_allocate(sizeof(string_t));
	
	str->size = initial_size;
	str->length = 0;

	str->data = memory_allocate(initial_size * sizeof(char));
	str->data[0] = '\0';

	str->print = cast(void (*)(void*), string_print);
	str->destroy = cast(void (*)(void*), string_destroy);
	
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
void string_append_char_begin(string_t* str, char c)
{
	if (str->length + 1 >= str->size) {
		str->size *= 2;
		str->data = memory_reallocate(str->data, str->size * sizeof(char));
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
void string_append_str_begin(string_t* str, const char* prefix)
{
	size_t prefix_len = strlen(prefix);

	while (str->length + prefix_len >= str->size) {
		str->size *= 2;
		str->data = memory_reallocate(str->data, str->size * sizeof(char));
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
void string_append_char(string_t* str, char c)
{
	if (str->length + 1 >= str->size) {
		str->size *= 2;
		str->data = memory_reallocate(str->data, str->size * sizeof(char));
	}

	str->data[str->length] = c;
	str->length++;
	str->data[str->length] = '\0';
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
void string_append_str(string_t* str, const char* suffix)
{
	size_t suffix_len = strlen(suffix);
	if (suffix_len == 0) return;

	while (str->length + suffix_len >= str->size) {
		str->size *= 2;
		str->data = memory_reallocate(str->data, str->size * sizeof(char));
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
void string_destroy(string_t* str)
{
	if (str != NULL) {
		if (str->data != NULL) {
			memory_destroy(str->data);
		}

		str->size = 0;
		str->length = 0;

		memory_destroy(str);
		str = NULL;
	}
}

/**
 * 
 * @function string_print
 * @brief Print a string
 * @params {string_t*} str - String
 * @returns {void}
 * 
 */
void string_print(string_t* str)
{
	if (str == NULL || str->data == NULL) printf("NULL\n");
	else printf("%s\n", str->data);
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
void string_append(string_t* str, string_t* value)
{
	if (value->length == 0) return;

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
void string_set(string_t* str, string_t* value)
{
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
void string_set_str(string_t* str, const char* value)
{
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
char* string_lower_str(const char* str)
{
	char* buffer = strdup(str);

	for (size_t i = 0; i < strlen(buffer); i++) {
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
char* string_upper_str(const char* str)
{
	char* buffer = strdup(str);

	for (size_t i = 0; i < strlen(buffer); i++) {
		buffer[i] = tolower(buffer[i]);
	}

	return buffer;
}
