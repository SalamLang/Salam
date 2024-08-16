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
 * @function is_english_digit
 * @brief Check if the character is an English digit
 * @params {wchar_t} ch - Character
 * @returns {bool} - True if the character is an English digit, false otherwise
 * 
 */
bool is_english_digit(wchar_t ch)
{
	DEBUG_ME;
	// 0123456789
	return ch >= L'0' && ch <= L'9';
}

/**
 * 
 * @function is_persian_digit
 * @brief Check if the character is a Persian digit
 * @params {wchar_t} ch - Character
 * @returns {bool} - True if the character is a Persian digit, false otherwise
 * 
 */
bool is_persian_digit(wchar_t ch)
{
	DEBUG_ME;
	// ۰۱۲۳۴۵۶۷۸۹
	return ch >= 0x06F0 && ch <= 0x06F9;
	// return ch >= '۰' && ch <= '۹';
}

/**
 * 
 * @function is_arabic_digit
 * @brief Check if the character is an Arabic digit
 * @params {wchar_t} ch - Character
 * @returns {bool} - True if the character is an Arabic digit, false otherwise
 * 
 */
bool is_arabic_digit(wchar_t ch)
{
	DEBUG_ME;
	// ٠١٢٣٤٥٦٧٨٩
	return ch >= 0x0660 && ch <= 0x0669;
}

/**
 * 
 * @function string_is_number
 * @brief Check if the string is a number
 * @params {const char*} value - Value
 * @returns {bool} - True if the string is a number, false otherwise
 * 
 */
bool string_is_number(const char* value)
{
	DEBUG_ME;
	size_t len = mbstowcs(NULL, value, 0);
	if (len == (size_t)-1) return false;

	wchar_t* wvalue = memory_allocate(sizeof(wchar_t) * (len + 1));
	mbstowcs(wvalue, value, len + 1);

	if (wvalue[0] == L'\0') {
		free(wvalue);

		return false;
	}

	size_t start = 0;
	if (wvalue[0] == L'+' || wvalue[0] == L'-') start = 1;

	if (start == 1 && wvalue[1] == L'\0') {
		free(wvalue);

		return false;
	}

	for (size_t i = start; wvalue[i] != L'\0'; i++) {
		if (!(is_english_digit(wvalue[i]) || is_persian_digit(wvalue[i]) || is_arabic_digit(wvalue[i]))) {
			free(wvalue);

			return false;
		}
	}

	free(wvalue);

	return true;
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
			char* content = array_value_string(values, ", ");

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
	// 		hashmap_entry_t *entry = block->styles->data[i];

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
	if (attribute->parent_node_type == AST_LAYOUT_NODE_TYPE_NONE) {
		if (is_attribute_type_in_array(attribute_key_type, valid_layout_attributes, valid_layout_attributes_length)) {
			return true;
		}
	}
	// IMG
	else if (attribute->parent_node_type == AST_LAYOUT_NODE_TYPE_IMG) {
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
		case AST_LAYOUT_NODE_TYPE_PARAGRAPH_RAW:
		case AST_LAYOUT_NODE_TYPE_INPUT:
		case AST_LAYOUT_NODE_TYPE_BR:
		case AST_LAYOUT_NODE_TYPE_HR:
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
					hashmap_entry_t *entry = block->attributes->data[i];

					while (entry) {
						char* attribute_key = entry->key;
						ast_layout_attribute_type_t attribute_key_type = name_to_ast_layout_attribute_type(attribute_key);
						ast_layout_attribute_t* attribute_value = entry->value;
						
						if (is_attribute_type_in_array(attribute_key_type, valid_layout_attributes, valid_layout_attributes_length)) {
							attribute_value->ignoreMe = true;
						}

						entry = entry->next;
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
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BACKGROUND:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_COLOR:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FONT:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FONT_FAMILY:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FONT_SIZE:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FONT_STYLE:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FONT_WEIGHT:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_HEIGHT:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_WIDTH:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_MARGIN:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_MARGIN_LEFT:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_MARGIN_RIGHT:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_MARGIN_TOP:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_MARGIN_BOTTOM:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_PADDING:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_PADDING_LEFT:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_PADDING_RIGHT:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_PADDING_TOP:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_PADDING_BOTTOM:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_TEXT_EMPHASIS_COLOR:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_TEXT_ALIGN:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_TEXT_ALIGN_LAST:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_TEXT_DECORATION:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_TEXT_DECORATION_STYLE:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_TEXT_EMPHASIS_POSITION:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_TEXT_EMPHASIS_STYLE:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_TEXT_DECORATION_THICKNESS:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_TEXT_DECORATION_LINE:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_TEXT_DECORATION_COLOR:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_TEXT_DECORATION_SKIP:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_TEXT_DECORATION_SKIP_INK:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_TEXT_TRANSFORM:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_VERTICAL_ALIGN:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_VISIBILITY:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_Z_INDEX:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_POSITION:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_TOP:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_RIGHT:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BOTTOM:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_LEFT:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BORDER:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BORDER_COLOR:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BORDER_STYLE:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BORDER_WIDTH:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BORDER_RADIUS:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FLOAT:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_CLEAR:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_OVERFLOW:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_TEXT_OVERFLOW:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_WHITE_SPACE:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_CURSOR:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FILTER:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_OPACITY:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_TRANSFORM:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_TRANSITION:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_ANIMATION:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BOX_SHADOW:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_TEXT_SHADOW:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_OUTLINE:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_OUTLINE_COLOR:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_OUTLINE_STYLE:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_OUTLINE_WIDTH:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BACKGROUND_COLOR:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BACKGROUND_IMAGE:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BACKGROUND_REPEAT:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BACKGROUND_POSITION:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BACKGROUND_SIZE:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BACKGROUND_ATTACHMENT:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BACKGROUND_CLIP:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BACKGROUND_ORIGIN:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BACKGROUND_BLEND_MODE:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_DISPLAY:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FLEX:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FLEX_DIRECTION:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_TEXT_ANCHOR:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_TEXT_COMBINE_UPRIGHT:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FLEX_WRAP:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FLEX_FLOW:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FLEX_GROW:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FLEX_SHRINK:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FLEX_BASIS:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_JUSTIFY_CONTENT:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_ALIGN_ITEMS:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_ALIGN_SELF:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_ALIGN_CONTENT:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_ORDER:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_GRID:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FILL:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_STROKE:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_STROKE_WIDTH:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_STROKE_LINECAP:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_STROKE_LINEJOIN:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_STROKE_DASHARRAY:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_STROKE_DASHOFFSET:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FILL_OPACITY:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_STROKE_OPACITY:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_SHAPE_RENDERING:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_TEXT_RENDERING:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_IMAGE_RENDERING:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_COLOR_INTERPOLATION:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_COLOR_RENDERING:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_VECTOR_EFFECT:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_MASK:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_CLIP_PATH:
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
 * @function validate_style_value_color
 * @brief Validate the style value color
 * @params {ast_layout_attribute_t*} attribute - Layout attribute
 * @returns {bool} - True if the style value is valid, false otherwise
 * 
 */
bool validate_style_value_color(ast_layout_attribute_t* attribute)
{
	DEBUG_ME;
	ast_value_t* first = attribute->values->data[0];
	char* value = first->data.string_value;

	if (first->type->kind == AST_TYPE_KIND_STRING) {
		if (strcmp(value, "red") == 0) return true;
		else if (strcmp(value, "white") == 0) return true;
		else if (strcmp(value, "black") == 0) return true;
		else if (strcmp(value, "orange") == 0) return true;
		else if (strcmp(value, "yellow") == 0) return true;
		else if (strcmp(value, "green") == 0) return true;
		else if (strcmp(value, "blue") == 0) return true;
		else if (strcmp(value, "pink") == 0) return true;
		else if (strcmp(value, "gray") == 0) return true;
	}

	return false;
}

/**
 * 
 * @function validate_style_value_size
 * @brief Validate the style value size
 * @params {ast_layout_attribute_t*} attribute - Layout attribute
 * @returns {bool} - True if the style value is valid, false otherwise
 * 
 */
bool validate_style_value_size(ast_layout_attribute_t* attribute)
{
	DEBUG_ME;
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

/**
 * 
 * @function validate_style_value_sizes
 * @brief Validate the style value sizes
 * @params {ast_layout_attribute_t*} attribute - Layout attribute
 * @returns {bool} - True if the style value is valid, false otherwise
 * 
 */
bool validate_style_value_sizes(ast_layout_attribute_t* attribute)
{
	DEBUG_ME;
	if (attribute->values->length == 1) {
		ast_value_t* first = attribute->values->data[0];
		char* value_normal = normalise_css_size(first->data.string_value);

		return value_normal;
	}

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
	char *value = first->data.string_value;

	if (new_styles) {}
	if (styles) {}

	if (attribute->values->length < 1) {
		error(2, "Style value is missing in '%s' element", generator_code_layout_node_type(attribute->parent_node_type));
		
		return false;
	}
	else if (first->type->kind == AST_TYPE_KIND_STRING && strlen(first->data.string_value) == 0) {
		error(2, "Empty value for '%s' attribute in '%s' element is not allowed at line %zu column %zu!", attribute->key, generator_code_layout_node_type(attribute->parent_node_type), attribute->value_location.start_line, attribute->value_location.start_column);

		return false;
	}
	// Global values
	else if (first->type->kind == AST_TYPE_KIND_STRING && attribute->values->length == 1) {
		if (strcmp(value, "inherit") == 0) return true;
		else if (strcmp(value, "initial") == 0) return true;
		else if (strcmp(value, "revert") == 0) return true;
		else if (strcmp(value, "revert-layer") == 0) return true;
		else if (strcmp(value, "unset") == 0) return true;
	}

	switch (attribute->type) {
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BACKGROUND:
			return true;

		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_COLOR:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BACKGROUND_COLOR:
			if (attribute->values->length > 1) {
				error(2, "Color value is too many in '%s' element", generator_code_layout_node_type(attribute->parent_node_type));
				return false;
			}
			return validate_style_value_color(attribute);

		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FONT:
			return true;
		
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FONT_FAMILY:
			return true;
		
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FONT_STYLE:
			if (attribute->values->length > 1) {
				error(2, "Font weight value is too many in '%s' element", generator_code_layout_node_type(attribute->parent_node_type));
				return false;
			}
			
			if (strcmp(value, "normal") == 0) return true;
			else if (strcmp(value, "italic") == 0) return true;
			else if (strcmp(value, "oblique") == 0) return true;
			else if (strcmp(value, "oblique 10deg") == 0) return true;
			else return false;
			break;

		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FONT_WEIGHT:
			if (attribute->values->length > 1) {
				error(2, "Font weight value is too many in '%s' element", generator_code_layout_node_type(attribute->parent_node_type));
				return false;
			}

			if (strcmp(value, "normal") == 0) return true;
			else if (strcmp(value, "bold") == 0) return true;
			else if (strcmp(value, "lighter") == 0) return true;
			else if (strcmp(value, "bolder") == 0) return true;
			else if (string_is_number(value) == true) {
				int value_int = atoi(value);
				if (value_int >= 1 && value_int <= 1000) return true;
				else {
					error(2, "Font weight value is invalid, it should be between 1 and 1000");
				}
			}

			else return false;
			break;
		

		// case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_TEXT_DECORATION:
		// 	ast_layout_attribute_type_t sub_types[] = {
		// 		AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_TEXT_DECORATION_LINE,
		// 		AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_TEXT_DECORATION_STYLE,
		// 		AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_TEXT_DECORATION_COLOR,
		// 	}; // one value should applied, we allow multiple value but only one value for each group, and user can leave the other groups empty (if he want)
		// 	size_t sub_types_length = sizeof(sub_types) / sizeof(sub_types[0]);
		// 	hashmap_t* sub_groups = hashmap_create_layout_attribute(1);

		// 	for (size_t i = 0; i < attribute->values->length; i++) {
		// 		bool _res = false;

		// 		ast_value_t* _value = attribute->values->data[i];

		// 		for (size_t j = 0; j < sub_types_length; j++) {
		// 			char* _key = ast_layout_attribute_type_to_name(sub_types[j]);

		// 			if (hashmap_has(sub_groups, _key) == true) {
		// 				continue;
		// 			}

		// 			array_value_t* _values = array_value_create(1);
		// 			array_push(_values, ast_value_copy(_value));

		// 			char* _values_str = array_value_string(_values, ", ");

		// 			ast_layout_attribute_t* _attribute = ast_layout_attribute_create(sub_types[j], _key, _values, attribute->parent_node_type, attribute->key_location, attribute->value_location);
		// 			_attribute->isStyle = attribute->isStyle;
		// 			_attribute->isContent = attribute->isContent;
		// 			_attribute->final_key = strdup(_key);
		// 			_attribute->final_value = strdup(_values_str);

		// 			bool sub_res = validate_style_value(styles, new_styles, _attribute);
					
		// 			if (_values_str != NULL) memory_destroy(_values_str);

		// 			if (sub_res == true) {
		// 				_res = true;
		// 				hashmap_put(sub_groups, _key, _attribute);
		// 				break;
		// 			}
		// 			else {
		// 				_attribute->destroy(_attribute);
		// 			}
		// 		}

		// 		if (_res == false) {
		// 			error(2, "Text decoration value is invalid in the '%s' element", generator_code_layout_node_type(attribute->parent_node_type));
		// 		}
		// 	}

		// 	if (attribute->values->length > 0) {
		// 		attribute->ignoreMe = true;

		// 		for (size_t i = 0; i < sub_groups->capacity; i++) {
		// 			hashmap_entry_t *entry = sub_groups->data[i];

		// 			while (entry) {
		// 				ast_layout_attribute_t* atr = entry->value;
		// 				ast_layout_attribute_t* atr_copy = ast_layout_attribute_copy(atr);
		// 				atr_copy->ignoreMe = false;
		// 				atr_copy->isStyle = true;

		// 				hashmap_put(new_styles, entry->key, atr_copy);

		// 				entry = entry->next;
		// 			}
		// 		}

		// 		sub_groups->destroy(sub_groups);

		// 		return true;
		// 	}

		// 	sub_groups->destroy(sub_groups);
		// 	return false;
		// 	break;

		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_WIDTH:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_HEIGHT:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FONT_SIZE:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_MARGIN_TOP:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_MARGIN_RIGHT:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_MARGIN_LEFT:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_MARGIN_BOTTOM:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_PADDING_TOP:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_PADDING_RIGHT:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_PADDING_LEFT:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_PADDING_BOTTOM:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_TOP:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_RIGHT:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_LEFT:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BOTTOM:
			if (attribute->values->length > 1) {
				error(2, "Size value for attribute '%s' is too many in the '%s' element, it should be only one", attribute->key, generator_code_layout_node_type(attribute->parent_node_type));
				return false;
			}
			// return validate_style_value_size(attribute);
			return true;

		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_MARGIN:
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_PADDING:
			if (attribute->values->length == 3) {
				error(2, "Number of size value for attribute '%s' is invalid in the '%s' element, it should be one, two or four", attribute->key, generator_code_layout_node_type(attribute->parent_node_type));
				return false;
			}
			else if (attribute->values->length > 4) {
				error(2, "Size value for attribute '%s' is too many in the '%s' element, it should be only one", attribute->key, generator_code_layout_node_type(attribute->parent_node_type));
				return false;
			}

			// return validate_style_value_sizes(attribute);
			return false;

		default:
			return false;
	}

	return false;
}