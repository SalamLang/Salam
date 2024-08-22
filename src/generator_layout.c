#include "generator_layout.h"

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

			for (size_t i = 0; i < block->states->capacity; i++) {
				hashmap_entry_t* entry = block->states->data[i];

				while (entry) {
					hashmap_entry_t* next = cast(hashmap_entry_t*, entry->next);

					ast_layout_style_state_t* pseudo_element = entry->value;

					if (pseudo_element->normal != NULL) {
						string_t* pseudo_element_styles = generator_code_layout_styles(pseudo_element->normal, block, css_attributes_length);
						ast_layout_attribute_style_state_type type = generator_code_layout_attribute_style_state_enduser_name_to_type(entry->key);

						if (pseudo_element_styles->length > 0 && type != AST_LAYOUT_ATTRIBUTE_STYLE_STATE_TYPE_ERROR) {
							string_append_char(css, '.');
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
 * @function generator_code_layout_block
 * @brief Generate the HTML code for the layout block
 * @params {generator_t*} generator - Generator
 * @params {array_t*} children - Children
 * @returns {string_t*}
 *
 */
string_t* generator_code_layout_block(generator_t* generator, array_t* children)
{
	DEBUG_ME;
	string_t* html = string_create(1024);

	for (size_t i = 0; i < children->length; i++) {
		ast_layout_node_t* node = array_get(children, i);

		string_t* layout_block_str = string_create(1024);
		string_t* node_attrs_str = generator_code_layout_attributes(generator, node->block);
		char* node_name = generator_code_layout_node_type(node->type);

		string_append_char(layout_block_str, '<');
		string_append_str(layout_block_str, node_name);
		if (node_attrs_str->length > 0) {
			string_append_char(layout_block_str, ' ');
			string_append(layout_block_str, node_attrs_str);
		}
		if (node_attrs_str != NULL) node_attrs_str->destroy(node_attrs_str);
		string_append_str(layout_block_str, ">");

		if (node->block->children->length > 0 || node->block->text_content != NULL) {
			bool has_content = false;

			if (node->block->text_content != NULL) {
				if (node->block->children->length == 0 && strchr(node->block->text_content, '\n') == NULL) {
					string_append_str(layout_block_str, node->block->text_content);
				}
				else {
					string_append_char(layout_block_str, '\n');
					string_append_str(layout_block_str, node->block->text_content);
					string_append_char(layout_block_str, '\n');

					has_content = true;
				}
			}

			if (node->block->children->length > 0) {
				string_t* layout_block_children = generator_code_layout_block(generator, node->block->children);

				if (has_content == false) {
					string_append_char(layout_block_str, '\n');
				}

				if (layout_block_children->length > 0) {
					string_append(layout_block_str, layout_block_children);
				}

				if (layout_block_children != NULL) {
					layout_block_children->destroy(layout_block_children);
				}
			}
		}

		string_append_str(layout_block_str, "</");
		string_append_str(layout_block_str, node_name);
		string_append_str(layout_block_str, ">\n");

		string_append(html, layout_block_str);

		if (layout_block_str != NULL) layout_block_str->destroy(layout_block_str);
	}

	return html;
}

/**
 *
 * @function generator_code_layout_body
 * @params {generator_t*} generator - Generator
 * @params {ast_layout_block_t*} layout_block - Layout block
 * @params {string_t*} body - Body
 * @returns {void}
 *
 */
void generator_code_layout_body(generator_t* generator, ast_layout_block_t* layout_block, string_t* body)
{
	DEBUG_ME;
	if (layout_block != NULL) {
		validate_layout_mainbody(layout_block);
	}

	string_t* body_tag = string_create(1024);
	string_t* body_content = string_create(1024);

	string_append_str(body_tag, "<body");
	string_t* body_attrs = generator_code_layout_attributes(generator, layout_block);
	if (body_attrs->length > 0) {
		string_append_char(body_tag, ' ');
	}
	string_append(body_tag, body_attrs);
	body_attrs->destroy(body_attrs);
	string_append_str(body_tag, ">");

	char* body_text_content = layout_block->text_content;
	size_t body_text_content_length = body_text_content != NULL ? strlen(body_text_content) : 0;

	string_t* body_child = generator_code_layout_block(generator, layout_block->children);
	if (body_child->length > 0 || body_text_content_length > 0) {
		string_append_char(body_tag, '\n');
	}

	// text content
	if (body_text_content != NULL && body_text_content_length > 0) {
		string_append_str(body_content, body_text_content);
	}

	// node content
	if (body_child->length > 0 && body_text_content != NULL && body_text_content_length > 0) {
		string_append_char(body_content, '\n');
	}
	if (body_child->length > 0) {
		string_append(body_content, body_child);
	}

	if (body_child != NULL) body_child->destroy(body_child);

	string_append(body_tag, body_content);

	string_set(body, body_tag);
	body_tag->destroy(body_tag);
	body_content->destroy(body_content);
}

/**
 *
 * @function generator_code_head_item
 * @params {ast_layout_attribute_t*} attribute - Attribute
 * @params {string_t*} head - Head
 * @returns {void}
 *
 */
void generator_code_head_item(ast_layout_attribute_t* attribute, string_t* head)
{
	DEBUG_ME;
	if (head == NULL) {
		return;
	}

	char* value = NULL;

	switch (attribute->type) {
		case AST_LAYOUT_ATTRIBUTE_TYPE_TITLE:
			value = array_value_stringify(attribute->values, ", ");

			string_append_str(head, "<title>");
			string_append_str(head, value);
			string_append_str(head, "</title>");
			string_append_char(head, '\n');
			break;

		case AST_LAYOUT_ATTRIBUTE_TYPE_AUTHOR:
			value = array_value_stringify(attribute->values, ", ");

			string_append_str(head, "<meta name=\"author\" content=\"");
			string_append_str(head, value);
			string_append_str(head, "\">");
			string_append_char(head, '\n');
			break;

		case AST_LAYOUT_ATTRIBUTE_TYPE_DESCRIPTION:
			value = array_value_stringify(attribute->values, ", ");

			string_append_str(head, "<meta name=\"description\" content=\"");
			string_append_str(head, value);
			string_append_str(head, "\">");
			string_append_char(head, '\n');
			break;

		case AST_LAYOUT_ATTRIBUTE_TYPE_KEYWORDS:
			value = array_value_stringify(attribute->values, ", ");

			string_append_str(head, "<meta name=\"keywords\" content=\"");
			string_append_str(head, value);
			string_append_str(head, "\">");
			string_append_char(head, '\n');
			break;

		case AST_LAYOUT_ATTRIBUTE_TYPE_CHARSET:
			value = array_value_stringify(attribute->values, ", ");

			string_append_str(head, "<meta charset=\"");
			string_append_str(head, value);
			string_append_str(head, "\">");
			string_append_char(head, '\n');
			break;

		case AST_LAYOUT_ATTRIBUTE_TYPE_VIEWPORT:
			value = array_value_stringify(attribute->values, ", ");

			string_append_str(head, "<meta name=\"viewport\" content=\"");
			string_append_str(head, value);
			string_append_str(head, "\">");
			string_append_char(head, '\n');
			break;

		case AST_LAYOUT_ATTRIBUTE_TYPE_REFRESH:
			value = array_value_stringify(attribute->values, ", ");

			string_append_str(head, "<meta http-equiv=\"refresh\" content=\"");
			string_append_str(head, value);
			string_append_str(head, "\">");
			string_append_char(head, '\n');
			break;

		default:
			break;
	}

	if (value != NULL) {
		memory_destroy(value);
	}
}

/**
 *
 * @function generator_code_head
 * @params {ast_layout_block_t*} layout_block - Layout block
 * @params {string_t*} head - Head
 * @returns {void}
 *
 */
void generator_code_head(ast_layout_block_t* block, string_t* head)
{
	DEBUG_ME;
	if (head == NULL) {
		return;
	}
	else if (block == NULL) {
		return;
	}

	size_t html_tags_length = 0;

	if (block->attributes != NULL) {
		hashmap_t* attributes = cast(hashmap_t*, block->attributes);

		for (size_t i = 0; i < attributes->capacity; i++) {
			hashmap_entry_t* entry = attributes->data[i];

			while (entry) {
				ast_layout_attribute_t* attribute = cast(ast_layout_attribute_t*, entry->value);

				if (attribute->isStyle == true || attribute->isContent == true) {}
				else {
					generator_code_head_item(attribute, head);

					html_tags_length++;
				}

				entry = cast(hashmap_entry_t*, entry->next);
			}
		}
	}
}

/**
 *
 * @function generator_code_layout
 * @brief Generate code for AST layout
 * @params {generator_t*} generator - Generator
 * @returns {void}
 *
 */
void generator_code_layout(generator_t* generator)
{
	if (generator->ast->layout != NULL) {
		if (generator->ast->layout->block != NULL) {
			string_t* head = string_create(1024);
			string_t* body = string_create(1024);
			string_t* html = string_create(1024);

			// Process the layout block
			generator_code_layout_body(generator, generator->ast->layout->block, body);

			// Process the head block
			generator_code_head(generator->ast->layout->block, head);

			// Generate the HTML code
			string_append_str(html, "<!doctype html>\n");
			string_append_str(html, "<html");

			generator_code_layout_html(generator->ast->layout->block, html);

			string_append_str(html, ">\n");

			string_append_str(html, "<head>\n");
			string_append_str(html, "<meta charset=\"UTF-8\">\n");

			string_append(html, head);

			if (generator->css != NULL && generator->css->length > 0) {
				if (generator->inlineCSS == true) {
					string_append_str(html, "<style>\n");
					string_append(html, generator->css);
					string_append_char(html, '\n');
					string_append_str(html, "</style>\n");
				}
				else {
					string_append_str(html, "<link rel=\"stylesheet\" href=\"style.css\">\n");
				}
			}

			string_append_str(html, "</head>\n");

			string_append(html, body);

			if (generator->js != NULL && generator->js->length > 0) {
				if (generator->inlineJS == true) {
					string_append_str(html, "<script>\n");
					string_append(html, generator->js);
					string_append_str(html, "</script>\n");
				}
				else {
					string_append_str(html, "<script src=\"script.js\"></script>\n");
				}
			}

			string_append_str(html, "</body>\n");
			string_append_str(html, "</html>");

			string_set(generator->html, html);

			string_destroy(head);
			string_destroy(body);
			string_destroy(html);
		}
	}
}

/**
 *
 * @function generator_code_layout_html
 * @brief Generate the HTML code for the layout block
 * @params {ast_layout_block_t*} layout_block - Layout block
 * @params {string_t*} html - HTML string
 * @returns {void}
 *
 */
void generator_code_layout_html(ast_layout_block_t* layout_block, string_t* html)
{
	DEBUG_ME;
	// LANG
	ast_layout_attribute_t* html_lang = hashmap_get(cast(hashmap_t*, layout_block->attributes), "lang");
	char* html_lang_value = NULL;
	string_append_str(html, " lang=\"");
	if (html_lang != NULL) {
		char* values = array_value_stringify(html_lang->values, ", ");
		html_lang_value = string_lower_str(values);

		if (values != NULL) {
			memory_destroy(values);
		}
	}
	if (html_lang_value == NULL || strcmp(html_lang_value, "") == 0) {
		string_append_str(html, "fa-IR"); // default
	}
	else if (strcmp(html_lang_value, "fa") == 0 || strcmp(html_lang_value, "fa-ir") == 0 || strcmp(html_lang_value, "fa_ir") == 0) {
		string_append_str(html, "fa-IR");
	}
	else if (strcmp(html_lang_value, "en") == 0 || strcmp(html_lang_value, "en-us") == 0 || strcmp(html_lang_value, "en_us") == 0) {
		string_append_str(html, "en-US");
	}
	else {
		if (html_lang_value != NULL) {
			memory_destroy(html_lang_value);
		}

		error_generator(2, "Invalid value for lang attribute in layout block!");
	}
	string_append_str(html, "\"");

	// DIR
	ast_layout_attribute_t* html_dir = hashmap_get(cast(hashmap_t*, layout_block->attributes), "dir");
	char* html_dir_value = NULL;
	string_append_str(html, " dir=\"");
	if (html_dir != NULL) {
		char* values = array_value_stringify(html_dir->values, ", ");
		html_dir_value = string_lower_str(values);

		if (values != NULL) {
			memory_destroy(values);
		}
	}
	if (html_dir_value == NULL || strcmp(html_dir_value, "") == 0) {
		string_append_str(html, "rtl"); // default
	}
	else if (strcmp(html_dir_value, "ltr") == 0) {
		string_append_str(html, "ltr");
	}
	else if (strcmp(html_dir_value, "rtl") == 0) {
		string_append_str(html, "rtl");
	}
	else {
		if (html_lang_value != NULL) {
			memory_destroy(html_lang_value);
		}
		if (html_dir_value != NULL) {
			memory_destroy(html_dir_value);
		}

		error_generator(2, "Invalid value for dir attribute in layout block!");
	}
	string_append_str(html, "\"");

	if (html_lang_value != NULL) {
		memory_destroy(html_lang_value);
	}
	if (html_dir_value != NULL) {
		memory_destroy(html_dir_value);
	}
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
	size_t css_attributes_length_local = 0;
	string_t* code = string_create(1024);

	if (styles != NULL) {
		for (size_t i = 0; i < styles->capacity; i++) {
			hashmap_entry_t* entry = styles->data[i];

			while (entry) {
				ast_layout_attribute_t* attribute = entry->value;

				generator_code_layout_style_value(block->styles->normal, block->styles->new, attribute);

				if (attribute->isStyle == false || attribute->ignoreMe == true) {}
				else {
					if (attribute->final_value == NULL) {
						error_generator(2, "Someting went wrong with the style value for '%s' attribute in '%s' element!", attribute->final_key, ast_layout_node_type_to_enduser_name(block->parent_node_type));
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
 * @function generator_code_layout_attributes
 * @brief Generate the HTML code for the layout block attributes
 * @params {generator_t} generator - Generator
 * @params {ast_layout_block_t*} block - Layout block
 * @returns {string_t*}
 *
 */
string_t* generator_code_layout_attributes(generator_t* generator, ast_layout_block_t* block)
{
	DEBUG_ME;
	size_t html_attributes_length = 0;
	size_t css_attributes_length = 0;

	string_t* html_attributes = string_create(1024);
	string_t* css_attributes = string_create(1024);

	if (block != NULL) {
		if (block->attributes != NULL) {
			hashmap_t* attributes = cast(hashmap_t*, block->attributes);

			for (size_t i = 0; i < attributes->capacity; i++) {
				hashmap_entry_t* entry = attributes->data[i];

				while (entry) {
					ast_layout_attribute_t* attribute = cast(ast_layout_attribute_t*, entry->value);
					if (attribute == NULL) {
						entry  = cast(hashmap_entry_t*, entry->next);
						continue;
					}

					if (attribute->ignoreMe == true || attribute->isContent == true || attribute->isStyle == true) {}
					else {
						char* attribute_values_str = attribute->final_value != NULL ? attribute->final_value : array_value_stringify(attribute->values, ", ");
						size_t attribute_value_length = attribute_values_str == NULL ? 0 : strlen(attribute_values_str);

						if (html_attributes_length != 0) {
							string_append_char(html_attributes, ' ');
						}

						string_append_str(html_attributes, attribute->final_key == NULL ? entry->key : attribute->final_key);
						string_append_str(html_attributes, "=");

						if (attribute_value_length > 1) {
							string_append_str(html_attributes, "\"");
						}
						string_append_str(html_attributes, attribute_values_str);
						if (attribute_value_length > 1) {
							string_append_str(html_attributes, "\"");
						}

						if (attribute_values_str != NULL) {
							memory_destroy(attribute_values_str);
						}

						html_attributes_length++;
					}

					entry  = cast(hashmap_entry_t*, entry->next);
				}
			}
		}

		string_t* this_style = generator_code_layout_styles(block->styles->normal, block, &css_attributes_length);
		string_append(css_attributes, this_style);
		string_destroy(this_style);

		// New styles
		for (size_t i = 0; i < block->styles->new->capacity; i++) {
			hashmap_entry_t* entry = block->styles->new->data[i];

			while (entry) {
				ast_layout_attribute_t* attribute = cast(ast_layout_attribute_t*, entry->value);
				if (attribute == NULL) {}
				else if (attribute->isStyle == false || attribute->ignoreMe == true) {}
				else {
					if (css_attributes_length != 0) {
						string_append_char(css_attributes, ';');
					}
					string_append_str(css_attributes, attribute->final_key);
					string_append_str(css_attributes, ":");
					string_append_str(css_attributes, attribute->final_value);

					css_attributes_length++;
				}

				entry  = cast(hashmap_entry_t*, entry->next);
			}
		}
	}

	if (css_attributes_length > 0 && css_attributes->length > 0) {
		if (html_attributes_length > 0 && html_attributes->length > 0) {
			string_append_char(html_attributes, ' ');
		}

		bool has_substate = false;

		if (hashmap_has_any_sub_value_layout_attribute_style_state(block->states) == true) {
			has_substate = true;
		}

		block->tag = generator_identifier_get(generator->identifier); // We will free its memory in the block_layout_destroy function

		if (generator->inlineCSS == true && has_substate == false) {
			string_append_str(html_attributes, "style=\"");
			string_append(html_attributes, css_attributes);
			string_append_str(html_attributes, "\"");

			html_attributes_length++;
		}
		else {
			string_append_char(generator->css, '.');
			string_append_str(generator->css, block->tag);
			string_append_char(generator->css, '{');
			string_append(generator->css, css_attributes);
			string_append_char(generator->css, '}');
		}

		if (block->tag != NULL) {
			if (html_attributes_length > 0) {
				string_append_char(html_attributes, ' ');
			}

			size_t tag_length = strlen(block->tag);

			string_append_str(html_attributes, "class=");
			if (tag_length > 1) {
				string_append_char(html_attributes, '\"');
			}

			string_append_str(html_attributes, block->tag);

			if (tag_length > 1) {
				string_append_char(html_attributes, '\"');
			}
			html_attributes_length++;
		}

		if (has_substate == true) {
			string_t* pseudo_elements = generator_code_layout_pseudo_elements(generator, block, &css_attributes_length);

			if (pseudo_elements != NULL) {
				string_append(generator->css, pseudo_elements);
				string_destroy(pseudo_elements);
			}
		}
	}

	if (css_attributes != NULL) css_attributes->destroy(css_attributes);

	return html_attributes;
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

	// Invalid value for '...' attribute in '...' element in case if not stopped by the condition
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
		#define ADD_LAYOUT_ATTRIBUTE_STYLE_TYPE_HIDE(TYPE, NAME, NAME_LOWER, GENERATED_NAME, ENDUSER_NAME, FILTER, ALLOWED_VALUES, SUBTAGS)

	    #include "ast_layout_attribute_style_type.h"

		#undef ADD_LAYOUT_ATTRIBUTE_TYPE
		#define ADD_LAYOUT_ATTRIBUTE_TYPE(TYPE, NAME, NAME_LOWER, GENERATED_NAME, ENDUSER_NAME) case TYPE: return "ERROR";

	    #include "ast_layout_attribute_type.h"
	}

	return "error????";
}

/**
 *
 * @function generator_code_layout_node_type
 * @brief Convert AST layout node type to HTML node name
 * @params {ast_layout_node_type_t} type - Layout Node Type
 * @returns {char*} name - Name
 *
 */
char* generator_code_layout_node_type(ast_layout_node_type_t type)
{
	DEBUG_ME;
	switch (type) {
		#undef ADD_LAYOUT_TYPE
		#undef ADD_LAYOUT_TYPE_HIDE
		#undef ADD_LAYOUT_TYPE_REPEAT

		#define ADD_LAYOUT_TYPE(TYPE, NAME, NAME_LOWER, GENERATED_NAME, ENDUSER_NAME) case TYPE: return GENERATED_NAME;
		#define ADD_LAYOUT_TYPE_HIDE(TYPE, NAME, NAME_LOWER, GENERATED_NAME, ENDUSER_NAME) case TYPE: return GENERATED_NAME;
		#define ADD_LAYOUT_TYPE_REPEAT(TYPE, NAME, NAME_LOWER, GENERATED_NAME, ENDUSER_NAME)

		#include "ast_layout_type.h"
	}

	return "error?";
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

	return "uknown style state endusername";
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

	return "uknown style state name";
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

	return "uknown style state generated name";
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
