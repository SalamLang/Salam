#include "generator.h"

/**
 *
 * @function generator_create
 * @brief Create a generator
 * @params {ast_t*} ast - AST tree
 * @returns {generator_t*}
 *
 */
generator_t* generator_create(ast_t* ast)
{
	DEBUG_ME;
	if (ast == NULL) {
		error(2, "AST tree is NULL and not correct!");
	}

	generator_t* generator = memory_allocate(sizeof(generator_t));
	generator->ast = ast;
	generator->html = string_create(4096);
	generator->css = string_create(4096);
	generator->js = string_create(4096);

	generator->inlineCSS = true;
	generator->inlineJS = true;
	generator->inlineCSS = false;
	generator->inlineJS = false;

	generator->identifier = malloc(sizeof(generator_identifier_t));

	generator_identifier_init(generator->identifier);

	return generator;
}

/**
 *
 * @function generator_destroy
 * @brief Destroy a generator
 * @params {generator_t*} generator
 * @returns {void}
 *
 */
void generator_destroy(generator_t* generator)
{
	DEBUG_ME;
	if (generator != NULL) {
		if (generator->html != NULL) {
			generator->html->destroy(generator->html);
		}

		if (generator->css != NULL) {
			generator->css->destroy(generator->css);
		}

		if (generator->js != NULL) {
			generator->js->destroy(generator->js);
		}

		if (generator->identifier != NULL) {
			generator_identifier_destroy(generator->identifier);
		}

		memory_destroy(generator);
	}
}

/**
 *
 * @function generator_debug
 * @brief Debug the generator
 * @params {generator_t*} generator - Generator
 * @returns {void}
 *
 */
void generator_debug(generator_t* generator)
{
	DEBUG_ME;
	if (generator == NULL) {
		printf("generator is NULL\n");
		return;
	}

	printf("generator->html: ");
	if (generator->html != NULL) generator->html->print(generator->html);
	else printf("NULL\n");

	printf("generator->css: ");
	if (generator->css != NULL) generator->css->print(generator->css);
	else printf("NULL\n");

	printf("generator->js: ");
	if (generator->js != NULL) generator->js->print(generator->js);
	else printf("NULL\n");

}

/**
 *
 * @function generator_save
 * @brief Save the generator
 * @params {generator_t*} generator - Generator
 * @returns {void}
 *
 */
void generator_save(generator_t* generator, const char* html_output, const char* css_output, const char* js_output)
{
	DEBUG_ME;
	if (generator == NULL) {
		return;
	}

	if (generator->html != NULL) {
		file_writes(html_output, generator->html->data);
	}

	if (generator->css != NULL) {
		file_writes(css_output, generator->css->data);
	}

	if (generator->js != NULL) {
		file_writes(js_output, generator->js->data);
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
		char* values = array_layout_attribute_value_string(html_lang->values, ", ");
		html_lang_value = string_lower_str(values);
		
		if (values != NULL) memory_destroy(values);
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
		if (html_lang_value != NULL) memory_destroy(html_lang_value);

		error(2, "Invalid value for lang attribute in layout block!");
	}
	string_append_str(html, "\"");

	// DIR
	ast_layout_attribute_t* html_dir = hashmap_get(cast(hashmap_t*, layout_block->attributes), "dir");
	char* html_dir_value = NULL;
	string_append_str(html, " dir=\"");
	if (html_dir != NULL) {
		char* values = array_layout_attribute_value_string(html_dir->values, 0);
		html_dir_value = string_lower_str(values);

		if (values != NULL) memory_destroy(values);
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
		if (html_lang_value != NULL) memory_destroy(html_lang_value);
		if (html_dir_value != NULL) memory_destroy(html_dir_value);

		error(2, "Invalid value for dir attribute in layout block!");
	}
	string_append_str(html, "\"");

	if (html_lang_value != NULL) memory_destroy(html_lang_value);
	if (html_dir_value != NULL) memory_destroy(html_dir_value);
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
				hashmap_entry_t *entry = attributes->data[i];

				while (entry) {
					ast_layout_attribute_t* attribute = cast(ast_layout_attribute_t*, entry->value);
					if (attribute == NULL) {
						entry = entry->next;
						continue;
					}

					if (attribute->ignoreMe == true || attribute->isContent == true || attribute->isStyle == true) {}
					else {
						array_t* attribute_values = cast(array_t*, attribute->values);
						char* attribute_values_str = array_layout_attribute_value_string(attribute_values, ", ");
						size_t attribute_value_length = attribute_values_str == NULL ? 0 : strlen(attribute_values_str);

						if (html_attributes_length != 0) string_append_char(html_attributes, ' ');

						string_append_str(html_attributes, entry->key);
						string_append_str(html_attributes, "=");

						if (attribute_value_length > 1) string_append_str(html_attributes, "\"");
						string_append_str(html_attributes, attribute_values_str);
						if (attribute_value_length > 1) string_append_str(html_attributes, "\"");

						if (attribute_values_str != NULL) memory_destroy(attribute_values_str);

						html_attributes_length++;
					}
					
					entry = entry->next;
				}
			}
		}

		if (block->styles != NULL) {
			hashmap_t* styles = cast(hashmap_t*, block->styles);

			for (size_t i = 0; i < styles->capacity; i++) {
				hashmap_entry_t *entry = styles->data[i];

				while (entry) {
					ast_layout_attribute_t* attribute = cast(ast_layout_attribute_t*, entry->value);

					if (attribute->isStyle == false) {}
					else {
						char* attribute_css_name = generator_code_layout_style_name(attribute->type);
						char* attribute_css_value = generator_code_layout_style_value(attribute, block->parent_node_type);

						if (attribute_css_value == NULL) {
							error(2, "Empty value for '%s' attribute in '%s' element at line %zu column %zu!", attribute_css_name, generator_code_layout_node_type(block->parent_node_type), attribute->value_location.start_line, attribute->value_location.start_column);
						}

						if (css_attributes_length != 0) string_append_char(css_attributes, ';');
						string_append_str(css_attributes, attribute_css_name);
						string_append_str(css_attributes, ":");
						string_append_str(css_attributes, attribute_css_value);

						css_attributes_length++;
					}
					
					entry = entry->next;
				}
			}
		}
	}

	if (css_attributes_length > 0 && css_attributes->length > 0) {
		if (html_attributes_length > 0 && html_attributes->length > 0) string_append_char(html_attributes, ' ');

		if (generator->inlineCSS == true) {
			string_append_str(html_attributes, "style=\"");
			string_append(html_attributes, css_attributes);
			string_append_str(html_attributes, "\"");
		}
		else {
			char* tag = generator_identifier_get(generator->identifier);
			size_t tag_length = strlen(tag);

			string_append_str(html_attributes, "class=");
			if (tag_length > 1) string_append_char(html_attributes, '\"');
			string_append_str(html_attributes, tag);
			if (tag_length > 1) string_append_char(html_attributes, '\"');

			string_append_char(generator->css, '.');
			string_append_str(generator->css, tag);
			string_append_char(generator->css, '{');
			string_append(generator->css, css_attributes);
			string_append_char(generator->css, '}');

			if (tag != NULL) memory_destroy(tag);
		}
	}

	if (css_attributes != NULL) css_attributes->destroy(css_attributes);

	return html_attributes;
}

/**
 * 
 * @function generator_code_layout_style_value
 * @brief Convert AST layout attribute values to CSS attribute values
 * @params {ast_layout_attribute_t*} attribute - Layout Attribute
 * @params {ast_layout_node_type_t} parent_node_type - Parent Node Type
 * @returns {char*} values_str - Values
 * 
 */
char* generator_code_layout_style_value(ast_layout_attribute_t* attribute, ast_layout_node_type_t parent_node_type)
{
	DEBUG_ME;
	char* values_str = array_layout_attribute_value_string(attribute->values, ", ");
	size_t values_str_length = values_str == NULL ? 0 : strlen(values_str);

	char* attribute_css_name = generator_code_layout_style_name(attribute->type);
	
	// 'Value=...' attribute allowed to be empty, but all other attributes must have a value (not empty)
	if ((attribute->values->length == 0 || values_str_length == 0) && attribute->type != AST_LAYOUT_ATTRIBUTE_TYPE_VALUE) {
		error(2, "Empty value for '%s' attribute in '%s' element at line %zu column %zu!", attribute_css_name, generator_code_layout_node_type(parent_node_type), attribute->value_location.start_line, attribute->value_location.start_column);
	}

	bool isValid = validate_style_value(attribute, values_str, parent_node_type);

	// Invalid value for '...' attribute in '...' element in case if not stopped by the condition
	if (isValid == false) {
		error(2, "Invalid value for '%s' attribute in '%s' element at line %zu column %zu!", attribute_css_name, generator_code_layout_node_type(parent_node_type), attribute->value_location.start_line, attribute->value_location.start_column);
	}
	
	if (values_str != NULL) memory_destroy(values_str);

	return attribute->final_value;
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
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BACKGROUND:
			return  "background";

		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_COLOR:
			return  "color";

		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FONT:
			return  "font";

		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FONT_FAMILY:
			return  "font-family";

		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FONT_SIZE:
			return  "font-size";

		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FONT_STYLE:
			return  "font-style";

		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FONT_WEIGHT:
			return  "font-weight";

		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_HEIGHT:
			return  "height";

		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_WIDTH:
			return  "width";

		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_MARGIN:
			return  "margin";

		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_MARGIN_LEFT:
			return  "margin-left";

		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_MARGIN_RIGHT:
			return  "margin-right";

		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_MARGIN_TOP:
			return  "margin-top";

		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_MARGIN_BOTTOM:
			return  "margin-bottom";

		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_PADDING:
			return  "padding";

		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_PADDING_LEFT:
			return  "padding-left";

		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_PADDING_RIGHT:
			return  "padding-right";

		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_PADDING_TOP:
			return  "padding-top";
			
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_PADDING_BOTTOM:
			return  "padding-bottom";

		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_TEXT_ALIGN:
			return  "text-align";

		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_TEXT_DECORATION_COLOR:
			return  "text-decoration-color";

		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_TEXT_DECORATION_STYLE:
			return  "text-decoration-style";

		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_TEXT_DECORATION_LINE:
			return  "text-decoration-line";

		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_TEXT_DECORATION:
			return  "text-decoration";

		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_TEXT_TRANSFORM:
			return  "text-transform";

		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_VERTICAL_ALIGN:
			return  "vertical-align";

		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_VISIBILITY:
			return  "visibility";

		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_Z_INDEX:
			return  "z-index";

		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_POSITION:
			return  "position";

		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_TOP:
			return  "top";

		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_RIGHT:
			return  "right";

		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BOTTOM:
			return  "bottom";

		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_LEFT:
			return  "left";

		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BORDER:
			return  "border";

		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BORDER_COLOR:
			return  "border-color";

		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BORDER_STYLE:
			return  "border-style";

		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BORDER_WIDTH:
			return  "border-width";

		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BORDER_RADIUS:
			return  "border-radius";

		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FLOAT:
			return  "float";

		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_CLEAR:
			return  "clear";

		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_OVERFLOW:
			return  "overflow";

		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_TEXT_OVERFLOW:
			return  "text-overflow";

		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_WHITE_SPACE:
			return  "white-space";

		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_CURSOR:
			return  "cursor";

		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FILTER:
			return  "filter";

		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_OPACITY:
			return  "opacity";

		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_TRANSFORM:
			return  "transform";

		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_TRANSITION:
			return  "transition";

		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_ANIMATION:
			return  "animation";

		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BOX_SHADOW:
			return  "box-shadow";

		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_TEXT_SHADOW:
			return  "text-shadow";

		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_OUTLINE:
			return  "outline";

		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_OUTLINE_COLOR:
			return  "outline-color";

		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_OUTLINE_STYLE:
			return  "outline-style";

		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_OUTLINE_WIDTH:
			return  "outline-width";

		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BACKGROUND_COLOR:
			return  "background-color";

		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BACKGROUND_IMAGE:
			return  "background-image";

		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BACKGROUND_REPEAT:
			return  "background-repeat";

		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BACKGROUND_POSITION:
			return  "background-position";

		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BACKGROUND_SIZE:
			return  "background-size";

		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BACKGROUND_ATTACHMENT:
			return  "background-attachment";

		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BACKGROUND_CLIP:
			return  "background-clip";

		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BACKGROUND_ORIGIN:
			return  "background-origin";

		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BACKGROUND_BLEND_MODE:
			return  "background-blend-mode";

		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_DISPLAY:
			return  "display";

		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FLEX:
			return  "flex";

		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FLEX_DIRECTION:
			return  "flex-direction";

		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FLEX_WRAP:
			return  "flex-wrap";

		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FLEX_FLOW:
			return  "flex-flow";

		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FLEX_GROW:
			return  "flex-grow";

		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FLEX_SHRINK:
			return  "flex-shrink";

		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FLEX_BASIS:
			return  "flex-basis";

		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_JUSTIFY_CONTENT:
			return  "justify-content";

		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_ALIGN_ITEMS:
			return  "align-items";

		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_ALIGN_SELF:
			return  "align-self";

		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_ALIGN_CONTENT:
			return  "align-content";

		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_ORDER:
			return  "order";

		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_GRID:
			return  "grid";

		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FILL:
			return  "fill";

		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_STROKE:
			return  "stroke";

		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_STROKE_WIDTH:
			return  "stroke-width";

		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_STROKE_LINECAP:
			return  "stroke-linecap";

		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_STROKE_LINEJOIN:
			return  "stroke-linejoin";

		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_STROKE_DASHARRAY:
			return  "stroke-dasharray";

		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_STROKE_DASHOFFSET:
			return  "stroke-dashoffset";

		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FILL_OPACITY:
			return  "fill-opacity";

		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_STROKE_OPACITY:
			return  "stroke-opacity";

		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_SHAPE_RENDERING:
			return  "shape-rendering";

		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_TEXT_RENDERING:
			return  "text-rendering";

		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_IMAGE_RENDERING:
			return  "image-rendering";

		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_COLOR_INTERPOLATION:
			return  "color-interpolation";

		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_COLOR_RENDERING:
			return  "color-rendering";

		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_VECTOR_EFFECT:
			return  "vector-effect";

		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_MASK:
			return  "mask";

		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_CLIP_PATH:
			return  "clip-path";
			
		case AST_LAYOUT_ATTRIBUTE_TYPE_ERROR:
			return "error";
		
		default:
			return "error????";
	}
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
		case AST_LAYOUT_NODE_TYPE_LINK:
			return "link";

		case AST_LAYOUT_NODE_TYPE_SCRIPT:
			return "script";

		case AST_LAYOUT_NODE_TYPE_STYLE:
			return "style";

		case AST_LAYOUT_NODE_TYPE_DIV:
			return "div";

		case AST_LAYOUT_NODE_TYPE_HEADER:
			return "header";

		case AST_LAYOUT_NODE_TYPE_FOOTER:
			return "footer";

		case AST_LAYOUT_NODE_TYPE_NAV:
			return "nav";

		case AST_LAYOUT_NODE_TYPE_MAIN:
			return "main";

		case AST_LAYOUT_NODE_TYPE_SECTION:
			return "section";

		case AST_LAYOUT_NODE_TYPE_ARTICLE:
			return "article";

		case AST_LAYOUT_NODE_TYPE_ASIDE:
			return "aside";

		case AST_LAYOUT_NODE_TYPE_PARAGRAPH:
			return "p";

		case AST_LAYOUT_NODE_TYPE_UL:
			return "ul";

		case AST_LAYOUT_NODE_TYPE_OL:
			return "ol";

		case AST_LAYOUT_NODE_TYPE_LI:
			return "li";

		case AST_LAYOUT_NODE_TYPE_IMG:
			return "img";

		case AST_LAYOUT_NODE_TYPE_TABLE:
			return "table";

		case AST_LAYOUT_NODE_TYPE_TABLE_TR:
			return "tr";

		case AST_LAYOUT_NODE_TYPE_TABLE_TD:
			return "td";

		case AST_LAYOUT_NODE_TYPE_TABLE_TH:
			return "th";

		case AST_LAYOUT_NODE_TYPE_FORM:
			return "form";

		case AST_LAYOUT_NODE_TYPE_INPUT:
			return "input";

		case AST_LAYOUT_NODE_TYPE_BUTTON:
			return "button";

		case AST_LAYOUT_NODE_TYPE_TEXTAREA:
			return "textarea";

		case AST_LAYOUT_NODE_TYPE_SELECT:
			return "select";

		case AST_LAYOUT_NODE_TYPE_OPTION:
			return "option";

		case AST_LAYOUT_NODE_TYPE_LABEL:
			return "label";

		case AST_LAYOUT_NODE_TYPE_IFRAME:
			return "iframe";

		case AST_LAYOUT_NODE_TYPE_CANVAS:
			return "canvas";

		case AST_LAYOUT_NODE_TYPE_AUDIO:
			return "audio";

		case AST_LAYOUT_NODE_TYPE_VIDEO:
			return "video";

		case AST_LAYOUT_NODE_TYPE_BLOCKQUOTE:
			return "blockquote";

		case AST_LAYOUT_NODE_TYPE_PRE:
			return "pre";

		case AST_LAYOUT_NODE_TYPE_CODE:
			return "code";

		case AST_LAYOUT_NODE_TYPE_BR:
			return "br";

		case AST_LAYOUT_NODE_TYPE_HR:
			return "hr";

		case AST_LAYOUT_NODE_TYPE_SPAN:
			return "span";

		case AST_LAYOUT_NODE_TYPE_STRONG:
			return "strong";

		case AST_LAYOUT_NODE_TYPE_EM:
			return "em";

		case AST_LAYOUT_NODE_TYPE_ITALIC:
			return "i";

		case AST_LAYOUT_NODE_TYPE_BOLD:
			return "b";

		case AST_LAYOUT_NODE_TYPE_UNDERLINE:
			return "u";

		case AST_LAYOUT_NODE_TYPE_S:
			return "s";

		case AST_LAYOUT_NODE_TYPE_SMALL:
			return "small";

		case AST_LAYOUT_NODE_TYPE_BIG:
			return "big";

		case AST_LAYOUT_NODE_TYPE_SUB:
			return "sub";

		case AST_LAYOUT_NODE_TYPE_SUP:
			return "sup";

		case AST_LAYOUT_NODE_TYPE_CENTER:
			return "center";

		case AST_LAYOUT_NODE_TYPE_DEL:
			return "del";

		case AST_LAYOUT_NODE_TYPE_INS:
			return "ins";

		case AST_LAYOUT_NODE_TYPE_MARK:
			return "mark";

		case AST_LAYOUT_NODE_TYPE_Q:
			return "q";

		case AST_LAYOUT_NODE_TYPE_CITE:
			return "cite";

		case AST_LAYOUT_NODE_TYPE_DFN:
			return "dfn";

		case AST_LAYOUT_NODE_TYPE_ADDRESS:
			return "address";

		case AST_LAYOUT_NODE_TYPE_TIME:
			return "time";

		case AST_LAYOUT_NODE_TYPE_PROGRESS:
			return "progress";

		case AST_LAYOUT_NODE_TYPE_METER:
			return "meter";

		case AST_LAYOUT_NODE_TYPE_DETAILS:
			return "details";

		case AST_LAYOUT_NODE_TYPE_SUMMARY:
			return "summary";

		case AST_LAYOUT_NODE_TYPE_DIALOG:
			return "dialog";

		case AST_LAYOUT_NODE_TYPE_MENU:
			return "menu";

		case AST_LAYOUT_NODE_TYPE_MENUITEM:
			return "menuitem";

		case AST_LAYOUT_NODE_TYPE_COMMAND:
			return "command";

		case AST_LAYOUT_NODE_TYPE_LEGEND:
			return "legend";

		case AST_LAYOUT_NODE_TYPE_FIELDSET:
			return "fieldset";

		case AST_LAYOUT_NODE_TYPE_CAPTION:
			return "caption";

		case AST_LAYOUT_NODE_TYPE_COL:
			return "col";

		case AST_LAYOUT_NODE_TYPE_COLGROUP:
			return "colgroup";

		case AST_LAYOUT_NODE_TYPE_TABLE_HEADER:
			return "thead";

		case AST_LAYOUT_NODE_TYPE_TABLE_BODY:
			return "tbody";

		case AST_LAYOUT_NODE_TYPE_TABLE_FOOTER:
			return "tfoot";
		
		case AST_LAYOUT_NODE_TYPE_NONE:
			return "layout";
		
		case AST_LAYOUT_NODE_TYPE_PARAGRAPH_RAW:
		case AST_LAYOUT_NODE_TYPE_ERROR:
			return "";
	}

	return "error?";
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
		char* node_name = ast_layout_node_type_to_name(node->type);

		string_append_char(layout_block_str, '<');
		string_append_str(layout_block_str, node_name);
		if (node_attrs_str->length > 0) {
			string_append_char(layout_block_str, ' ');
			string_append(layout_block_str, node_attrs_str);
		}
		if (node_attrs_str != NULL) node_attrs_str->destroy(node_attrs_str);
		string_append_str(layout_block_str, ">");

		if (node->block->children->length > 0 || node->block->text_content != NULL) {
			if (node->block->text_content != NULL) {
				// string_append_char(layout_block_str, '{');
				if (node->block->children->length == 0 && strchr(node->block->text_content, '\n') == NULL) {
					string_append_str(layout_block_str, node->block->text_content);
				}
				else {
					string_append_char(layout_block_str, '\n');
					string_append_str(layout_block_str, node->block->text_content);
					string_append_char(layout_block_str, '\n');
				}
			}

			if (node->block->children->length > 0) {
				string_t* layout_block_children = generator_code_layout_block(generator, node->block->children);

				if (layout_block_children->length > 0) string_append(layout_block_str, layout_block_children);
				if (layout_block_children != NULL) layout_block_children->destroy(layout_block_children);
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
	if (body_attrs->length > 0) string_append_char(body_tag, ' ');
	string_append(body_tag, body_attrs);
	body_attrs->destroy(body_attrs);
	string_append_str(body_tag, ">");

	char* body_text_content = layout_block->text_content;
	size_t body_text_content_length = body_text_content != NULL ? strlen(body_text_content) : 0;

	string_t* body_child = generator_code_layout_block(generator, layout_block->children);
	if (body_child->length > 0 || body_text_content_length > 0) string_append_char(body_tag, '\n');

	// text content
	if (body_text_content != NULL && body_text_content_length > 0) string_append_str(body_content, body_text_content);

	// node content
	if (body_child->length > 0 && body_text_content != NULL && body_text_content_length > 0) string_append_char(body_content, '\n');
	if (body_child->length > 0) string_append(body_content, body_child);

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
	if (head == NULL) return;

	char* value = NULL;

	switch (attribute->type) {
		case AST_LAYOUT_ATTRIBUTE_TYPE_TITLE:
			value = array_layout_attribute_value_string(attribute->values, ", ");

			string_append_str(head, "<title>");
			string_append_str(head, value);
			string_append_str(head, "</title>");
			string_append_char(head, '\n');
			break;
		
		case AST_LAYOUT_ATTRIBUTE_TYPE_AUTHOR:
			value = array_layout_attribute_value_string(attribute->values, ", ");

			string_append_str(head, "<meta name=\"author\" content=\"");
			string_append_str(head, value);
			string_append_str(head, "\">");
			string_append_char(head, '\n');
			break;

		case AST_LAYOUT_ATTRIBUTE_TYPE_DESCRIPTION:
			value = array_layout_attribute_value_string(attribute->values, ", ");

			string_append_str(head, "<meta name=\"description\" content=\"");
			string_append_str(head, value);
			string_append_str(head, "\">");
			string_append_char(head, '\n');
			break;
		
		case AST_LAYOUT_ATTRIBUTE_TYPE_KEYWORDS:
			value = array_layout_attribute_value_string(attribute->values, ", ");

			string_append_str(head, "<meta name=\"keywords\" content=\"");
			string_append_str(head, value);
			string_append_str(head, "\">");
			string_append_char(head, '\n');
			break;
		
		case AST_LAYOUT_ATTRIBUTE_TYPE_CHARSET:
			value = array_layout_attribute_value_string(attribute->values, ", ");

			string_append_str(head, "<meta charset=\"");
			string_append_str(head, value);
			string_append_str(head, "\">");
			string_append_char(head, '\n');
			break;
		
		case AST_LAYOUT_ATTRIBUTE_TYPE_VIEWPORT:
			value = array_layout_attribute_value_string(attribute->values, ", ");

			string_append_str(head, "<meta name=\"viewport\" content=\"");
			string_append_str(head, value);
			string_append_str(head, "\">");
			string_append_char(head, '\n');
			break;
		
		case AST_LAYOUT_ATTRIBUTE_TYPE_REFRESH:
			value = array_layout_attribute_value_string(attribute->values, ", ");

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
	if (head == NULL) return;
	else if (block == NULL) return;

	size_t html_tags_length = 0;

	if (block->attributes != NULL) {
		hashmap_t* attributes = cast(hashmap_t*, block->attributes);

		for (size_t i = 0; i < attributes->capacity; i++) {
			hashmap_entry_t *entry = attributes->data[i];

			while (entry) {
				ast_layout_attribute_t* attribute = cast(ast_layout_attribute_t*, entry->value);

				if (attribute->isStyle == true || attribute->isContent == true) {}
				else {
					generator_code_head_item(attribute, head);

					html_tags_length++;
				}
				
				entry = entry->next;
			}
		}
	}
}

/**
 *
 * @function generator_code
 * @params {generator_t*} generator - Generator
 * @returns {void}
 *
 */
void generator_code(generator_t* generator)
{
	DEBUG_ME;
	if (generator->ast == NULL) {
		error(2, "AST tree is NULL and is not valid!");
	}

	ast_t* ast = generator->ast;

	if (generator->ast->layout != NULL) {
		ast_layout_t* layout = ast->layout;

		if (layout->block != NULL) {
			string_t* head = string_create(1024);
			string_t* body = string_create(1024);
			string_t* html = string_create(1024);

			generator_code_layout_body(generator, layout->block, body);

			generator_code_head(layout->block, head);

			string_append_str(html, "<!doctype html>\n");
			string_append_str(html, "<html");

			generator_code_layout_html(layout->block, html);

			string_append_str(html, ">\n");

			string_append_str(html, "<head>\n");
			string_append_str(html, "<meta charset=\"UTF-8\">\n");

			string_append(html, head);

			if (generator->css != NULL && generator->css->length > 0) {
				printf("it seems we have some css code\n");
				if (generator->inlineCSS == true) {
					string_append_str(html, "<style>\n");
					string_append(html, generator->css);
					string_append_str(html, "</style>\n");
				}
				else {
					string_append_str(html, "<link rel=\"stylesheet\" href=\"style.css\">\n");
				}
			}
			else {
				printf("no css code\n");
			}

			string_append_str(html, "</head>\n");

			string_append(html, body);

			if (generator->js != NULL && generator->js->length > 0) {
				printf("it seems we have some js code\n");
				if (generator->inlineJS == true) {
					string_append_str(html, "<script>\n");
					string_append(html, generator->js);
					string_append_str(html, "</script>\n");
				}
				else {
					string_append_str(html, "<script src=\"script.js\"></script>\n");
				}
			}
			else {
				printf("no js code\n");
			}
			
			string_append_str(html, "</body>\n");

			string_append_str(html, "</html>");

			string_set(generator->html, html);

			string_destroy(head);
			string_destroy(body);
			string_destroy(html);
		}
	}

	generator_code_functions(generator);
}

/**
 * 
 * @function generator_code_node
 * @brief Generate the code for the node
 * @params {generator_t*} generator - Generator
 * @params {ast_node_t*} node - Node
 * @returns {string_t*} code - Code
 * 
 */
string_t* generator_code_node(generator_t* generator, ast_node_t* node)
{
	DEBUG_ME;
	string_t* code = string_create(1024);

	switch (node->type) {
		case AST_NODE_TYPE_ERROR:
			break;

		case AST_NODE_TYPE_IMPORT:
			break;
		
		case AST_NODE_TYPE_LAYOUT:
			break;
		
		case AST_NODE_TYPE_BLOCK:
			string_t* block_code = generator_code_block(generator, node->data.block);

			if (block_code != NULL) {
				string_append(code, block_code);

				block_code->destroy(block_code);
			}
			break;

		case AST_NODE_TYPE_FUNCTION:
			string_t* function_code = generator_code_function(generator, node->data.function);

			if (function_code != NULL) {
				string_append(code, function_code);

				function_code->destroy(function_code);
			}
			break;
		
		case AST_NODE_TYPE_IF:
		case AST_NODE_TYPE_ELSE_IF:
			string_t* if_code = generator_code_if(generator, node->data.ifclause);

			if (if_code != NULL) {
				string_append(code, if_code);

				if_code->destroy(if_code);
			}
			break;

		case AST_NODE_TYPE_RETURN:
			string_t* return_code = generator_code_return(generator, node->data.returns);

			if (return_code != NULL) {
				string_append(code, return_code);

				return_code->destroy(return_code);
			}
			break;

		case AST_NODE_TYPE_PRINT:
			string_t* print_code = generator_code_print(generator, node->data.print);

			if (print_code != NULL) {
				string_append(code, print_code);

				print_code->destroy(print_code);
			}
			break;
	}

	return code;
}

/**
 * 
 * @function generator_code_type
 * @brief Generate the code for the type
 * @params {generator_t*} generator - Generator
 * @params {ast_value_type_t*} type - Type
 * @returns {string_t*} code - Code
 * 
 */
string_t* generator_code_type(generator_t* generator, ast_value_type_t* type)
{
	DEBUG_ME;
	string_t* code = string_create(1024);

	if (generator) {}

	switch (type->kind) {
		case AST_TYPE_KIND_VOID:
			string_append_str(code, "void");
			return code;
		
		case AST_TYPE_KIND_NULL:
			string_append_str(code, "null");
			return code;
		
		case AST_TYPE_KIND_INT:
			string_append_str(code, "int");
			return code;
		
		case AST_TYPE_KIND_FLOAT:
			string_append_str(code, "float");
			return code;
		
		// case AST_TYPE_KIND_DOUBLE:
		//     string_append_str(code, "double");
		//     return code;
		
		case AST_TYPE_KIND_CHAR:
			string_append_str(code, "char");
			return code;
		
		case AST_TYPE_KIND_STRING:
			string_append_str(code, "string");
			return code;
		
		case AST_TYPE_KIND_BOOL:
			string_append_str(code, "bool");
			return code;
		
		case AST_TYPE_KIND_ARRAY:
			string_append_str(code, "array");
			return code;
		
		// case AST_TYPE_KIND_MAP:
		//     string_append_str(code, "map");
		//     return code;
		
		case AST_TYPE_KIND_STRUCT:
			string_append_str(code, "struct");
			return code;
		
		case AST_TYPE_KIND_ENUM:
			string_append_str(code, "enum");
			return code;
		
		// case AST_TYPE_KIND_UNION:
		//     string_append_str(code, "union");
		//     return code;

		case AST_TYPE_KIND_POINTER:
			string_append_str(code, "pointer");
			return code;
		
		case AST_TYPE_KIND_FUNCTION:
			string_append_str(code, "function");
			return code;
	}

	string_append_str(code, "unknown type");
	return code;
}

/**
 * 
 * @function generator_code_value
 * @brief Generate the code for the value
 * @params {generator_t*} generator - Generator
 * @params {ast_value_t*} value - Value
 * @returns {string_t*} code - Code
 * 
 */
string_t* generator_code_value(generator_t* generator, ast_value_t* value)
{
	DEBUG_ME;
	string_t* code = string_create(1024);

	if (generator) {}

	if (value == NULL) {
		error(2, "Value is NULL in value statement");
		return code;
	}
	else if (value->data == NULL) {
		error(2, "Value data is NULL in value statement");
		return code;
	}

	if (value->type != NULL) {
		switch (value->type->kind) {
			case AST_TYPE_KIND_VOID:
				string_append_str(code, value->data);
				return code;
			
			case AST_TYPE_KIND_INT:
				string_append_str(code, value->data);
				return code;

			case AST_TYPE_KIND_FLOAT:
				string_append_str(code, value->data);
				return code;

			// case AST_TYPE_KIND_DOUBLE:
			// 	string_append_str(code, value->data);
			// 	return code;

			case AST_TYPE_KIND_CHAR:
				string_append_str(code, value->data);
				return code;

			case AST_TYPE_KIND_NULL:
				string_append_str(code, value->data);
				return code;

			case AST_TYPE_KIND_STRING:
				string_append_char(code, '"');
				string_append_str(code, value->data);
				string_append_char(code, '"');
				return code;

			case AST_TYPE_KIND_BOOL:
				string_append_str(code, value->data);
				return code;

			case AST_TYPE_KIND_STRUCT:
				string_append_str(code, value->data);
				return code;

			case AST_TYPE_KIND_ENUM:
				string_append_str(code, value->data);
				return code;

			case AST_TYPE_KIND_POINTER:
				string_append_str(code, value->data);
				return code;

			case AST_TYPE_KIND_ARRAY:
				string_append_str(code, value->data);
				return code;

			case AST_TYPE_KIND_FUNCTION:
				string_append_str(code, value->data);
				return code;
		}
	}

	string_append_str(code, "unknown value");

	return code;
}

/**
 * 
 * @function generator_code_values
 * @brief Generate the code for the values
 * @params {generator_t*} generator - Generator
 * @params {array_t*} values - Values
 * @returns {string_t*} code - Code
 * 
 */
string_t* generator_code_values(generator_t* generator, array_t* values)
{
	DEBUG_ME;
	string_t* code = string_create(1024);

	if (values != NULL) {
		for (size_t i = 0; i < values->length; i++) {
			ast_value_t* value = array_get(values, i);

			if (value != NULL) {
				string_t* value_code = generator_code_value(generator, value);

				if (value_code != NULL) {
					string_append(code, value_code);

					if (i != values->length - 1) {
						string_append_str(code, ", ");
					}

					value_code->destroy(value_code);
				}
			}
		}
	}

	return code;
}

/**
 * 
 * @function generator_code_print
 * @brief Generate the code for the print
 * @params {generator_t*} generator - Generator
 * @params {ast_print_t*} print - Print
 * @returns {string_t*} code - Code
 * 
 */
string_t* generator_code_print(generator_t* generator, ast_print_t* print)
{
	DEBUG_ME;
	string_t* code = string_create(1024);

	if (print->values == NULL) {
		error(2, "Print values are NULL in print statement");
	}

	if (print->values->length == 0) {
		error(2, "Print values length is 0 in print statement");
	}

	string_t* values_code = generator_code_values(generator, print->values);

	if (values_code == NULL) {
		error(2, "Error generating code for print values in print statement");
	}

	// string_append_str(code, "print");
	string_append_str(code, "console.log");
	string_append_char(code, '(');
	string_append(code, values_code);
	string_append_char(code, ')');
	string_append_char(code, ';');
	string_append_char(code, '\n');

	values_code->destroy(values_code);

	return code;
}

/**
 * 
 * @function generator_code_return
 * @brief Generate the code for the return
 * @params {generator_t*} generator - Generator
 * @params {ast_return_t*} returns - Returns
 * @returns {string_t*} code - Code
 * 
 */
string_t* generator_code_return(generator_t* generator, ast_return_t* returns)
{
	DEBUG_ME;
	string_t* code = string_create(1024);

	if (returns->values == NULL) {
		error(2, "Return values are NULL in return statement");
	}

	if (returns->values->length == 0) {
		string_append_str(code, "return;");
		string_append_char(code, '\n');
	}
	else {
		string_t* values_code = generator_code_values(generator, returns->values);

		if (values_code == NULL) {
			error(2, "Error generating code for return values in return statement");
		}

		string_append_str(code, "return");
		string_append_char(code, '(');
		string_append(code, values_code);
		string_append_char(code, ')');
		string_append_char(code, ';');
		string_append_char(code, '\n');

		values_code->destroy(values_code);
	}

	return code;
}

/**
 * 
 * @function generator_code_if
 * @brief Generate the code for the if clause
 * @params {generator_t*} generator - Generator
 * @params {ast_if_t*} ifclause - If clause
 * @returns {string_t*} code - Code
 * 
 */
string_t* generator_code_if(generator_t* generator, ast_if_t* ifclause)
{
	DEBUG_ME;
	string_t* code = string_create(1024);

	if (ifclause->condition != NULL) {
		string_t* condition_code = generator_code_value(generator, ifclause->condition);

		if (condition_code == NULL) {
			error(2, "Error generating code for if clause condition");
		}

		string_append_str(code, "if (");
		string_append(code, condition_code);
		string_append_str(code, ") ");

		if (condition_code != NULL) {
			condition_code->destroy(condition_code);
		}
	}

	string_t* block_code = generator_code_block(generator, ifclause->block);

	if (block_code != NULL) {
		string_append(code, block_code);

		block_code->destroy(block_code);
	}

	if (ifclause->else_blocks != NULL) {
		size_t elseif_length = ifclause->else_blocks->length;

		if (elseif_length > 0) {
			for (size_t i = 0; i < elseif_length; i++) {
				ast_if_t* else_if = array_get(ifclause->else_blocks, i);

				string_t* else_if_code = generator_code_if(generator, else_if);

				if (else_if_code != NULL) {
					if (i != elseif_length - 1) {
						string_append_str(code, "else ");
					}

					string_append(code, else_if_code);

					else_if_code->destroy(else_if_code);
				}
			}
		}
	}

	return code;
}

/**
 * 
 * @function generator_code_block
 * @brief Generate the code for the block
 * @params {generator_t*} generator - Generator
 * @params {ast_block_t*} block - Block
 * @returns {string_t*} code - Code
 * 
 */
string_t* generator_code_block(generator_t* generator, ast_block_t* block)
{
	DEBUG_ME;
	string_t* code = string_create(1024);

	string_append_char(code, '{');
	string_append_char(code, '\n');

	if (block != NULL) {
		for (size_t i = 0; i < block->children->length; i++) {
			ast_node_t* node = array_get(block->children, i);

			if (node != NULL) {
				string_t* node_code = generator_code_node(generator, node);

				if (node_code != NULL) {
					string_append(code, node_code);

					node_code->destroy(node_code);
				}
			}
		}
	}

	string_append_char(code, '}');
	string_append_char(code, '\n');

	return code;
}

/**
 * 
 * @function generator_code_function
 * @brief Generate the code for the function
 * @params {generator_t*} generator - Generator
 * @params {ast_function_t*} function - Function
 * @returns {string_t*} code - Code
 * 
 */
string_t* generator_code_function(generator_t* generator, ast_function_t* function)
{
	DEBUG_ME;
	string_t* code = string_create(1024);

	string_append_str(code, "function ");
	string_append_str(code, function->name);
	string_append_char(code, '(');
	string_append_char(code, ')');

	string_t* code_block = generator_code_block(generator, function->block);
	string_append(code, code_block);
	code_block->destroy(code_block);

	return code;
}

/**
 * 
 * @function generator_code_functions
 * @brief Generate the code for the functions
 * @params {generator_t*} generator - Generator
 * @returns {void}
 * 
 */
void generator_code_functions(generator_t* generator)
{
	DEBUG_ME;
	if (generator != NULL && generator->ast != NULL && generator->ast->functions != NULL && generator->ast->functions->data != NULL && generator->ast->functions->length > 0) {
		for (size_t i = 0; i < generator->ast->functions->length; i++) {
			ast_function_t* function = array_get(generator->ast->functions, i);

			if (function != NULL) {
				string_t* function_code = generator_code_function(generator, function);

				if (function_code != NULL) {
					string_append(generator->js, function_code);

					function_code->destroy(function_code);
				}
			}
		}
	}
}

/**
 * 
 * @function generator_identifier_init
 * @params {generator_identifier_t*} gen - Generator Identifier
 * @returns {void}
 * 
 */
void generator_identifier_init(generator_identifier_t* gen)
{
	DEBUG_ME;
	gen->current = memory_allocate(2);

	if (gen->current) {
		gen->current[0] = 'a';
		gen->current[1] = '\0';
	}
}

/**
 * 
 * @function generator_identifier_get
 * @params {generator_identifier_t*} gen - Generator Identifier
 * @returns {char*} identifier - Identifier
 * 
 */
char* generator_identifier_get(generator_identifier_t* gen)
{
	DEBUG_ME;
	int length = strlen(gen->current);
	char *identifier = memory_allocate(length + 1);

	strcpy(identifier, gen->current);

	for (int i = length - 1; i >= 0; i--) {
		if (gen->current[i] < 'z') {
			gen->current[i]++;
			
			return identifier;
		}

		gen->current[i] = 'a';

		if (i == 0) {
			char *new_current = memory_allocate(length + 2);

			memset(new_current, 'a', length + 1);

			new_current[length + 1] = '\0';

			free(gen->current);

			gen->current = new_current;
		}
	}

	return identifier;
}

/**
 * 
 * @function generator_identifier_destroy
 * @params {generator_identifier_t*} gen - Generator Identifier
 * @returns {void}
 * 
 */
void generator_identifier_destroy(generator_identifier_t* gen)
{
	DEBUG_ME;
	if (gen == NULL) return;

	if (gen->current != NULL) free(gen->current);
	
	free(gen);
}
