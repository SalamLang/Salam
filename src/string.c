#include "string.h"

/**
 * 
 * @function string_create
 * @brief Create a string
 * @params {size_t} initial_capacity - Initial size of the string
 * @returns {string_t*}
 * 
 */
string_t* string_create(size_t initial_capacity)
{
	DEBUG_ME;
	string_t* str = memory_allocate(sizeof(string_t));
	
	str->capacity = initial_capacity;
	str->length = 0;

	str->data = memory_allocate(initial_capacity * sizeof(char));
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
	DEBUG_ME;
	if (str->length + 1 >= str->capacity) {
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
void string_append_str_begin(string_t* str, const char* prefix)
{
	DEBUG_ME;
	size_t prefix_len = strlen(prefix);

	while (str->length + prefix_len >= str->capacity) {
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
void string_append_char(string_t* str, char c)
{
	DEBUG_ME;
	if (str->length + 1 >= str->capacity) {
		str->capacity *= 2;
		str->data = memory_reallocate(str->data, str->capacity * sizeof(char));
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
	DEBUG_ME;
	size_t suffix_len = strlen(suffix);
	if (suffix_len == 0) return;

	while (str->length + suffix_len >= str->capacity) {
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
void string_destroy(string_t* str)
{
	DEBUG_ME;
	if (str != NULL) {
		if (str->data != NULL) {
			memory_destroy(str->data);
		}

		str->capacity = 0;
		str->length = 0;

		memory_destroy(str);
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
	DEBUG_ME;
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
	DEBUG_ME;
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
void string_set_str(string_t* str, const char* value)
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
char* string_lower_str(const char* str)
{
	DEBUG_ME;
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
	DEBUG_ME;
	char* buffer = strdup(str);

	for (size_t i = 0; i < strlen(buffer); i++) {
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
char* replace_all_substrings(const char* str, const char* old_substr, const char* new_substr)
{
	DEBUG_ME;
	if (str == NULL || old_substr == NULL || new_substr == NULL) return NULL;

	size_t str_len = strlen(str);
	size_t old_len = strlen(old_substr);
	size_t new_len = strlen(new_substr);

	if (old_len == 0) {
		char *result = memory_allocate(str_len + 1);

		strcpy(result, str);
		return result;
	}

	size_t max_result_len = str_len;
	const char *tmp = str;
	while ((tmp = strstr(tmp, old_substr)) != NULL) {
		max_result_len += (new_len - old_len);
		tmp += old_len;
	}

	char *result = memory_allocate(max_result_len + 1);

	char *result_ptr = result;
	const char *search_start = str;
	const char *pos;

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
