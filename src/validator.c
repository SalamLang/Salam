#include "validator.h"

#include "ast_layout_attribute_style_value.h"

/**
 *
 * @var valid_layout_attributes_length
 * @brief Valid layout attributes length
 * @type {size_t}
 */
size_t valid_layout_attributes_length = sizeof(valid_layout_attributes) / sizeof(valid_layout_attributes[0]);

/**
 *
 * @function has_css_size_prefix
 * @brief Check if the CSS value has a size prefix
 * @params {char*} css_value - CSS value
 * @params {char**} css_output_value - CSS output value
 * @returns {bool} - True if the CSS value has a size prefix, false otherwise
 *
 */
bool has_css_size_prefix(char* css_value, char** css_output_value)
{
	DEBUG_ME;
	const char* prefixes[] = {
		"px", "pixel", "pixels",
		"em", "em",
		"rem",
		"vw", "viewport width", "viewport width",
		"vh", "viewport height", "viewport height",
		"%",
		"cm", "centimeter", "centimeters",
		"mm", "millimeter", "millimeters",
		"in", "inch", "inches",
		"pt", "point", "points",
		"pc", "pica", "picas",
		"ex",
		"ch",
		"vmin", "viewport minimum",
		"vmax", "viewport maximum"
	};

	const char* persian_prefixes[] = {
		"پیکسل", "پیکسل‌ها", "پیکسلها",
		"ای ام", "ایم",
		"رایم",
		"ویو ویدث", "ویو ویدت", "ویویدث",
		"ویو هایت", "ویو هایت", "وی هایت",
		"درصد",
		"سانتیمتر", "سانتی متر", "سانت",
		"میلیمتر", "میلی متر", "میلیم",
		"اینچ", "اینچ", "اینچ",
		"پوینت", "پوینت‌ها", "پوینتها",
		"پیکا", "پیکاها", "پیکاها",
		"اکس",
		"سی اچ",
		"وی مین", "وی مینیمم",
		"وی مکس", "وی ماکسیمم"
	};

	const char* generated_prefixes[] = {
		"px", "px", "px",
		"em", "em",
		"rem",
		"vw", "vw", "vw",
		"vh", "vh", "vh",
		"%",
		"cm", "cm", "cm",
		"mm", "mm", "mm",
		"in", "in", "in",
		"pt", "pt", "pt",
		"pc", "pc", "pc",
		"ex",
		"ch",
		"vmin", "vmin",
		"vmax", "vmax"
	};

	int num_prefixes = sizeof(prefixes) / sizeof(prefixes[0]);
	int num_persian_prefixes = sizeof(persian_prefixes) / sizeof(persian_prefixes[0]);

	size_t len = strlen(css_value);
	if (len == 0) {
		*css_output_value = NULL;

		return false;
	}

	string_t* buffer = string_create(len + 1);

	size_t i = 0;
	if (css_value[i] == '-' || css_value[i] == '+') {
		if (css_value[i] == '-') {
			string_append_char(buffer, css_value[i]);
		}

		i++;
	}

	if (!isdigit(css_value[i])) {
		*css_output_value = NULL;

		string_destroy(buffer);

		return false;
	}

	bool decimal_point_found = false;
	while (i < len && (isdigit(css_value[i]) || (css_value[i] == '.' && !decimal_point_found))) {
		if (css_value[i] == '.') {
			string_append_char(buffer, css_value[i]);
			decimal_point_found = true;
		}
		else {
			string_append_char(buffer, css_value[i]);
		}

		i++;
	}

	while (i < len && isspace(css_value[i])) i++;

	for (int j = 0; j < num_prefixes; j++) {
		size_t prefix_len = strlen(prefixes[j]); // TODO: mb2strlen

		if (len - i == prefix_len && strncmp(css_value + i, prefixes[j], prefix_len) == 0) {
			string_append_str(buffer, generated_prefixes[j]);

			*css_output_value = strdup(buffer->data);

			string_destroy(buffer);

			return true;
		}
	}

	for (int j = 0; j < num_persian_prefixes; j++) {
		size_t prefix_len = strlen(persian_prefixes[j]); // TODO: mb2strlen

		if (len - i == prefix_len && strncmp(css_value + i, persian_prefixes[j], prefix_len) == 0) {
			string_append_str(buffer, generated_prefixes[j]);

			*css_output_value = strdup(buffer->data);

			string_destroy(buffer);

			return true;
		}
	}

	*css_output_value = NULL;

	string_destroy(buffer);

	return false;
}

/**
 *
 * @function normalise_css_size
 * @brief Normalise the CSS size
 * @params {char*} attribute_value - Attribute value
 * @returns {char*} - Normalised CSS size
 *
 */
char* normalise_css_size(char* attribute_value)
{
	DEBUG_ME;
	if (!string_is_number(attribute_value) == true) {
		return strdup(attribute_value);
	}

	size_t value_length = strlen(attribute_value) + 3; // TODO: mb2strlen

	char* res = memory_allocate(value_length * sizeof(char));

	snprintf(res, value_length, "%spx", attribute_value);

	return res;
}

/**
 *
 * @function attribute_css_size_value
 * @brief Convert the attribute value to a CSS size value
 * @params {char*} attribute_value - Attribute value
 * @returns {char*} - CSS size value
 *
 */
char* attribute_css_size_value(char* attribute_value)
{
	DEBUG_ME;
	int attribute_value_length = strlen(attribute_value) + 5; // mb2strlen

	char* res = memory_allocate(attribute_value_length * sizeof(char));

	strcpy(res, attribute_value);

	if (string_is_number(attribute_value) == true) {
		strcat(res, "px");
	}

	return res;
}

/**
 *
 * @function validate_layout_block
 * @brief Validate and modify the layout block
 * @params {ast_layout_block_t*} block - Layout block
 * @returns {void}
 *
 */
void validate_layout_block(ast_layout_block_t* block)
{
	DEBUG_ME;
	hashmap_layout_attribute_t* attributes = block->attributes;

	ast_layout_attribute_t* attribute_content = hashmap_get(attributes, "content");

	if (attribute_content != NULL) {
		attribute_content->isContent = true;

		array_value_t* values = attribute_content->values;

		if (values->length > 0) {
			char* content = array_value_stringify(values, ", ");

			if (content != NULL) {
				if (strlen(content) > 0) {
					block->text_content = content;
				}
				else {
					if (content != NULL) {
						memory_destroy(content);
					}
				}
			}
		}
	}
}

/**
 *
 * @function is_attribute_type_in_array
 * @brief Check if the attribute type is in the array
 * @params {ast_layout_attribute_type_t} type - Attribute type
 * @params {ast_layout_attribute_type_t*} array - Array of attribute types
 * @params {size_t} array_size - Array size
 * @returns {bool} - True if the attribute type is in the array, false otherwise
 *
 */
bool is_attribute_type_in_array(ast_layout_attribute_type_t type, ast_layout_attribute_type_t* array, size_t array_size)
{
	DEBUG_ME;
	for (size_t i = 0; i < array_size; i++) {
		if (array[i] == type) {
			return true;
		}
	}

	return false;
}

/**
 *
 * @function token_belongs_to_ast_layout_node
 * @brief Check if the token belongs to the AST layout node
 * @params {ast_layout_attribute_type_t} attribute_key_type - Attribute key type
 * @params {ast_layout_attribute_t*} attribute - AST layout attribute
 * @returns {bool} - True if the token belongs to the AST layout node, false otherwise
 *
 */
bool token_belongs_to_ast_layout_node(ast_layout_attribute_type_t attribute_key_type, ast_layout_attribute_t* attribute)
{
	DEBUG_ME;
	if (is_layout_node_a_single_tag(attribute->parent_node_type) && attribute_key_type == AST_LAYOUT_ATTRIBUTE_TYPE_CONTENT) {
		return false;
	}
	else if (attribute_key_type == AST_LAYOUT_ATTRIBUTE_TYPE_CONTENT) {
		return true;
	}
	else if (is_style_attribute(attribute_key_type)) {
		attribute->isStyle = true;

		return true;
	}
	else if (is_attribute_type_a_style(attribute_key_type)) {
		return true;
	}

	// LAYOUT
	if (attribute->parent_node_type == AST_LAYOUT_TYPE_NONE) {
		if (is_attribute_type_in_array(attribute_key_type, valid_layout_attributes, valid_layout_attributes_length)) {
			return true;
		}
	}
	// IMG
	else if (attribute->parent_node_type == AST_LAYOUT_TYPE_IMG) {
		ast_layout_attribute_type_t valid_attributes[] = {
			AST_LAYOUT_ATTRIBUTE_TYPE_SRC,
		};

		size_t valid_attributes_length = sizeof(valid_attributes) / sizeof(valid_attributes[0]);

		if (is_attribute_type_in_array(attribute_key_type, valid_attributes, valid_attributes_length)) {
			return true;
		}
	}

	return false;
}

/**
 *
 * @function is_layout_node_a_single_tag
 * @brief Check if the layout node is a single tag
 * @params {ast_layout_node_type_t} type - Layout node type
 * @returns {bool} - True if the layout node is a single tag, false otherwise
 *
 */
bool is_layout_node_a_single_tag(ast_layout_node_type_t type)
{
	DEBUG_ME;
	switch (type) {
		case AST_LAYOUT_TYPE_PARAGRAPH_RAW:
		case AST_LAYOUT_TYPE_INPUT:
		case AST_LAYOUT_TYPE_BR:
		case AST_LAYOUT_TYPE_HR:
			return true;

		default:
			return false;
	}
}

/**
 *
 * @function validate_layout_mainbody
 * @brief Validate the layout mainbody
 * @params {ast_layout_block_t*} block - Layout block
 * @returns {void}
 *
 */
void validate_layout_mainbody(ast_layout_block_t* block)
{
	DEBUG_ME;
	if (block != NULL) {
		if (block->attributes != NULL) {
			if (block->attributes->data != NULL) {
				size_t attributes_capacity = block->attributes->capacity;

				for (size_t i = 0; i < attributes_capacity; i++) {
					hashmap_entry_t* entry = block->attributes->data[i];

					while (entry) {
						char* attribute_key = entry->key;
						ast_layout_attribute_type_t attribute_key_type = name_to_ast_layout_attribute_type(attribute_key);
						ast_layout_attribute_t* attribute_value = entry->value;

						if (is_attribute_type_in_array(attribute_key_type, valid_layout_attributes, valid_layout_attributes_length)) {
							attribute_value->ignoreMe = true;
						}

						entry = cast(hashmap_entry_t*, entry->next);
					}
				}
			}
		}
	}
}

/**
 *
 * @function is_style_attribute
 * @brief Check if the attribute type is a CSS attribute
 * @params {ast_layout_attribute_type_t} type - Attribute type
 * @returns {bool} - True if the attribute type is a style, false otherwise
 *
 */
bool is_style_attribute(ast_layout_attribute_type_t type)
{
	DEBUG_ME;
	switch (type) {
		#undef ADD_LAYOUT_ATTRIBUTE_STYLE_TYPE
		#define ADD_LAYOUT_ATTRIBUTE_STYLE_TYPE(TYPE, NAME, NAME_LOWER, ENDUSER_NAME, GENERATED_NAME, FILTER, ALLOWED_VALUES, SUBTAGS) case TYPE:

		#include "ast_layout_attribute_style_type.h"
			return true;

		default:
			return false;
	}
}

/**
 *
 * @function is_attribute_type_a_style
 * @brief Check if the attribute type is a style
 * @params {ast_layout_attribute_type_t} type - Attribute type
 * @returns {bool} - True if the attribute type is a style, false otherwise
 *
 */
bool is_attribute_type_a_style(ast_layout_attribute_type_t type)
{
	DEBUG_ME;
	switch (type) {
		case AST_LAYOUT_ATTRIBUTE_TYPE_CLASS:
		case AST_LAYOUT_ATTRIBUTE_TYPE_ID:
			return true;

		default:
			return false;
	}
}

/**
 *
 * @function validate_style_value_string
 * @brief Validate the style value string
 * @params {hashmap_t*} styles - Styles
 * @params {hashmap_t*} new_styles - New styles
 * @params {ast_layout_attribute_t*} attribute - Layout attribute
 * @params {const ast_layout_attribute_style_pair_t*} allowed_values1 - Allowed values 1
 * @params {const ast_layout_attribute_style_pair_t*} allowed_values2 - Allowed values 2
 * @returns {bool} - True if the style value is valid, false otherwise
 *
 */
bool validate_style_value_string(hashmap_t* styles, hashmap_t* new_styles, ast_layout_attribute_t* attribute, const ast_layout_attribute_style_pair_t* allowed_values1, const ast_layout_attribute_style_pair_t* allowed_values2)
{
	DEBUG_ME;
	if (styles) {}
	if (new_styles) {}

	if (attribute->values == NULL || attribute->values->data == NULL || attribute->values->data[0] == NULL) {
		return false;
	}

	ast_value_t* first = attribute->values->data[0];

	if (first->type->kind == AST_TYPE_KIND_STRING) {
		char* value = first->data.string_value;

		if (strlen(value) == 0) {
			return false;
		}

		if (allowed_values2 != NULL) {
			size_t i = 0;
			while (allowed_values2[i].input != NULL) {
				if (strcmp(value, allowed_values2[i].input) == 0) {
					attribute->final_value = strdup(allowed_values2[i].output);

					return true;
				}
				i++;
			}
		}

		if (allowed_values1 != NULL) {
			size_t i = 0;
			while (allowed_values1[i].input != NULL) {
				if (strcmp(value, allowed_values1[i].input) == 0) {
					attribute->final_value = strdup(allowed_values1[i].output);

					return true;
				}
				i++;
			}
		}
	}

	return false;
}

/**
 *
 * @function validate_style_value_sizes_colors
 * @brief Validate the style values color or size
 * @params {hashmap_t*} styles - Styles
 * @params {hashmap_t*} new_styles - New styles
 * @params {ast_layout_attribute_t*} attribute - Layout attribute
 * @params {const ast_layout_attribute_style_pair_t*} allowed_values1 - Allowed values 1
 * @params {const ast_layout_attribute_style_pair_t*} allowed_values2 - Allowed values 2
 * @returns {bool} - True if the style value is valid, false otherwise
 *
 */
bool validate_style_value_sizes_colors(hashmap_t* styles, hashmap_t* new_styles, ast_layout_attribute_t* attribute, const ast_layout_attribute_style_pair_t* allowed_values1, const ast_layout_attribute_style_pair_t* allowed_values2)
{
	DEBUG_ME;

	for (size_t i = 0; i < attribute->values->length; i++) {
		if (
			validate_style_value_color(styles, new_styles, attribute, allowed_values1, allowed_values2) == false
			&&
			validate_style_value_size(styles, new_styles, attribute, allowed_values1, allowed_values2) == false
		) {
			return false;
		}
	}

	return true;
}

/**
 *
 * @function validate_style_value_colors
 * @brief Validate the style values color
 * @params {hashmap_t*} styles - Styles
 * @params {hashmap_t*} new_styles - New styles
 * @params {ast_layout_attribute_t*} attribute - Layout attribute
 * @params {const ast_layout_attribute_style_pair_t*} allowed_values1 - Allowed values 1
 * @params {const ast_layout_attribute_style_pair_t*} allowed_values2 - Allowed values 2
 * @returns {bool} - True if the style value is valid, false otherwise
 *
 */
bool validate_style_value_colors(hashmap_t* styles, hashmap_t* new_styles, ast_layout_attribute_t* attribute, const ast_layout_attribute_style_pair_t* allowed_values1, const ast_layout_attribute_style_pair_t* allowed_values2)
{
	DEBUG_ME;

	for (size_t i = 0; i < attribute->values->length; i++) {
		if (validate_style_value_color(styles, new_styles, attribute, allowed_values1, allowed_values2) == false) {
			return false;
		}
	}

	return true;
}

/**
 *
 * @function validate_style_value_color
 * @brief Validate the style value color
 * @params {hashmap_t*} styles - Styles
 * @params {hashmap_t*} new_styles - New styles
 * @params {ast_layout_attribute_t*} attribute - Layout attribute
 * @params {const ast_layout_attribute_style_pair_t*} allowed_values1 - Allowed values 1
 * @params {const ast_layout_attribute_style_pair_t*} allowed_values2 - Allowed values 2
 * @returns {bool} - True if the style value is valid, false otherwise
 *
 */
bool validate_style_value_color(hashmap_t* styles, hashmap_t* new_styles, ast_layout_attribute_t* attribute, const ast_layout_attribute_style_pair_t* allowed_values1, const ast_layout_attribute_style_pair_t* allowed_values2)
{
	DEBUG_ME;
	if (styles) {}
	if (new_styles) {}

	if (attribute->values == NULL || attribute->values->data == NULL || attribute->values->data[0] == NULL) {
		return false;
	}

	ast_value_t* first = attribute->values->data[0];

	if (first->type->kind == AST_TYPE_KIND_STRING) {
		char* value = first->data.string_value;

		if (strlen(value) == 0) {
			return false;
		}

		if (allowed_values2 != NULL) {
			size_t i = 0;
			
			while (allowed_values2[i].input != NULL) {
				if (strcmp(value, allowed_values2[i].input) == 0) {
					attribute->final_value = strdup(allowed_values2[i].output);

					return true;
				}
				i++;
			}
		}

		if (allowed_values1 != NULL) {
			size_t i = 0;
			while (allowed_values1[i].input != NULL) {
				if (strcmp(value, allowed_values1[i].input) == 0) {
					attribute->final_value = strdup(allowed_values1[i].output);

					return true;
				}
				i++;
			}
		}
	}

	return false;
}

/**
 *
 * @function validate_style_value_integer
 * @brief Validate the style value integer
 * @params {hashmap_t*} styles - Styles
 * @params {hashmap_t*} new_styles - New styles
 * @params {ast_layout_attribute_t*} attribute - Layout attribute
 * @params {const ast_layout_attribute_style_pair_t*} allowed_values1 - Allowed values 1
 * @params {const ast_layout_attribute_style_pair_t*} allowed_values2 - Allowed values 2
 * @returns {bool} - True if the style value is valid, false otherwise
 *
 */
bool validate_style_value_integer(hashmap_t* styles, hashmap_t* new_styles, ast_layout_attribute_t* attribute, const ast_layout_attribute_style_pair_t* allowed_values1, const ast_layout_attribute_style_pair_t* allowed_values2)
{
	DEBUG_ME;
	if (styles) {}
	if (new_styles) {}

	ast_value_t* first = attribute->values->data[0];

	if (first->type->kind == AST_TYPE_KIND_INT) {
		attribute->final_value = memory_allocate(20 * sizeof(char));
		snprintf(attribute->final_value, 20, "%d", first->data.int_value);

		return true;
	}
	else if (first->type->kind == AST_TYPE_KIND_FLOAT) {
		if (first->data.float_value == (int)first->data.float_value) {
			attribute->final_value = memory_allocate(20 * sizeof(char));
			snprintf(attribute->final_value, 20, "%d", (int)first->data.float_value);

			return true;
		}

		return false;
	}
	else if (first->type->kind == AST_TYPE_KIND_STRING) {
		char* value = first->data.string_value;

		if (strlen(value) == 0) {
			return false;
		}

		if (allowed_values2 != NULL) {
			size_t i = 0;
			while (allowed_values2[i].input != NULL) {
				if (strcmp(value, allowed_values2[i].input) == 0) {
					attribute->final_value = strdup(allowed_values2[i].output);

					return true;
				}

				i++;
			}
		}

		if (allowed_values1 != NULL) {
			size_t i = 0;
			while (allowed_values1[i].input != NULL) {
				if (strcmp(value, allowed_values1[i].input) == 0) {
					attribute->final_value = strdup(allowed_values1[i].output);

					return true;
				}

				i++;
			}
		}

		if (string_is_integer(value)) {
			return true;
		}

		return false;
	}

	return false;
}

/**
 *
 * @function validate_style_value_number
 * @brief Validate the style value number
 * @params {hashmap_t*} styles - Styles
 * @params {hashmap_t*} new_styles - New styles
 * @params {ast_layout_attribute_t*} attribute - Layout attribute
 * @params {const ast_layout_attribute_style_pair_t*} allowed_values1 - Allowed values 1
 * @params {const ast_layout_attribute_style_pair_t*} allowed_values2 - Allowed values 2
 * @returns {bool} - True if the style value is valid, false otherwise
 *
 */
bool validate_style_value_number(hashmap_t* styles, hashmap_t* new_styles, ast_layout_attribute_t* attribute, const ast_layout_attribute_style_pair_t* allowed_values1, const ast_layout_attribute_style_pair_t* allowed_values2)
{
	DEBUG_ME;
	if (styles) {}
	if (new_styles) {}

	ast_value_t* first = attribute->values->data[0];

	if (first->type->kind == AST_TYPE_KIND_INT) {
		return true;
	}
	else if (first->type->kind == AST_TYPE_KIND_FLOAT) {
		return true;
	}
	else if (first->type->kind == AST_TYPE_KIND_STRING) {
		char* value = first->data.string_value;

		if (strlen(value) == 0) {
			return false;
		}

		if (allowed_values2 != NULL) {
			size_t i = 0;
			while (allowed_values2[i].input != NULL) {
				if (strcmp(value, allowed_values2[i].input) == 0) {
					attribute->final_value = strdup(allowed_values2[i].output);

					return true;
				}

				i++;
			}
		}

		if (allowed_values1 != NULL) {
			size_t i = 0;
			while (allowed_values1[i].input != NULL) {
				if (strcmp(value, allowed_values1[i].input) == 0) {
					attribute->final_value = strdup(allowed_values1[i].output);

					return true;
				}

				i++;
			}
		}

		if (string_is_number(value)) {
			return true;
		}

		return false;
	}

	return false;
}

/**
 *
 * @function validate_style_value_float
 * @brief Validate the style value float
 * @params {hashmap_t*} styles - Styles
 * @params {hashmap_t*} new_styles - New styles
 * @params {ast_layout_attribute_t*} attribute - Layout attribute
 * @params {const ast_layout_attribute_style_pair_t*} allowed_values1 - Allowed values 1
 * @params {const ast_layout_attribute_style_pair_t*} allowed_values2 - Allowed values 2
 * @returns {bool} - True if the style value is valid, false otherwise
 *
 */
bool validate_style_value_float(hashmap_t* styles, hashmap_t* new_styles, ast_layout_attribute_t* attribute, const ast_layout_attribute_style_pair_t* allowed_values1, const ast_layout_attribute_style_pair_t* allowed_values2)
{
	DEBUG_ME;
	if (styles) {}
	if (new_styles) {}

	ast_value_t* first = attribute->values->data[0];

	if (first->type->kind == AST_TYPE_KIND_INT) {
		return false;
	}
	else if (first->type->kind == AST_TYPE_KIND_FLOAT) {
		return true;
	}
	else if (first->type->kind == AST_TYPE_KIND_STRING) {
		char* value = first->data.string_value;

		if (strlen(value) == 0) {
			return false;
		}

		if (allowed_values2 != NULL) {
			size_t i = 0;
			while (allowed_values2[i].input != NULL) {
				if (strcmp(value, allowed_values2[i].input) == 0) {
					attribute->final_value = strdup(allowed_values2[i].output);

					return true;
				}

				i++;
			}
		}

		if (allowed_values1 != NULL) {
			size_t i = 0;
			while (allowed_values1[i].input != NULL) {
				if (strcmp(value, allowed_values1[i].input) == 0) {
					attribute->final_value = strdup(allowed_values1[i].output);

					return true;
				}

				i++;
			}
		}

		if (string_is_float(value)) {
			return true;
		}

		return false;
	}

	return false;
}

/**
 *
 * @function validate_style_value_percentage
 * @brief Validate the style value percentage
 * @params {hashmap_t*} styles - Styles
 * @params {hashmap_t*} new_styles - New styles
 * @params {ast_layout_attribute_t*} attribute - Layout attribute
 * @params {const ast_layout_attribute_style_pair_t*} allowed_values1 - Allowed values 1
 * @params {const ast_layout_attribute_style_pair_t*} allowed_values2 - Allowed values 2
 * @returns {bool} - True if the style value is valid, false otherwise
 *
 */
bool validate_style_value_percentage(hashmap_t* styles, hashmap_t* new_styles, ast_layout_attribute_t* attribute, const ast_layout_attribute_style_pair_t* allowed_values1, const ast_layout_attribute_style_pair_t* allowed_values2)
{
	DEBUG_ME;
	if (styles) {}
	if (new_styles) {}

	ast_value_t* first = attribute->values->data[0];

	if (first->type->kind == AST_TYPE_KIND_INT) {
		if (first->data.int_value <= 0 || first->data.int_value >= 100) {
			error_validator(2, "Percentage value must be between 0 and 100 for attribute '%s' in '%s' element at line %zu column %zu!", attribute->key, ast_layout_node_type_to_enduser_name(attribute->parent_node_type), attribute->value_location.start_line, attribute->value_location.start_column);

			return false;
		}
		else {
			attribute->final_value = memory_allocate(20 * sizeof(char));
			snprintf(attribute->final_value, 20, "%.2f", first->data.int_value / 100.0);

			return true;
		}
	}
	else if (first->type->kind == AST_TYPE_KIND_FLOAT) {
		if (first->data.float_value <= 0 || first->data.float_value >= 1) {
			error_validator(2, "Percentage value must be between 0 and 100 for attribute '%s' in '%s' element at line %zu column %zu!", attribute->key, ast_layout_node_type_to_enduser_name(attribute->parent_node_type), attribute->value_location.start_line, attribute->value_location.start_column);

			return false;
		}
		else {
			attribute->final_value = memory_allocate(20 * sizeof(char));
			snprintf(attribute->final_value, 20, "%.2f", first->data.float_value);

			return true;
		}
	}
	else if (first->type->kind == AST_TYPE_KIND_STRING) {
		char* value = first->data.string_value;

		if (strlen(value) == 0) {
			return false;
		}
		
		if (allowed_values2 != NULL) {
			size_t i = 0;
			while (allowed_values2[i].input != NULL) {
				if (strcmp(value, allowed_values2[i].input) == 0) {
					attribute->final_value = strdup(allowed_values2[i].output);

					return true;
				}

				i++;
			}
		}

		if (allowed_values1 != NULL) {
			size_t i = 0;
			while (allowed_values1[i].input != NULL) {
				if (strcmp(value, allowed_values1[i].input) == 0) {
					attribute->final_value = strdup(allowed_values1[i].output);

					return true;
				}

				i++;
			}
		}

		return true; // TODO
	}

	return false;
}

/**
 *
 * @function validate_style_value_size
 * @brief Validate the style value size
 * @params {hashmap_t*} styles - Styles
 * @params {hashmap_t*} new_styles - New styles
 * @params {ast_layout_attribute_t*} attribute - Layout attribute
 * @params {const ast_layout_attribute_style_pair_t*} allowed_values1 - Allowed values 1
 * @params {const ast_layout_attribute_style_pair_t*} allowed_values2 - Allowed values 2
 * @returns {bool} - True if the style value is valid, false otherwise
 *
 */
bool validate_style_value_size(hashmap_t* styles, hashmap_t* new_styles, ast_layout_attribute_t* attribute, const ast_layout_attribute_style_pair_t* allowed_values1, const ast_layout_attribute_style_pair_t* allowed_values2)
{
	DEBUG_ME;
	if (styles) {}
	if (new_styles) {}
	if (allowed_values1) {}
	if (allowed_values2) {}

	ast_value_t* first = attribute->values->data[0];
	if (first->type->kind == AST_TYPE_KIND_INT) {
		attribute->final_value = memory_allocate(20 * sizeof(char));
		snprintf(attribute->final_value, 20, "%dpx", first->data.int_value);

		return true;
	}
	else if (first->type->kind == AST_TYPE_KIND_FLOAT) {
		attribute->final_value = memory_allocate(20 * sizeof(char));
		snprintf(attribute->final_value, 20, "%fpx", first->data.float_value);

		return true;
	}
	else if (first->type->kind == AST_TYPE_KIND_STRING) {
		if (strlen(first->data.string_value) == 0) {
			return false;
		}
		else if (first->data.string_value[0] == '#') {
			return true;
		}

		char* buffer = normalise_css_size(first->data.string_value);

		char* out_value;
		if (!has_css_size_prefix(buffer, &out_value)) {
			if (out_value != NULL) {
				memory_destroy(out_value);
			}

			memory_destroy(buffer);

			return false;
		}

		attribute->final_value = strdup(out_value);
		memory_destroy(buffer);
		memory_destroy(out_value);

		return true;
	}

	return false;
}

/**
 *
 * @function validate_style_value_sizes
 * @brief Validate the style values size
 * @params {hashmap_t*} styles - Styles
 * @params {hashmap_t*} new_styles - New styles
 * @params {ast_layout_attribute_t*} attribute - Layout attribute
 * @params {const ast_layout_attribute_style_pair_t*} allowed_values1 - Allowed values 1
 * @params {const ast_layout_attribute_style_pair_t*} allowed_values2 - Allowed values 2
 * @returns {bool} - True if the style value is valid, false otherwise
 *
 */
bool validate_style_value_sizes(hashmap_t* styles, hashmap_t* new_styles, ast_layout_attribute_t* attribute, const ast_layout_attribute_style_pair_t* allowed_values1, const ast_layout_attribute_style_pair_t* allowed_values2)
{
	DEBUG_ME;

	for (size_t i = 0; i < attribute->values->length; i++) {
		if (validate_style_value_size(styles, new_styles, attribute, allowed_values1, allowed_values2) == false) {
			return false;
		}
	}

	return true;
}

/**
 *
 * @function validate_style_value
 * @brief Validate the style value
 * @params {hashmap_t*} styles - Styles
 * @params {hashmap_t*} new_styles - New styles
 * @params {ast_layout_attribute_t*} attribute - Layout attribute
 * @returns {bool} - True if the style value is valid, false otherwise
 *
 */
bool validate_style_value(hashmap_t* styles, hashmap_t* new_styles, ast_layout_attribute_t* attribute)
{
	DEBUG_ME;
	ast_value_t* first = attribute->values->data[0];
	char* value = first->data.string_value;

	if (attribute->values->length < 1) {
		error_validator(2, "Style value is missing in '%s' element", ast_layout_node_type_to_enduser_name(attribute->parent_node_type));

		return false;
	}
	else if (first->type->kind == AST_TYPE_KIND_STRING && strlen(first->data.string_value) == 0) {
		error_validator(2, "Empty value for '%s' attribute in '%s' element is not allowed at line %zu column %zu!", attribute->key, ast_layout_node_type_to_enduser_name(attribute->parent_node_type), attribute->value_location.start_line, attribute->value_location.start_column);

		return false;
	}
	// Global values
	else if (first->type->kind == AST_TYPE_KIND_STRING && attribute->values->length == 1) {
		if (false) {}
		#undef ADD_LAYOUT_ATTRIBUTE_STYLE_TYPE

		#define ADD_LAYOUT_ATTRIBUTE_STYLE_TYPE(GENERATED_NAME, ENDUSER_NAME) else if (strcmp(value, ENDUSER_NAME) == 0) { return true; }

		#include "ast_layout_attribute_style_global.h"
	}

	switch (attribute->type) {
		#undef ADD_LAYOUT_ATTRIBUTE_TYPE

		#define ADD_LAYOUT_ATTRIBUTE_TYPE(TYPE, NAME, NAME_LOWER, GENERATED_NAME, ENDUSER_NAME) case TYPE: return false;

		#include "ast_layout_attribute_type.h"

		#undef ADD_LAYOUT_ATTRIBUTE_STYLE_TYPE
		#undef ADD_LAYOUT_ATTRIBUTE_STYLE_TYPE_HIDE

		#define ADD_LAYOUT_ATTRIBUTE_STYLE_TYPE(TYPE, NAME, NAME_LOWER, ENDUSER_NAME, GENERATED_NAME, FILTER, ALLOWED_VALUES, SUBTAGS) case TYPE: \
			{ \
				attribute->final_key = strdup(GENERATED_NAME); \
				const ast_layout_attribute_style_pair_t *values = ALLOWED_VALUES; \
				\
				if (FILTER == AST_LAYOUY_ATTRIBUTE_STYLE_FILTER_COLOR) { \
					return validate_style_value_color(styles, new_styles, attribute, ast_layout_allowed_style_color, values); \
				} \
				else if (FILTER == AST_LAYOUY_ATTRIBUTE_STYLE_FILTER_STRING) { \
					return validate_style_value_string(styles, new_styles, attribute, NULL, values); \
				} \
				else if (FILTER == AST_LAYOUY_ATTRIBUTE_STYLE_FILTER_STRING_ANY) { \
					return true; \
				} \
				else if (FILTER == AST_LAYOUY_ATTRIBUTE_STYLE_FILTER_SIZE) { \
					return validate_style_value_size(styles, new_styles, attribute, NULL, values); \
				} \
				else if (FILTER == AST_LAYOUY_ATTRIBUTE_STYLE_FILTER_SIZE_COLOR) { \
					return validate_style_value_size_color(styles, new_styles, attribute, NULL, values); \
				} \
				else if (FILTER == AST_LAYOUY_ATTRIBUTE_STYLE_FILTER_SIZES) { \
					return validate_style_value_sizes(styles, new_styles, attribute, NULL, values); \
				} \
				else if (FILTER == AST_LAYOUY_ATTRIBUTE_STYLE_FILTER_COLORS) { \
					return validate_style_value_colors(styles, new_styles, attribute, NULL, values); \
				} \
				else if (FILTER == AST_LAYOUY_ATTRIBUTE_STYLE_FILTER_PERCENTAGE) { \
					return validate_style_value_percentage(styles, new_styles, attribute, NULL, values); \
				} \
				else if (FILTER == AST_LAYOUY_ATTRIBUTE_STYLE_FILTER_SIZES_COLORS) { \
					return validate_style_value_sizes_colors(styles, new_styles, attribute, NULL, values); \
				} \
				else if (FILTER == AST_LAYOUY_ATTRIBUTE_STYLE_FILTER_INTEGER) { \
					return validate_style_value_integer(styles, new_styles, attribute, NULL, values); \
				} \
				else if (FILTER == AST_LAYOUY_ATTRIBUTE_STYLE_FILTER_FLOAT) { \
					return validate_style_value_float(styles, new_styles, attribute, NULL, values); \
				} \
				else if (FILTER == AST_LAYOUY_ATTRIBUTE_STYLE_FILTER_NUMBER) { \
					return validate_style_value_number(styles, new_styles, attribute, NULL, values); \
				} \
				else if (FILTER == AST_LAYOUY_ATTRIBUTE_STYLE_FILTER_TIME) { \
					return true; \
				} \
				else if (FILTER == AST_LAYOUY_ATTRIBUTE_STYLE_FILTER_PIXEL) { \
					return true; \
				} \
				else if (FILTER == AST_LAYOUY_ATTRIBUTE_STYLE_FILTER_GROUP) { \
					return true; \
				} \
				else if (FILTER == AST_LAYOUY_ATTRIBUTE_STYLE_FILTER_POSITIVE_PERCENTAGE) { \
					return true; \
				} \
			}\
			return false;
		
		#define ADD_LAYOUT_ATTRIBUTE_STYLE_TYPE_HIDE(TYPE, NAME, NAME_LOWER, ENDUSER_NAME, GENERATED_NAME, FILTER, ALLOWED_VALUES, SUBTAGS)

		#include "ast_layout_attribute_style_type.h"
	}

	return false;
}
