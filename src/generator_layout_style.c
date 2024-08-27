#include "generator_layout_style.h"
/**
 *
 * @function generator_code_layout_pseudo_elements
 * @brief Generate the CSS code for the layout block pseudo elements
 * @params {generator_t*} generator - Generator
 * @params {ast_layout_block_t*} block - Layout block
 * @params {size_t*} css_attributes_length - CSS attributes length
 * @returns {string_t*}
 *
 */
string_t* generator_code_layout_pseudo_elements(generator_t* generator, ast_layout_block_t* block, size_t* css_attributes_length)
{
	DEBUG_ME;
	if (generator) {}

	if (block != NULL) {
		if (block->states != NULL) {
			string_t* css = string_create(1024);

			size_t states_capacity = block->states->capacity;

			for (size_t i = 0; i < states_capacity; i++) {
				hashmap_entry_t* entry = block->states->data[i];

				while (entry) {
					hashmap_entry_t* next = cast(hashmap_entry_t*, entry->next);

					ast_layout_style_state_t* pseudo_element = entry->value;

					if (pseudo_element->normal != NULL) {
						string_t* pseudo_element_styles = generator_code_layout_styles(pseudo_element->normal, block, css_attributes_length);
						ast_layout_attribute_style_state_type type = generator_code_layout_attribute_style_state_enduser_name_to_type(entry->key);

						if (pseudo_element_styles->length > 0 && type != AST_LAYOUT_ATTRIBUTE_STYLE_STATE_TYPE_ERROR) {
							string_append_char(css, STYLE_STYLE_LINKING);
							string_append_str(css, block->tag);
							string_append_char(css, ':');
							string_append_str(css, generator_code_layout_attribute_style_state_type_to_generated_name(type));
							string_append_char(css, '{');
							string_append(css, pseudo_element_styles);
							string_append_char(css, '}');

							if (css_attributes_length != NULL) {
								(*css_attributes_length)++;
							}
						}

                        string_destroy(pseudo_element_styles);
					}

					entry = next;
				}
			}

			return css;
		}
	}

	return NULL;
}

/**
 *
 * @function generator_code_layout_style
 * @brief Generate the CSS code for the layout block
 * @params {hashmap_layout_attribute_t*} styles - Styles
 * @params {ast_layout_block_t*} block - Layout block
 * @params {size_t*} css_attributes_length - CSS attributes length
 * @returns {string_t*}
 *
 */
string_t* generator_code_layout_styles(hashmap_layout_attribute_t* styles, ast_layout_block_t* block, size_t* css_attributes_length)
{
    DEBUG_ME;
	size_t css_attributes_length_local = 0;
	string_t* code = string_create(1024);

	if (styles != NULL) {
		size_t styles_capacity = styles->capacity;
		for (size_t i = 0; i < styles_capacity; i++) {
			hashmap_entry_t* entry = styles->data[i];

			while (entry) {
				ast_layout_attribute_t* attribute = entry->value;

				generator_code_layout_style_value(block->styles->normal, block->styles->new, attribute);

				if (attribute->isStyle == false || attribute->ignoreMe == true) {}
				else {
					if (attribute->final_value == NULL) {
						error_generator(2, "Something went wrong with the style value for '%s' attribute in '%s' element!", attribute->final_key, ast_layout_node_type_to_enduser_name(block->parent_node_type));
					}

					if (css_attributes_length_local != 0) {
						string_append_char(code, ';');
					}

					string_append_str(code, attribute->final_key);
					string_append_str(code, ":");
					string_append_str(code, attribute->final_value);

					if (css_attributes_length != NULL) {
						(*css_attributes_length)++;
					}

					css_attributes_length_local++;
				}

				entry  = cast(hashmap_entry_t*, entry->next);
			}
		}
	}

	return code;
}

/**
 *
 * @function generator_code_layout_style_value
 * @brief Convert AST layout attribute values to CSS attribute values
 * @params {hashmap_t*} styles - Styles
 * @params {hashmap_t*} new_styles - New Styles
 * @params {ast_layout_attribute_t*} attribute - Layout Attribute
 * @returns {void}
 *
 */
void generator_code_layout_style_value(hashmap_t* styles, hashmap_t* new_styles, ast_layout_attribute_t* attribute)
{
	DEBUG_ME;
	bool isValid = validate_style_value(styles, new_styles, attribute);

	attribute->print(attribute);

	if (isValid == false) {
		printf("%d\n", attribute->parent_node_type);
		error_generator(2, "Invalid value for '%s' attribute in '%s' element at line %zu column %zu!", attribute->key, ast_layout_node_type_to_enduser_name(attribute->parent_node_type), attribute->value_location.start_line, attribute->value_location.start_column);

		return;
	}

	if (attribute->final_key == NULL) {
		attribute->final_key = strdup(attribute->key);
	}

	if (attribute->final_value == NULL) {
		attribute->final_value = strdup(cast(ast_value_t*, attribute->values->data[0])->data.string_value);
	}
}

/**
 *
 * @function generator_code_layout_style_name
 * @brief Convert AST layout attribute type to CSS attribute name
 * @params {ast_layout_attribute_type_t} type - Layout Attribute Type
 * @returns {char*} name - Name
 *
 */
char* generator_code_layout_style_name(ast_layout_attribute_type_t type)
{
	DEBUG_ME;
	switch (type) {
		#undef ADD_LAYOUT_ATTRIBUTE_STYLE_TYPE
		#undef ADD_LAYOUT_ATTRIBUTE_STYLE_TYPE_HIDE

		#define ADD_LAYOUT_ATTRIBUTE_STYLE_TYPE(TYPE, NAME, NAME_LOWER, ENDUSER_NAME, GENERATED_NAME, FILTER, ALLOWED_VALUES, SUBTAGS) case TYPE: return GENERATED_NAME;
		#define ADD_LAYOUT_ATTRIBUTE_STYLE_TYPE_HIDE(TYPE, NAME, NAME_LOWER, ENDUSER_NAME, GENERATED_NAME, FILTER, ALLOWED_VALUES, SUBTAGS)

	    #include "ast_layout_attribute_style_type.h"

		#undef ADD_LAYOUT_ATTRIBUTE_TYPE
		#undef ADD_LAYOUT_ATTRIBUTE_TYPE_REPEAT

		#define ADD_LAYOUT_ATTRIBUTE_TYPE(TYPE, NAME, NAME_LOWER, GENERATED_NAME, ENDUSER_NAME) case TYPE: return "ERROR";
		#define ADD_LAYOUT_ATTRIBUTE_TYPE_REPEAT(TYPE, NAME, NAME_LOWER, GENERATED_NAME, ENDUSER_NAME)

	    #include "ast_layout_attribute_type.h"
	}

	return "error????";
}

/**
 *
 * @function generator_code_layout_attribute_style_state_type_to_enduser_name
 * @brief Convert style attribute state type to enduser name
 * @params {ast_layout_attribute_style_state_type} type - Style Attribute State Type
 * @returns {char*} name - Name
 *
 */
char* generator_code_layout_attribute_style_state_type_to_enduser_name(ast_layout_attribute_style_state_type type)
{
	DEBUG_ME;
	switch (type) {
		#undef ADD_LAYOUT_ATTRIBUTE_STYLE_STATE_TYPE

		#define ADD_LAYOUT_ATTRIBUTE_STYLE_STATE_TYPE(TYPE, NAME, NAME_LOWER, ENDUSER_NAME, GENERATED_NAME) case TYPE: return ENDUSER_NAME;

		#include "ast_layout_attribute_style_state_type.h"
	}

	return "unknown style state endusername";
}

/**
 *
 * @function generator_code_layout_attribute_style_state_type_to_name
 * @brief Convert style attribute state type to name
 * @params {ast_layout_attribute_style_state_type} type - Style Attribute State Type
 * @returns {char*} name - Name
 *
 */
char* generator_code_layout_attribute_style_state_type_to_name(ast_layout_attribute_style_state_type type)
{
	DEBUG_ME;
	switch (type) {
		#undef ADD_LAYOUT_ATTRIBUTE_STYLE_STATE_TYPE

		#define ADD_LAYOUT_ATTRIBUTE_STYLE_STATE_TYPE(TYPE, NAME, NAME_LOWER, ENDUSER_NAME, GENERATED_NAME) case TYPE: return NAME;

		#include "ast_layout_attribute_style_state_type.h"
	}

	return "unknown style state name";
}

/**
 *
 * @function generator_code_layout_attribute_style_state_enduser_name_to_type
 * @brief Convert style attribute state enduser name to type
 * @params {char*} name - Name
 * @returns {ast_layout_attribute_style_state_type} type - Type
 *
 */
ast_layout_attribute_style_state_type generator_code_layout_attribute_style_state_enduser_name_to_type(char* name)
{
	DEBUG_ME;
	if (name == NULL) {
		return AST_LAYOUT_ATTRIBUTE_STYLE_STATE_TYPE_ERROR;
	}

	#undef ADD_LAYOUT_ATTRIBUTE_STYLE_STATE_TYPE

	#define ADD_LAYOUT_ATTRIBUTE_STYLE_STATE_TYPE(TYPE, NAME, NAME_LOWER, ENDUSER_NAME, GENERATED_NAME) if (strcmp(name, ENDUSER_NAME) == 0) { return TYPE; }

	#include "ast_layout_attribute_style_state_type.h"

	return AST_LAYOUT_ATTRIBUTE_STYLE_STATE_TYPE_ERROR;
}

/**
 *
 * @function generator_code_layout_attribute_style_state_type_to_generated_name
 * @brief Convert style attribute state type to generated name
 * @params {ast_layout_attribute_style_state_type} type - Style Attribute State Type
 * @returns {char*} name - Name
 *
 */
char* generator_code_layout_attribute_style_state_type_to_generated_name(ast_layout_attribute_style_state_type type)
{
	DEBUG_ME;
	switch (type) {
		#undef ADD_LAYOUT_ATTRIBUTE_STYLE_STATE_TYPE

		#define ADD_LAYOUT_ATTRIBUTE_STYLE_STATE_TYPE(TYPE, NAME, NAME_LOWER, ENDUSER_NAME, GENERATED_NAME) case TYPE: return GENERATED_NAME;

		#include "ast_layout_attribute_style_state_type.h"
	}

	return "unknown style state generated name";
}

/**
 *
 * @function generator_code_layout_attribute_style_state_name_to_type
 * @brief Convert style attribute state name to type
 * @params {char*} name - Name
 * @returns {ast_layout_attribute_style_state_type} type - Type
 *
 */
ast_layout_attribute_style_state_type generator_code_layout_attribute_style_state_name_to_type(char* name)
{
	DEBUG_ME;
	if (name == NULL) {
		return AST_LAYOUT_ATTRIBUTE_STYLE_STATE_TYPE_ERROR;
	}

	#undef ADD_LAYOUT_ATTRIBUTE_STYLE_STATE_TYPE

	#define ADD_LAYOUT_ATTRIBUTE_STYLE_STATE_TYPE(TYPE, NAME, NAME_LOWER, ENDUSER_NAME, GENERATED_NAME) if (strcmp(name, NAME) == 0) { return TYPE; }

	#include "ast_layout_attribute_style_state_type.h"

	return AST_LAYOUT_ATTRIBUTE_STYLE_STATE_TYPE_ERROR;
}
