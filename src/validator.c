#include "validator.h"

/**
 * 
 * @var valid_layout_attributes
 * @brief Valid layout attributes
 * @type {ast_layout_attribute_type_t[]}
 */
ast_layout_attribute_type_t valid_layout_attributes[] = {
	AST_LAYOUT_ATTRIBUTE_TYPE_TITLE,
	AST_LAYOUT_ATTRIBUTE_TYPE_DESCRIPTION,
	AST_LAYOUT_ATTRIBUTE_TYPE_AUTHOR,
	AST_LAYOUT_ATTRIBUTE_TYPE_KEYWORDS,
	AST_LAYOUT_ATTRIBUTE_TYPE_ICON,
	AST_LAYOUT_ATTRIBUTE_TYPE_CHARSET,
	AST_LAYOUT_ATTRIBUTE_TYPE_DIR,
	AST_LAYOUT_ATTRIBUTE_TYPE_LANG,
	AST_LAYOUT_ATTRIBUTE_TYPE_VIEWPORT,
	AST_LAYOUT_ATTRIBUTE_TYPE_REFRESH,
	AST_LAYOUT_ATTRIBUTE_TYPE_CHARSET,
};

const ast_layout_attribute_style_pair_t allowed_background_colors[] = {
    {"red", "red_output"},
    {"white", "white_output"},
    {"black", "black_output"},
    {"orange", "orange_output"},
    {"yellow", "yellow_output"},
    {"green", "green_output"},
    {"blue", "blue_output"},
    {"pink", "pink_output"},
    {"gray", "gray_output"},
	{"", ""},
};

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
	const char* prefixes[] = {"px", "em", "rem", "vw", "vh", "%", "cm", "mm", "in", "pt", "pc", "ex", "ch", "vmin", "vmax"};
	int num_prefixes = sizeof(prefixes) / sizeof(prefixes[0]);

	const char* persian_prefixes[] = {"پیکسل", "ای ام", "رایم", "ویو ویدث", "ویو هایت", "درصد", "سانتیمتر", "میلیمتر", "اینچ", "پوینت", "پیکا", "اکس", "سی اچ", "وی مین", "وی مکس"};
	int num_persian_prefixes = sizeof(persian_prefixes) / sizeof(persian_prefixes[0]);

	size_t len = strlen(css_value);
	if (len == 0) {
		*css_output_value = NULL;

		return false;
	}

	string_t* buffer = string_create(len);

	size_t i = 0;
	if (css_value[i] == '-' || css_value[i] == '+') {
		if (css_value[i] == '-') string_append_char(buffer, css_value[i]);

		i++;
	}

	if (!isdigit(css_value[i])) {
		*css_output_value = NULL;

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
		size_t prefix_len = strlen(prefixes[j]);
		if (len - i == prefix_len && strncmp(css_value + i, prefixes[j], prefix_len) == 0) {
			string_append_str(buffer, prefixes[j]);
			*css_output_value = strdup(buffer->data);

			string_destroy(buffer);

			return true;
		}
	}

	for (int j = 0; j < num_persian_prefixes; j++) {
		size_t prefix_len = strlen(persian_prefixes[j]);
		if (len - i == prefix_len && strncmp(css_value + i, persian_prefixes[j], prefix_len) == 0) {
			string_append_str(buffer, prefixes[j]);
			*css_output_value = strdup(buffer->data);

			string_destroy(buffer);

			return true;
		}
	}

	*css_output_value = NULL;

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

	int value_length = strlen(attribute_value) + 5;

	char* res = memory_allocate(value_length * sizeof(char));

	strcat(res, "px");

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
	int attribute_value_length = strlen(attribute_value) + 5;

	char* res = memory_allocate(attribute_value_length * sizeof(char));

	strcpy(res, attribute_value);

	if (string_is_number(attribute_value) == true) strcat(res, "px");

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
	hashmap_attribute_t* attributes = block->attributes;

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
					if (content != NULL) memory_destroy(content);
				}
			}
		}
	}

	// if (block->styles != NULL) {
	// 	hashmap_attribute_t* new_styles = hashmap_create_layout_attribute(1);

	// 	for (size_t i = 0; i < block->styles->capacity; i++) {
	// 		hashmap_entry_t* entry = block->styles->data[i];

	// 		while (entry) {
	// 			ast_layout_attribute_t* attribute = entry->value;
				
	// 			generator_code_layout_style_value(block->styles, block->new_styles, attribute, block->parent_node_type);
	// 		}
	// 	}
	// }
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
		// attribute->isStyle = false;
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


// bool validate_style_value_string(hashmap_t* styles, hashmap_t* new_styles, ast_layout_attribute_t* attribute, void** ALLOWED_VALUES, void** SUBTAGS)
// {
// 	if (styles) {}
// 	if (new_styles) {}
// 	if (attribute) {}
// 	if (ALLOWED_VALUES) {}
// 	if (SUBTAGS) {}
// }

/**
 * 
 * @function validate_style_value_color
 * @brief Validate the style value color
 * @params {hashmap_t*} styles - Styles
 * @params {hashmap_t*} new_styles - New styles
 * @params {ast_layout_attribute_t*} attribute - Layout attribute
 * @returns {bool} - True if the style value is valid, false otherwise
 * 
 */
bool validate_style_value_color(hashmap_t* styles, hashmap_t* new_styles, ast_layout_attribute_t* attribute, ast_layout_attribute_style_pair_t* allowed_values, size_t allowed_values_count)
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

        if (allowed_values == NULL || allowed_values_count == 0) {
            return false;
        }

        for (size_t i = 0; i < allowed_values_count; ++i) {
            if (strcmp(value, allowed_values[i].input) == 0) {
				attribute->final_value = strdup(allowed_values[i].output);
                return true;
            }
        }
    }

    return false;
}

bool validate_style_value_color3(hashmap_t* styles, hashmap_t* new_styles, ast_layout_attribute_t* attribute, ast_layout_attribute_style_pair_t* ALLOWED_VALUES)
{
    DEBUG_ME;

    // if (attribute->values == NULL || attribute->values->data == NULL || attribute->values->data[0] == NULL) {
    //     return false;
    // }

	printf("number of attr args: %zu\n", attribute->values->length);
    ast_value_t* first = attribute->values->data[0];

	if (ALLOWED_VALUES != NULL) {
		printf("going to check allowed values\n");
		if (first->type->kind == AST_TYPE_KIND_STRING) {
			printf("okay value is string\n");
			char* value = first->data.string_value;
			printf("value is %s\n", value);


// const ast_layout_attribute_style_pair_t allowed_background_colors[] = {
//     {"red", "red_output"},
//     {"white", "white_output"},
//     {"black", "black_output"},
//     {"orange", "orange_output"},
//     {"yellow", "yellow_output"},
//     {"green", "green_output"},
//     {"blue", "blue_output"},
//     {"pink", "pink_output"},
//     {"gray", "gray_output"},
// 	{"", ""},
// };

			size_t i = 0;
			while (true) {
				printf("----checking %zu\n", i);
				if (strcmp(allowed_background_colors[i].input, "") == 0) {
					break;
				}

				printf("compare %s -> %s\n", allowed_background_colors[i].input);
				printf("-->%s\n", value);

				if (strcmp(value, allowed_background_colors[i].input) == 0) {
					attribute->final_value = strdup(allowed_background_colors[i].output);
					// attribute->final_value = memory_allocate(strlen(allowed_background_colors[i].output) * sizeof(char));
					// strcpy(attribute->final_value, allowed_background_colors[i].output);
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
 * @function validate_style_value_size
 * @brief Validate the style value size
 * @params {hashmap_t*} styles - Styles
 * @params {hashmap_t*} new_styles - New styles
 * @params {ast_layout_attribute_t*} attribute - Layout attribute
 * @returns {bool} - True if the style value is valid, false otherwise
 * 
 */
/*
bool validate_style_value_size(hashmap_t* styles, hashmap_t* new_styles, ast_layout_attribute_t* attribute, int FILTER, void* ALLOWED_VALUES, void* SUBTAGS)
{
	DEBUG_ME;
	if (styles) {}
	if (new_styles) {}
	if (attribute) {}

	return true;
	// ast_value_t* first = attribute->values->data[0];
	// if (first->type->kind == AST_TYPE_KIND_INT) {
	// 	first->type->kind = AST_TYPE_KIND_STRING;
	// 	first->data.string_value = memory_allocate(20 * sizeof(char));
	// 	sprintf(first->data.string_value, "%dpx", first->data.int_value);

	// 	return true;
	// }
	// else if (first->type->kind == AST_TYPE_KIND_FLOAT) {
	// 	first->type->kind = AST_TYPE_KIND_STRING;
	// 	first->data.string_value = memory_allocate(20 * sizeof(char));
	// 	sprintf(first->data.string_value, "%fpx", first->data.float_value);

	// 	return true;
	// }
	// else if (first->type->kind == AST_TYPE_KIND_STRING) {
	// 	char* out_value;
	// 	if (!has_css_size_prefix(first->data.string_value, &out_value)) return false;

	// 	if (out_value != NULL) free(out_value);

	// 	return true;
	// }

	// return false;
}
*/

/**
 * 
 * @function validate_style_value_sizes
 * @brief Validate the style value sizes
 * @params {hashmap_t*} styles - Styles
 * @params {hashmap_t*} new_styles - New styles
 * @params {ast_layout_attribute_t*} attribute - Layout attribute
 * @returns {bool} - True if the style value is valid, false otherwise
 * 
 */
bool validate_style_value_sizes(hashmap_t* styles, hashmap_t* new_styles, ast_layout_attribute_t* attribute)
{
	DEBUG_ME;
	if (styles) {}
	if (new_styles) {}
	if (attribute) {}

	// if (attribute->values->length == 1) {
	// 	ast_value_t* first = attribute->values->data[0];
	// 	char* value_normal = normalise_css_size(first->data.string_value);

	// 	return value_normal;
	// }

	// attribute->final_value = memory_allocate(20 * sizeof(char));
	// strcpy(attribute->final_value, "val");

	return true;

	// string_t* buffer = string_create(10);

	// for (size_t i = 0; i < attribute->values->length; i++) {
	// 	ast_value_t* value = attribute->values->data[0];
	// 	char* value_normal = normalise_css_size(value->data);

	// 	char* out_value;
	// 	if (!has_css_size_prefix(value_normal, &out_value)) return NULL;

	// 	if (value_normal != NULL) free(value_normal);

	// 	if (out_value == NULL) {
	// 		return false;
	// 	}
	// 	string_append_str(buffer, out_value);

	// 	if (out_value != NULL) free(out_value);

	// 	if (i != attribute->values->length - 1) string_append_char(buffer, ' ');
	// }

	// attribute->final_value = strdup(buffer->data);
	// string_destroy(buffer);

	// return true;
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

	if (new_styles) {}
	if (styles) {}

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
		#define ADD_LAYOUT_ATTRIBUTE_STYLE_TYPE(GENERATED_NAME, ENDUSER_NAME) else if (strcmp(value, ENDUSER_NAME) == 0) return true;

		#include "ast_layout_attribute_style_global.h"
	}

	DEBUG_ME;
	printf("Attr Type: %s\n", ast_layout_attribute_type_to_name(attribute->type));
	attribute->print(attribute);

	switch (attribute->type) {

		// case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BACKGROUND:
		// 	attribute->final_key = strdup("background");
		// 	return true;
		// 	break;

		#undef ADD_LAYOUT_ATTRIBUTE_TYPE
		#define ADD_LAYOUT_ATTRIBUTE_TYPE(TYPE, NAME, NAME_LOWER, GENERATED_NAME, ENDUSER_NAME) case TYPE: return false;

		#include "ast_layout_attribute_type.h"

		#undef ADD_LAYOUT_ATTRIBUTE_STYLE_TYPE
		#undef ADD_LAYOUT_ATTRIBUTE_STYLE_TYPE_HIDE
		#define ADD_LAYOUT_ATTRIBUTE_STYLE_TYPE_HIDE(TYPE, NAME, NAME_LOWER, GENERATED_NAME, ENDUSER_NAME, FILTER, ALLOWED_VALUES, SUBTAGS) 
		#define ADD_LAYOUT_ATTRIBUTE_STYLE_TYPE(TYPE, NAME, NAME_LOWER, ENDUSER_NAME, GENERATED_NAME, FILTER, ALLOWED_VALUES, SUBTAGS) case TYPE: \
			attribute->final_key = strdup(GENERATED_NAME); \
			if (FILTER == AST_LAYOUY_ATTRIBUTE_STYLE_FILTER_COLOR) {\
				size_t allowed_values_count = (ALLOWED_VALUES == NULL) ? 0 : sizeof(ALLOWED_VALUES) / sizeof(ALLOWED_VALUES[0]); \
				return validate_style_value_color(styles, new_styles, attribute, ALLOWED_VALUES, allowed_values_count);\
			}\
			return false;

		#include "ast_layout_attribute_style_type.h"
	}

	return false;
}